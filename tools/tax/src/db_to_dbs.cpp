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
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include "dbs.h"
#include "config_db_to_dbs.h"

int main(int argc, char const *argv[])
{
	Config config(argc, argv);

	std::vector<hash_t> db;
	int kmer_len = DBSIO::load_dbs(config.db, db);

	std::vector<DBS::KmerTax> dbs;
	dbs.reserve(db.size());
	for (auto &kmer : db)
		dbs.push_back(DBS::KmerTax(kmer, config.tax_id));

	DBSIO::save_dbs(config.dbs, dbs, kmer_len);
}
