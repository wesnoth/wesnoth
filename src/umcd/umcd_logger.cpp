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

#include "umcd/umcd_logger.hpp"

const char* umcd_logger::severity_level_name[] = {
   "trace",
   "debug",
   "info",
   "warning",
   "error",
   "fatal"
};

log_line_cache::log_line_cache(umcd_logger& logger, severity_level severity)
: logger_(logger)
, enabled_(logger.get_current_severity() <= severity)
, severity_(severity)
, line_(boost::make_shared<std::stringstream>())
{}

log_line_cache::~log_line_cache()
{
   if(enabled_)
   {
      logger_.add_line(*this);
   }
}