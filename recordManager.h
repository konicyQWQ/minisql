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

    RecordManager(BufferManager *bm);

    void createTable(std::string tableName);
    int dropTabel(std::string tableName);
    int insert(Table *table, Tuple tuple);
    Table* del(Table *table, std::vector<WhereQuery> &wq);
    /* select 结果会保存在tabel中 */
    int select(Table *table, std::vector<WhereQuery> &wq);
    
    void addRecord(Table *table, int address);
    void deleteRecord(Table *table, int address);
    bool judge(Table *table, std::vector<WhereQuery> &wq, Tuple *tuple);
    Tuple* readRecord(Table *table, Block *blk, int pos);
    void choose(Table *table, std::vector<WhereQuery> &wq);
    int calcLength(Table *table);
};

#endif