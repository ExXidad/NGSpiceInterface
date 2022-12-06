#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "NGSpiceInterface.hpp"
#include "sharedspice.h"

using namespace std;

int main()
{
    NGSpiceInterface ngSpiceInterface;

//    ngSpiceInterface.loadNetlistFromString("test array,V1 1 0 1,R1 1 2 1,C1 2 0 1 ic=0,.tran 10u 3 uic,.end", ",");

    ngSpiceInterface.loadNetlistFromFile("/Users/xidad/CLionProjects/ngspice_test/testArray.cir");

    ngSpiceInterface.run();

//    std::cout << ngSpice_AllVecs("time") << std::endl;

    for (const auto x:ngSpiceInterface.getPlot("V(2)"))
    {
        std::cout <<x << std::endl;
    }

    return 0;
}