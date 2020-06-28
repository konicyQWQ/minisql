#ifndef RECORD_MANAGER_H
#define RECORD_MANAGER_H

#include <string>
#include <vector>
#include "header/typedef.h"
#include "bufferManager.h"

class RecordManager {
private:
    BufferManager *bufferManager;
public:

    std::string errorMessage;

    RecordManager(BufferManager *bm);

    void createTable(std::string tableName);
    int dropTabel(std::string tableName);
    int insert(Table *table, Tuple tuple);
    int del(Table *table, std::vector<WhereQuery> &wq);
    /* select 结果会保存在tabel中 */
    int select(Table *table, std::vector<WhereQuery> &wq);
    
};

#endif