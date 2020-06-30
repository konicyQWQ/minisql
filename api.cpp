#include "api.h"
#include <exception>
#include <iostream>
using namespace std;

Api::Api() {
    cm = new CatalogManager;
    bm = new BufferManager;
    im = new IndexManager(bm);
    rm = new RecordManager(bm);
}

Api::~Api() {
    delete cm;
    delete bm;
    delete im;
    delete rm;
}

void Api::createTable(std::string tableName, std::vector<Attribute> attr, std::string primaryKey) {
    Table table;

    // 填充好表信息
    table.name = tableName;
    table.attrCnt = attr.size();
    for(int i=0; i<attr.size(); i++)
        table.attr[i] = attr[i];
    if(primaryKey != "") {  // 主键不为空需要建立索引
        table.indexCnt = 1;
        table.index[0].name = primaryKey + "_" + tableName;
        while(cm->queryIndex(table.index[0].name) != 0) {
            table.index[0].name += "0";
        }
        for(int i=0; i<table.attrCnt; i++)
            if(table.attr[i].name == primaryKey) {
                table.index[0].indexNum = i;
                table.primary = i;
                table.attr[i].isUnique = 1;
            }
    } else {
        table.indexCnt = 0;
        table.primary = - 1;
    }

    // catalog 创建表
    int ret = cm->createTable(table);
    if(ret == 1) {
        throw std::exception("error: table already exists!");
    } else if(ret == 2) {
        throw std::exception("error: duplicate attribute name!");
    } else {
        // 一切正常，接下来由 record manager 创建表，index manager 创建索引
        rm->createTable(tableName);
        if(primaryKey != "") {
            im->createIndex(table, table.primary);
        }
    }
}

void Api::showTable(std::string tableName) {
    Table *table = cm->selectTable(tableName);
    cm->showTable(*table);
    delete table;
}

void Api::createIndex(std::string tableName, std::string indexName, std::string colName) {
    Table *table = cm->selectTable(tableName);

    if(table == nullptr) {
        throw std::exception("error: table is not exist!");
    } else {
        // 存在这张表
        int indexNum = -1;
        for(int i=0; i<table->attrCnt; i++)
            if(table->attr[i].name == colName)
                indexNum = i;
        if(indexNum == -1) {
            throw std::exception("error: attribute name is not found!");
        }
        int ret = cm->createIndex(tableName, indexName, indexNum);
        if(ret == 2) {
            throw std::exception("error: index name is exist!");
        } else {
            // catalog manager 成功创建索引了
            im->createIndex(*table, indexNum);
            std::vector<WhereQuery> wq;
            Table *all = this->select(tableName, wq);
            // 把里面已经有的元组都加进去
            for(int i=0; i<table->tuple.size(); i++)
                im->insert(indexName, table->tuple[i]->data[indexNum], table->tuple[i]->address);
        }
    }

    delete table;
}

void Api::dropTable(std::string tableName) {
    Table *table = cm->selectTable(tableName);
    if(table == nullptr) {
        throw std::exception("error: table is not exist!");
    } else {
        cm->dropTable(tableName);
        rm->dropTabel(tableName);
        for(int i=0; i<table->indexCnt; i++)
            im->deleteIndex(table->index[i].name);
    }
}

void Api::dropIndex(std::string indexName) {
    int ret = cm->dropIndex(indexName);
    if(ret == 1) {
        throw std::exception("error: index is not exist!");
    } else {
        im->deleteIndex(indexName);
    }
}

void Api::insertInto(std::string tableName, std::vector<Data*> data) {
    Table *table = cm->selectTable(tableName);
    if(table == nullptr) {
        throw std::exception("error: table is not exist!");
    } else {
        try {
            for(int i=0; i<table->attrCnt; i++) {
                if(table->attr[i].type == 1 && data[i]->type == 0) {
                    iData* ptr = (iData*)data[i];
                    data[i] = new fData((float)(((iData*)data[i])->value));
                    delete ptr;
                }
            }

            int address = rm->insert(table, Tuple(data));
            #ifdef DEBUG
                std::cout << "record manager ok!" << endl;
            #endif
            for(int i=0; i<table->indexCnt; i++) {
                #ifdef DEBUG
                    std::cout << "table->index[i].name = " << table->index[i].name << endl;
                    std::cout << "address = " << address << std::endl;
                    std::cout << "data[table->index[i].indexNum].type = " << data[table->index[i].indexNum]->type << std::endl;
                    std::cout << "value = " << ((iData*)(data[table->index[i].indexNum]))->value << endl;
                #endif
                im->insert(table->index[i].name, data[table->index[i].indexNum], address);
            }
        } catch(const std::exception& e) {
            throw e;
        }
    }
}

Table* Api::select(std::string tableName, std::vector<WhereQuery> wq) {
    #ifdef DEBUG
        cout << "select " << tableName << ":" << endl;
    #endif

    Table *table = cm->selectTable(tableName);
    if(table == nullptr) {
        throw std::exception("error: table is not exist!");
    } else {
        for(int i=0; i<wq.size(); i++) {
            int flag = 0;
            for(int j=0; j<table->attrCnt; j++) {
                if(table->attr[j].name == wq[i].col) {
                    flag = 1;
                    if(table->attr[j].type == 1 && wq[i].d->type == 0) {
                        iData* ptr = (iData*)wq[i].d->type;
                        wq[i].d = new fData((float)(((iData*)wq[i].d)->value));
                        delete ptr;
                    }   

                    if((table->attr[j].type == 0 && wq[i].d->type != 0)
                    || (table->attr[j].type == 1 && (wq[i].d->type != 1 && wq[i].d->type != 0))
                    || (table->attr[j].type == 2 && (wq[i].d->type - 2 > table->attr[j].length  || wq[i].d->type - 2 < 0)))
                        throw std::exception("error: values are not proper for attribute!");
                }
            }
            if(flag == 0) {
                std::string str = std::string("error: table has no attribute called ") + wq[i].col;
                throw std::exception(str.c_str());
            }
        }
        /*
        for(int i=0; i<table->indexCnt; i++)
            for(int j=0; j<wq.size(); j++) {
                if(table->attr[table->index[i].indexNum].name == wq[j].col) {
                    // 有索引，利用索引读入
                    if(wq[j].op == COMPARE::e) {
                        int address = im->search(table->index[i].name, wq[j].d);
                        #ifdef DEBUG
                            cout << "whereQuery index address = " << address << endl;
                        #endif
                        if(address == -1)
                            break;
                        rm->addRecord(table, address);
                    }
                    if(wq[j].op == COMPARE::ge || wq[j].op == COMPARE::gt) {
                        std::vector<int> arr = im->rangeSearch(table->index[i].name, wq[j].d, nullptr);
                        #ifdef DEBUG
                            cout << "whereQuery index arr.size() = " << arr.size() << endl;
                        #endif
                        for(int i=0; i<arr.size(); i++) {
                            #ifdef DEBUG
                                cout << "whereQuery index address = " << arr[i] << endl;
                            #endif
                            if(arr[i] >= 0)
                                rm->addRecord(table, arr[i]);
                        }
                    }
                    if(wq[j].op == COMPARE::le || wq[j].op == COMPARE::lt) {
                        std::vector<int> arr = im->rangeSearch(table->index[i].name, nullptr, wq[j].d);
                        #ifdef DEBUG
                            cout << "whereQuery index arr.size() = " << arr.size() << endl;
                        #endif
                        for(int i=0; i<arr.size(); i++) {
                            #ifdef DEBUG
                                cout << "whereQuery index address = " << arr[i] << endl;
                            #endif
                            if(arr[i] >= 0)
                                rm->addRecord(table, arr[i]);
                        }
                    }
                    break;
                }
            }*/
        #ifdef DEBUG
            cout << "table->tuple.size() = " << table->tuple.size() << endl; 
        #endif 
        if(table->tuple.size() == 0) {
            rm->select(table, wq);
        } else {
            rm->choose(table, wq);
        }
        return table;
    }
}

void Api::showTuple(Table *table) {
    cout << "===== " << table->name << " ======" << endl;
    if(table->tuple.size() == 0) {
        cout << " ---- empty ---- " << endl;;
        return ;
    }
    for(int i=0; i<table->attrCnt; i++)
        cout << table->attr[i].name << "\t";
    cout << "\n";
    for(int i=0; i<table->tuple.size(); i++) {
        for(int j=0; j<table->tuple[i]->data.size(); j++)
            if(table->tuple[i]->data[j]->type == 0)
                cout << ((iData*)table->tuple[i]->data[j])->value << "\t";
            else if(table->tuple[i]->data[j]->type == 1)
                cout << ((fData*)table->tuple[i]->data[j])->value << "\t";
            else
                cout << ((sData*)table->tuple[i]->data[j])->value << "\t";
        cout << endl;
    }
}

int Api::deleteRecord(std::string tableName, std::vector<WhereQuery> wq) {
    Table *table = cm->selectTable(tableName);
    if(table == nullptr) {
        throw std::exception("error: table is not exist!");
    } else {
        for(int i=0; i<wq.size(); i++) {
            int flag = 0;
            for(int j=0; j<table->attrCnt; j++) {
                if(table->attr[j].name == wq[i].col) {
                    if(table->attr[j].type == 1 && wq[i].d->type == 0) {
                        iData* ptr = (iData*)wq[i].d->type;
                        wq[i].d = new fData((float)(((iData*)wq[i].d)->value));
                        delete ptr;
                    } 

                    flag = 1;
                    if((table->attr[j].type == 0 && wq[i].d->type != 0)
                    || (table->attr[j].type == 1 && (wq[i].d->type != 1 && wq[i].d->type != 0))
                    || (table->attr[j].type == 2 && (wq[i].d->type - 2 > table->attr[j].length  || wq[i].d->type - 2 < 0)))
                        throw std::exception("error: values are not proper for attribute!");
                }
            }
            if(flag == 0) {
                std::string str = std::string("error: table has no attribute called ") + wq[i].col;
                throw std::exception(str.c_str());
            }
        }
        /*
        for(int i=0; i<table->indexCnt; i++)
            for(int j=0; j<wq.size(); j++) {
                if(table->attr[table->index[i].indexNum].name == wq[j].col) {
                    // 有索引，利用索引读入
                    if(wq[j].op == COMPARE::e) {
                        int address = im->search(table->index[i].name, wq[j].d);
                        #ifdef DEBUG
                            cout << "whereQuery index address = " << address << endl;
                        #endif
                        if(address == -1)
                            break;
                        rm->addRecord(table, address);
                    }
                    if(wq[j].op == COMPARE::ge || wq[j].op == COMPARE::gt) {
                        std::vector<int> arr = im->rangeSearch(table->index[i].name, wq[j].d, nullptr);
                        for(int i=0; i<arr.size(); i++)
                            if(arr[i] >= 0)
                                rm->addRecord(table, arr[i]);
                    }
                    if(wq[j].op == COMPARE::le || wq[j].op == COMPARE::lt) {
                        std::vector<int> arr = im->rangeSearch(table->index[i].name, nullptr, wq[j].d);
                        for(int i=0; i<arr.size(); i++)
                            if(arr[i] >= 0)
                                rm->addRecord(table, arr[i]);
                    }
                    break;
                }
            }*/
        if(table->tuple.size() == 0) {
            #ifdef DEBUG
                cout << "whereQuery no index" << endl;
            #endif
            rm->del(table, wq);
        } else {
            #ifdef DEBUG
                cout << "whereQuery have index" << endl;
                cout << "table->tuple.size() = " << table->tuple.size() << endl;
            #endif
            rm->choose(table, wq);
            #ifdef DEBUG
                cout << "choose function" << endl;
                cout << "table->tuple.size() = " << table->tuple.size() << endl;
            #endif
            for(int i=0; i<table->tuple.size(); i++)
                rm->deleteRecord(table, table->tuple[i]->address);
        }
        #ifdef DEBUG
            cout << "delete index" << endl;
        #endif
        for(int i=0; i<table->indexCnt; i++)
            for(int j=0; j<table->tuple.size(); j++) {
                #ifdef DEBUG
                    cout << "table->index[i].name = " << table->index[i].name << endl;
                    cout << "type = "
                        << ((iData*)table->tuple[j]->data[table->index[i].indexNum])->type << endl;
                    cout << "value = "
                        << ((iData*)table->tuple[j]->data[table->index[i].indexNum])->value << endl;
                #endif
                im->eliminate(table->index[i].name, table->tuple[j]->data[table->index[i].indexNum]);
            }
        return table->tuple.size();
    }
}
