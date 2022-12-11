#include "NGSpiceInterface.hpp"

int NGSpiceInterface::receiveChar(char *what, int id, void *user)
{
    NGSpiceInterface *interfaceClass = reinterpret_cast<NGSpiceInterface *>(user);

#ifdef DEBUG
    // strip stdout/stderr from the line
    if ((strncasecmp(what, "stdout ", 7) == 0)
        || (strncasecmp(what, "stderr ", 7) == 0))
        what += 7;
    std::cout << what << std::endl;
#endif

    return 0;
}

int NGSpiceInterface::receiveStatus(char *what, int id, void *user)
{
    NGSpiceInterface *interfaceClass = reinterpret_cast<NGSpiceInterface *>(user);

#ifdef DEBUG
    std::cout << what << std::endl;
#endif

    return 0;
}

int NGSpiceInterface::ngExit(int status, bool immediate, bool exit_upon_quit, int id, void *user)
{
#ifdef DEBUG
    std::cout << "quit: " << status << std::endl;
    std::cout << "stat " << status << "; immed " << !!immediate << "; quit " << !!exit_upon_quit << std::endl;
#endif
    return 0;
}

int NGSpiceInterface::ngRunning(bool is_running, int id, void *user)
{
    NGSpiceInterface *interfaceClass = reinterpret_cast<NGSpiceInterface *>(user);
//
//    if( sim->m_reporter )
//        // I know the test below seems like an error, but well, it works somehow..
//        sim->m_reporter->OnSimStateChange( sim, is_running ? SIM_IDLE : SIM_RUNNING );

    std::cout << "Running: " << is_running << std::endl;


    return 0;
}

NGSpiceInterface::NGSpiceInterface()
{
    int status = init();

#ifdef DEBUG
    std::cout << "Initialized NGSpice with code " << status << std::endl;
#endif
}

int NGSpiceInterface::init()
{
    return ngSpice_Init(&receiveChar, &receiveStatus, &ngExit,
                        NULL, NULL, &ngRunning, this);;
}

void NGSpiceInterface::loadNetlistFromFile(const string &fname)
{
    _uploaded = Uploaded::Netlist;
    ngSpice_Command((char *) ("source " + fname).c_str());
}

void NGSpiceInterface::loadNetlistLineByLine(const string &line)
{
    _uploaded = Uploaded::Netlist;
    ngSpice_Command((char *) ("circbyline " + line).c_str());
}

void NGSpiceInterface::loadNetlistFromString(const string &netlist, const string delimiter)
{
    _uploaded = Uploaded::Netlist;
    // Parse
    auto start = 0U;
    auto end = netlist.find(delimiter);
    std::vector<string> netlistLines{};
    while (end != std::string::npos)
    {
        netlistLines.emplace_back(netlist.substr(start, end - start));
        start = end + delimiter.length();
        end = netlist.find(delimiter, start);
    }
    netlistLines.emplace_back(netlist.substr(start, end - start));

    // Convert
    // TODO: сделать delete этого массива
    char **circArray = new char *[netlistLines.size() + 1];
    for (int i = 0; i < netlistLines.size(); ++i)
        circArray[i] = strdup((char *) netlistLines[i].c_str());
    circArray[netlistLines.size() + 1] = NULL;

    // Upload
    ngSpice_Circ(circArray);
}

void NGSpiceInterface::sendCommand(const string &command)
{
    ngSpice_Command((char *) command.c_str());
}

bool NGSpiceInterface::isRunning()
{
    return ngSpice_running();
}

void NGSpiceInterface::bgStop()
{
    sendCommand("bg_halt");
}

void NGSpiceInterface::bgRun()
{
    sendCommand("bg_run");
}

void NGSpiceInterface::run()
{
    switch (_uploaded)
    {
        case Uploaded::None:
        {
            throw std::runtime_error("Please specify a netlist!");
            break;
        }
        case Uploaded::Netlist:
        {
            sendCommand("run");
            break;
        }
        case Uploaded::Circuit:
        {
            string rawString = "";
            if (!_raw.empty())
                for (const string &raw_el:_raw)
                    rawString += raw_el + _delimiter;

            string optionsString = "";
            if (!_options.empty())
            {
                optionsString = ".options";
                for (const auto &[option, value]:_options)
                    optionsString += " " + option + "=" + value;
            }

            string parametersString = "";
            if (!_parameters.empty())
            {
                parametersString = ".param";
                for (const auto &[param, value]:_options)
                    parametersString += " " + param + "=" + value;
            }

            string analysisString = "";
            if (!_analyses.empty())
                for (const string &analysis:_analyses)
                    analysisString += analysis + _delimiter;

            string includesString = "";
            if (!_includes.empty())
                for (const string &include:_includes)
                    includesString += ".include " + include + _delimiter;

            string libsString = "";
            if (!_libs.empty())
                for (const string &lib:_libs)
                    includesString += ".lib " + lib + _delimiter;

            string netlist = (""
                              + _circuit + _delimiter
                              + includesString + _delimiter
                              + rawString + _delimiter
                              + optionsString + _delimiter
                              + parametersString + _delimiter
                              + analysisString + _delimiter
                              + ".end"
            );
            loadNetlistFromString(netlist, _delimiter);
            _uploaded = Uploaded::Circuit;

            sendCommand("run");
            break;
        }
    }

}

void NGSpiceInterface::quit()
{
    sendCommand("quit");
}

void NGSpiceInterface::bgResume()
{
    sendCommand("bg_resume");
}

void NGSpiceInterface::clear()
{
    sendCommand("destroy all");
}

ComplexVector NGSpiceInterface::getPlot(const string &name, int maxLen)
{
    ComplexVector data;
    vector_info *vi = ngGet_Vec_Info((char *) name.c_str());
    if (vi)
    {
        int length = maxLen < 0 ? vi->v_length : std::min(maxLen, vi->v_length);
        data.reserve(length);

        if (vi->v_realdata)
        {
            for (int i = 0; i < length; i++)
                data.push_back(complex(vi->v_realdata[i], 0.0));
        } else if (vi->v_compdata)
        {
            for (int i = 0; i < length; i++)
                data.push_back(complex(vi->v_compdata[i].cx_real, vi->v_compdata[i].cx_imag));
        }
    }

    return data;
}

DoubleVector NGSpiceInterface::getRealPlot(const string &name, int maxLen)
{
    DoubleVector data;
    vector_info *vi = ngGet_Vec_Info((char *) name.c_str());

    if (vi)
    {
        int length = maxLen < 0 ? vi->v_length : std::min(maxLen, vi->v_length);
        data.reserve(length);

        if (vi->v_realdata)
        {
            for (int i = 0; i < length; i++)
            {
                data.push_back(vi->v_realdata[i]);
            }
        } else if (vi->v_compdata)
        {
            for (int i = 0; i < length; i++)
            {
                assert(vi->v_compdata[i].cx_imag == 0.0);
                data.push_back(vi->v_compdata[i].cx_real);
            }
        }
    }

    return data;
}

DoubleVector NGSpiceInterface::getImagPlot(const string &name, int maxLen)
{
    DoubleVector data;
    vector_info *vi = ngGet_Vec_Info((char *) name.c_str());

    if (vi)
    {
        int length = maxLen < 0 ? vi->v_length : std::min(maxLen, vi->v_length);
        data.reserve(length);

        if (vi->v_compdata)
        {
            for (int i = 0; i < length; i++)
            {
                data.push_back(vi->v_compdata[i].cx_imag);
            }
        }
    }

    return data;
}

DoubleVector NGSpiceInterface::getMagPlot(const string &name, int maxLen)
{
    DoubleVector data;
    vector_info *vi = ngGet_Vec_Info((char *) name.c_str());

    if (vi)
    {
        int length = maxLen < 0 ? vi->v_length : std::min(maxLen, vi->v_length);
        data.reserve(length);

        if (vi->v_realdata)
        {
            for (int i = 0; i < length; i++)
                data.push_back(vi->v_realdata[i]);
        } else if (vi->v_compdata)
        {
            for (int i = 0; i < length; i++)
                data.push_back(hypot(vi->v_compdata[i].cx_real, vi->v_compdata[i].cx_imag));
        }
    }

    return data;
}

DoubleVector NGSpiceInterface::getPhasePlot(const string &name, int maxLen)
{
    DoubleVector data;
    vector_info *vi = ngGet_Vec_Info((char *) name.c_str());

    if (vi)
    {
        int length = maxLen < 0 ? vi->v_length : std::min(maxLen, vi->v_length);
        data.reserve(length);

        if (vi->v_realdata)
        {
            for (int i = 0; i < length; i++)
                data.push_back(0.0);      // well, that's life
        } else if (vi->v_compdata)
        {
            for (int i = 0; i < length; i++)
                data.push_back(atan2(vi->v_compdata[i].cx_imag, vi->v_compdata[i].cx_real));
        }
    }

    return data;
}

string NGSpiceInterface::readFileToString(const string &fname)
{
    std::ifstream file(fname);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void NGSpiceInterface::loadCircuitFromFile(const string &fname)
{
    _uploaded = Uploaded::Circuit;
    _delimiter = "\n";
    _circuit = readFileToString(fname);
}

void NGSpiceInterface::loadCircuitFromString(const string &circuit, const string delimiter)
{
    _uploaded = Uploaded::Circuit;
    _circuit = circuit;
    _delimiter = delimiter;
}

Options &NGSpiceInterface::options()
{
    return _options;
}

Options NGSpiceInterface::options() const
{
    return _options;
}

StringVector &NGSpiceInterface::analyses()
{
    return _analyses;
}

StringVector NGSpiceInterface::analyses() const
{
    return _analyses;
}

StringVector &NGSpiceInterface::raw()
{
    return _raw;
}

StringVector NGSpiceInterface::raw() const
{
    return _raw;
}

string NGSpiceInterface::ACAnalysis(const string &variation, const uint nPtsPerVariation, const double fStart,
                                    const double fStop,
                                    const string &addon) const
{
    return (
            ".ac "
            + variation + " "
            + std::to_string(nPtsPerVariation) + " "
            + std::to_string(fStart) + " "
            + std::to_string(fStop) + " "
            + addon
    );
}

void NGSpiceInterface::addACAnalysis(const string &variation, const uint nPtsPerVariation, const double fStart,
                                     const double fStop,
                                     const string &addon)
{
    addAnalysis(ACAnalysis(variation, nPtsPerVariation, fStart, fStop, addon));
}

StringVector NGSpiceInterface::currentPlotVectors()
{
    return plotVectors(ngSpice_CurPlot());
}

uint NGSpiceInterface::countElements(char **arr)
{
    uint n = 0;
    while (arr[n] != nullptr)
        ++n;
    return n;
}

StringVector NGSpiceInterface::allPlots()
{
    char **tmp = ngSpice_AllPlots();
    const uint n = countElements(tmp);

    StringVector vectors(n);
    for (int i = 0; i < n; ++i)
        vectors[i] = tmp[i];

    return vectors;
}

StringVector NGSpiceInterface::plotVectors(const string &plotName)
{
    char **tmp = ngSpice_AllVecs((char *) plotName.c_str());
    const uint n = countElements(tmp);

    StringVector vectors(n);
    for (int i = 0; i < n; ++i)
        vectors[i] = tmp[i];

    return vectors;
}

void NGSpiceInterface::printSolutionInfo()
{
    std::cout << "############### Solution info ###############" << std::endl;

    for (const string &plotName: allPlots())
    {
        std::cout << "Plot name: " << plotName << std::endl;
        std::cout << "Vectors: ";
        for (const string &vecName: plotVectors(plotName))
            std::cout << vecName << ", ";
        std::cout << std::endl;
    }

    std::cout << "#############################################" << std::endl;
}

string NGSpiceInterface::DCAnalysis(const string &sourceName, const double vStart, const double vStop,
                                    const double vStep, const string &addon) const
{
    return (
            ".dc "
            + sourceName + " "
            + std::to_string(vStart) + " "
            + std::to_string(vStop) + " "
            + std::to_string(vStep)
    );
}

void
NGSpiceInterface::addDCAnalysis(const string &sourceName, const double vStart, const double vStop, const double vStep,
                                const string &addon)
{
    addAnalysis(DCAnalysis(sourceName, vStart, vStop, vStep, addon));
}

void NGSpiceInterface::addAnalysis(const string &analysis)
{
    _analyses.emplace_back(analysis);
}

string NGSpiceInterface::noiseAnalysis(const string &output, const string &ref,
                                       const string &source,
                                       const string &variation, const uint pts, const double fStart, const double fStop,
                                       const uint pointsPerSummary,
                                       const string &addon) const
{
    return (
            ".noise v(" + output + "," + ref + ") "
            + source + " "
            + variation + " "
            + std::to_string(pts) + " "
            + std::to_string(fStart) + " "
            + std::to_string(fStop) + " "
            + std::to_string(pointsPerSummary) + " "
            + addon
    );
}

void NGSpiceInterface::addNoiseAnalysis(const string &output, const string &ref, const string &source,
                                        const string &variation, const uint pts, const double fStart,
                                        const double fStop, const uint pointsPerSummary, const string &addon)
{
    addAnalysis(noiseAnalysis(output, ref, source, variation, pts, fStart, fStop, pointsPerSummary, addon));
}

string NGSpiceInterface::OPAnalysis(const string &addon)
{
    return ".op";
}

void NGSpiceInterface::addOPAnalysis(const string &addon)
{
    addAnalysis(OPAnalysis(addon));
}

StringVector NGSpiceInterface::parameters() const
{
    return _parameters;
}

StringVector &NGSpiceInterface::parameters()
{
    return _parameters;
}

string NGSpiceInterface::transientAnalysis(const double tStart, const double tStop, const double tStep,
                                           const string &addon) const
{
    return (
            ".tran "
            + std::to_string(tStep) + " "
            + std::to_string(tStop) + " "
            + std::to_string(tStart) + " "
            + addon
    );
}

void
NGSpiceInterface::addTransientAnalysis(const double tStart, const double tStop, const double tStep, const string &addon)
{
    addAnalysis(transientAnalysis(tStart, tStop, tStep, addon));
}

NGSpiceInterface::~NGSpiceInterface()
{
    quit();
}

void NGSpiceInterface::include(const string &path)
{
    _includes.emplace_back(path);
}

void NGSpiceInterface::includeLibrary(const string &path)
{
    _libs.emplace_back(path);
}

int NGSpiceInterface::setExternalVoltageFunction(
        int (*voltageFunction)(double *vReturn, double time, char *nodeName, int id, void *user)
)
{
    return ngSpice_Init_Sync(voltageFunction, NULL, NULL, NULL, NULL);
}

int NGSpiceInterface::setExternalCurrentFunction(
        int (*currentFunction)(double *iReturn, double time, char *nodeName, int id, void *user)
)
{
    return ngSpice_Init_Sync(NULL, currentFunction, NULL, NULL, NULL);
}

int
NGSpiceInterface::setExternalVoltageAndCurrentFunctions(
        int (*voltageFunction)(double *vReturn, double time, char *nodeName, int id, void *user),
        int (*currentFunction)(double *iReturn, double time, char *nodeName, int id, void *user)
)
{
    return ngSpice_Init_Sync(voltageFunction, currentFunction, NULL, NULL, NULL);
}

int NGSpiceInterface::setExternalFunctions(GetVSRCData *voltageFunction, GetISRCData *currentFunction,
                                           GetSyncData *syncFunction, int *id,
                                           void *user)
{
    return ngSpice_Init_Sync(voltageFunction, currentFunction, syncFunction, id, user);
}
