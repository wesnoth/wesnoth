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

#ifndef UMCD_NETWORK_DATA_HPP
#define UMCD_NETWORK_DATA_HPP

#include <string>
#include <vector>

#include "config.hpp"

class network_data
{
public:
   typedef config metadata_type;
   typedef std::string binary_data_type;
protected:
   config metadata;
   binary_data_type binary_data;

public:
   network_data(const metadata_type& metadata, const binary_data_type& binary_data)
   : metadata(metadata)
   , binary_data(binary_data)
   {}

   network_data(const network_data& data)
   : metadata(data.metadata)
   , binary_data(data.binary_data)
   {}

   network_data(){}

   const metadata_type& get_metadata() const
   {
      return metadata;
   }
   const binary_data_type& get_binary() const
   {
      return binary_data;
   }
   metadata_type& get_metadata()
   {
      return metadata;
   }
   binary_data_type& get_binary()
   {
      return binary_data;
   }  
};

#endif // UMCD_NETWORK_DATA_HPP