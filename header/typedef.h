#ifndef MINISQL_TYPEDEF_H
#define MINISQL_TYPEDEF_H

#include <string>
#include <utility>
#include <vector>

typedef unsigned char byte;

/* 缓冲区块的定义 */
struct Block {
    char buf[4096];
    std::string filename;
    int time, offset;
    bool writeMark, pin;
};

class Data
{
public:
    int type;//0:int;1:float;otherwise vchar
};

class iData : public Data
{
public:
    int value;

    explicit iData(int x) : Data(), value(x)
    {
        type = 0;
    }
};

class fData : public Data
{
public:
    float value;

    explicit fData(float x) : Data(), value(x)
    {
        type = 1;
    }
};

class sData : public Data
{
public:
    std::string value;

    explicit sData(std::string x) : Data(), value(std::move(x))
    {
        type = x.size() + 2;//为了防止和0 1冲突
    }
};

class Tuple
{
private:
    std::vector<Data *> data;
public:
    explicit Tuple(std::vector<Data *> d) : data(std::move(d))
    {

    }
};


/* 属性 */
struct Attribute
{
    std::string name;   // 属性名(不超过80个字符)
    byte type, length;  // 类型：type=0 表示 int，type=1 表示 float，type=2 表示 char（length为char的长度）
    bool isUnique;      // 是否是唯一的
    bool notNULL;       // 是否不能为空
};

/* 索引 */
struct Index
{
    std::string name;  // 索引名(不超过80个字符)
    byte indexNum;     // 索引在第几个属性上，Index和它所在的表有关，不能与表分开使用
};

/* 表 */
class Table
{
public:
    std::string name;           // 表名
    byte attrCnt;               // 属性数量
    byte indexCnt;              // 索引数量
    byte primary;               // 主键在第几个属性上, -1也就是255表示没有主键
    Index index[32];            // 最多32个索引，从0开始
    Attribute attr[32];         // 最多32个属性，从0开始
    std::vector<Tuple *> tuple; // 所有的元组
};

typedef enum
{
    e, ne, gt, lt, ge, le
} COMPARE;

struct WhereQuery
{
    explicit WhereQuery(Data &d) : d(d)
    {
    }
    std::string col;
    COMPARE op;
    Data& d;//这里用了值传递，没有用指针
};
#endif //MINISQL_TYPEDEF_H
