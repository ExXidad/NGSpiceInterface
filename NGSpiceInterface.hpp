//
// Created by Ivan Kalesnikau on 04.12.2022.
//

/*
 * Part of my ngspice interface was taken from KiCad files and
 * modified for my needs, its license text is presented below
 * changes (shortly):
 * modified so that .dll is no more, only .so
 * added my own netlist load options
 * added my bg process operation functions
 * added DEBUG to disable cout
*/

/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2016 CERN
 * @author Tomasz Wlostowski <tomasz.wlostowski@cern.ch>
 * @author Maciej Suminski <maciej.suminski@cern.ch>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * https://www.gnu.org/licenses/gpl-3.0.html
 * or you may search the http://www.gnu.org website for the version 3 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#ifndef NGSPICE_TEST_NGSPICEINTERFACE_HPP
#define NGSPICE_TEST_NGSPICEINTERFACE_HPP

#include <iostream>
#include <string>
#include "sharedspice.h"
#include <vector>
#include <complex>
#include <fstream>
#include <algorithm>
#include <map>

#define DEBUG

using string = std::string;
using complex = std::complex<double>;
using ComplexVector = std::vector<complex>;
using DoubleVector = std::vector<double>;
using StringVector = std::vector<string>;
using Options = std::map<string, string>;
using Analyses = StringVector;

class NGSpiceInterface
{
private:
    enum class Uploaded
    {
        Circuit, Netlist, None
    } _uploaded = Uploaded::None;
    string _circuit, _delimiter;
    Options _options;
    Analyses _analyses;
    StringVector _raw;

public:
    NGSpiceInterface();
    ~NGSpiceInterface() = default;

    int init();

public:
    // Netlists
    void loadNetlistFromFile(const string &fname);
    void loadNetlistLineByLine(const string &line);
    void loadNetlistFromString(const string &netlist, const string delimiter = "\n");

public:
    // Circuits
    void loadCircuitFromFile(const string &fname);
    void loadCircuitFromString(const string &circuit, const string delimiter = "\n");

public:
    // Analyses
    [[nodiscard]] string
    DCAnalysis(const string &sourceName, const double vStart, const double vStop, const double vStep,
               const string &addon = "") const;
    void addDCAnalysis(const string &sourceName, const double vStart, const double vStop, const double vStep,
                       const string &addon = "");


    // variation: lin, dec, oct
    [[nodiscard]] string
    ACAnalysis(const string &variation, const uint nPtsPerVariation, const double fStart, const double fStop,
               const string &addon = "") const;
    void addACAnalysis(const string &variation, const uint nPtsPerVariation, const double fStart, const double fStop,
                       const string &addon = "");

public:
    Options &options();
    [[nodiscard]] Options options() const;

    Analyses &analyses();
    [[nodiscard]] Analyses analyses() const;

    StringVector &raw();
    [[nodiscard]] StringVector raw() const;

public:
    // Commands
    void sendCommand(const string &command);
    bool isRunning();
    void bgStop();
    void bgRun();
    void bgResume();
    void run();
    void quit();
    void clear();

public:
    // Plots
    ComplexVector getPlot(const string &name, int maxLen = -1);
    DoubleVector getRealPlot(const string &name, int maxLen = -1);
    DoubleVector getImagPlot(const string &name, int maxLen = -1);
    DoubleVector getMagPlot(const string &name, int maxLen = -1);
    DoubleVector getPhasePlot(const string &name, int maxLen = -1);

public:
    // Callbacks
    static int receiveChar(char *what, int id, void *user);

    static int receiveStatus(char *what, int id, void *user);

    static int ngExit(int status, bool immediate, bool exit_upon_quit, int id, void *user);

    static int ngRunning(bool is_running, int id, void *user);

public:
    // Utils
    StringVector currentPlotVectors();
    StringVector allPlots();
    StringVector plotVectors(const string &plotName);
    void printSolutionInfo();

private:
    // Utils
    [[nodiscard]] string readFileToString(const string &fname);
    [[nodiscard]] uint countElements(char **arr);
};


#endif //NGSPICE_TEST_NGSPICEINTERFACE_HPP
