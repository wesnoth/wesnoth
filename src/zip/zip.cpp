#include "../config.hpp"
#include "../game_config.hpp"
#include <iostream>
#include <sstream>
#include <vector>

bool try_unzip(const std::string& data_in, std::string& data_out)
{
	config cfg;
	compression_schema schema;
	try {
		cfg.read_compressed(data_in,schema);
	} catch(config::error&) {
		return false;
	}
	data_out = cfg.write();
	return true;
}

bool try_zip(const std::string& data_in, std::string& data_out)
{
	config cfg;
	compression_schema schema;
	cfg.read(data_in);
	try {
		data_out = cfg.write_compressed(schema);
	} catch(config::error&) {
		return false;
	}
	return true;
}

int main(int argc, char** argv)
{
	std::vector<std::string> args;
	args.reserve(argc);
	for(int arg = 0; arg != argc; ++arg)
		args.push_back(std::string(argv[arg]));

	bool to_stdout = false, from_stdin = true;
	std::vector<std::string> files;
	for(std::vector<std::string>::const_iterator i = args.begin()+1; i != args.end(); ++i) {
		if(i->empty())
			continue;
		if(*i == "-h" || *i == "--help") {
 			std::cout
				<< "usage: " << args[0]
				<< " [-clhV] [files...]\n"
 				<< "  -c, --std-out     Write output on standard output; keep original files unchanged.\n"
 				<< "  -h, --help        Prints this message and exits\n"
 				<< "  -V, --version     Prints the game's version number and exits\n";
			return 0;
		} else if(*i == "-V" || *i == "--version") {
 			std::cout << "Battle for Wesnoth " << game_config::version << "\n";
			std::cout << args[0] << " 0.1\n";
			return 0;
		} else if(*i == "-c" || *i == "--std-out") {
			to_stdout = true;
			continue;
		}
		// Any other options must be file names
		files.push_back(*i);
		from_stdin = false;
	}
	if(from_stdin) {
		const std::string data = read_stdin();
		std::string output;
		if(!try_unzip(data,output) && !try_zip(data,output)) {
			std::cerr << "Standard input data not compressed or uncompressed WML\n";
			return 3;
		}
		std::cout << output;
		return 0;
	}
	// Process input files
	int error = 0;
	for(std::vector<std::string>::const_iterator i = files.begin(); i != files.end(); ++i) {
		const std::string data = read_file(*i);
		if(data.empty()) {
			std::cerr << "Could not read file: " << *i << '\n';
			error = 1;
			continue;
		}
		std::string output;
		if(!try_unzip(data,output) && !try_zip(data,output)) {
			std::cerr << "Input file not compressed or uncompressed WML: " << *i << '\n';
			error = 3;
			continue;
		}
		if(to_stdout) {
			std::cout << output;
		} else {
			try {
				write_file(*i,output);
			} catch (io_exception& e) {
				std::cerr << "Could not write file: " << *i << '\n';
				error = 2;
			}
		}
	}
	return error;
}
