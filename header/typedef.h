#ifndef TYPEDEF_H
#define TYPEDEF_H

#include <string>
#include <vector>

typedef unsigned char byte;


/* 属性 */
struct Attribute {
    std::string name;   // 属性名(不超过80个字符)
    byte type, length;  // 类型：type=0 表示 int，type=1 表示 float，type=2 表示 char（length为char的长度）
    bool isUnique;      // 是否是唯一的
    bool notNULL;       // 是否不能为空
};

/* 索引 */
struct Index {
    std::string name;  // 索引名(不超过80个字符)
    byte indexNum;     // 索引在第几个属性上，Index和它所在的表有关，不能与表分开使用
};

/* 表 */
class Table {
public:
    std::string name;           // 表名
    byte attrCnt;               // 属性数量
    byte indexCnt;              // 索引数量
    byte primary;               // 主键在第几个属性上, -1也就是255表示没有主键
    Index index[32];            // 最多32个索引，从0开始
    Attribute attr[32];         // 最多32个属性，从0开始
    //std::vector<Tuple *> tuple; // 所有的元组
};

#endif