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

#ifndef UMCD_OPTIONS_HPP
#define UMCD_OPTIONS_HPP

#include <string>
#include <fstream>

#include "config.hpp"
#include <boost/program_options.hpp>

class server_options
{
  private:
    std::string header;
    std::string version;
    boost::program_options::options_description options_desc;
    boost::program_options::options_description config_file_options;
    boost::program_options::variables_map vm;

    std::string port;
    std::string wesnoth_directory;
    int threads;
    std::string config_file_name;

    void build_options_desc();
    /**
      @brief Merge command line argument and config file argument.
    */
    void merge_cfg();

  public:
    /**
      @brief Accept argument as describe with the "--help" option. 
             Merge arguments from the command line and the config file (umcd.cfg or file specify in argument).
             The command line arguments override the config file arguments.
      @post Build a server_options object.
      @throw If the command line argument are not recognized by the current pattern.
    */
    server_options(int argc, char* argv[]);

    /**
      @post: If requested, help or version message is printed.
      @returns: If the user asked to print help or version message, return true.
                Otherwise, return false.
    */
    bool print_info();

    /**
      @brief A config file is builded with the values available to us.
      @returns: The config file. A value is readed in this priority order:
                (1) Command line ;
                (2) Configuration file ;
                (3) Default value if any.
    */
    config build_config();

    static const std::string DEFAULT_PORT;
    static const int DEFAULT_THREADS;
};

#endif //UMCD_OPTIONS_HPP