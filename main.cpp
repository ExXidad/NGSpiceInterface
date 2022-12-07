#include "NGSpiceInterface.hpp"
#include "sharedspice.h"
#include <fstream>
#include <cmath>

using namespace std;

int testVoltage(double *vReturn, double time, char *nodeName, int id, void *user)
{
    (*vReturn) = 2. * std::sin(20.e3 * 2. * M_PI * time);
    return 0;
}

int main()
{
    NGSpiceInterface ngSpiceInterface;

//    ngSpiceInterface.loadNetlistFromString("test array,V1 1 0 1,R1 1 2 1,C1 2 0 1 ic=0,.tran 10u 3 uic,.end", ",");
    ngSpiceInterface.loadCircuitFromString("test array,V1 1 0 SIN(0 1 10k),R1 1 2 1k,C1 2 0 1u", ",");
    ngSpiceInterface.options()["temp"] = "60";
    ngSpiceInterface.addACAnalysis("dec", 10, 1., 10e9);
    ngSpiceInterface.addTransientAnalysis(0., 1.e-3, 1.e-6);

//    ngSpiceInterface.loadNetlistFromFile("/Users/xidad/CLionProjects/ngspice_test/testArray.cir");

    ngSpiceInterface.run();

    ngSpiceInterface.printSolutionInfo();

//    DoubleVector freq = ngSpiceInterface.getRealPlot("frequency");
//    DoubleVector mag = ngSpiceInterface.getMagPlot("V(2)");
//    DoubleVector phase = ngSpiceInterface.getPhasePlot("V(2)");

    DoubleVector time = ngSpiceInterface.getRealPlot("tran1.time");
    DoubleVector volt = ngSpiceInterface.getRealPlot("tran1.V(2)");

    std::ofstream file("../test.txt");


//    for (int i = 0; i < freq.size(); ++i)
//        file << freq[i] << "\t" << mag[i] << "\t" << phase[i] << std::endl;
    for (int i = 0; i < time.size(); ++i)
        file << time[i] << "\t" << volt[i] << std::endl;

    file.close();
    return 0;
}