#include "recordManager.h"

RecordManager::RecordManager(BufferManager *bm) {
    this->bufferManager = bm;
}

void RecordManager::createTable(std::string tableName) {
    
}

void RecordManager::dropTabel(std::string tableName) {

}

int RecordManager::insert(Table *table, Tuple tuple) {

}

int RecordManager::del(Table *table, std::vector<WhereQuery> &wq) {

}

/* select 结果会保存在tabel中 */
int RecordManager::select(Table *table, std::vector<WhereQuery> &wq) {

}