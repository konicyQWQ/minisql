//
// Created by yMac on 2020/6/28.
//

#ifndef BPT_INDEXMANAGER_H
#define BPT_INDEXMANAGER_H

#include "header/typedef.h"
#include "tree.h"

class IndexManager
{
public:
    /*
     * IndexManager不用做错误判断
     * 这是CatalogManager模块完成的事情
     * 在这里我们假设一切操作全部合法
     */

    void createIndex(Table &t, int indexNo); //建立索引，参数：表（引用）与索引序号

    void deleteIndex(Table& t, int indexNo); //删除索引，参数：表（引用）与索引序号

    void insert(string indexName, Data data, int offset); //插入记录，参数：索引名，数据与偏移量

    void eliminate(string indexName, Data data); //删除记录，参数：索引名，数据

    int search(string indexName, Data data); //查询记录，参数：索引名，数据

    std::vector<int> rangeSearch(string indexName, Data inf, Data sup); //范围查询记录，参数：索引名，数据下界，数据上界

};

#endif //BPT_INDEXMANAGER_H
