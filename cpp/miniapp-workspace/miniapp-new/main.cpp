/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * main.cpp
 *
 *  Created on: May 8, 2018
 *      Author: Lipeng Wan, wanl@ornl.gov
 */

#include <cstdlib>
#include <iostream>
#include <mpi.h>
#include <thread>         // std::this_thread::sleep_for
#include <chrono>
#include <memory>
#include <stdexcept>

#include "Settings.h"
#include "IO.h"
#include "MiniApp.h"

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);

    int rank, nproc;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);

	try 
    {
        Settings settings(argc, argv, rank, nproc);

    //std::cout << "rank: " << rank << std::endl;
        if (!rank) 
        {
            //std::cout << "type of each variable: " << settings.var_type << std::endl;    
            //std::cout << "number of variables: " << settings.var_num << std::endl;
            //std::cout << "length of each variable: " << settings.var_len << std::endl;
            std::cout << "write frequency (seconds between two consecutive writes): " << settings.wr_freq << std::endl;
            std::cout << "total times of write: " << settings.steps << std::endl;
        }
    
//    IO io(settings, MPI_COMM_WORLD);
//    for (unsigned int t = 0; t < settings.steps; ++t)
//        {
//			int sec = int(3600/settings.wr_freq);
//			if (!rank)
//				{
//					std::cout << "sleep: " << sec << " seconds" << std::endl;
//				}
//			
//			std::this_thread::sleep_for(std::chrono::seconds(sec));
//            io.write(t, settings, MPI_COMM_WORLD);
//        }
	    MiniApp miniapp(settings, MPI_COMM_WORLD);
	    miniapp.run(settings, MPI_COMM_WORLD);
    


        MPI_Barrier(MPI_COMM_WORLD);
	}
	catch (std::invalid_argument &e) // command-line argument errors
    {
        std::cout << e.what() << std::endl;
        //printUsage();
    }
    catch (std::ios_base::failure &e) // I/O failure (e.g. file not found)
    {
        std::cout << "I/O base exception caught\n";
        std::cout << e.what() << std::endl;
    }
    catch (std::exception &e) // All other exceptions
    {
        std::cout << "Exception caught\n";
        std::cout << e.what() << std::endl;
    }
    MPI_Finalize();
    
    return 0;
}
