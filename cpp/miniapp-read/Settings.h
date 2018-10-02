/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * Settings.h
 *
 *  Created on: May 8, 2018
 *      Author: Lipeng Wan, wanl@ornl.gov
 */

#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <string>
#include <unordered_map>
#include <vector>

class Settings
{
 public:

    std::string configfile;
    std::string inputfile;    
    std::string varsParameterFile;
    //std::string var_type;

    std::unordered_map<std::string, unsigned int> var_type_num;

    std::unordered_map<std::string, unsigned int> var_type_dims;

    std::unordered_map<std::string, std::vector<unsigned int>> var_type_dim_size;

    //unsigned int var_num;  // number of variables
    //unsigned long long var_size; // size of each variable in bytes
    //unsigned int var_len; // length of each variable

    float wr_freq;  // 
    int steps;  // total times of write
 
    int rank; // MPI rank

    unsigned int nproc;

    Settings(int argc, char *argv[], int rank, int nproc);

    void ParseVarsParameterFile(std::string varsfilename, std::unordered_map<std::string, unsigned int> &var_type_num,
    std::unordered_map<std::string, unsigned int> &var_type_dims, 
    std::unordered_map<std::string, std::vector<unsigned int>> &var_type_dim_size);
};

#endif
