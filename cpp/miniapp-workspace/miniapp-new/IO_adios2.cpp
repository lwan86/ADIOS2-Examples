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

static std::vector<double> generate_fake_var(unsigned int length)
{
    std::vector<double> var = {};
    for (unsigned int i = 0; i < length; i++)
    {
        var.push_back(double(i));
    }
    return var;
}

static void generate_fake_var2(unsigned int length, std::vector<double> &var)
{
    for (unsigned int i = 0; i < length; i++)
    {
        var[i] = double(i);
    }
}

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
    m_outputfilename = MakeFilename(s.outputfile, ".bp");
    //ad = new adios2::ADIOS(s.configfile, comm, adios2::DebugON);
    
    ad = adios2::ADIOS(s.configfile, comm, adios2::DebugON);
    
    //io = &ad->DeclareIO("MiniAppOutput");
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

    // define T as 2D global array
//    varT = &io.DefineVariable<double>(
//        "T",
//        // Global dimensions
//        {s.gndx, s.gndy},
//        // starting offset of the local array in the global space
//        {s.offsx, s.offsy},
//        // local size, could be defined later using SetSelection()
//        {s.ndx, s.ndy});
    //for (unsigned int i = 0; i < s.var_num; i++)
    //{
            //bpFloats = new adios2::Variable<float>();
    //    bpDouble = bpio.DefineVariable<double>("var"+std::to_string(i), {s.nproc*s.var_len}, {s.rank * s.var_len}, {s.var_len}, adios2::ConstantDims);
            //std::cout << bpFloats << std::endl;
    //    allVars.push_back(bpDouble);
            
            //bpFloats = nullptr;
    //}

    std::vector<unsigned int> p_factors = prime_factors(s.nproc);
    //unsigned int fake_nproc = 60;
    //static std::vector<unsigned int> p_factors = prime_factors(fake_nproc);

    for (auto& it : s.var_type_num) 
    {
        //std::cout << it.first << ", " << it.second << std::endl;
        for (unsigned int i = 0; i < it.second; ++i)
        {
            std::string var_type = it.first;
            //std::cout << s.var_type_dims.at(var_type) << std::endl;

            unsigned int var_dims = s.var_type_dims.at(var_type);
            std::vector<unsigned int> var_dim_size = s.var_type_dim_size.at(var_type);
            //std::cout << var_dim_size[0] << ", " << var_dim_size[1] << ", " << var_dim_size[2] << std::endl;
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

            /*
            for (unsigned int fake_rank = 0; fake_rank < fake_nproc; fake_rank++)
            {
                unsigned int prev_div;
                std::cout << fake_rank << ": ";
                for (unsigned int j = 0; j < p_factors.size(); ++j)
                {
                    if (j == 0) 
                    {
                        proc_pos[j] = fake_rank%p_factors[j];
                        prev_div = fake_rank/p_factors[j];
                    }
                    else
                    {
                        proc_pos[j] = prev_div%p_factors[j];
                        prev_div = prev_div/p_factors[j];
                    }
                    std::cout << proc_pos[j] << " ";
                    
                    global_offset[j] = proc_pos[j]*size_per_proc_per_dim[j];

                    std::cout << global_offset[j] << " ";

                    if (global_offset[j]+size_per_proc_per_dim[j] <= var_dim_size[j]) 
                    {
                        local_size[j] = size_per_proc_per_dim[j];
                    }
                    else
                    {
                        local_size[j] = var_dim_size[j]-global_offset[j];
                    }
                    std::cout << local_size[j] << " ";

                }
                std::cout << std::endl;
            }
            */
            //int r;
            //MPI_Comm_rank(comm, &r);
            //std::cout << "s.rank: " << s.rank << ", " << "mpi rank: " << r << proc_per_dim[0] << std::endl;
                
     
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
                //std::cout << global_offset[j] << " ";

                if (global_offset[j]+size_per_proc_per_dim[j] <= var_dim_size[j]) 
                {
                    local_size[j] = size_per_proc_per_dim[j];
                }
                else
                {
                    local_size[j] = var_dim_size[j]-global_offset[j];
                }
                //std::cout << local_size[j] << " ";

            }
            //std::cout << std::endl;
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
            
            adiosVarDouble = io.DefineVariable<double>(var_type+std::to_string(i), g_dim_size, g_offset, l_size, adios2::ConstantDims);

            //std::cout << bpDouble.Name() << "," << s.rank << ": ";
            /*
            for (unsigned int m = 0; m < var_dims; m++)
            {

                std::cout << bpDouble.Count()[m] << " ";
            }
            std::cout << std::endl;
            */
            //bpDouble = bpio.DefineVariable<double>(var_type+std::to_string(i), var_dim_size, global_offset, local_size, adios2::ConstantDims);
            //bpDouble = bpio.DefineVariable<double>(var_type+std::to_string(i), {var_dim_size[0],var_dim_size[1],var_dim_size[2]}, {global_offset[0],global_offset[1],global_offset[2]}, {local_size[0],local_size[1],local_size[2]}, adios2::ConstantDims);
            //bpDouble = bpio.DefineVariable<double>(var_type+std::to_string(i), var_dim_size, global_offset, local_size);

            allVars.push_back(adiosVarDouble);

            unsigned int var_local_size = 1;
            for (unsigned int l = 0; l < local_size.size(); l++)
            {
                var_local_size *= local_size[l];
            }
            //std::cout << var_local_size << std::endl;
            all_vars_size[var_type+std::to_string(i)+"-"+std::to_string(s.rank)] = var_local_size;
            //std::cout << var_type+std::to_string(i)+","+std::to_string(s.rank) << ": " << all_vars_size[var_type+std::to_string(i)+std::to_string(s.rank)] << std::endl;

        }
    }

    //for (auto& it : s.var_type_dim_size)
    //{
    //    std::cout << it.first << ", " << it.second[0] << ", " << it.second[1] << ", " << it.second[2] << std::endl;
    //}

    Writer = io.Open(m_outputfilename, adios2::Mode::Write, comm);
    
    
    //bpWriter.FixedSchedule();
}



IO::~IO()
{
    Writer.Close();
    //delete ad;
}

void IO::write(int step, const Settings &s, MPI_Comm comm)
{
    //std::vector<std::vector<double>> vars_data;
    //std::vector<double> myDouble;
    //std::vector<float> tmp;
    /*
    for (unsigned int i = 0; i < s.var_num; i++)
    {
        myDouble = generate_fake_var(s.var_len);
        vars_data.push_back(myDouble);
    }
    */

    //int r;
    //MPI_Comm_rank(comm, &r);
    //std::cout << "s.rank: " << s.rank << ", " << "mpi rank: " << r << std::endl;
    double timeStart = MPI_Wtime(); 

    Writer.BeginStep();
    // using PutDeferred() you promise the pointer to the data will be intact
    // until the end of the output step.
    // We need to have the vector object here not to destruct here until the end
    // of function.

	/*
    for (unsigned int i = 0; i < s.var_num; i++)
    {
        bpWriter.Put<double>(allVars[i], vars_data[i].data());
    }
    */
    //std::vector<double> v = ht.data_noghost();
    //writer->PutDeferred<double>(*varT, v.data());
    //std::cout << "line 355, " << "s.rank: " << s.rank << ", " << "mpi rank: " << r << ", allVars size: " << allVars.size() << std::endl;
    for (unsigned int i = 0; i < allVars.size(); i++)
    {
        std::string var_name = allVars[i].Name();
        //std::unordered_map<std::string, unsigned int>::const_iterator got = all_vars_size.find(var_name+"-"+std::to_string(s.rank));

        /*
        if (got == all_vars_size.end())
        {
            std::cout << "rank " << s.rank << "not used!" << std::endl;
            continue;
        }
        else
        {
            std::cout << got->first << " is " << got->second << std::endl;
        }
        */
        //myDouble = generate_fake_var(all_vars_size[var_name+"-"+std::to_string(s.rank)]);
        unsigned int myDoubleSize = all_vars_size[var_name+"-"+std::to_string(s.rank)];
        //std::cout << myDoubleSize << std::endl;
        std::vector<double> myDouble(myDoubleSize, 0);
        generate_fake_var2(myDoubleSize, myDouble);
        //double min, max;
        //min = *std::min_element(myDouble.begin(), myDouble.end());
        //max = *std::max_element(myDouble.begin(), myDouble.end());

        std::cout << var_name << "," << s.rank << ": " << allVars[i].Shape()[0] << " " << allVars[i].Shape()[1] << " " << allVars[i].Shape()[2] << ", " 
            << allVars[i].Start()[0] << " " << allVars[i].Start()[1] << " " << allVars[i].Start()[2] << ", " 
            << allVars[i].Count()[0] << " " << allVars[i].Count()[1] << " " << allVars[i].Count()[2] << std::endl;

        
        //std::cout << var_name << "," << s.rank << ": ";
        //for (unsigned int m = 0; m < allVars[i].Count().size(); m++)
        //{
        //    std::cout << allVars[i].Count()[m] << " ";
        //}
        //std::cout << std::endl;
        //std::cout << var_name << "," << s.rank << ": min: " << min << ", max: " << max << std::endl;
        //for (unsigned int k = 0; k < myDouble.size(); k++)
        //{
        //    std::cout << myDouble[k] << " ";
        //}
        //std::cout << std::endl;
        Writer.Put<double>(allVars[i], myDouble.data());
    }
    //std::cout << "line 397, " << "s.rank: " << s.rank << ", " << "mpi rank: " << r << std::endl;
    Writer.EndStep();

    double timeEnd = MPI_Wtime();
    //std::cout << "line 400, " << "s.rank: " << s.rank << ", " << "mpi rank: " << r << std::endl;
	if (s.rank == 0)
		std::cout << "I/O time = " << timeEnd - timeStart << std::endl;
}
