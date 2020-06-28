#ifndef BPT_TREE_H
#define BPT_TREE_H

#include "bufferManager.h"
#include "header/typedef.h"
#include <iostream>
#include <string>
#include <fstream>
#include <exception>
#include <cstdarg>

#define CHAR2INT(array) *(int*)(array)
#define CHAR2FLOAT(array) *(float*)(array)

using namespace std;
BufferManager BM;

#ifndef CONST_INTEGERS
// 偏移量
const int BLOCKSIZE = 4096;
const int HEADER = 24;
const int POINTER = 20;
const int NKEYS = 12;

// 类型相关
const int INTERNAL = 0;
const int LEAF = 1;
const int INT_TYPE = 0;
const int INT_LEN = 4;
const int FLOAT_TYPE = 1;
const int FLOAT_LEN = 4;
const int VCHAR_TYPE_START = 2;
const int VCHAR_TYPE_END = 257;
const int VCHAR_LEN = 20;
const int _ = 0xffffffff;
#endif

class BPTreeException: public exception{
    string msg;
public:
    BPTreeException(string msg);
    const char* what() noexcept;
};

class BPTree{
private:
    string name;
    int type;
    int order;
    int keylength[VCHAR_TYPE_END];
    int number;
    void internalInsert(char* b, Data* mid, int lpos, int rpos);
    void leafSplit(char* block1, char* block2, char* block, Data* key, int addr);
    void internalSplit(char* block1, char* block2, char* block, Data* mid, int lpos, int rpos);
    int* split(char* b, Data* mid, Data* key, int addr, int lpos, int rpos);
public:
    BPTree(string filename);
    ~BPTree();
    void initialize(Data* key, int addr, int keyType); // 参数：第一个键，地址和键的类型(0: int, 1: float, 2~257: vchar)
    int find(Data* key); // 参数：键名
    int* rangeFind(Data* key1, Data* key2); // 参数：范围的下界，范围的上界
    void insert(Data* key, int addr); // 参数：键名，地址
    void remove(Data* key); // 参数：键名
};

static void input(char* array, ...);

#endif //BPT_TREE_H
