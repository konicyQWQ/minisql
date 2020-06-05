#ifndef MINISQL_INTERPRETER_H
#define MINISQL_INTERPRETER_H

#include <iostream>
#include <vector>
#include "head/typedef.h"

class Interpreter
{
private:
    std::string query;

    std::vector<std::string> words;

    std::vector<bool> isString;

    void Pretreatment();

    void setWords();

    std::vector<WhereQuery> runWhere(int k);//这里的k意思是where后的第一个单词出现在words的下标k处

    void runExecFile();

public:
    void setQuery(std::string s);

    int runQuery();
};


#endif //MINISQL_INTERPRETER_H
