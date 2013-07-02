/*
   Copyright (C) 2013 by Pierre Talbot <ptalbot@mopong.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef UMCD_WML_REQUEST_HPP
#define UMCD_WML_REQUEST_HPP

#include <istream>
#include <string>

#include <boost/shared_ptr.hpp>

#include "umcd/network_data.hpp"
#include "serialization/one_hierarchy_validator.hpp"
#include "serialization/parser.hpp"

class wml_request
{
private:
   network_data data;
   const config& server_conf;

   void check_stream_state(std::istream& raw_data_stream, std::string error_msg)
   {
      if(!raw_data_stream.good())
      {
         throw std::runtime_error(error_msg);
      }
   }

   std::string peek_name(std::istream& raw_data_stream)
   {
      // Try to read the first tag which is the name of the packet.
      std::string error_msg("Invalid packet. The request name could not have been read.");
      char first_bracket;
      raw_data_stream >> first_bracket;
      check_stream_state(raw_data_stream, error_msg);
      if(first_bracket != '[')
         throw std::runtime_error(error_msg);
      std::string request_name;
      getline(raw_data_stream, request_name, ']');
      check_stream_state(raw_data_stream, error_msg);

      // Put back name in the stream. So the parsing of the config will be ok.
      raw_data_stream.putback(']');
      std::string::const_reverse_iterator b = request_name.rbegin();
      std::string::const_reverse_iterator e = request_name.rend();
      for(; b != e; ++b)
         raw_data_stream.putback(*b);
      raw_data_stream.putback(first_bracket);

      return request_name;
   }

public:
   //wml_request(){}
   wml_request(std::istream& raw_data_stream, const config& server_conf)
   : server_conf(server_conf)
   {
      using namespace schema_validation;
      std::string request_name = peek_name(raw_data_stream);
      std::string validator_file = server_conf["wesnoth_dir"].str() + "data/umcd/protocol_schema/"+request_name+".cfg";
      boost::shared_ptr<one_hierarchy_validator> validator(new one_hierarchy_validator(validator_file));
      read(data.get_metadata(), raw_data_stream, validator.get());
      std::cout << "[Info] Request read:\n" << data.get_metadata();
   }

   network_data& get_data() { return data; }

   const config& get_conf() const { return server_conf; }

   std::string name() const
   {
      config::all_children_iterator iter = data.get_metadata().ordered_begin();
      if(iter == data.get_metadata().ordered_end())
        return "";
      return iter->key;
   }
};


#endif // UMCD_WML_REQUEST_HPP
