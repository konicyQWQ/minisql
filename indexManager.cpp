#include "indexManager.h"
//
// Created by sky on 2020/6/6.
//

#include "indexManager.h"
#include <fstream>
#include <utility>

using namespace std;

void IndexManager::createIndex(Table &t, int indexNo) {
    BPTree(t.index[indexNo].name);
}

void IndexManager::deleteIndex(Table& t, int indexNo){
    remove(t.index[indexNo].name.c_str());
}

void IndexManager::insert(string indexName, Data data, int offset){
    BPTree t(indexName);
    if (t.isEmpty())
        t.initialize(&data, offset, data.type);
    else
        t.insert(&data, offset);
}

void IndexManager::eliminate(string indexName, Data data){
    BPTree t(indexName);
    if(t.isEmpty())
        throw exception();
    else
        t.remove(&data);
}

int IndexManager::search(string indexName, Data data){
    BPTree t(indexName);
    return t.isEmpty()? -1: t.find(&data);
}

std::vector<int> IndexManager::rangeSearch(string indexName, Data inf, Data sup){
    BPTree t(indexName);
    std::vector<int> zero;
    return t.isEmpty()? zero: t.rangeFind(&inf, &sup);
}
