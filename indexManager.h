#ifndef MINISQL_INDEXMANAGER_H
#define MINISQL_INDEXMANAGER_H

#include "header/typedef.h"

class IndexManager
{
private:
    Table t;
    Attribute onAttr;
    std::string fileName;
public:
    /*
     * IndexManager不用做错误判断
     * 这是CatalogManager模块完成的事情
     * 在这里我们假设一切操作全部合法
     */
    IndexManager(Table &t, const Attribute& onAttr);

    //~IndexManager();

    int createIndex(std::string indexName);

    void deleteIndex();//直接删

    int getOffset(Data *d);//一个壳，其实是调用bptree的search

    std::vector<int> getInterval(Data *d1, Data *d2);
};


#endif //MINISQL_INDEXMANAGER_H
