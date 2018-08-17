#ifndef PARSE_H_
#define PARSE_H_

#pragma once 

#include <string>
#include <iostream>
#include <cstring>

class CLIParser {
    protected:
        int _argc;
        char **_argv;
        std::string _instanceName;
        std::string _instancePath;
        double _threshold;
        double _discretizationLength;
        bool _writeLP;
        bool _writeLog;
        bool _cplexOutput;
        std::string _lpFile;
        std::string _logFile;
        std::string _resultFile;

    public:
        CLIParser(int argc, char **argv) : _argc(argc), _argv(argv), _instanceName(),
            _instancePath(), _threshold(), _discretizationLength(), _writeLP(false), _writeLog(false),
            _cplexOutput(false), _lpFile(), _logFile(), _resultFile() {}; 
        
        std::string getInstanceName() const { return _instanceName; };
        std::string getInstancePath() const { return _instancePath; };
        double getThreshold() const { return _threshold; };
        double getDiscretizationLength() const { return _discretizationLength; };
        bool getWriteLP() const { return _writeLP; };
        bool getWriteLog() const { return _writeLog; };
        bool getCplexOutput() const { return _cplexOutput; };
        std::string getLPFile() const { return _lpFile; };
        std::string getLogFile() const { return _logFile; };
        std::string getResultFile() const { return _resultFile; };

        void parse(); 
};

void CLIParser::parse() {

    bool givenInstanceFile = false;
    bool givenInstancePath = false;
    bool givenThreshold = false;
    bool givenDiscretizationLength = false;

    for (int i=0; i<_argc; ++i) {
        if (std::strcmp(_argv[i], "-f") == 0) {
            _instanceName = _argv[i+1];
            ++i;
            givenInstanceFile = true;
            continue;
        }

        if (std::strcmp(_argv[i], "-p") == 0) {
            _instancePath = _argv[i+1];
            ++i;
            givenInstancePath = true;
            continue;
        }

        if (std::strcmp(_argv[i], "-t") == 0) {
            _threshold = atof(_argv[i+1]);
            ++i;
            givenThreshold = true;
            continue;
        }

        if (std::strcmp(_argv[i], "-d") == 0) {
            _discretizationLength = atof(_argv[i+1]);
            ++i;
            givenDiscretizationLength = true;
            continue;
        }

        if (std::strcmp(_argv[i], "--log") == 0) {
            _writeLog = true;
            continue;
        }

        if (std::strcmp(_argv[i], "--lp") == 0) {
            _writeLP = true;
            continue; 
        }

        if (std::strcmp(_argv[i], "--display") == 0) {
            _cplexOutput = true;
            continue;
        }

        if (std::strcmp(_argv[i], "--help") == 0) {
            std::cout << std::endl << "Usage: " << std::endl
                << "./main [-f filename] [-p filepath] [-t threshold] " << std::endl 
                << " \t [-d discretization-length] [--log] [--lp] [--display]" << std::endl 
                << "command line argument options and defaults:" << std::endl << std::endl 
                << "mandatory arguments are" << std::endl << std::endl 
                << "-f \t instance file name" << std::endl 
                << "-p \t instance file path" << std::endl 
                << "-t \t threshold or vehicle sensor radius" << std::endl  
                << "-d \t edge discretization length" << std::endl << std::endl 
                << "optional arguements are" << std::endl << std::endl 
                << "--log        write cplex output to log file (default: false)" << std::endl 
                << "             assumes there is a folder entitled logs/ in cwd" << std::endl
                << "--lp         write lp file (default: false)" << std::endl  
                << "             assumes there is a folder entitled lp/ in cwd" << std::endl
                << "--display    switch on cplex display (default: false)" << std::endl << std::endl
                << "if both --log and --display flags are used, the log file will be " << std::endl
                << "created and cplex display with by turned off" << std::endl; 
        }

    }

    if (!givenInstanceFile && !givenInstancePath && !givenDiscretizationLength && !givenThreshold) {
        std::cout << "mandatory command line arguments are missing " << std::endl 
            << "run ./main --help for the info on command line arguments " << std::endl;
        exit(1);
    }

    _lpFile = "lp/" + _instanceName + 
                "-" + std::to_string(int(_discretizationLength)) +
                "-" + std::to_string(int(_threshold)) + ".lp";
    _logFile = "logs/" + _instanceName + 
                "-" + std::to_string(int(_discretizationLength)) +
                "-" + std::to_string(int(_threshold)) + ".log";
    _resultFile = "results/" + _instanceName + 
                "-" + std::to_string(int(_discretizationLength)) +
                "-" + std::to_string(int(_threshold)) + ".txt";

    return;
};

#endif
