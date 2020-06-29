//
// Created by yMac on 2020/6/28.
//

#ifndef BPT_INDEXMANAGER_H
#define BPT_INDEXMANAGER_H

#include "header/typedef.h"
#include "tree.h"

class IndexManager
{
public:
    /*
     * IndexManager�����������ж�
     * ����CatalogManagerģ����ɵ�����
     * ���������Ǽ���һ�в���ȫ���Ϸ�
     */

    void createIndex(Table &t, int indexNo); //���������������������ã����������

    void deleteIndex(Table& t, int indexNo); //ɾ�������������������ã����������

    void insert(string indexName, Data data, int offset); //�����¼����������������������ƫ����

    void eliminate(string indexName, Data data); //ɾ����¼��������������������

    int search(string indexName, Data data); //��ѯ��¼��������������������

    std::vector<int> rangeSearch(string indexName, Data inf, Data sup); //��Χ��ѯ��¼���������������������½磬�����Ͻ�

};

#endif //BPT_INDEXMANAGER_H
