#include "recordManager.h"
#include <string>
using std::string;

RecordManager::RecordManager(BufferManager *bm) {
    this->bufferManager = bm;
}

void RecordManager::createTable(std::string tableName) {
    FILE *fp = fopen((string("table/") + tableName + ".rdf").c_str(), "w");
    fclose(fp);
}

int RecordManager::dropTabel(std::string tableName) {
    if(remove((std::string("table/") + tableName + ".rdf").c_str()) < 0)
        return 1;   // 删除失败
    return 0;
}

int RecordManager::insert(Table *table, Tuple tuple) {
    
}

int RecordManager::del(Table *table, std::vector<WhereQuery> &wq) {

}

/* select 结果会保存在tabel中 */
int RecordManager::select(Table *table, std::vector<WhereQuery> &wq) {

}