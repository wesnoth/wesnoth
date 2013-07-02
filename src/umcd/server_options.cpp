/*
   Copyright (C) 2012-2013 by Pierre Talbot <ptalbot@mopong.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "umcd/server_options.hpp"

const std::string server_options::DEFAULT_PORT = "12523";
const int server_options::DEFAULT_THREADS = 0;

server_options::server_options(int argc, char* argv[]) : 
    header("  Wesnoth campaign server.\n  Development version by Pierre Talbot. Copyright (C) 2013.\n"), 
    version("Wesnoth campaign server - Development version"),
    options_desc(), config_file_name()
{
  build_options_desc();

  // Positional options (we don't need the option "cfg-file" to specify the config file).
  po::positional_options_description p;
  p.add("cfg-file", -1);

  // Parse the command line.
  po::store(po::command_line_parser(argc, argv).options(options_desc).positional(p).run(), vm);
  po::notify(vm);
}

void server_options::build_options_desc()
{
  // Help messages.
  std::string cfg_help_msg("The config file in which we read the server configuration.");
  std::string port_help_msg("The TCP/IP port to listen to for incoming requests.");
  std::string threads_help_msg("The number of working threads to start. A value of 0 means start as many threads as there are detected hardware cores.");
  std::string wesnoth_directory_help_msg("The wesnoth directory.");

  // Generic options.
  po::options_description generic("General options");
  generic.add_options()
    ("help", "produce help message")
    ("version,v", "output the version number")
  ;

  // Options related to the config file.
  po::options_description file_options("Config file options");
  file_options.add_options()
    ("cfg-file", po::value<std::string>(&config_file_name), cfg_help_msg.c_str())
  ;

  // Options related to the command line.
  po::options_description cmdline("Server configuration (override any config file)"); 
  cmdline.add_options()
    ("port,p", po::value<std::string>(&port)->default_value(DEFAULT_PORT), port_help_msg.c_str())
    ("threads,t", po::value<int>(&threads)->default_value(DEFAULT_THREADS), threads_help_msg.c_str())
    ("wesnoth_dir,d", po::value<std::string>(&wesnoth_directory)->required(), wesnoth_directory_help_msg.c_str())
  ;

  options_desc.add(generic).add(file_options).add(cmdline);
  config_file_options.add(cmdline); 
}

bool server_options::print_info()
{
  if(vm.count("version"))
  {
    std::cout << version << std::endl;
    return true;
  }
  if(vm.count("help"))
  {
    std::cout << header << options_desc << std::endl;
    return true;
  }
  return false;
}

void server_options::merge_cfg()
{
  std::ifstream cfgfile(config_file_name.c_str());
  if(!cfgfile)
    throw po::reading_file(config_file_name.c_str());

  po::store(po::parse_config_file(cfgfile, config_file_options), vm);
  po::notify(vm);
}

config server_options::build_config()
{
  if(vm.count("cfg-file"))
  {
    merge_cfg();
  }

  config server_cfg;
  server_cfg["threads"] = threads;
  server_cfg["port"] = port;
  std::string::const_reverse_iterator it = wesnoth_directory.rbegin();
  server_cfg["wesnoth_dir"] = wesnoth_directory + ((*it == '/') ? "":"/");
  return server_cfg;
}

