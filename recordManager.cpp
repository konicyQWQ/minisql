#include "recordManager.h"
#include <string>
#include <iostream>
#include <cmath>
using std::string;
using namespace std;

RecordManager::RecordManager(BufferManager *bm) {
    this->bufferManager = bm;
}

void RecordManager::createTable(std::string tableName) {
    FILE *fp = fopen((string("table/") + tableName + ".rdf").c_str(), "w");
    fclose(fp);
}

int RecordManager::dropTabel(std::string tableName) {
    if(remove((std::string("table/") + tableName + ".rdf").c_str()) < 0)
        return 1;   // 删除失败
    return 0;
}

int RecordManager::insert(Table *table, Tuple tuple) {
    #ifdef DEBUG
        std::cout << "tuple.data.size = " << tuple.data.size() << std::endl;
        std::cout << "table->attrCnt = " << (int)table->attrCnt << std::endl;
    #endif

    // 首先进行类型检验
    if(table->attrCnt > tuple.data.size()) {
        std::logic_error e("error: too less values!");
        throw std::exception(e);
    }
    if(table->attrCnt < tuple.data.size()) {
        std::logic_error e("error: too many values!");
        throw std::exception(e);
    }
    for(int i=0; i<table->attrCnt; i++) {
        if((table->attr[i].type == 0 && tuple.data[i]->type != 0)
        || (table->attr[i].type == 1 && (tuple.data[i]->type != 1 && tuple.data[i]->type != 0))
        || (table->attr[i].type == 2 && (tuple.data[i]->type - 2 > table->attr[i].length || tuple.data[i]->type - 2 < 0))) {
            std::logic_error e("error: values are not proper for attribute!");
            throw std::exception(e);
        }
    }
    // 读取记录，进行合法性检验
    Block *blk = bufferManager->getBlock(string("table/") + table->name + ".rdf", 0);
    int insertPos = -1;
    int len = calcLength(table);

    if(blk->validChar == 0) {
        insertPos = 0;
    } else {
        int pos = 0;
        while(blk->validChar != 0) {
            if(pos + len >= 4096) {
                blk = bufferManager->getBlock(string("table/") + table->name + ".rdf", blk->offset+1);
                pos = 0;
            }

            if(blk->validChar == 0) {
                if(insertPos == -1)
                    insertPos = 4096 * blk->offset;
                break;
            }

            if(blk->buf[pos] == 0) {
                if(insertPos == -1)
                    insertPos = 4096 * blk->offset + pos;
                pos += len;
            } else {
                // 一个已经存在的记录
                pos++;
                for(int i=0; i<table->attrCnt; i++)
                    if(table->attr[i].type == 0) {
                        if(table->attr[i].isUnique) {
                            int *value = (int*)&(blk->buf[pos]);
                            if(*value == ((iData*)tuple.data[i])->value) {
                                std::logic_error e((string("error: insert failed beacuse of the unique attribute ") + table->attr[i].name).c_str());
                                throw std::exception(e);
                            }
                        }
                        pos+=4;
                    } else if(table->attr[i].type == 1) {
                        if(table->attr[i].isUnique) {
                            float *value = (float*)&(blk->buf[pos]);
                            float number;
                            if(tuple.data[i]->type == 0)
                                number = ((iData*)tuple.data[i])->value;
                            else
                                number = ((fData*)tuple.data[i])->value;
                            if(*value == number) {
                                std::logic_error e((string("error: insert failed beacuse of the unique attribute ") + table->attr[i].name).c_str());
                                throw std::exception(e);
                            }
                        }
                        pos+=4;
                    } else {
                        if(table->attr[i].isUnique) {
                            char *value = &(blk->buf[pos]);
                            string str;
                            for(int i=0; i<table->attr[i].length; i++)
                                str.push_back(value[i]);
                            if(str == ((sData*)tuple.data[i])->value) {
                                std::logic_error e((string("error: insert failed beacuse of the unique attribute ") + table->attr[i].name).c_str());
                                throw std::exception(e);
                            }
                        }
                        pos+=table->attr[i].length;
                    }
            }
        }
    }

    #ifdef DEBUG
        cout << "insertPos = " << insertPos << endl;
    #endif

    blk = bufferManager->getBlock(string("table/") + table->name + ".rdf", insertPos / 4096);
    bufferManager->writeBlock(blk);
    int pos = insertPos % 4096;
    blk->buf[pos] = 1;
    pos++;
    for(int i=0; i<table->attrCnt; i++)
        if(table->attr[i].type == 0) {
            char *src = (char*)&((iData*)tuple.data[i])->value;
            memcpy(blk->buf + pos, src, 4);
            pos+=4;
        } else if(table->attr[i].type == 1) {
            float value = 0;
            if(tuple.data[i]->type == 0)
                value = ((iData*)tuple.data[i])->value;
            else
                value = ((fData*)tuple.data[i])->value;
            memcpy(blk->buf + pos, (char*)&value, 4);
            pos+=4;
        } else {
            char src[256];
            memset(src, 0, sizeof(src));
            string str = ((sData*)tuple.data[i])->value;
            for(int j=0; j<str.length(); j++)
                src[j] = str[j];
            memcpy(blk->buf + pos, src, table->attr[i].length);
            pos+=table->attr[i].length;
        }
    return insertPos;
}

Table* RecordManager::del(Table *table, std::vector<WhereQuery> &wq) {
    Block *blk = bufferManager->getBlock(string("table/") + table->name + ".rdf", 0);
    int len = calcLength(table);    

    int pos = 0;
    while(blk->validChar != 0) {
        if(pos + len >= 4096) {
            blk = bufferManager->getBlock(string("table/") + table->name + ".rdf", blk->offset+1);
            pos = 0;
        }

        if(blk->validChar == 0)
            break;

        if(blk->buf[pos] == 1) {
            Tuple *tuple = readRecord(table, blk, pos);
            #ifdef DEBUG
                cout << "whereQuery judge" << endl;
            #endif
            if(judge(table, wq, tuple)) {
                #ifdef DEBUG
                    cout << "whereQuery true" << endl;
                #endif

                bufferManager->writeBlock(blk);
                blk->buf[pos] = 0;
                table->tuple.push_back(tuple);
                pos+=len;
            } else {
                #ifdef DEBUG
                    cout << "whereQuery false" << endl;
                #endif
                pos+=len;
                delete tuple;
            }
        } else {
            pos += len;
        }
    }
    return table;
}

/* select 结果会保存在tabel中 */
int RecordManager::select(Table *table, std::vector<WhereQuery> &wq) {
    #ifdef DEBUG
        cout << "RecordManager::select " << table->name << ":" << endl;
    #endif

    Block *blk = bufferManager->getBlock(string("table/") + table->name + ".rdf", 0);
    int len = calcLength(table);

    int pos = 0;
    while(blk->validChar != 0) {
        if(pos + len >= 4096) {
            blk = bufferManager->getBlock(string("table/") + table->name + ".rdf", blk->offset+1);
            pos = 0;
        }

        if(blk->validChar == 0)
            break;
        if(blk->buf[pos] == 1) {
            Tuple *tuple = readRecord(table, blk, pos);
            if(judge(table, wq, tuple)) {
                table->tuple.push_back(tuple);
            }
            pos += len;
        } else {
            pos += len;
        }
    }
    choose(table, wq);
    return 0;
}

void RecordManager::choose(Table *table, std::vector<WhereQuery> &wq) {
    for(int i=0; i<table->tuple.size(); i++) {
        for(int j=0; j<wq.size(); j++) {
            if(judge(table, wq, table->tuple[i]) == false) {
                table->tuple.erase(table->tuple.begin() + i);
                i--;
            }
        }
    }
}

bool RecordManager::judge(Table *table, std::vector<WhereQuery> &wq, Tuple *tuple) {
    for(int i=0; i<table->attrCnt; i++) {
        for(int j=0; j<wq.size(); j++)
            if(wq[j].col == table->attr[i].name) {
                if(wq[j].op == COMPARE::e) {
                    if(tuple->data[i]->type == 0) {
                        if(((iData*)tuple->data[i])->value != ((iData*)wq[j].d)->value)
                            return false;
                    } else if(tuple->data[i]->type == 1) {
                        #ifdef DEBUG
                            cout << "wq[j].d->type = " << (int)wq[j].d->type << endl;
                            cout << "((iData*)wq[j].d)->value = " << ((iData*)wq[j].d)->value << endl;
                            cout << "((fData*)tuple->data[i])->value = " << ((fData*)tuple->data[i])->value << endl;
                        #endif
                        if(wq[j].d->type == 0 && abs(((fData*)tuple->data[i])->value - (float)((iData*)wq[j].d)->value) > 0.000001)
                            return false;
                        else if(wq[j].d->type == 1 && abs(((fData*)tuple->data[i])->value - ((fData*)wq[j].d)->value) > 0.0000001)
                            return false;
                    } else {
                        if(((sData*)tuple->data[i])->value != ((sData*)wq[j].d)->value)
                            return false;
                    }
                } else if(wq[j].op == COMPARE::ne) {
                    if(tuple->data[i]->type == 0) {
                        if(((iData*)tuple->data[i])->value == ((iData*)wq[j].d)->value)
                            return false;
                    } else if(tuple->data[i]->type == 1) {
                        if(wq[j].d->type == 0 && abs(((fData*)tuple->data[i])->value - (float)((iData*)wq[j].d)->value) < 0.00000001)
                            return false;
                        else if(wq[j].d->type == 1 && abs(((fData*)tuple->data[i])->value - ((fData*)wq[j].d)->value) < 0.000000001)
                            return false;
                    } else {
                        if(((sData*)tuple->data[i])->value == ((sData*)wq[j].d)->value)
                            return false;
                    }
                } else if(wq[j].op == COMPARE::ge) {
                    if(tuple->data[i]->type == 0) {
                        if(((iData*)tuple->data[i])->value < ((iData*)wq[j].d)->value)
                            return false;
                    } else if(tuple->data[i]->type == 1) {
                        if(wq[j].d->type == 0 && ((fData*)tuple->data[i])->value < (float)((iData*)wq[j].d)->value)
                            return false;
                        else if(wq[j].d->type == 1 && ((fData*)tuple->data[i])->value < ((fData*)wq[j].d)->value)
                            return false;
                    } else {
                        if(((sData*)tuple->data[i])->value < ((sData*)wq[j].d)->value)
                            return false;
                    }
                } else if(wq[j].op == COMPARE::le) {
                    if(tuple->data[i]->type == 0) {
                        if(((iData*)tuple->data[i])->value > ((iData*)wq[j].d)->value)
                            return false;
                    } else if(tuple->data[i]->type == 1) {
                        if(wq[j].d->type == 0 && ((fData*)tuple->data[i])->value > (float)((iData*)wq[j].d)->value)
                            return false;
                        else if(wq[j].d->type == 1 && ((fData*)tuple->data[i])->value > ((fData*)wq[j].d)->value)
                            return false;
                    } else {
                        if(((sData*)tuple->data[i])->value > ((sData*)wq[j].d)->value)
                            return false;
                    }
                } else if(wq[j].op == COMPARE::lt) {
                    if(tuple->data[i]->type == 0) {
                        if(((iData*)tuple->data[i])->value >= ((iData*)wq[j].d)->value)
                            return false;
                    } else if(tuple->data[i]->type == 1) {
                        if(wq[j].d->type == 0 && ((fData*)tuple->data[i])->value >= (float)((iData*)wq[j].d)->value)
                            return false;
                        else if(wq[j].d->type == 1 && ((fData*)tuple->data[i])->value >= ((fData*)wq[j].d)->value)
                            return false;
                    } else {
                        if(((sData*)tuple->data[i])->value >= ((sData*)wq[j].d)->value)
                            return false;
                    }
                } else if(wq[j].op == COMPARE::gt) {
                    if(tuple->data[i]->type == 0) {
                        if(((iData*)tuple->data[i])->value <= ((iData*)wq[j].d)->value)
                            return false;
                    } else if(tuple->data[i]->type == 1) {
                        if(wq[j].d->type == 0 && ((fData*)tuple->data[i])->value <= (float)((iData*)wq[j].d)->value)
                            return false;
                        else if(wq[j].d->type == 1 && ((fData*)tuple->data[i])->value <= ((fData*)wq[j].d)->value)
                            return false;
                    } else {
                        if(((sData*)tuple->data[i])->value <= ((sData*)wq[j].d)->value)
                            return false;
                    }
                }
            }
    }
    return true;
}

Tuple* RecordManager::readRecord(Table *table, Block *blk, int pos) {
    int srcPos = pos;
    pos++;
    std::vector<Data *> data;
    for(int i=0; i<table->attrCnt; i++) {
        if(table->attr[i].type == 0) {
            int *value = (int*)&(blk->buf[pos]);
            data.push_back(new iData(*value));
            pos+=4;
        } else if(table->attr[i].type == 1) {
            float *value = (float*)&(blk->buf[pos]);
            data.push_back(new fData(*value));
            pos+=4;
        } else {
            char *value = &(blk->buf[pos]);
            string str;
            for(int j=0; j<table->attr[i].length; j++)
                if(value[j])
                    str.push_back(value[j]);
            #ifdef DEBUG
                cout << "readRecord: read string = " << str << endl;
            #endif
            data.push_back(new sData(str));
            pos+=table->attr[i].length;
        }
    }
    Tuple *tuple = new Tuple(data);
    tuple->address = blk->offset * 4096 + srcPos;
    return tuple;
}

void RecordManager::addRecord(Table *table, int address) {
    Block* blk = bufferManager->getBlock(string("table/") + table->name + ".rdf", address / 4096);
    int pos = address % 4096;
    table->tuple.push_back(readRecord(table, blk, pos));
}

void RecordManager::deleteRecord(Table *table, int address) {
    #ifdef DEBUG
        cout << "delete record address = " << address << endl;
    #endif
    Block* blk = bufferManager->getBlock(string("table/") + table->name + ".rdf", address / 4096);
    int pos = address % 4096;
    bufferManager->writeBlock(blk);
    blk->buf[pos] = 0;
}

int RecordManager::calcLength(Table *table) {
    int cnt = 0;
    for(int i=0; i<table->attrCnt; i++)
        if(table->attr[i].type == 0 || table->attr[i].type == 1)
            cnt += 4;
        else
            cnt += table->attr[i].length;
    return cnt + 1; // 有一个标记位，标记是否删除
}