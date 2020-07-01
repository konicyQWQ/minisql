#include "catalogManager.h"

#include <cstring>
#include <iostream>

CatalogManager::CatalogManager()
{
    FILE *tableFile = fopen("table/tbf.inf", "r");
    FILE *indexFile = fopen("table/idx.inf", "r");
    char str[64], str2[64];

    // 所有表名装载进内存
    while (tableFile && fscanf(tableFile, "%s", str) == 1)
        allTable[str] = 1;
    // 所有索引装载进内存
    while (indexFile && fscanf(indexFile, "%s%s", str, str2) == 2)
        allIndex[str] = str2;

    if (tableFile)
        fclose(tableFile);
    if (indexFile)
        fclose(indexFile);
}

CatalogManager::~CatalogManager()
{
    FILE *tableFile = fopen("table/tbf.inf", "w");
    FILE *indexFile = fopen("table/idx.inf", "w");

    // 所有表名重新写入文件
    for (auto it = allTable.begin(); it != allTable.end(); ++it)
        fprintf(tableFile, "%s\n", it->first.c_str());
    // 所有索引重新写入文件
    for (auto it = allIndex.begin(); it != allIndex.end(); ++it)
        fprintf(indexFile, "%s %s\n", it->first.c_str(), it->second.c_str());

    fclose(tableFile);
    fclose(indexFile);
}

int CatalogManager::createTable(Table &table)
{
    if (allTable.count(table.name) == 1)
        return 1; // 已经存在相同的表名了！

    for (int i = 0; i < table.attrCnt; i++)
        for (int j = 0; j < table.attrCnt; j++)
            if (i != j && table.attr[i].name == table.attr[j].name)
                return 2; // 属性名重复

    // 正常处理
    FILE *file = fopen((std::string("table/") + table.name + ".tbf").c_str(), "wb");
    char buf[5284], *p = buf;
    // 填充头部
    p[0] = table.attrCnt;
    p[1] = table.indexCnt;
    p[2] = table.primary;
    p += 3;
    // 填充属性
    for (int i = 0; i < buf[0]; i++)
    {
        Attribute &at = table.attr[i];
        memcpy(p, at.name.c_str(), at.name.length());
        memset(p + at.name.length(), 0x00, 80 - at.name.length());
        p[80] = at.type;
        p[81] = at.length;
        p[82] = at.isUnique;
        p[83] = at.notNULL;
        p += 84;
    }
    // 填充索引
    for (int i = 0; i < buf[1]; i++)
    {
        Index &id = table.index[i];
        memcpy(p, id.name.c_str(), id.name.length());
        memset(p + id.name.length(), 0x00, 80 - id.name.length());
        p[80] = id.indexNum;
        p += 81;
        this->allIndex[id.name] = table.name;
    }
    // 写入到文件，更新allTable
    fwrite(buf, sizeof(char), p - buf, file);
    this->allTable[table.name] = 1;
    fclose(file);
    return 0;
}

int CatalogManager::dropTable(std::string tableName)
{
    if (remove((std::string("table/") + tableName + ".tbf").c_str()) < 0)
        return 1; // 删除失败，表不存在

    allTable.erase(tableName);
    for (auto p = allIndex.begin(); p != allIndex.end();)
        if (p->second == tableName)
            p = allIndex.erase(p);
    return 0;
}

int CatalogManager::queryIndex(std::string indexName)
{
    return allIndex.count(indexName);
}

int CatalogManager::createIndex(std::string tableName, std::string indexName, byte indexNum)
{
    if (allTable.count(tableName) == 0)
        return 1; // 表不存在
    if (allIndex.count(indexName) == 1)
        return 2; // 索引名重复

    FILE *file = fopen((std::string("table/") + tableName + ".tbf").c_str(), "r+");
    char buf[81];
    // 索引数量+1
    fseek(file, 1L, SEEK_SET);
    fread(buf, sizeof(char), 1, file);
    buf[0]++;
    fseek(file, 1L, SEEK_SET);
    fwrite(buf, sizeof(char), 1, file);
    // 末尾新建索引
    memset(buf, 0x00, sizeof(buf));
    memcpy(buf, indexName.c_str(), indexName.length());
    buf[80] = indexNum;
    fseek(file, 0L, SEEK_END);
    fwrite(buf, sizeof(char), 81, file);

    allIndex[indexName] = tableName;
    fclose(file);
    return 0;
}

int CatalogManager::dropIndex(std::string indexName)
{
    if (allIndex.count(indexName) == 0)
        return 1;

    FILE *file = fopen((std::string("table/") + allIndex[indexName] + ".tbf").c_str(), "r");
    char buf[5284], *p = buf, *last;

    last = buf + fread(buf, sizeof(char), 5284, file);
    fclose(file);

    // 定位索引位置并删除
    p = p + 3 + buf[0] * 84;
    while ((p[79] && memcmp(p, indexName.c_str(), 80) != 0) || strcmp(p, indexName.c_str()) != 0)
        p += 81;
    if (p != last - 81)
        memcpy(p, last - 81, 81); // 如果他不是最后一个索引，用最后一个索引代替他
    buf[1]--;

    // 重新写入文件
    file = fopen((std::string("table/") + allIndex[indexName] + ".tbf").c_str(), "w");
    fwrite(buf, sizeof(char), last - buf - 81, file);
    fclose(file);
    allIndex.erase(indexName);
    return 0;
}

Table *CatalogManager::selectTable(std::string tableName)
{
    if (allTable.count(tableName) == 0)
        return nullptr; // 表不存在

    // 正常处理
    FILE *file = fopen((std::string("table/") + tableName + ".tbf").c_str(), "rb");
    Table *tb = new Table;
    char buf[5284], *p = buf;

    fread(buf, sizeof(char), 5284, file);
    // 填充头部
    tb->name = tableName;
    tb->attrCnt = p[0];
    tb->indexCnt = p[1];
    tb->primary = p[2];
    p += 3;
    // 填充属性
    for (int i = 0; i < buf[0]; i++)
    {
        Attribute &at = tb->attr[i];
        at.name = p;
        at.type = p[80];
        at.length = p[81];
        at.isUnique = p[82];
        at.notNULL = p[83];
        p += 84;
    }
    // 填充索引
    for (int i = 0; i < buf[1]; i++)
    {
        Index &id = tb->index[i];
        id.name = p;
        id.indexNum = p[80];
        p += 81;
    }

    fclose(file);
    return tb;
}

void CatalogManager::showTable(Table &table)
{
    using std::cout;
    using std::endl;

    cout << "---" << table.name << "---" << endl;

    for (int i = 0; i < table.attrCnt; i++)
    {
        cout << table.attr[i].name << " ";
        if (table.attr[i].type == 0)
            cout << "int ";
        else if (table.attr[i].type == 1)
            cout << "float ";
        else
            cout << "char(" << (int)table.attr[i].length << ") ";
        if (table.attr[i].isUnique)
            cout << "unique ";
        if (table.attr[i].notNULL)
            cout << "not NULL ";
        if (table.primary == i)
            cout << "primary key";
        cout << endl;
    }

    for (int i = 0; i < table.indexCnt; i++)
        cout << "index: " << table.index[i].name << "("
             << table.attr[table.index[i].indexNum].name << ")" << endl;
}