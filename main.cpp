#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "NGSpiceInterface.hpp"
#include "sharedspice.h"
#include <fstream>

using namespace std;

int main()
{
    NGSpiceInterface ngSpiceInterface;

//    ngSpiceInterface.loadNetlistFromString("test array,V1 1 0 1,R1 1 2 1,C1 2 0 1 ic=0,.tran 10u 3 uic,.end", ",");
    ngSpiceInterface.loadCircuitFromString("test array,V1 0 1 dc 0 ac 1,R1 1 2 1k,C1 2 0 1u", ",");
    ngSpiceInterface.options()["temp"] = "60";
    ngSpiceInterface.addACAnalysis("dec", 10, 1., 10e9);
    ngSpiceInterface.addACAnalysis("dec", 10, 10., 10e9);

//    ngSpiceInterface.loadNetlistFromFile("/Users/xidad/CLionProjects/ngspice_test/testArray.cir");

    ngSpiceInterface.run();

    ngSpiceInterface.printSolutionInfo();

    DoubleVector freq = ngSpiceInterface.getRealPlot("frequency");
    DoubleVector mag = ngSpiceInterface.getMagPlot("V(2)");
    DoubleVector phase = ngSpiceInterface.getPhasePlot("V(2)");
    std::ofstream file("../test.txt");


    for (int i = 0; i < freq.size(); ++i)
        file << freq[i] << "\t" << mag[i] << "\t" << phase[i] << std::endl;


    file.close();
    ngSpiceInterface.quit();
    return 0;
}