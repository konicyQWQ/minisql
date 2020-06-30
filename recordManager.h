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

    /* 创建表与删除表 */
    void createTable(std::string tableName);
    int dropTabel(std::string tableName);
    /* insert 语句，新加一个元组 */
    int insert(Table *table, Tuple tuple);
    /* delete 语句，被删除的记录会存在 table 的元组中 */
    Table* del(Table *table, std::vector<WhereQuery> &wq);
    /* select 结果会保存在 tabel 中 */
    int select(Table *table, std::vector<WhereQuery> &wq);
    

    // 都是内部使用的函数，但是为了方便，也直接放在public中
    void addRecord(Table *table, int address);
    void deleteRecord(Table *table, int address);
    bool judge(Table *table, std::vector<WhereQuery> &wq, Tuple *tuple);
    Tuple* readRecord(Table *table, Block *blk, int pos);
    void choose(Table *table, std::vector<WhereQuery> &wq);
    int calcLength(Table *table);
};

#endif