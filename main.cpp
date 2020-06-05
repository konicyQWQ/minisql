#include <iostream>
#include "header/typedef.h"
#include "catalogManager.h"
using namespace std;

int main() {
    CatalogManager *cm = new CatalogManager;

    delete cm;
    return 0;
}