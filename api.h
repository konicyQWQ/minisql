#ifndef API_H
#define API_H

#include "header/typedef.h"
#include "catalogManager.h"
#include "bufferManager.h"
#include "recordManager.h"
#include "indexManager.h"
#include <string>
#include <vector>

class Api
{
private:
    CatalogManager *cm;
    RecordManager *rm;
    IndexManager *im;
    BufferManager *bm;

public:
    Api();
    ~Api();

    void showTable(std::string tableName);
    void showTuple(Table *table);

    void createTable(std::string tableName, std::vector<Attribute> attr, std::string primaryKey);
    void createIndex(std::string tableName, std::string indexName, std::string colName);

    void dropTable(std::string tableName);
    void dropIndex(std::string indexName);

    void insertInto(std::string tableName, std::vector<Data *> data);
    Table *select(std::string tableName, std::vector<WhereQuery> wq);
    int deleteRecord(std::string tableName, std::vector<WhereQuery> wq);
};

#endif