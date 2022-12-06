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

#define DEBUG

using string = std::string;
using complex = std::complex<double>;
using ComplexVector = std::vector<complex>;
using DoubleVector = std::vector<double>;

enum class State
{
    Idle, Running
};

class NGSpiceInterface
{
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
};


#endif //NGSPICE_TEST_NGSPICEINTERFACE_HPP
