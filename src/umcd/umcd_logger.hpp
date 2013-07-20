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

#ifndef UMCD_LOGGER_HPP
#define UMCD_LOGGER_HPP

#include <ostream>
#include <iostream>
#include <sstream>

// boost::thread < 1.51 conflicts with C++11-capable compilers
#if BOOST_VERSION < 105100
    #include <time.h>
    #undef TIME_UTC
#endif
#include <boost/thread/mutex.hpp>
#include <umcd/boost/thread/lock_guard.hpp>
#include <boost/thread/thread.hpp>
#include <boost/array.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/noncopyable.hpp>

enum severity_level {
   trace,
   debug,
   info,
   warning,
   error,
   fatal,
   nb_severity_level
};

class logging_cache
{
   bool enabled;
   std::string header;
   boost::shared_ptr<std::stringstream> cache;
   boost::mutex cache_access;
public:
   class logging_cache_chain
   {
      private:
      logging_cache& logcache;
      public:
      logging_cache_chain(logging_cache& logcache)
      : logcache(logcache)
      {}

      template <class Streamable>
      logging_cache_chain& operator<<(const Streamable& log)
      {
         logcache.write_to_cache(log);
         return *this;
      }

      ~logging_cache_chain()
      {
         logcache.write_to_cache("\n");
      }
   };

   logging_cache()
   : enabled(true)
   , header()
   , cache(boost::make_shared<std::stringstream>())
   {
   }

   void set_header(const std::string& header)
   {
      this->header = "[" + header + "] ";
   }

   void set_enabled(bool value)
   {
      enabled = value;
   }

   // Return the old cache
   boost::shared_ptr<std::stringstream> make_new_cache()
   {
      boost::lock_guard<boost::mutex> guard(cache_access);
      boost::shared_ptr<std::stringstream> old_cache = cache;
      cache = boost::make_shared<std::stringstream>();
      return old_cache;
   }

   template <class Streamable>
   void write_to_cache(const Streamable& log)
   {
      if(enabled)
      {
         boost::lock_guard<boost::mutex> guard(cache_access);
         *cache << log;
      }
   }

   template <class Streamable>
   logging_cache_chain operator<<(const Streamable& log)
   {
      write_to_cache(header);
      write_to_cache(log);
      return logging_cache_chain(*this);
   }
};

class umcd_logger : boost::noncopyable
{
   static const char* severity_level_name[];

   severity_level current_sev_lvl;
   boost::array<boost::shared_ptr<std::ostream>, nb_severity_level> logging_output;
   boost::array<logging_cache, nb_severity_level> logging_caches;

   umcd_logger()
   : current_sev_lvl(trace)
   {
      int sev;
      for(sev=0; sev <= warning; ++sev)
      {
         logging_output[sev] = boost::make_shared<std::ostream>(std::cout.rdbuf());
         logging_caches[sev].set_header(severity_level_name[sev]);
      }
      for(; sev < nb_severity_level; ++sev)
      {
         logging_output[sev] = boost::make_shared<std::ostream>(std::cerr.rdbuf());
         logging_caches[sev].set_header(severity_level_name[sev]);
      }
   }

public:
   static umcd_logger& get()
   {
      static umcd_logger logger;
      return logger;
   }

   void run_once()
   {
      for(int sev=0; sev < nb_severity_level; ++sev)
      {
         boost::shared_ptr<std::stringstream> log = logging_caches[sev].make_new_cache();
         *logging_output[sev] << log->str();
      }
   }

   void run()
   {
      boost::chrono::milliseconds time_to_sleep_between_two_log_write(100);
      while(true)
      {
         run_once();
         boost::this_thread::sleep_for(time_to_sleep_between_two_log_write);
      }
   }

   void set_severity(severity_level level)
   {
      current_sev_lvl = level;
      int sev;
      for(sev=0; sev < current_sev_lvl; ++sev)
      {
         logging_caches[sev].set_enabled(false);
      }
      for(; sev < nb_severity_level; ++sev)
      {
         logging_caches[sev].set_enabled(true);
      }
   }

   void set_output(severity_level sev, const std::ostream& stream)
   {
      logging_output[sev] = boost::make_shared<std::ostream>(stream.rdbuf());
   }

   logging_cache& get_logger(severity_level level)
   {
      return logging_caches[level];
   }
};

#define UMCD_LOG(severity) (umcd_logger::get().get_logger(severity))
#define UMCD_LOG_IP(severity, socket) ((umcd_logger::get().get_logger(severity)) << socket.remote_endpoint())

#endif // UMCD_LOGGER_HPP
