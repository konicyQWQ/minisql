#ifndef CATALOG_MANAGER
#define CATALOG_MANAGER

#include <string>
#include <map>
#include "header/typedef.h"

class CatalogManager {
private:
    std::map<std::string, int> allTable;
    std::map<std::string, std::string> allIndex;

public:
    CatalogManager();
    ~CatalogManager();

    /* 新建表 */
    int createTable(Table &table);
    /* 删除表 */
    int dropTable(std::string tableName);
    /* 新建索引 */
    int createIndex(std::string tableName, std::string indexName, byte indexNum);
    /* 删除索引 */
    int dropIndex(std::string indexName);
    /* 读取一张表的信息，如果不存在则返回 nullptr */
    Table* selectTable(std::string tableName);
    /* 展示表的信息 */
    void showTable(Table &table);

    int queryIndex(std::string indexName);
};

#endif