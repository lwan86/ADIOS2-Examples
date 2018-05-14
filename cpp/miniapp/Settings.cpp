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

Settings::Settings(int argc, char *argv[], int rank, int nproc) : rank{rank}
{
    if (argc < 8)
    {
        throw std::invalid_argument("Not enough arguments");
    }
    this->nproc = (unsigned int)nproc;
    configfile = argv[1];
    outputfile = argv[2];
    
    var_type = argv[3];
    var_num = atoi(argv[4]);
    var_size = convert2Bytes(argv[5]);

    if (var_type.compare("float") == 0)
        {
            var_len = var_size/4;
        }
    else if (var_type.compare("int") == 0)
        {
            var_len = var_size/4;
        }

    wr_freq = atof(argv[6]);
    steps = atoi(argv[7]);
       
}
