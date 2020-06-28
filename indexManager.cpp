//
// Created by sky on 2020/6/6.
//

#include "indexManager.h"
#include <fstream>
#include <utility>

using namespace std;

IndexManager::IndexManager(Table &t, const Attribute& onAttr) : t(t), onAttr(onAttr)
{
    fileName = t.name + "_" + onAttr.name + ".inf";
}

int IndexManager::createIndex(std::string indexName)
{
    int i;
    //找到名字对应着第几个属性
    for (i = 0; i < t.attrCnt; ++i)
        if (t.attr[i].name == onAttr.name)
            break;

    Index idx;
    idx.name = std::move(indexName);
    idx.indexNum = i;
    t.index[t.indexCnt++] = idx;

    /*
     * 建立Bptree，传入t和onAttr
     * 然后按如下格式创建文件fileName
     * 第一个Byte：M
     * 第2~81个byte：表名
     * 第82~161个byte：属性名
     * 第162个byte：属性在表中是第几个属性
     * 第163个byte开始用memcpy直接把Bptree给copy进去
     */
    return 0;
}

void IndexManager::deleteIndex()
{
    remove(fileName.c_str());
}

int IndexManager::getOffset(Data *d)
{
    /*
     * if (d.type==0)
     *      Bptree<int> bp(fileName);
     *      return bp.search(((iData *)d).value)
     * else if (d.type==1)
     *      blablabla
     */
}

std::vector<int> IndexManager::getInterval(Data *d1, Data *d2)
{
    /*
     * if (d1.type==0)
     *      Bptree<int> bp(fileName)
     *
     */
}
