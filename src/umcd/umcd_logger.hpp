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

// NOTE: Please use the Boost.Log when the supported Boost version will be >= 1.54.

#ifndef UMCD_LOGGER_HPP
#define UMCD_LOGGER_HPP

#include <ostream>
#include <iostream>
#include <sstream>

#include "umcd/boost/thread/workaround.hpp"
#include <boost/thread/mutex.hpp>
#include <umcd/boost/thread/lock_guard.hpp>
#include <boost/thread/thread.hpp>
#include <boost/array.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/noncopyable.hpp>

#include "boost/date_time/posix_time/posix_time.hpp"

enum severity_level {
   trace,
   debug,
   info,
   warning,
   error,
   fatal,
   nb_severity_level
};

class umcd_logger;
struct log_line;

class log_line_cache
{
   umcd_logger& logger;
   bool enabled;
   severity_level severity;
   boost::shared_ptr<std::stringstream> line;

   friend struct log_line;

public:

   log_line_cache(umcd_logger& logger, severity_level severity);
   ~log_line_cache();

   template <class Streamable>
   log_line_cache& operator<<(const Streamable& log)
   {
      if(enabled)
      {
         *line << log;
      }
      return *this;
   }
};

struct log_line
{
   severity_level severity;
   std::string data;
   boost::posix_time::ptime time;

   log_line(const log_line_cache& cache_line)
   : severity(cache_line.severity)
   , data(cache_line.line->str())
   , time(boost::posix_time::second_clock::universal_time())
   {}
};

class umcd_logger : boost::noncopyable
{
   static const char* severity_level_name[];

   typedef std::vector<log_line> cache_type;
   typedef boost::shared_ptr<cache_type> cache_ptr;

   severity_level current_sev_lvl;
   boost::array<boost::shared_ptr<std::ostream>, nb_severity_level> logging_output;

   boost::mutex cache_access;
   boost::shared_ptr<cache_type> cache;

   umcd_logger()
   : current_sev_lvl(trace)
   , cache(boost::make_shared<cache_type>())
   {
      int sev;
      for(sev=0; sev <= warning; ++sev)
      {
         logging_output[sev] = boost::make_shared<std::ostream>(std::cout.rdbuf());
      }
      for(; sev < nb_severity_level; ++sev)
      {
         logging_output[sev] = boost::make_shared<std::ostream>(std::cerr.rdbuf());
      }
   }

   // Returns the old cache.
   cache_ptr make_new_cache()
   {
      cache_ptr old_cache = cache;
      lock_guard<boost::mutex> guard(cache_access);
      cache = boost::make_shared<cache_type>();
      return old_cache;
   }

   std::string make_header(severity_level sev) const
   {
      return std::string("[") + severity_level_name[sev] + "] ";
   }

public:
   static umcd_logger& get()
   {
      static umcd_logger logger;
      return logger;
   }

   void add_line(const log_line_cache& line)
   {
      lock_guard<boost::mutex> guard(cache_access);
      cache->push_back(log_line(line));
   }

   void run_once()
   {
      cache_ptr old_cache = make_new_cache();
      for(std::size_t i=0; i < old_cache->size(); ++i)
      {
         const log_line& line = (*old_cache)[i];
         *logging_output[line.severity] << make_header(line.severity) 
            << boost::posix_time::to_simple_string(line.time) << ": "
            << line.data
            << "\n";
      }
   }

   void run()
   {
      while(true)
      {
         run_once();
         // NOTE: Replace this function by boost::this_thread::sleep_for when more recent Boost version will be supported.
         //       (Or better: the C++11 version)
         boost::this_thread::sleep(boost::posix_time::milliseconds(100));
      }
   }

   void set_severity(severity_level level)
   {
      current_sev_lvl = level;
   }

   severity_level get_current_severity() const
   {
      return current_sev_lvl;
   }

   void set_output(severity_level sev, const std::ostream& stream)
   {
      logging_output[sev] = boost::make_shared<std::ostream>(stream.rdbuf());
   }

   log_line_cache get_logger(severity_level level)
   {
      return log_line_cache(*this, level);
   }
};

#define UMCD_LOG(severity) (umcd_logger::get().get_logger(severity))
#define UMCD_LOG_IP(severity, socket) ((umcd_logger::get().get_logger(severity)) << socket.remote_endpoint())

#endif // UMCD_LOGGER_HPP
