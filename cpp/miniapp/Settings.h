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

class Settings
{
 public:

    std::string configfile;
    std::string outputfile;    

    std::string var_type;
    unsigned int var_num;  // number of variables
    unsigned long long var_size; // size of each variable in bytes
    unsigned int var_len; // length of each variable

    float wr_freq;  // times of write in an hour
    unsigned int steps;  // total times of write
 
    int rank; // MPI rank

    unsigned int nproc;

    Settings(int argc, char *argv[], int rank, int nproc);
};

#endif
