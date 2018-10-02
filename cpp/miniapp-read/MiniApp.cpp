/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * Miniapp.cpp
 *
 *  Created on: May 7, 2018
 *      Author: Lipeng Wan, wanl@ornl.gov
 */

#include <iostream>
#include <thread>         // std::this_thread::sleep_for
#include <chrono>

#include "MiniApp.h"

MiniApp::MiniApp(const Settings &settings, MPI_Comm comm):io(settings, comm)
{
//	this->rank = settings.rank;
//	this->wr_freq = settings.wr_freq;
//	this->steps = settings.steps;
//	this->comm = comm;
}

void MiniApp::run(const Settings &s, MPI_Comm comm)
{
    for (unsigned int t = 0; t < s.steps; ++t)
    {
        int sec = int(s.wr_freq);
	int msec = int((s.wr_freq-sec)*1000);

	if (!s.rank)
	{
	    std::cout << "step: " << t << std::endl;
	    std::cout << "sleep: " << sec << " seconds " << msec << " milliseconds" << std::endl;
        }
	
	std::this_thread::sleep_for(std::chrono::seconds(sec));
	std::this_thread::sleep_for(std::chrono::milliseconds(msec));
	if (!s.rank)
	{
	    std::cout << "start reading..." << std::endl;
	}
        io.read(t, s, comm);
    }
}
