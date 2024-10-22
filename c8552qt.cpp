#include <iostream>
#include <vector>
#include <string>
#include <stack>
#include <math.h>
#include "quadtree.h"

using namespace std;


int main()
{
    cout << "***** Quadtree" << endl;
    
    QuadtreeTest qt;
    qt.useQuadTree = true;
    qt.Run();
    
    QuadtreeTest qt2;
    qt2.useQuadTree = false;
    qt2.Run();

    return 0;
}
