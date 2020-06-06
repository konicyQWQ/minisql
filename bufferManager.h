#ifndef BUFFER_MANAGER_H
#define BUFFER_MANAGER_H

#include "header/typedef.h"
#include <cstdio>
#include <string>
#include <list>
#include <vector>
#include <map>

const int Buffer_Number = 100;

class BufferManager {
private:
    Block buf[Buffer_Number];
    std::map<std::string, std::vector<Block*> > idx;
    std::list<Block*> pool;
    int time;

    Block* newBlock();  // 返回一个新的块，如果没有，LRU策略去掉一个块

public:
    BufferManager();
    ~BufferManager();
    
    Block* getBlock(std::string filename, int offset); // 返回该文件的 offset 块
    void writeBlock(Block* blk);  // 写标记
    void pinBlock(Block *blk);    // pin标记
    void removeBlock(Block* blk); // 写回一个块到文件

};

#endif