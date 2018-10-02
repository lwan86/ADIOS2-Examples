/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * MiniApp.h
 *
 *  Created on: May 7, 2018
 *      Author: Lipeng Wan, wanl@ornl.gov
 */

#ifndef MINIAPP_H_
#define MINIAPP_H_

#include <mpi.h>

#include "Settings.h"
#include "IO.h"


class MiniApp
{
public:

    IO io;

	//int rank;
	//float wr_freq;
	//unsigned int steps;
	//MPI_Comm comm;
	
    MiniApp(const Settings &settings, MPI_Comm comm);

    //~MiniApp();

	void run(const Settings &settings, MPI_Comm comm);

};

#endif


