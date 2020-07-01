#include "header/typedef.h"
#include "bufferManager.h"

#include <cstring>
#include <iostream>

BufferManager::BufferManager() {
    time = 0;
    for(int i=0; i<Buffer_Number; i++) {
        memset(buf[i].buf, 0, sizeof(buf[i].buf));
        buf[i].offset = 0;
        buf[i].pin = 0;
        buf[i].time = 0;
        buf[i].validChar = 0;
        buf[i].writeMark = 0;
        pool.push_back(&buf[i]);
    }
}

BufferManager::~BufferManager() {
    for(int i=0; i<Buffer_Number; i++)
        if(buf[i].time)
            removeBlock(buf + i);
}

Block* BufferManager::newBlock() {
    if(pool.empty()) {
        // 如果一块空间也没了
        Block *early = nullptr;
        for(int i=0; i<Buffer_Number; i++)
            if(!early || (buf[i].pin == 0 && early->time > buf[i].time))
                early = buf+i;
        removeBlock(early);
    }
    Block *ptr = pool.front();
    pool.pop_front();
    ptr->time = ++time;
    return ptr;
}

Block* BufferManager::getBlock(std::string filename, int offset) {
    if(idx.count(filename)) {
        std::vector<Block*> &vc = idx[filename];
        for(int i=0; i<vc.size(); i++)
            if(vc[i]->offset == offset)
                return vc[i];
    }
    Block* blk = newBlock();
    FILE *fp = fopen(filename.c_str(), "r");

    fseek(fp, offset * 4096, SEEK_SET);
    int p = fread(blk->buf, sizeof(char), 4096, fp);
    memset(blk->buf + p, 0x00, 4096 - p);
    fclose(fp);
    blk->validChar = p;
    blk->pin = 0;
    blk->offset = offset;
    blk->writeMark = 0;
    blk->filename = filename;

    if(idx.count(filename) == 0)
        idx[filename] = std::vector<Block*>();
    idx[filename].push_back(blk);
    return blk;
}

void BufferManager::writeBlock(Block* blk) {
    blk->writeMark = 1;
    blk->validChar = 4096;
}

void BufferManager::pinBlock(Block *blk) {
    blk->pin = 1;
}

void BufferManager::removeBlock(Block* blk) {
    if(blk->writeMark) {
        FILE *fp = fopen(blk->filename.c_str(), "r+");
        fseek(fp, blk->offset * 4096, SEEK_SET);
        fwrite(blk->buf, sizeof(char), 4096, fp);
        fclose(fp);
    }
    if(idx.count(blk->filename)) {
        std::vector<Block*> &vc = idx[blk->filename];
        for(int i=0; i<vc.size(); i++)
            if(vc[i]->offset == blk->offset) {
                vc.erase(vc.begin() + i);
                break;
            }
    }
    blk->time = 0;
    pool.push_back(blk);
}

void BufferManager::discardBlock(std::string filename) {
    if(idx.count(filename)) {
        std::vector<Block*> &vc = idx[filename];
        for(int i=0; i<vc.size(); i++) {
            vc[i]->time = 0;
            pool.push_back(vc[i]);
        }
        idx.erase(idx.find(filename));
    }
}