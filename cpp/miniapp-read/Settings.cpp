/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * Settings.cpp
 *
 *  Created on: May 8, 2018
 *      Author: Lipeng Wan, wanl@ornl.gov
 */

#include <cstdlib>
//#include <cstring>
#include <stdexcept>
#include <fstream>
#include <iostream>
#include <sstream>
#include <iterator>


#include "Settings.h"

static unsigned int convert2Bytes(char *arg)
{
    unsigned int val;
    float num, unit = 1.0;
    char ch;
    sscanf(arg, "%f%c", &num, &ch);
    switch(ch)
        {
        case 'k':
        case 'K':
            unit = 1024;
            break;
        case 'm':
        case 'M':
            unit = 1024*1024;
            break;            
        case 'g':
        case 'G':
            unit = 1024*1024*1024;
            break;                      
        }
    val = (unsigned int)(num*unit);
    return val;
}

void Settings::ParseVarsParameterFile(std::string varsfilename, std::unordered_map<std::string, unsigned int> &var_type_num,
    std::unordered_map<std::string, unsigned int> &var_type_dims, 
    std::unordered_map<std::string, std::vector<unsigned int>> &var_type_dim_size)
{
    std::string line;
    std::ifstream fs;
    std::vector<std::string> tokens;
    tokens.resize(0);
    fs.open(varsfilename);
    if (!fs) 
    {
        std::cout << "can't open the file!" << std::endl;

    }
    else 
    {
        unsigned int var_num;
        unsigned int var_dims;
        //std::vector<unsigned int> var_dim_size;
        while (std::getline(fs, line)) 
        {
            //std::cout << line << std::endl;
            std::stringstream iss(line);
            
            std::copy(std::istream_iterator<std::string>(iss),
                      std::istream_iterator<std::string>(),
                      std::back_inserter(tokens));
            //for (auto i = tokens.begin(); i != tokens.end(); ++i)
            //    std::cout << *i << ' ';
            std::string var_type = tokens[0];
            var_num = std::stoi(tokens[1]);
            var_dims = std::stoi(tokens[2]);

            
            //var_dim_size.resize(0);
            var_type_dim_size.insert(std::make_pair(var_type, std::vector<unsigned int>(var_dims, 0)));
            for (int i = 0; i < int(var_dims); i++)
            {
                //var_dim_size.push_back(std::stoi(tokens[i+3]));
                var_type_dim_size[var_type][i] = std::stoi(tokens[i+3]);
            }
            var_type_num.insert(std::make_pair(var_type, var_num));
            var_type_dims.insert(std::make_pair(var_type, var_dims));  
            //var_type_dim_size.insert(std::make_pair(var_type, var_dim_size));  
            //var_type_dim_size.emplace(var_type, var_dim_size);  
            //var_type_dim_size[var_type] = var_dim_size;
            //std::cout << std::endl;
            tokens.resize(0);
        }
        fs.close();
    }
}

Settings::Settings(int argc, char *argv[], int rank, int nproc) : rank{rank}
{
    if (argc < 3)
    {
        throw std::invalid_argument("Not enough arguments");
    }
    this->nproc = (unsigned int)nproc;
    //this->rank = rank;
    configfile = argv[1];
    outputfile = argv[2];
    
    varsParameterFile = argv[3];
    ParseVarsParameterFile(varsParameterFile, var_type_num, var_type_dims, var_type_dim_size);

    //var_type = argv[3];
    //var_num = atoi(argv[4]);
    //var_size = convert2Bytes(argv[5]);

    /*
    if (var_type.compare("float") == 0)
        {
            var_len = var_size/4;
        }
    else if (var_type.compare("int") == 0)
        {
            var_len = var_size/4;
        }
    */
    this->wr_freq = atof(argv[4]);
    this->steps = atoi(argv[5]);
       
}
