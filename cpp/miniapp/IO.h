/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * IO.h
 *
 *  Created on: Feb 2017
 *      Author: Norbert Podhorszki
 */

#ifndef IO_H_
#define IO_H_

//#include "HeatTransfer.h"
#include "Settings.h"

#include <mpi.h>
//#include <adios2.h>

class IO
{
public:

    //adios2::ADIOS adios;
    //adios2::Engine writer;
    //std::vector<adios2::Variable<float>> vars;
    
    IO(const Settings &s, MPI_Comm comm);
    ~IO();
    void write(int step, const Settings &s, MPI_Comm comm);
};

#endif /* IO_H_ */
