#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

#include <string>

struct Config
{
	const char *accession;
	int min_contig_len;
	bool unaligned_only;
	std::string filter_file;
    bool exclude_filter;

	Config(int argc, char const *argv[]) : 
		accession(nullptr), 
		min_contig_len(200), 
		unaligned_only(false),
        exclude_filter(false)
	{
		auto cmdline_acc = get_cmdline_accession(argc, argv);
		if (cmdline_acc)
		{
			accession = cmdline_acc;
			parse_options(argc, argv, 2);
		}
		else
		{
            LOG("missing accession");
            exit(1);
		}
	}

	static const char *get_cmdline_accession(int argc, char const *argv[])
	{
		return argc >= 2 ? argv[1] : nullptr;
	}

	static void print_usage()
	{
        LOG("need <accession> [options]" << std::endl
            << "options:" << std::endl
            << "-unaligned_only" << std::endl
            << "-min_contig_len <number>" << std::endl
 //		<< "-max_ram <gigabytes>" << std::endl
            << "-filter_file <filename>" << std::endl
            << "-exclude_filter");
	}

	void parse_options(int argc, char const *argv[], int pos)
	{
		for(; pos < argc; pos++)
		{
			std::string arg(argv[pos]);
			if (arg == "-unaligned_only")
				unaligned_only = true;
            else if (arg == "-exclude_filter")
				exclude_filter = true;
			else if (get_int_value("-min_contig_len", argv, argc, pos, &min_contig_len))
				pos++;
//			else if (get_int_value("-max_ram", argv, argc, pos, &max_ram))
//				pos++;
			else if (get_str_value("-filter_file", argv, argc, pos, &filter_file))
				pos++;
			else {
                LOG("bad option: " << arg);
                exit(1);
            }
		}
	}

	static bool get_str_value(const std::string &option_name, const char *argv[], int argc, int pos, std::string *value)
	{
		if (option_name != argv[pos])
			return false;

		if (pos + 1 >= argc) {
            LOG("missing value for " << option_name);
            exit(1);
        }

		*value = std::string(argv[pos + 1]);

		return true;
	}

	static bool get_int_value(const std::string &option_name, const char *argv[], int argc, int pos, int *value)
	{
		std::string s;
		if (!get_str_value(option_name, argv, argc, pos, &s))
			return false;

		*value = str_to_int(s);
		return true;

		//if (option_name != argv[pos])
		//	return false;

		//if (pos + 1 >= argc)
		//	throw std::string("missing value for ") + option_name;

		//*value = str_to_int(argv[pos + 1]);
		//return true;
	}

	static int str_to_int(const std::string &s)
	{
		return std::stoi(s); // throws an exception
	}
};

#endif
