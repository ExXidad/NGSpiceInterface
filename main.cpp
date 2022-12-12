#include "NGSpiceInterface.hpp"
#include "sharedspice.h"
#include <fstream>
#include <cmath>

using namespace std;

int testVoltage(double *vReturn, double time, char *nodeName, int id, void *user)
{
    (*vReturn) = std::sin(10.e3 * 2. * M_PI * time) * 1e-3;
    return 0;
}

int testCurrent(double *vReturn, double time, char *nodeName, int id, void *user)
{
    return 0;
}

int (testSync)(double actTime, double *delTime, double oldDelTime, int redostep, int id, int someId, void *user)
{
    (*delTime) = 0;
    return 0;
}

int main()
{
    NGSpiceInterface ngSpiceInterface;
    struct A{double a = 1;} B;
    auto tmp = [](double *vReturn, double time, char *nodeName, int id, void *user) -> int {
        (*vReturn) = std::sin(10.e3 * 2. * M_PI * time) * 1e-6;
        return 0;
    };

//    ngSpiceInterface.setExternalVoltageFunction(tmp);
    ngSpiceInterface.setExternalCurrentFunction(tmp);

//    ngSpiceInterface.include("/Users/xidad/CLionProjects/ngspice_test/rcf.cir");
//    ngSpiceInterface.include("/Users/xidad/CLionProjects/ngspice_test/ad8655.cir");
    ngSpiceInterface.include("/Users/xidad/CLionProjects/ngspice_test/tia.cir");
    ngSpiceInterface.include("/Users/xidad/CLionProjects/ngspice_test/photodiode.cir");
//    ngSpiceInterface.include("/Users/xidad/CLionProjects/ngspice_test/LM324.cir");

//    ngSpiceInterface.loadNetlistFromString("test array,V1 1 0 1,R1 1 2 1,C1 2 0 1 ic=0,.tran 10u 3 uic,.end", ",");
//    ngSpiceInterface.loadCircuitFromString("test array,V1 1 0 SIN(0 1 10k),R1 1 2 1k,C1 2 0 1u", ",");
//    ngSpiceInterface.loadCircuitFromString("test array,Vext 1 0 AC 0 EXTERNAL,R1 1 2 1k,C1 2 0 1u", ",");
    ngSpiceInterface.loadCircuitFromFile("/Users/xidad/CLionProjects/ngspice_test/testCircuit.cir");
    ngSpiceInterface.options()["temp"] = "60";
//    ngSpiceInterface.addACAnalysis("dec", 10, 1., 10e9);
    ngSpiceInterface.addTransientAnalysis(0., 1.e-3, 1.e-6);
    ngSpiceInterface.addNoiseAnalysis("3","0","ipd","dec",10,1.,100e6);

//    ngSpiceInterface.loadNetlistFromFile("/Users/xidad/CLionProjects/ngspice_test/testCircuit.cir");

    ngSpiceInterface.run();

    ngSpiceInterface.printSolutionInfo();
//    DoubleVector freq = ngSpiceInterface.getRealPlot("frequency");
//    DoubleVector mag = ngSpiceInterface.getMagPlot("V(2)");
//    DoubleVector phase = ngSpiceInterface.getPhasePlot("V(2)");


    DoubleVector time = ngSpiceInterface.getRealPlot("tran1.time");
    std::vector<DoubleVector> volts{
            ngSpiceInterface.getRealPlot("tran1.V(1)"),
            ngSpiceInterface.getRealPlot("tran1.V(3)")
    };


    std::ofstream file("../test.txt");
    for (int i = 0; i < time.size(); ++i)
    {
        file << time[i] << "\t";
        for (int j = 0; j < volts.size(); ++j)
        {
            file << volts[j][i] << "\t";
        }
        file << std::endl;
    }
    file.close();


    DoubleVector freq = ngSpiceInterface.getRealPlot("noise1.frequency");
    std::vector<DoubleVector> noise{
            ngSpiceInterface.getRealPlot("noise1.inoise_spectrum"),
            ngSpiceInterface.getRealPlot("noise1.onoise_spectrum")
    };


    file.open("../noise.txt");
    for (int i = 0; i < freq.size(); ++i)
    {
        file << time[i] << "\t";
        for (int j = 0; j < noise.size(); ++j)
        {
            file << noise[j][i] << "\t";
        }
        file << std::endl;
    }
    file.close();

    std::cout << "Total noise: " << std::endl;
    std::cout << "In noise: " <<ngSpiceInterface.getRealPlot("noise2.inoise_total")[0] << std::endl;
    std::cout << "Out noise: " <<ngSpiceInterface.getRealPlot("noise2.onoise_total")[0] << std::endl;
    std::cout << std::endl;






    return 0;
}