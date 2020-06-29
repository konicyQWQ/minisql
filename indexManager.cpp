//
// Created by yMac on 2020/6/28.
//

#include "indexManager.h"
//
// Created by sky on 2020/6/6.
//

#include <fstream>
#include <utility>
#include <iostream>

using namespace std;

void IndexManager::createIndex(Table &t, int indexNo) {
    BPTree(string("table/") + t.index[indexNo].name + ".idx");
}

void IndexManager::deleteIndex(string indexName){
    remove((string("table/") + indexName + ".idx").c_str());
}

void IndexManager::insert(string indexName, Data* data, int offset){
    BPTree t(string("table/") + indexName + ".idx");
    if (t.isEmpty())
        t.initialize(data, offset, data->type);
    else
        t.insert(data, offset);
}

void IndexManager::eliminate(string indexName, Data* data){
    BPTree t(string("table/") + indexName + ".idx");
    if(t.isEmpty())
        throw exception();
    else
        t.remove(data);
}

int IndexManager::search(string indexName, Data* data){
    BPTree t(string("table/") + indexName + ".idx");
    return t.isEmpty()? -1: t.find(data);
}

std::vector<int> IndexManager::rangeSearch(string indexName, Data* inf, Data* sup){
    BPTree t(string("table/") + indexName + ".idx");
    std::vector<int> zero;
    return t.isEmpty()? zero: t.rangeFind(inf, sup);
}
