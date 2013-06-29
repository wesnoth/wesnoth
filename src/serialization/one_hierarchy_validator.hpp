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

#ifndef SERIALIZATION_ONE_HIERARCHY_VALIDATOR_HPP_INCLUDED
#define SERIALIZATION_ONE_HIERARCHY_VALIDATOR_HPP_INCLUDED

#include "serialization/schema_validator.hpp"

namespace schema_validation
{

class one_hierarchy_validator : public schema_validator
{
private:
  std::string first_tag_open;
  std::size_t num_tags_open;
public:
  one_hierarchy_validator(const std::string & filename);

  virtual ~one_hierarchy_validator(){}

  virtual void open_tag(const std::string & name,
              int start_line,
              const std::string &file,
              bool addition = false);

  virtual void close_tag();

  virtual bool is_over(const config& cfg) const;
};
}

#endif // SERIALIZATION_ONE_HIERARCHY_VALIDATOR_HPP_INCLUDED