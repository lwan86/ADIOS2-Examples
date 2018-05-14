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
#include <adios2.h>

adios2::ADIOS *ad = nullptr;
adios2::Engine *writer = nullptr;
adios2::IO *io = nullptr;
std::vector<adios2::Variable<float> *> vars;
adios2::Variable<float> *bpFloats = nullptr;
//adios2::Variable<float> &bpFloats;
//adios2::Variable<double> *varT = nullptr;
//adios2::Variable<unsigned int> *varGndx = nullptr;

static std::vector<float> generate_fake_var(unsigned int length)
{
    std::vector<float> var = {};
    for (unsigned int i = 0; i < length; i++)
        {
            var.push_back(float(i));
        }
    return var;
}

IO::IO(const Settings &s, MPI_Comm comm)
{
    ad = new adios2::ADIOS(s.configfile, comm, adios2::DebugON);

    io = &ad->DeclareIO("MiniAppOutput");
    if (!io->InConfigFile())
    {
        // if not defined by user, we can change the default settings
        // BPFile is the default writer
        io->SetEngine("BPFile");
        io->SetParameters({{"num_threads", "1"}});

        // ISO-POSIX file output is the default transport (called "File")
        // Passing parameters to the transport
        io->AddTransport("File", {{"Library", "POSIX"}});
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
    for (unsigned int i = 0; i < s.var_num; i++)
        {
            //bpFloats = new adios2::Variable<float>();
            bpFloats = &io->DefineVariable<float>("var"+std::to_string(i), {s.nproc*s.var_len}, {s.rank * s.var_len}, {s.var_len}, adios2::ConstantDims);
            //std::cout << bpFloats << std::endl;
            vars.push_back(bpFloats);
            
            bpFloats = nullptr;
        }


    writer = &io->Open(s.outputfile, adios2::Mode::Write, comm);
}



IO::~IO()
{
    writer->Close();
    delete ad;
}

void IO::write(int step, const Settings &s, MPI_Comm comm)
{
    std::vector<std::vector<float>> vars_data;
    std::vector<float> myFloats;
    //std::vector<float> tmp;
    for (unsigned int i = 0; i < s.var_num; i++)
        {
            myFloats = generate_fake_var(s.var_len);
            vars_data.push_back(myFloats);
        }
	double timeStart = MPI_Wtime();
    writer->BeginStep();
    // using PutDeferred() you promise the pointer to the data will be intact
    // until the end of the output step.
    // We need to have the vector object here not to destruct here until the end
    // of function.

	
    for (unsigned int i = 0; i < s.var_num; i++)
        {
            writer->PutDeferred<float>(*vars[i], vars_data[i].data());
        }

    //std::vector<double> v = ht.data_noghost();
    //writer->PutDeferred<double>(*varT, v.data());
    writer->EndStep();
	double timeEnd = MPI_Wtime();	
	if (s.rank == 0)
		std::cout << "I/O time = " << timeEnd - timeStart << std::endl;
}
