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

#pragma once

#include <string>

struct Config
{
	std::string db, input;

	int argc;
	char const **argv;

	std::string arg(int index) const
	{
		if (!arg_exists(index))
			fail();

		return std::string(argv[index]);
	}

	bool arg_exists(int index) const
	{
		return index > 0 && index < argc;
	}

	Config(int argc, char const *argv[]) : argc(argc), argv(argv)
	{
		db = arg(1);
		input = arg(2);
	}

	void fail() const
	{
		print_usage();
        exit(1);
	}

	static void print_usage()
	{
		std::cerr << "prints spot ids matching db file to stdout" << std::endl;
		std::cerr << "need <.db file> <vdb run file>" << std::endl;
	}
};