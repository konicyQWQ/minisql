#ifndef MINISQL_INTERPRETER_H
#define MINISQL_INTERPRETER_H

#include <iostream>
#include <vector>
#include "header/typedef.h"
#include "api.h"

class Interpreter
{
private:
    std::string query;

    std::vector<std::string> words;

    std::vector<int> isString;

    Api *api;    // 新增

    void Pretreatment();

    void setWords();

    std::vector<WhereQuery> runWhere(int k);//这里的k意思是where后的第一个单词出现在words的下标k处

    void runExecFile();

public:
    Interpreter();  // 新增：因为 api 需要 new 出来，所以这里添加了构造函数
    ~Interpreter(); // 新增：因为 api 需要 delete，所以加了析构函数

    void setQuery(std::string s);

    int runQuery();
};


#endif //MINISQL_INTERPRETER_H
