/*===========================================================================
*
*                            PUBLIC DOMAIN NOTICE
*               National Center for Biotechnology Information
*
*  This software/database is a "United States Government Work" under the
*  terms of the United States Copyright Act.  It was written as part of
*  the author's official duties as a United States Government employee and
*  thus cannot be copyrighted.  This software/database is freely available
*  to the public for use. The National Library of Medicine and the U.S.
*  Government have not placed any restriction on its use or reproduction.
*
*  Although all reasonable efforts have been taken to ensure the accuracy
*  and reliability of the software and data, the NLM and the U.S.
*  Government do not and cannot warrant the performance or results that
*  may be obtained by using this software or data. The NLM and the U.S.
*  Government disclaim all warranties, express or implied, including
*  warranties of performance, merchantability or fitness for any particular
*  purpose.
*
*  Please cite the author in any work or product based on this material.
*
* ===========================================================================
*
*/

#include "config_align_to.h"

#include <iostream>
#include <chrono>
#include <thread>
#include <list>
#include "omp_adapter.h"

<<<<<<< HEAD
const std::string VERSION = "0.63";
=======
const std::string VERSION = "0.40";
>>>>>>> engineering

typedef uint64_t hash_t;

#include "dbs.h"
#include "aligns_to_job.h"
#include "aligns_to_db_job.h"
#include "aligns_to_dbs_job.h"
#include "aligns_to_dbss_job.h"

using namespace std;
using namespace std::chrono;

int main(int argc, char const *argv[])
{
    #ifdef __GLIBCXX__
    std::set_terminate(__gnu_cxx::__verbose_terminate_handler);
    #endif
    
    LOG("aligns_to version " << VERSION);
    LOG("hardware threads: "  << std::thread::hardware_concurrency() << ", omp threads: " << omp_get_max_threads());
    Config config(argc, argv);

    auto before = high_resolution_clock::now();

<<<<<<< HEAD
    unique_ptr<Job> job;

    if (!config.db.empty())
        job = unique_ptr<DBJob>(new DBJob(config.db));
    else if (!config.dbs.empty())
        job = unique_ptr<DBSBasicJob>(new DBSBasicJob(config.dbs));
    else if (!config.dbsm.empty())
        job = unique_ptr<DBSMJob>(new DBSMJob(config.dbsm));
    else if (!config.dbss.empty())
        job = unique_ptr<DBSSJob>(new DBSSJob(config.dbss, config.dbss_tax_list));
//    else if (!config.many.empty())
//        job = make_unique<ManyJobs>(config.many);
=======
    Job *job = nullptr;

    if (!config.db.empty())
        job = new DBJob(config);
    else if (!config.dbs.empty())
        job = new DBSBasicJob(config);
    else if (!config.dbss.empty())
        job = new DBSSJob(config);
>>>>>>> engineering
    else
        Config::fail();

    LOG("loading time (sec) " << std::chrono::duration_cast<std::chrono::seconds>( high_resolution_clock::now() - before ).count());
    if (job->db_kmers() > 0)
        LOG("kmers " << job->db_kmers() << " (" << (job->db_kmers() / 1000 / 1000) << "m)");

<<<<<<< HEAD
    for (auto &contig_file : config.contig_files)
    {
        LOG(contig_file);

        IO::Writer writer(config.contig_files.size() == 1 ? config.out : contig_file + config.out);
		try 
		{
	        job->run(contig_file, writer, config);
		}
		catch (std::exception &e)
		{
			LOG(e.what());
			if (config.contig_files.size() == 1)
				throw e;
		}
=======
    if (!config.contig_files.empty())
	{
        for (auto &filename : config.contig_files)
            {
                LOG(filename);
                before = high_resolution_clock::now();
                {
                    ofstream out_f(filename + ".matches");
                    out_f.flush(); // ?
                    job->run(filename, out_f);
                }

                auto processing_time = std::chrono::duration_cast<std::chrono::seconds>( high_resolution_clock::now() - before ).count();
                LOG("processing time (sec) " << processing_time);
            }
    }
    else
	{
        if (config.contig_file.empty())
            throw std::runtime_error("contig file(s) is empty");

        LOG(config.contig_file);
        job->run(config.contig_file, cout);
>>>>>>> engineering
    }

    LOG("total time (sec) " << std::chrono::duration_cast<std::chrono::seconds>( high_resolution_clock::now() - before ).count());

//    std::exit(0); // dont want to wait for destructors
    return 0;
}

