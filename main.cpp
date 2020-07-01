#include <iostream>
#include "Interpreter.h"
#include "header/typedef.h"
#include "indexManager.h"
#include "bufferManager.h"
#include <time.h>

using namespace std;

#define TIME_MEASURE

int main()
{
#ifdef TIME_MEASURE
    ios::sync_with_stdio(false);
#endif
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
#ifdef TIME_MEASURE
        clock_t start, finish;
        start = clock();
#endif
        try
        { 
            ret = i.runQuery();
        }
        catch (exception &e)
        {
            cout << "MiniSQL: " << e.what() << endl;
        }
#ifdef TIME_MEASURE
        finish = clock();
        cout << (double)(finish-start) / CLOCKS_PER_SEC << " seconds" << endl;
#endif
        q.clear();
    } while (ret);
}
