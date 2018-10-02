/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * IO_adios2.cpp
 *
 *  Created on: May 9, 2018
 *      Author: Lipeng Wan, wanl@ornl.gov
 */

#include "IO.h"

#include <string>
#include <iostream>
#include <cmath>

#include <adios2.h>

//adios2::ADIOS *ad = nullptr;
adios2::ADIOS ad;
//adios2::Engine *writer = nullptr;
adios2::Engine Writer;
//adios2::IO *io = nullptr;
std::vector<adios2::Variable<double>> allVars;
adios2::Variable<double> adiosVarDouble;

std::unordered_map<std::string, unsigned int> all_vars_size;
//adios2::Variable<float> &bpFloats;
//adios2::Variable<double> *varT = nullptr;
//adios2::Variable<unsigned int> *varGndx = nullptr;


static std::vector<unsigned int> prime_factors(unsigned int n)
{
    std::vector<unsigned int> p_factors = {};
    while (n%2 == 0)
    {
        //printf("%d ", 2);
        p_factors.push_back(2);
        n = n/2;
    }
    // n must be odd at this point.  So we can skip 
    // one element (Note i = i +2)
    for (int i = 3; i <= sqrt(n); i = i+2)
    {
        // While i divides n, print i and divide n
        while (n%i == 0)
        {
            //printf("%d ", i);
            p_factors.push_back(i);
            n = n/i;
        }
    }
 
    // This condition is to handle the case when n 
    // is a prime number greater than 2
    if (n > 2)
        //printf ("%d ", n);
        p_factors.push_back(n);

    if (n == 1)
    {
        p_factors.push_back(n);
    }
    return p_factors;
}

IO::IO(const Settings &s, MPI_Comm comm)
{
    m_inputfilename = MakeFilename(s.inputfile, ".bp");
    
    ad = adios2::ADIOS(s.configfile, comm, adios2::DebugON);
    
    adios2::IO io = ad.DeclareIO("MiniAppOutput");
    
    if (!io.InConfigFile())
    {
        // if not defined by user, we can change the default settings
        // BPFile is the default writer
        io.SetEngine("BPFile");
        io.SetParameters({{"num_threads", "1"}});

        // ISO-POSIX file output is the default transport (called "File")
        // Passing parameters to the transport
        io.AddTransport("File", {{"Library", "POSIX"}});
    }

    Reader = io.Open(m_inputfilename, adios2::Mode::Read, comm);
    
}



IO::~IO()
{
    Reader.Close();
    //delete ad;
}

void IO::read(int step, const Settings &s, MPI_Comm comm)
{


    std::vector<double> myDouble;
    double timeStart = MPI_Wtime(); 

    std::vector<unsigned int> p_factors = prime_factors(s.nproc);

    Reader.BeginStep();

    for (auto& it : s.var_type_num) 
    {
        for (unsigned int i = 0; i < it.second; ++i)
        {
            std::string var_type = it.first;
	    adiosVarDouble = io.InquireVariable<double>(var_type+std::to_string(i));
	    
            unsigned int var_dims = s.var_type_dims.at(var_type);
            std::vector<unsigned int> var_dim_size = s.var_type_dim_size.at(var_type);

            std::vector<unsigned int> proc_per_dim;

            if (var_dims == p_factors.size()) 
            {
                proc_per_dim = p_factors;
            }
            else if (var_dims > p_factors.size())
            {
                proc_per_dim = p_factors;
                for (int v = 0; v < var_dims-p_factors.size(); v++)
                {
                    proc_per_dim.push_back(1);
                }
            }
            else
            {
                unsigned int diff = p_factors.size()-var_dims;
                std::sort(p_factors.begin(), p_factors.end());
                proc_per_dim = p_factors;
                unsigned int prod = 1;
                for (int u = 0; u < diff+1; u++) 
                {

                    prod *= proc_per_dim[u];
                }
                proc_per_dim.erase(proc_per_dim.begin(), proc_per_dim.begin()+diff+1);
                proc_per_dim.push_back(prod);
            }

            unsigned int all_dim_size_product = 1;
            std::vector<unsigned int> global_offset(var_dims, 0);
            std::vector<unsigned int> local_size(var_dims, 0);
            std::vector<unsigned int> proc_pos(var_dims, 0);
            std::vector<unsigned int> size_per_proc_per_dim(var_dims, 0);
            for (unsigned int k = 0; k < var_dims; ++k)
            {
                size_per_proc_per_dim[k] = std::ceil(double(var_dim_size[k])/double(proc_per_dim[k]));
            }

            unsigned int prev_div;
            int flag = 0;
            for (unsigned int j = 0; j < proc_per_dim.size(); ++j)
            {
                if (j == 0) 
                {
                    proc_pos[j] = s.rank%proc_per_dim[j];
                    prev_div = s.rank/proc_per_dim[j];
                }
                else
                {
                    proc_pos[j] = prev_div%proc_per_dim[j];
                    prev_div = prev_div/proc_per_dim[j];
                }
                //std::cout << proc_pos[j] << " ";            
                if (proc_pos[j]*size_per_proc_per_dim[j] >= var_dim_size[j])
                {
                    std::cout << "this dim has been covered, no more proc is needed!" << std::endl;
                    flag = -1;
                    break;
                }

                global_offset[j] = proc_pos[j]*size_per_proc_per_dim[j];

                if (global_offset[j]+size_per_proc_per_dim[j] <= var_dim_size[j]) 
                {
                    local_size[j] = size_per_proc_per_dim[j];
                }
                else
                {
                    local_size[j] = var_dim_size[j]-global_offset[j];
                }

            }

            if (flag < 0)
            {
                std::cout << "jump to another block!" << std::endl;
                continue;
            } 

            std::vector<size_t> g_dim_size;
            std::vector<size_t> g_offset;
            std::vector<size_t> l_size;

            for (unsigned int m = 0; m < var_dims; m++)
            {
                g_dim_size.push_back(var_dim_size[m]);
                g_offset.push_back(global_offset[m]);
                l_size.push_back(local_size[m]);
            }

	    adiosVarDouble.SetSelection(adios2::Box<adios2::Dims>(g_offset, l_size));
	    Reader.Get<double>(adiosVarDouble, myDouble.data());
	}
    }
    Reader.EndStep();

    double timeEnd = MPI_Wtime();
    //std::cout << "line 400, " << "s.rank: " << s.rank << ", " << "mpi rank: " << r << std::endl;
	if (s.rank == 0)
		std::cout << "I/O time = " << timeEnd - timeStart << std::endl;
}
