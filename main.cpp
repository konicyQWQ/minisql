#include <iostream>
#include "Interpreter.h"
#include "catalogManager.h"
#include "bufferManager.h"
#include "recordManager.h"
#include "header/typedef.h"

using namespace std;

int main()
{
    cout << "Welcome to MiniSQL" << endl;

    Interpreter i;
    auto *cm = new CatalogManager;
    auto *bm = new BufferManager;
    auto *rm = new RecordManager(bm);
    
    int ret;
    do
    {
        string q;
        string tmp;
        do
        {
            cout << "> ";
            getline(cin, tmp);
            tmp.erase(0, 1);//there is a space at the beginning of tmp
            q.append(" " + tmp);
        } while (tmp.back() != ';');
        i.setQuery(q);
        try
        { ret = i.runQuery(); }
        catch (exception &e)
        {
            cout << "MiniSQL: " << e.what();
        }
    } while (ret);

    delete cm;
    delete rm;
    delete bm;
}
