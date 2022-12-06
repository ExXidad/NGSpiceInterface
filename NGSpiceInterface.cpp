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
            string optionsString = "";
            if (!_options.empty())
            {
                optionsString = ".options";
                for (const auto &[option, value]:_options)
                    optionsString += " " + option + "=" + value;
            }
            string analysisString = "";
            if (!_analyses.empty())
            {
                for (const string &analysis:_analyses)
                    analysisString += analysis + _delimiter;
            }
            string netlist = _circuit + _delimiter + optionsString + _delimiter + analysisString + _delimiter + ".end";
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
    sendCommand(NULL);
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
    _circuit = readFileToString(fname);
    _delimiter = "\n";
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

Analyses &NGSpiceInterface::analyses()
{
    return _analyses;
}

Analyses NGSpiceInterface::analyses() const
{
    return _analyses;
}
