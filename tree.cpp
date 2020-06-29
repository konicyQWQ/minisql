#include "tree.h"

static void input(int count, char* array, ...){
    va_list parg;
    va_start(parg, array);
    for(int i = va_arg(parg, int), j = 0; j < count; i = va_arg(parg, int), j++)
        if(i != _)
            CHAR2INT(array + j * 4) = i;
}

BPTreeException::BPTreeException(string msg): msg(msg){
}

const char* BPTreeException::what() noexcept{
    return msg.c_str();
}

BPTree::BPTree(string filename, BufferManager *bm){
    this->BM = bm;
    keylength[INT_TYPE] = INT_LEN;
    keylength[FLOAT_TYPE] = FLOAT_LEN;
    for(int i = VCHAR_TYPE_START; i <= VCHAR_TYPE_END; i++)
        keylength[i] = VCHAR_LEN;
    fstream f;
    f.open(filename, ios::in);
    name = filename;
    if (!f){
        f.open(filename, ios::app);
        number = 0;
        f.close();
        return;
    }
    f.seekp(0, ios::end);
    number = (int)(f.tellg() / BLOCKSIZE);
    f.close();
    if (!number)
        return;
    Block* block = BM->getBlock(name, 0);
    type = CHAR2INT(block->buf + 20);
    order = (BLOCKSIZE - HEADER) / (keylength[type] + POINTER) - 1;
}

void BPTree::initialize(Data* key, int addr, int keyType){
    char* root = new char[BLOCKSIZE];
    int nkeys = 1;
    input(6, root, INTERNAL, 0, -1, nkeys, 0, keyType);
    type = keyType;
    const int KL = keylength[type];

    CHAR2INT(root + HEADER + KL) = nkeys;
    switch(key->type){
        case INT_TYPE: CHAR2INT(root + HEADER + nkeys * (KL + POINTER)) = ((iData*)key)->value; break;
        case FLOAT_TYPE: CHAR2FLOAT(root + HEADER + nkeys * (KL + POINTER)) = ((fData*)key)->value; break;
        default: memcpy((char*)(root + HEADER +nkeys * (KL + POINTER)), ((sData*)key)->value.c_str(), ((sData*)key)->value.length() + 1);
    }
    input(5, root + HEADER + nkeys * (2 * KL + POINTER), -1, -1, nkeys, 1, 0);
    Block *b = BM->getBlock(name, 0);
    memcpy(b->buf, root, BLOCKSIZE);
    BM->writeBlock(b);
    BM->removeBlock(b);

    char* block = new char[BLOCKSIZE];
    input(4, block, LEAF, 1, 0, 1, 0);
    nkeys = 1;
    CHAR2INT(block + HEADER + KL) = nkeys;
    if (key->type == INT_TYPE)
        CHAR2INT(block + HEADER + (KL + POINTER)) = ((iData*)key)->value;
    else if (key->type == FLOAT_TYPE)
        CHAR2FLOAT(block + HEADER + (KL + POINTER)) = ((fData*)key)->value;
    else
        memcpy((char*)(block + HEADER + (KL + POINTER)), ((sData*)key)->value.c_str(), ((sData*)key)->value.length() + 1);
    input(5, block + HEADER + nkeys * (KL + POINTER) + KL, -1, addr, nkeys, _, 0);
    b = BM->getBlock(name, 1);
    memcpy(b->buf, block, BLOCKSIZE);
    BM->writeBlock(b);
    BM->removeBlock(b);

    order = (BLOCKSIZE - HEADER) / (KL + POINTER) - 1;
    number = 2;
    type = keyType;
    delete[] block;
    delete[] root;
}

int BPTree::find(Data* key){
    char *block = new char[BLOCKSIZE];
    Block* b = BM->getBlock(name, 0);
    memcpy(block, b->buf, BLOCKSIZE);
    const int KL = keylength[type];
    int leafType = CHAR2INT(block);
    int bro = CHAR2INT(block + HEADER + KL);
    int tempBro, pos;

    while (leafType == INTERNAL) {
        int nkeys = CHAR2INT(block + 12);
        int i = 0;
        bool flag;
        for (i = 0; i < nkeys; i++) {
            if (type == INT_TYPE) {
                int tempKey = CHAR2INT(block + HEADER + bro*(KL + POINTER));
                flag = ((iData*)key)->value < tempKey;
            }  else if (type == FLOAT_TYPE) {
                float tempKey = CHAR2FLOAT(block + HEADER + bro*(KL + POINTER));
                flag = ((fData*)key)->value < tempKey;
            } else {
                string tempKey((char*)(block + HEADER + bro*(KL + POINTER)));
                flag = (((sData*)key)->value.compare(tempKey)) < 0;
            }
            tempBro = bro;
            pos = CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL + 4);
            bro = CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL);
            if(flag)
                break;
        }
        if (i == nkeys)
            pos = CHAR2INT(block + HEADER + tempBro*(KL + POINTER) + KL + 12);
        if (pos == -1) {
            return -1;
        }
        b = BM->getBlock(name, pos);
        memcpy(block, b->buf, BLOCKSIZE);
        leafType = CHAR2INT(block);
        bro = CHAR2INT(block + HEADER + KL);
    }

    int nkeys = CHAR2INT(block + 12);
    bro = CHAR2INT(block + HEADER + KL);
    
    for (int i = 0; i < nkeys; i++) {
        if (type == INT_TYPE) {
            int tempKey = CHAR2INT(block + HEADER + bro*(KL + POINTER));
            tempBro = bro;
            int addr = CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL + 4);
            pos = CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL + 4);
            bro = CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL);
            if ((((iData*)key)->value == tempKey) && CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL + 16) != 1) {
                delete[]block;
                return addr;
            }
        }  else if (type == FLOAT_TYPE) {
            float tempKey = CHAR2FLOAT(block + HEADER + bro*(KL + POINTER));
            tempBro = bro;
            int addr = CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL + 4);
            pos = CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL + 4);
            bro = CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL);
            if ((((fData*)key)->value == tempKey) && CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL + 16) != 1) {
                delete[]block;
                return addr;
            }
        } else {
            string tempKey((char*)(block + HEADER + bro*(KL + POINTER)));
            tempBro = bro;
            int addr = CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL + 4);
            pos = CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL + 4);
            bro = CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL);
            if ((((sData*)key)->value.compare(tempKey)) == 0 && CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL + 16) != 1) {
                delete[]block;
                return addr;
            }
        }
    }
    delete[]block;
    return -1;
}

std::vector<int> BPTree::rangeFind(Data* key1, Data* key2){
    char *block = new char[BLOCKSIZE];
    std::vector<int> res;
    res[0] = -1;
    Block* b = BM->getBlock(name, 0);
    memcpy(block, b->buf, BLOCKSIZE);
    
    const int KL = keylength[type];
    int leafType = CHAR2INT(block);
    int bro = CHAR2INT(block + HEADER + KL);
    int tempBro, pos;

    while (leafType == INTERNAL) {
        int nkeys = CHAR2INT(block + 12);
        bool flag;
        int i;
        for (i = 0; i < nkeys; i++) {
            if (type == INT_TYPE) {
                int tempKey = CHAR2INT(block + HEADER + bro*(KL + POINTER));
                flag = (!key1)||((iData*)key1)->value < tempKey;
            }  else if (type == FLOAT_TYPE) {
                float tempKey = CHAR2FLOAT(block + HEADER + bro*(KL + POINTER));
                flag = (!key1)||((fData*)key1)->value < tempKey;
            } else {
                string tempKey((char*)(block + HEADER + bro*(KL + POINTER)));
                flag = (!key1)||(((sData*)key1)->value.compare(tempKey)) < 0;
            }
            tempBro = bro;
            pos = CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL + 4);
            bro = CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL);
            if(flag)
                break;
        }
        if (i == nkeys)
            pos = CHAR2INT(block + HEADER + tempBro*(KL + POINTER) + KL + 12);
        if (pos == -1) 
            return res;
        b = BM->getBlock(name, pos);
        memcpy(block, b->buf, BLOCKSIZE);
        leafType = CHAR2INT(block);
        bro = CHAR2INT(block + HEADER + KL);
    }
    int nkeys = CHAR2INT(block + 12);
    bro = CHAR2INT(block + HEADER + KL);
    for (int i = 0; i < nkeys; i++) {
        if (type == INT_TYPE) {
            int tempKey = CHAR2INT(block + HEADER + bro*(KL + POINTER));
            int addr = CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL + 4);
            bro = CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL);
            if ((!key1 || (((iData*)key1)->value == tempKey)) && CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL + 16) != 1) {
                int j = 0;
                while (bro != -1 && (!key2 || tempKey < ((iData*)key2)->value)) {
                    res[j++] = addr;
                    bro = CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL);
                    tempKey = CHAR2INT(block + HEADER + bro*(KL + POINTER));
                    addr = CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL + 4);
                }
                res[j] = -1;
                delete[]block;
                return res;
            }
        }  else if (type == FLOAT_TYPE) {
            float tempKey = CHAR2FLOAT(block + HEADER + bro*(KL + POINTER));
            int addr = CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL + 4);
            bro = CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL);
            if ((!key1 || (((fData*)key1)->value == tempKey)) && CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL + 16) != 1) {
                int j = 0;
                while (bro != -1 && (!key2 || tempKey < ((fData*)key2)->value)) {
                    res[j++] = addr;
                    bro = CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL);
                    tempKey = CHAR2INT(block + HEADER + bro*(KL + POINTER));
                    addr = CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL + 4);
                }
                res[j] = -1;
                delete[]block;
                return res;
            }
        } else {
            string tempKey((char*)(block + HEADER + bro*(KL + POINTER)));
            int addr = CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL + 4);
            bro = CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL);
            if ((!key1 || (((sData*)key1)->value.compare(tempKey)) == 0) && CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL + 16) != 1) {
                int j = 0;
                while (bro != -1 && (!key2 || (((sData*)key2)->value.compare(tempKey)) > 0)) {
                    res[j++] = addr;
                    bro = CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL);
                    tempKey = CHAR2INT(block + HEADER + bro*(KL + POINTER));
                    addr = CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL + 4);
                }
                res[j] = -1;
                delete[]block;
                return res;
            }
        }
    }
    delete[]block;
    return res;
}

void BPTree::insert(Data* key, int addr){
    if (find(key) != -1){
        throw BPTreeException("Inserted duplicate key!");
        return;
    }
    Block* b = BM->getBlock(name, 0);
    char *block = new char[BLOCKSIZE];
    memcpy(block, b->buf, BLOCKSIZE);
    
    const int KL = keylength[type];
    int leafType = CHAR2INT(block);
    int bro = CHAR2INT(block + HEADER + KL);
    int tempBro = 0, pos = 0;

    while (leafType == INTERNAL) {
        int nkeys = CHAR2INT(block + 12);
        int i = 0;
        bool flag = false;
        for (i = 0; i < nkeys; i++) {
            if (type == INT_TYPE) {
                int tempKey = CHAR2INT(block + HEADER + bro*(KL + POINTER));
                if ((((iData*)key)->value < tempKey))
                    flag = true;
            }  else if (type == FLOAT_TYPE) {
                float tempKey = CHAR2FLOAT(block + HEADER + bro*(KL + POINTER));
                if ((((fData*)key)->value < tempKey))
                    flag = true;
            } else {
                string tempKey((char*)(block + HEADER + bro*(KL + POINTER)));
                if ((((sData*)key)->value.compare(tempKey)) < 0)
                    flag = true;
            }
            tempBro = bro;
            pos = CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL + 4);
            bro = CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL);
            if(flag)
                break;
        }
        if (i == nkeys)
            pos = CHAR2INT(block + HEADER + tempBro*(KL + POINTER) + KL + 12);
        if (pos == -1) {
            char* newBlock = new char[BLOCKSIZE];
            input(5, newBlock, LEAF, number++, CHAR2INT(block + 4), 1, 0);
            CHAR2INT(newBlock + HEADER + KL) = CHAR2INT(newBlock + 12);
            if (key->type == INT_TYPE)
                CHAR2INT(newBlock + HEADER + (KL + POINTER)) = ((iData*)key)->value;
            else if (key->type == FLOAT_TYPE)
                CHAR2FLOAT(newBlock + HEADER + (KL + POINTER)) = ((fData*)key)->value;
            else
                memcpy((char*)(newBlock + HEADER + (KL + POINTER)), ((sData*)key)->value.c_str(), ((sData*)key)->value.length() + 1);
            input(5, newBlock + HEADER + KL + POINTER + KL, -1, addr, 1, _, 0);
            pos = number - 1;
            Block* b = BM->getBlock(name, pos);
            memcpy(b->buf, newBlock, BLOCKSIZE);
            BM->writeBlock(b);
            BM->removeBlock(b);

            CHAR2INT(block + HEADER + tempBro*(KL + POINTER) + KL + 4) = number - 1;
            b = BM->getBlock(name, CHAR2INT(block + 4));
            memcpy(b->buf, block, BLOCKSIZE);
            BM->writeBlock(b);
            BM->removeBlock(b);
            return;
        }
        b = BM->getBlock(name, pos);
        memcpy(block, b->buf, BLOCKSIZE);
        leafType = CHAR2INT(block);
        bro = CHAR2INT(block + HEADER + KL);
    }

    int nkeys = CHAR2INT(block + 12);
    bro = CHAR2INT(block + HEADER + KL);
    if (nkeys < order) {
        CHAR2INT(block + 12) += 1;
        nkeys++;
        int lastBro = 0;
        int i = 0;

        for (i = 0; i < nkeys; i++) {
            if (bro == -1 && i == nkeys - 1)
                break;
            if (type == INT_TYPE) {
                int tempKey = CHAR2INT(block + HEADER + bro*(KL + POINTER));
                if ((((iData*)key)->value < tempKey))
                    break;
            }  else if (type == FLOAT_TYPE) {
                float tempKey = CHAR2FLOAT(block + HEADER + bro*(KL + POINTER));
                if ((((fData*)key)->value < tempKey))
                    break;
            } else {
                string tempKey((char*)(block + HEADER + bro*(KL + POINTER)));
                if ((((sData*)key)->value.compare(tempKey)) < 0)
                    break;
            }
            lastBro = bro;
            bro = CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL);
        }

        if (key->type == INT_TYPE)
            CHAR2INT(block + HEADER + nkeys*(KL + POINTER)) = ((iData*)key)->value;
        else if (key->type == FLOAT_TYPE)
            CHAR2FLOAT(block + HEADER + nkeys*(KL + POINTER)) = ((fData*)key)->value;
        else
            memcpy(block + HEADER + nkeys*(KL + POINTER), ((sData*)key)->value.c_str(), ((sData*)key)->value.length() + 1);

        input(3, block + HEADER + nkeys*(KL + POINTER) + KL, CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL + 8), addr, nkeys);
        CHAR2INT(block + HEADER + lastBro*(KL + POINTER) + KL) = nkeys;
        if (i == nkeys - 1)
            CHAR2INT(block + HEADER + nkeys*(KL + POINTER) + keylength[type]) = -1;

        Block* b = BM->getBlock(name, CHAR2INT(block + 4));
        memcpy(b->buf, block, BLOCKSIZE);
        BM->writeBlock(b);
        BM->removeBlock(b);
    } else {
        CHAR2INT(block + 12) += 1;
        Data* mid = nullptr;
        std::vector<int> res = split(block, mid, key, addr, 0, 0);
        BM->getBlock(name, 8);
    }
    delete[]block;
}

std::vector<int> BPTree::split(char* block, Data* mid, Data* key, int addr, int lpos, int rpos){
    int nkeys = CHAR2INT(block + 12);
    int leafType = CHAR2INT(block);
    std::vector<int> father;
    father[0] =  father[1] = 0;
    if ((CHAR2INT(block + 8) == -1) && nkeys >= order - 1) {
        char* newBlock1 = new char[BLOCKSIZE];
        char* newBlock2 = new char[BLOCKSIZE];
        internalSplit(newBlock1, newBlock2, block, mid, lpos, rpos);
        lpos = CHAR2INT(newBlock1 + 4);
        rpos = CHAR2INT(newBlock2 + 4);
        CHAR2INT(newBlock1 + 8) = 0;
        CHAR2INT(newBlock2 + 8) = 0;
        if (type == INT_TYPE) 
            mid = new iData(CHAR2INT(newBlock2 + HEADER + keylength[type] + POINTER));
        else if (type == FLOAT_TYPE)
            mid = new fData(CHAR2FLOAT(newBlock2 + HEADER + keylength[type] + POINTER));
        else
            mid = new sData(newBlock2 + HEADER + keylength[type] + POINTER);

        Block* b = BM->getBlock(name, CHAR2INT(newBlock1 + 4));
        memcpy(b->buf, newBlock1, BLOCKSIZE);
        BM->writeBlock(b);
        BM->removeBlock(b);
        
        b = BM->getBlock(name, CHAR2INT(newBlock2 + 4));
        memcpy(b->buf, newBlock2, BLOCKSIZE);
        BM->writeBlock(b);
        BM->removeBlock(b);
        
        char* root = new char[BLOCKSIZE];
        input(6, root, INTERNAL, 0, -1, 1, 0, type);

        int nkeys = CHAR2INT(root + 12);
        CHAR2INT(root + HEADER + keylength[type]) = nkeys;
        if (key->type == INT_TYPE)
            CHAR2INT(root + HEADER + nkeys*(keylength[type] + POINTER)) = ((iData*)mid)->value;
        else if (key->type == FLOAT_TYPE)
            CHAR2FLOAT(root + HEADER + nkeys*(keylength[type] + POINTER)) = ((fData*)mid)->value;
        else
            memcpy((char*)(root + HEADER + nkeys*(keylength[type] + POINTER)), ((sData*)mid)->value.c_str(), ((sData*)mid)->value.length() + 1);
        input(5, root + HEADER + nkeys*(keylength[type] + POINTER) + keylength[type], -1, lpos, nkeys, rpos, 0);

        b = BM->getBlock(name, CHAR2INT(root + 4));
        memcpy(b->buf, root, BLOCKSIZE);
        BM->writeBlock(b);
        BM->removeBlock(b);

        father[0] = CHAR2INT(newBlock1 + 4);
        father[1] = CHAR2INT(newBlock2 + 4);

        delete[]root;
        delete[]newBlock1;
        delete[]newBlock2;
        return father;
    } else {
        if (leafType == LEAF && nkeys >= order) {
            char* newBlock1 = new char[BLOCKSIZE];
            char* newBlock2 = new char[BLOCKSIZE];
            std::vector<int> temp;
            leafSplit(newBlock1, newBlock2, block, key, addr);
            CHAR2INT(block + 16) = 1;
            lpos = CHAR2INT(newBlock1 + 4);
            rpos = CHAR2INT(newBlock2 + 4);

            if (type == INT_TYPE) {
                mid = new iData(CHAR2INT(newBlock2 + HEADER + keylength[type] + POINTER));
            } else if (type == FLOAT_TYPE) {
                mid = new fData(CHAR2FLOAT(newBlock2 + HEADER + keylength[type] + POINTER));
            } else {
                mid = new sData((char*)(newBlock2 + HEADER + keylength[type] + POINTER));
            }

            Block* b = BM->getBlock(name, CHAR2INT(block + 8));
            char* fatherBlock = new char[BLOCKSIZE];
            memcpy(fatherBlock, b->buf, BLOCKSIZE);

            temp = split(fatherBlock, mid, key, addr, lpos, rpos);

            CHAR2INT(newBlock1 + 8) = temp[0];
            CHAR2INT(newBlock2 + 8) = temp[1];

            b = BM->getBlock(name, CHAR2INT(block + 4));
            memcpy(b->buf, block, BLOCKSIZE);
            BM->writeBlock(b);
            BM->removeBlock(b);

            b = BM->getBlock(name, CHAR2INT(newBlock1 + 4));
            memcpy(b->buf, newBlock1, BLOCKSIZE);
            BM->writeBlock(b);
            BM->removeBlock(b);
            
            b = BM->getBlock(name, CHAR2INT(newBlock2 + 4));
            memcpy(b->buf, newBlock2, BLOCKSIZE);
            BM->writeBlock(b);
            BM->removeBlock(b);

            father[0] = CHAR2INT(newBlock1 + 4);
            father[1] = CHAR2INT(newBlock2 + 4);

            delete[]newBlock1;
            delete[]newBlock2;
            delete[]fatherBlock;
            return father;
        }  else if (leafType == INTERNAL && nkeys >= order - 1) {
            char*newBlock1 = new char[BLOCKSIZE];
            char*newBlock2 = new char[BLOCKSIZE];
            internalSplit(newBlock1, newBlock2, block, mid, lpos, rpos);
            lpos = CHAR2INT(newBlock1 + 4);
            rpos = CHAR2INT(newBlock2 + 4);
            CHAR2INT(block + 16) = 1;
            std::vector<int> temp;

            if (type == INT_TYPE) {
                mid = new iData(CHAR2INT(newBlock2 + HEADER + keylength[type] + POINTER));
            }  else if (type == FLOAT_TYPE) {
                mid = new fData(CHAR2FLOAT(newBlock2 + HEADER + keylength[type] + POINTER));
            } else {
                mid = new sData((char*)(newBlock2 + HEADER + keylength[type] + POINTER));
            }

            Block* b = BM->getBlock(name, CHAR2INT(block + 8));
            char*fatherBlock = new char[BLOCKSIZE];
            memcpy(fatherBlock, b->buf, BLOCKSIZE);

            temp = split(fatherBlock, mid, key, addr, lpos, rpos);

            CHAR2INT(newBlock1 + 8) = temp[0];
            CHAR2INT(newBlock2 + 8) = temp[1];

            b = BM->getBlock(name, CHAR2INT(block + 4));
            memcpy(b->buf, block, BLOCKSIZE);
            BM->writeBlock(b);
            BM->removeBlock(b);

            b = BM->getBlock(name, CHAR2INT(newBlock1 + 4));
            memcpy(b->buf, newBlock1, BLOCKSIZE);
            BM->writeBlock(b);
            BM->removeBlock(b);

            b = BM->getBlock(name, CHAR2INT(newBlock2 + 4));
            memcpy(b->buf, newBlock2, BLOCKSIZE);
            BM->writeBlock(b);
            BM->removeBlock(b);

            father[0] = CHAR2INT(newBlock1 + 4);
            father[1] = CHAR2INT(newBlock2 + 4);

            delete[]newBlock1;
            delete[]newBlock2;
            delete[]fatherBlock;
            return father;
        }  else if (leafType == INTERNAL && nkeys < order - 1) {
            CHAR2INT(block + 12) += 1;
            internalInsert(block, mid, lpos, rpos);
            Block* b = BM->getBlock(name, 8);
            b = BM->getBlock(name, CHAR2INT(block + 4));
            memcpy(b->buf, block, BLOCKSIZE);
            BM->writeBlock(b);
            BM->removeBlock(b);
            b = BM->getBlock(name, 8);
            father[0] = CHAR2INT(block + 4);
            father[1] = CHAR2INT(block + 4);
            return father;
        }
    }
}

void BPTree::internalInsert(char* block, Data* mid, int lpos, int rpos){
    int nkeys = CHAR2INT(block + 12);
    int bro = CHAR2INT(block + HEADER + keylength[type]);
    int lastBro = 0;
    int i = 0;

    for (i = 0; i < nkeys; i++) {
        if (bro == -1 && i == nkeys - 1)
            break;
        if (type == INT_TYPE) {
            int tempKey = CHAR2INT(block + HEADER + bro*(keylength[type] + POINTER));
            if ((((iData*)mid)->value < tempKey))
                break;
        }  else if (type == FLOAT_TYPE) {
            float tempKey = CHAR2FLOAT(block + HEADER + bro*(keylength[type] + POINTER));
            if ((((fData*)mid)->value < tempKey))
                break;
        } else {
            string tempKey = (char*)(block + HEADER + bro*(keylength[type] + POINTER));
            if ((((sData*)mid)->value.compare(tempKey)) < 0)
                break;
        }
        lastBro = bro;
        bro = CHAR2INT(block + HEADER + bro*(keylength[type] + POINTER) + keylength[type]);
    }
    if (type == INT_TYPE)
        CHAR2INT(block + HEADER + nkeys*(keylength[type] + POINTER)) = ((iData*)mid)->value;
    else if (type == FLOAT_TYPE)
        CHAR2FLOAT(block + HEADER + nkeys*(keylength[type] + POINTER)) = ((fData*)mid)->value;
    else
        memcpy((char*)(block + HEADER + nkeys*(keylength[type] + POINTER)), ((sData*)mid)->value.c_str(), ((sData*)mid)->value.length() + 1);

    if (i == nkeys - 1) {
        input(4, block + HEADER + nkeys*(keylength[type] + POINTER) + keylength[type], -1, lpos, nkeys, rpos);
        input(4, block + HEADER + lastBro*(keylength[type] + POINTER) + keylength[type], nkeys, _, _, lpos);
    } else {
        input(4, block + HEADER + nkeys*(keylength[type] + POINTER) + keylength[type], bro, lpos, nkeys, rpos);
        input(4, block + HEADER + lastBro*(keylength[type] + POINTER) + keylength[type], nkeys, _, _, lpos);
        CHAR2INT(block + HEADER + bro*(keylength[type] + POINTER) + keylength[type] + 4) = rpos;
    }
}

void BPTree::leafSplit(char* block1, char* block2, char* block, Data* key, int addr){
    int nkeys = CHAR2INT(block + 12);
    const int KL = keylength[type];
    int bro = CHAR2INT(block + HEADER + KL);
    int address, flag = 1;
    int i, pos = 1;
    input(5, block1, LEAF, number++, 0, nkeys / 2, 0);
    input(5, block2, LEAF, number++, 0,nkeys - nkeys / 2, 0);
    CHAR2INT(block1 + HEADER + KL) = CHAR2INT(block2 + HEADER + KL) = 1;

    for (i = 0; i < nkeys / 2; i++, pos++) {
        if (type == INT_TYPE) {
            int tempKey = CHAR2INT(block + HEADER + bro*(KL + POINTER));
            address = CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL + 4);
            if ((((iData*)key)->value < tempKey) && flag) {
                CHAR2INT(block1 + HEADER + pos*(KL + POINTER)) = ((iData*)key)->value;
                input(4, block1 + HEADER + pos*(KL + POINTER) + KL, pos + 1, addr, pos, 0);
                flag = 0;
            } else {
                CHAR2INT(block1 + HEADER + pos*(KL + POINTER)) = tempKey;
                input(4, block1 + HEADER + pos*(KL + POINTER) + KL, pos + 1, address, pos, 0);
                bro = CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL);
            }
        }  else if (type == FLOAT_TYPE) {
            float tempKey = CHAR2FLOAT(block + HEADER + bro*(KL + POINTER));
            address = CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL + 4);
            if ((((fData*)key)->value < tempKey) && flag) {
                CHAR2FLOAT(block1 + HEADER + pos*(KL + POINTER)) = ((fData*)key)->value;
                input(4, block1 + HEADER + pos*(KL + POINTER) + KL, pos + 1, addr, pos, 0);
                flag = 0;
            } else {
                CHAR2FLOAT(block1 + HEADER + pos*(KL + POINTER)) = tempKey;
                input(4, block1 + HEADER + pos*(KL + POINTER) + KL, pos + 1, address, pos, 0);
                bro = CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL);
            }
        } else {
            string tempKey;
            tempKey = (char*)(block + HEADER + bro*(KL + POINTER));
            address = CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL + 4);
            if ((((sData*)key)->value.compare(tempKey)) < 0 && flag) {
                memcpy((char*)(block1 + HEADER + pos*(KL + POINTER)), ((sData*)key)->value.c_str(), ((sData*)key)->value.length() + 1);
                input(4, block1 + HEADER + pos*(KL + POINTER) + KL, pos + 1, addr, pos, 0);
                flag = 0;
            } else {
                memcpy((char*)(block1 + HEADER + pos*(KL + POINTER)), tempKey.c_str(), tempKey.length() + 1);
                input(4, block1 + HEADER + pos*(KL + POINTER) + KL, pos + 1, address, pos, 0);
                bro = CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL);
            }
        }
    }
    CHAR2INT(block1 + HEADER + (pos - 1)*(KL + POINTER) + KL) = -1;

    pos = 1;
    for (; i < nkeys; i++, pos++) {
        if (bro == -1 && i == nkeys - 1)
            break;
        if (type == INT_TYPE) {
            int tempKey = CHAR2INT(block + HEADER + bro*(KL + POINTER));
            address = CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL + 4);
            if ((((iData*)key)->value < tempKey) && flag) {
                CHAR2INT(block2 + HEADER + pos*(KL + POINTER)) = ((iData*)key)->value;
                input(4, block2 + HEADER + pos*(KL + POINTER) + KL, pos + 1, addr, pos, 0);
                flag = 0;
            } else {
                CHAR2INT(block2 + HEADER + pos*(KL + POINTER)) = tempKey;
                input(4, block2 + HEADER + pos*(KL + POINTER) + KL, pos + 1, address, pos, 0);
                bro = CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL);
            }
        }  else if (type == FLOAT_TYPE) {
            float tempKey = CHAR2FLOAT(block + HEADER + bro*(KL + POINTER));
            address = CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL + 4);
            if ((((fData*)key)->value < tempKey) && flag) {
                CHAR2FLOAT(block2 + HEADER + pos*(KL + POINTER)) = ((fData*)key)->value;
                input(4, block2 + HEADER + pos*(KL + POINTER) + KL, pos + 1, addr, pos, 0);
                flag = 0;
            } else {
                CHAR2FLOAT(block2 + HEADER + pos*(KL + POINTER)) = tempKey;
                input(4, block2 + HEADER + pos*(KL + POINTER) + KL, pos + 1, address, pos, 0);
                bro = CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL);
            }
        } else {
            string tempKey = (char*)(block + HEADER + bro*(KL + POINTER));
            address = CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL + 4);
            if ((((sData*)key)->value.compare(tempKey)) < 0 && flag) {
                memcpy((char*)(block2 + HEADER + pos*(KL + POINTER)), ((sData*)key)->value.c_str(), ((sData*)key)->value.length() + 1);
                input(4, block2 + HEADER + pos*(KL + POINTER) + KL, pos + 1, addr, pos, 0);
                flag = 0;
            } else {
                memcpy((char*)(block2 + HEADER + pos*(KL + POINTER)), tempKey.c_str(), tempKey.length() + 1);
                input(4, block2 + HEADER + pos*(KL + POINTER) + KL, pos + 1, address, pos, 0);
                bro = CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL);
            }
        }
    }
    if (flag) {
        if (key->type == INT_TYPE)
            CHAR2INT(block2 + HEADER + pos*(KL + POINTER)) = ((iData*)key)->value;
        else if (key->type == FLOAT_TYPE)
            CHAR2FLOAT(block2 + HEADER + pos*(KL + POINTER)) = ((fData*)key)->value;
        else
            memcpy((char*)(block2 + HEADER + pos*(KL + POINTER)), ((sData*)key)->value.c_str(), ((sData*)key)->value.length() + 1);
        input(5, block2 + HEADER + pos*(KL + POINTER) + KL, -1, addr, pos, _, 0);
        pos++;
    }
    CHAR2INT(block2 + HEADER + (pos - 1)*(KL + POINTER) + KL) = -1;
}

void BPTree::internalSplit(char* block1, char* block2, char* block, Data* mid, int lpos, int rpos){
    CHAR2INT(block + 12) += 1;
    const int KL = keylength[type];
    int nkeys = CHAR2INT(block + 12);
    int bro = CHAR2INT(block + HEADER + KL);
    int addr1, addr2;
    int i, pos = 1, flag = 1, lastBro = 0;
    input(5, block1, INTERNAL, number++, 0, nkeys / 2, 0);
    input(5, block2, INTERNAL, number++, 0, nkeys - nkeys / 2, 0);
    CHAR2INT(block1 + HEADER + KL) = CHAR2INT(block2 + HEADER + KL) = 1;

    for (i = 0; i < nkeys / 2; i++, pos++) {
        addr1 = CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL + 4);
        addr2 = CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL + 12);
        if (type == INT_TYPE) {
            int tempKey = CHAR2INT(block + HEADER + bro*(KL + POINTER));
            if ((((iData*)mid)->value <= tempKey) && flag) {
                CHAR2INT(block1 + HEADER + pos*(KL + POINTER)) = ((iData*)mid)->value;
                input(5, block1 + HEADER + pos*(KL + POINTER) + KL, pos + 1, lpos, pos, rpos, 0);
                CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL + 4) = rpos;
                CHAR2INT(block1 + HEADER + (pos - 1)*(KL + POINTER) + KL + 12) = lpos;
                flag = 0;
            } else {
                CHAR2INT(block1 + HEADER + pos*(KL + POINTER)) = tempKey;
                input(5, block1 + HEADER + pos*(KL + POINTER) + KL, pos + 1, addr1, pos, addr2, 0);
                lastBro = bro;
                bro = CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL);
            }
        }  else if (type == FLOAT_TYPE) {
            float tempKey = CHAR2FLOAT(block + HEADER + bro*(KL + POINTER));
            if ((((fData*)mid)->value <= tempKey) && flag) {
                CHAR2FLOAT(block1 + HEADER + pos*(KL + POINTER)) = ((fData*)mid)->value;
                input(5, block1 + HEADER + pos*(KL + POINTER) + KL, pos + 1, lpos, pos, rpos, 0);
                CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL + 4) = rpos;
                CHAR2INT(block1 + HEADER + (pos - 1)*(KL + POINTER) + KL + 12) = lpos;
                flag = 0;
            } else {
                CHAR2FLOAT(block1 + HEADER + pos*(KL + POINTER)) = tempKey;
                input(5, block1 + HEADER + pos*(KL + POINTER) + KL, pos + 1, addr1, pos, addr2, 0);
                lastBro = bro;
                bro = CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL);
            }
        } else {
            string tempKey = (char*)(block + HEADER + bro*(KL + POINTER));
            if ((((sData*)mid)->value.compare(tempKey)) <= 0 && flag) {
                memcpy((char*)(block1 + HEADER + pos*(KL + POINTER)), ((sData*)mid)->value.c_str(), ((sData*)mid)->value.length() + 1);
                input(5, block1 + HEADER + pos*(KL + POINTER) + KL, pos + 1, lpos, pos, rpos, 0);
                CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL + 4) = rpos;
                CHAR2INT(block1 + HEADER + (pos - 1)*(KL + POINTER) + KL + 12) = lpos;
                flag = 0;
            } else {
                memcpy((char*)(block1 + HEADER + pos*(KL + POINTER)), tempKey.c_str(), tempKey.length() + 1);
                input(5, block1 + HEADER + pos*(KL + POINTER) + KL, pos + 1, addr1, pos, addr2, 0);
                lastBro = bro;
                bro = CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL);
            }
        }
    }

    pos = 1;
    int j = i;
    for (; i < nkeys; i++, pos++) {
        if (bro == -1 && i == nkeys - 1)
            break;
        if (type == INT_TYPE) {
            int tempKey = CHAR2INT(block + HEADER + bro*(KL + POINTER));
            addr1 = CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL + 4);
            addr2 = CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL + 12);
            if ((((iData*)mid)->value <= tempKey) && flag) {
                CHAR2INT(block2 + HEADER + pos*(KL + POINTER)) = ((iData*)mid)->value;
                input(4, block2 + HEADER + pos*(KL + POINTER) + KL, pos + 1, lpos, pos, rpos);
                CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL + 4) = rpos;
                if (i == j) 
                    CHAR2INT(block1 + HEADER + CHAR2INT(block1 + 12) * (KL + POINTER) + KL + 12) = lpos;
                else 
                    CHAR2INT(block2 + HEADER + (pos - 1)*(KL + POINTER) + KL + 12) = lpos;
                CHAR2INT(block2 + HEADER + pos*(KL + POINTER) + KL + 16) = 0;
                flag = 0;
            } else {
                CHAR2INT(block2 + HEADER + pos*(KL + POINTER)) = tempKey;
                input(5, block2 + HEADER + pos*(KL + POINTER) + KL, pos + 1, addr1, pos, addr2, 0);
                lastBro = bro;
                bro = CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL);
            }
        }  else if (type == FLOAT_TYPE) {
            int tempKey = CHAR2INT(block + HEADER + bro*(KL + POINTER));
            addr1 = CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL + 4);
            addr2 = CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL + 12);
            if ((((fData*)mid)->value <= tempKey) && flag) {
                CHAR2INT(block2 + HEADER + pos*(KL + POINTER)) = ((fData*)mid)->value;
                input(4, block2 + HEADER + pos*(KL + POINTER) + KL, pos + 1, lpos, pos, rpos);
                CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL + 4) = rpos;
                if (i == j)
                    CHAR2INT(block1 + HEADER + CHAR2INT(block1 + 12) * (KL + POINTER) + KL + 12) = lpos;
                else
                    CHAR2INT(block2 + HEADER + (pos - 1)*(KL + POINTER) + KL + 12) = lpos;
                CHAR2INT(block2 + HEADER + pos*(KL + POINTER) + KL + 16) = 0;
                flag = 0;
            } else {
                CHAR2INT(block2 + HEADER + pos*(KL + POINTER)) = tempKey;
                input(5, block2 + HEADER + pos*(KL + POINTER) + KL, pos + 1, addr1, pos, addr2, 0);
                lastBro = bro;
                bro = CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL);
            }
        } else {
            string tempKey = (char*)(block + HEADER + bro*(KL + POINTER));
            addr1 = CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL + 4);
            addr2 = CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL + 12);
            if ((((sData*)mid)->value.compare(tempKey)) <= 0 && flag) {
                memcpy((char*)(block2 + HEADER + pos*(KL + POINTER)), ((sData*)mid)->value.c_str(), ((sData*)mid)->value.length() + 1);
                input(4, block2 + HEADER + pos*(KL + POINTER) + KL, pos + 1, lpos, pos, rpos);
                CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL + 4) = rpos;
                if (i == j)
                    CHAR2INT(block1 + HEADER + CHAR2INT(block1 + 12) * (KL + POINTER) + KL + 12) = lpos;
                else
                    CHAR2INT(block2 + HEADER + (pos - 1)*(KL + POINTER) + KL + 12) = lpos;
                CHAR2INT(block2 + HEADER + pos*(KL + POINTER) + KL + 16) = 0;
                flag = 0;
            } else {
                memcpy((char*)(block2 + HEADER + pos*(KL + POINTER)), tempKey.c_str(), tempKey.length() + 1);
                input(5, block2 + HEADER + pos*(KL + POINTER) + KL, pos + 1, addr1, pos, addr2, 0);
                lastBro = bro;
                bro = CHAR2INT(block + HEADER + bro*(KL + POINTER) + KL);
            }
        }
    }
    if (flag) {
        if (mid->type == INT_TYPE)
            CHAR2INT(block2 + HEADER + pos*(KL + POINTER)) = ((iData*)mid)->value;
        else if (mid->type == FLOAT_TYPE)
            CHAR2FLOAT(block2 + HEADER + pos*(KL + POINTER)) = ((fData*)mid)->value;
        else
            memcpy((char*)(block2 + HEADER + pos*(KL + POINTER)), ((sData*)mid)->value.c_str(), ((sData*)mid)->value.length() + 1);
        input(5, block2 + HEADER + pos*(KL + POINTER) + KL, -1, lpos, pos, rpos, 0);
        CHAR2INT(block2 + HEADER + (pos - 1)*(KL + POINTER) + KL + 12) = lpos;
        pos++;
    }
}

void BPTree::remove(Data* key){
    char *block = new char[BLOCKSIZE];
    Block* b = BM->getBlock(name, 0);
    memcpy(block, b->buf, BLOCKSIZE);
    int leafType = CHAR2INT(block);
    int bro = CHAR2INT(block + HEADER + keylength[type]);
    int tempBro, pos;
    while (leafType == INTERNAL) {
        int nkeys = CHAR2INT(block + 12);
        int i = 0;
        bool flag = false;
        for (i = 0; i < nkeys; i++) {
            if (type == INT_TYPE) {
                int tempKey = CHAR2INT(block + HEADER + bro*(keylength[type] + POINTER));
                flag = ((iData*)key)->value < tempKey;
            }  else if (type == FLOAT_TYPE) {
                float tempKey = CHAR2FLOAT(block + HEADER + bro*(keylength[type] + POINTER));
                flag = ((fData*)key)->value < tempKey;
            } else {
                string tempKey = (char*)(block + HEADER + bro*(keylength[type] + POINTER));
                flag = (((sData*)key)->value.compare(tempKey)) < 0;
            }
            tempBro = bro;
            pos = CHAR2INT(block + HEADER + bro*(keylength[type] + POINTER) + keylength[type] + 4);
            bro = CHAR2INT(block + HEADER + bro*(keylength[type] + POINTER) + keylength[type]);
            if (flag)
                break;
        }
        if (i == nkeys)
            pos = CHAR2INT(block + HEADER + tempBro*(keylength[type] + POINTER) + keylength[type] + 12);
        if (pos == -1)
            throw BPTreeException("Wrong deletion!");
        b = BM->getBlock(name, pos);
        memcpy(block, b->buf, BLOCKSIZE);
        leafType = CHAR2INT(block);
        bro = CHAR2INT(block + HEADER + keylength[type]);
    }

    int nkeys = CHAR2INT(block + 12);
    bro = CHAR2INT(block + HEADER + keylength[type]);
    int i = 0;
    bool flag = true;
    for (i = 0; i < nkeys; i++) {
        if (type == INT_TYPE) {
            int tempKey = CHAR2INT(block + HEADER + bro*(keylength[type] + POINTER));
            flag = (((iData*)key)->value == tempKey) && CHAR2INT(block + HEADER + bro*(keylength[type] + POINTER) + keylength[type] + 16) != 1;
        }  else if (type == FLOAT_TYPE) {
            float tempKey = CHAR2FLOAT(block + HEADER + bro*(keylength[type] + POINTER));
            flag = (((fData*)key)->value == tempKey) && CHAR2INT(block + HEADER + bro*(keylength[type] + POINTER) + keylength[type] + 16) != 1;
        } else {
            string tempKey = (char*)(block + HEADER + bro*(keylength[type] + POINTER));
            (((sData*)key)->value.compare(tempKey)) == 0 && CHAR2INT(block + HEADER + bro*(keylength[type] + POINTER) + keylength[type] + 16) != 1;
        }
        bro = CHAR2INT(block + HEADER + bro*(keylength[type] + POINTER) + keylength[type]);
        if (flag){
            CHAR2INT(block + HEADER + bro*(keylength[type] + POINTER) + keylength[type] + 16) = 1;
            Block* b = BM->getBlock(name, CHAR2INT(block + 4));
            memcpy(b->buf, block, BLOCKSIZE);
            break;
        } else
            throw BPTreeException("Wrong deletion!");
        
        if (i == nkeys)
            throw BPTreeException("Wrong deletion!");
    }
    delete[]block;
}

bool BPTree::isEmpty() const{
    return number == 0;
}
