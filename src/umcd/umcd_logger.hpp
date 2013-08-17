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
#include <fstream>
#include <ios>
#include <iostream>
#include <sstream>
#include <map>

#include "config.hpp"

#include "umcd/boost/thread/workaround.hpp"
#include "umcd/boost/thread/lock_guard.hpp"
#include <boost/thread/mutex.hpp>
#include <boost/array.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/noncopyable.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

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
private:
	friend struct log_line;

public:

	log_line_cache(umcd_logger& logger, severity_level severity);
	~log_line_cache();

	template <class Streamable>
	log_line_cache& operator<<(const Streamable& log)
	{
		if(enabled_)
		{
			 *line_ << log;
		}
		return *this;
	}

private:
	umcd_logger& logger_;
	bool enabled_;
	severity_level severity_;
	boost::shared_ptr<std::stringstream> line_;
};

struct log_line
{
	severity_level severity;
	std::string data;
	boost::posix_time::ptime time;

	log_line(const log_line_cache& cache_line)
	: severity(cache_line.severity_)
	, data(cache_line.line_->str())
	, time(boost::posix_time::second_clock::universal_time())
	{}
};

class log_stream
{
public:

	virtual ~log_stream() {}

	virtual boost::shared_ptr<std::ostream> stream() = 0;
};

class standard_log_stream : public log_stream
{
public:
	standard_log_stream(const std::ostream& log_stream)
	: stream_(boost::make_shared<std::ostream>(log_stream.rdbuf()))
	{}

	virtual boost::shared_ptr<std::ostream> stream()
	{
		return stream_;
	}

private:
	boost::shared_ptr<std::ostream> stream_;
};

class file_log_stream : public log_stream
{
public:
	file_log_stream(const std::string& filename)
	: filename_(filename)
	{}

	virtual boost::shared_ptr<std::ostream> stream()
	{
		return boost::make_shared<std::ofstream>(filename_.c_str(), std::ios_base::app);
	}

private:
	std::string filename_;
};


class umcd_logger : boost::noncopyable
{
	static const char* severity_level_name[];

	typedef std::vector<log_line> cache_type;
	typedef boost::shared_ptr<cache_type> cache_ptr;

	void default_logging_output()
	{
		int sev;
		for(sev=0; sev <= warning; ++sev)
		{
			set_output(static_cast<severity_level>(sev), boost::make_shared<standard_log_stream>(std::cout));
		}
		for(; sev < nb_severity_level; ++sev)
		{
			set_output(static_cast<severity_level>(sev), boost::make_shared<standard_log_stream>(std::cerr));
		}
	}

	// Returns the old cache.
	cache_ptr make_new_cache()
	{
		cache_ptr old_cache = cache_;
		lock_guard<boost::mutex> guard(cache_access_);
		cache_ = boost::make_shared<cache_type>();
		return old_cache;
	}

	std::string make_header(severity_level sev) const
	{
		return std::string("[") + severity_level_name[sev] + "] ";
	}

	void set_log_output(const std::string& levels_list, const boost::shared_ptr<log_stream>& stream)
	{
		std::vector<std::string> levels_to_stream;
		boost::algorithm::split(levels_to_stream, levels_list, boost::algorithm::is_any_of(" ,"));
		for(std::size_t i = 0; i < levels_to_stream.size(); ++i)
		{
			set_output(level_str2enum_[levels_to_stream[i]], stream);
		}
	}

	void set_standard_output(const config& log_cfg, const std::string& stream_name, const std::ostream& stream)
	{
		if(log_cfg.has_child(stream_name))
		{
			std::string log_to_stream = log_cfg.child(stream_name)["level"].str();
			set_log_output(log_to_stream, boost::make_shared<standard_log_stream>(stream));
		}
	}

	void set_files_output(const config& log_cfg)
	{
		config::const_child_itors frange = log_cfg.child_range("file");
		for(;frange.first != frange.second; ++(frange.first))
		{
			const config& file_detail = *(frange.first);
			set_log_output(file_detail["level"].str(), boost::make_shared<file_log_stream>(file_detail["filename"].str()));
		}
	}

public:
	umcd_logger()
	: current_sev_lvl_(trace)
	, cache_(boost::make_shared<cache_type>())
	{
		default_logging_output();
		// Init map "textual representation of the severity level" to "severity level enum".
		for(int sev=0; sev < nb_severity_level; ++sev)
			level_str2enum_[severity_level_name[sev]] = static_cast<severity_level>(sev);
	}

	void add_line(const log_line_cache& line)
	{
		lock_guard<boost::mutex> guard(cache_access_);
		cache_->push_back(log_line(line));
	}

	void run_once()
	{
		cache_ptr old_cache = make_new_cache();
		boost::array<boost::shared_ptr<std::ostream>, nb_severity_level> log_streams;
		for(std::size_t i=0; i < nb_severity_level; ++i)
		{
			log_streams[i] = logging_output_[i]->stream();
		}

		for(std::size_t i=0; i < old_cache->size(); ++i)
		{
			const log_line& line = (*old_cache)[i];
			*log_streams[line.severity] << make_header(line.severity) 
				<< boost::posix_time::to_simple_string(line.time) << ": "
				<< line.data
				<< "\n";
		}
	}

	void load_config(const config& log_cfg)
	{
		// Set the severity level.
		if(log_cfg.has_attribute("log_if_greater_or_equal"))
		{
			set_severity(level_str2enum_[log_cfg["log_if_greater_or_equal"].str()]);
		}

		set_standard_output(log_cfg, "cout", std::cout);
		set_standard_output(log_cfg, "cerr", std::cerr);
		set_files_output(log_cfg);
	}

	void set_severity(severity_level level)
	{
		current_sev_lvl_ = level;
	}

	severity_level get_current_severity() const
	{
		return current_sev_lvl_;
	}

	void set_output(severity_level sev, const boost::shared_ptr<log_stream>& stream)
	{
		logging_output_[sev] = stream;
	}

	log_line_cache get_logger(severity_level level)
	{
		return log_line_cache(*this, level);
	}

private:
	severity_level current_sev_lvl_;
	boost::array<boost::shared_ptr<log_stream>, nb_severity_level> logging_output_;
	boost::mutex cache_access_;
	boost::shared_ptr<cache_type> cache_;
	std::map<std::string, severity_level> level_str2enum_;
};

class asio_logger
{
	asio_logger()
	: running_(false)
	{}

	/**
	@param local_timer is necessary if the sequence [run, stop, run] occurs too fast.
	*/
	void run_impl(boost::shared_ptr<boost::asio::deadline_timer> local_timer, const boost::system::error_code& error)
	{
		get().run_once();
		if(running_ && !error)
		{
			timer->async_wait(boost::bind(&asio_logger::run_impl, this,
				local_timer, boost::asio::placeholders::error));
		}
	}
public:

	static asio_logger& get_asio_log()
	{
		static asio_logger lg;
		return lg;
	}

	static umcd_logger& get()
	{
		return get_asio_log().logger_;
	}

	void run(boost::asio::io_service& io_service_, boost::posix_time::time_duration timing)
	{
		running_ = true;
		timer = boost::make_shared<boost::asio::deadline_timer>(boost::ref(io_service_), timing);
		timer->async_wait(boost::bind(&asio_logger::run_impl, this,
				timer, boost::asio::placeholders::error));
	}

	void stop()
	{
		running_ = false;
		timer->cancel();
	}

private:
	umcd_logger logger_;
	bool running_;
	boost::shared_ptr<boost::asio::deadline_timer> timer;
};

#define CURRENT_FUNCTION_STRING "in " << BOOST_CURRENT_FUNCTION

#define UMCD_LOG(severity) (asio_logger::get().get_logger(severity))
#define UMCD_LOG_IP(severity, socket) ((asio_logger::get().get_logger(severity)) << socket.remote_endpoint())
#define UMCD_LOG_IP_FUNCTION_TRACER(socket) (UMCD_LOG_IP(trace, socket) << CURRENT_FUNCTION_STRING)
#define UMCD_LOG_FUNCTION_TRACER() (UMCD_LOG(trace) << CURRENT_FUNCTION_STRING)
#define RUN_ONCE_LOGGER() (asio_logger::get().run_once());

#endif // UMCD_LOGGER_HPP
