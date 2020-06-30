#include <iostream>
#include "Interpreter.h"
#include "header/typedef.h"
#include "indexManager.h"
#include "bufferManager.h"

using namespace std;

int main()
{
    cout << "Welcome to MiniSQL" << endl;

    Interpreter i;
    
    int ret;
    do
    {
        string q;
        string tmp;
        do
        {
            cout << "> ";
            getline(cin, tmp);
            //tmp.erase(0, 1);//there is a space at the beginning of tmp
            q.append(" " + tmp);
        } while (tmp.back() != ';');
        i.setQuery(q);
        try
        { ret = i.runQuery(); }
        catch (exception &e)
        {
            cout << "MiniSQL: " << e.what() << endl;
        }
        // =============== 新增 ==============
        q.clear();
        // ==================================
    } while (ret);
}
