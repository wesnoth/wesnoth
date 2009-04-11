/* $Id$ */
/*
   Copyright (C) 2003 by David White <dave@whitevine.net>
                 2004 - 2009 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file log.hpp
 * Standard logging facilities (interface).
 */

#ifndef LOG_HPP_INCLUDED
#define LOG_HPP_INCLUDED

#include <iostream> // needed else all files including log.hpp need to do it.
#include <string>
#include <vector>

namespace lg {

class logger;

struct logd {
	char const *name_;
	int severity_;
};

class log_domain {
	int domain_;
public:
	log_domain(char const *name);
	friend class logger;
};

//exposed to make inlining possible
extern std::vector<logd> log_domains;

bool set_log_domain_severity(std::string const &name, int severity);
std::string list_logdomains();

class logger {
	char const *name_;
	int severity_;
public:
	logger(char const *name, int severity): name_(name), severity_(severity) {}
	std::ostream &operator()(log_domain const &domain,
		bool show_names = true, bool do_indent = false) const;

	bool dont_log(log_domain const &domain) const
	{
		logd const &d = log_domains[domain.domain_];
		return severity_ > d.severity_;
	}
};

void timestamps(bool);
std::string get_timestamp(const time_t& t, const std::string& format="%Y%m%d %H:%M:%S ");

extern logger err, warn, info, debug;
extern log_domain general, ai, ai_configuration, ai_manager, formula_ai, cache, config,
	display, engine, network, mp_server, filesystem, audio, replay, help, gui, gui_parse,
	gui_layout, gui_draw, gui_event, editor, wml, mp_user_handler, lua;

class scope_logger
{
	int ticks_;
	std::ostream *output_;
	std::string str_;
public:
	scope_logger(log_domain const &domain, const char* str) :
		ticks_(0),
		output_(0),
		str_()
	{
		if (!debug.dont_log(domain)) do_log_entry(domain, str);
	}
	scope_logger(log_domain const &domain, const std::string& str) :
		ticks_(0),
		output_(0),
		str_()
	{
		if (!debug.dont_log(domain)) do_log_entry(domain, str);
	}
	~scope_logger()
	{
		if (output_) do_log_exit();
	}
	void do_indent() const;
private:
	void do_log_entry(log_domain const &domain, const std::string& str);
	void do_log_exit();
};

/**
 * Use this logger to send errors due to deprecated WML.
 * The preferred format is:
 * xxx is deprecated, support will be removed in version X. or
 * xxx is deprecated, support has been removed in version X.
 *
 * After every wml-event the errors are shown to the user,
 * so they can inform the campaign maintainer.
 */
extern std::stringstream wml_error;

} // namespace lg

#define log_scope(a) lg::scope_logger scope_logging_object__(lg::general, a);
#define log_scope2(a,b) lg::scope_logger scope_logging_object__(lg::a, b);

#define LOG_STREAM(a, b) if (lg::a.dont_log(lg::b)) ; else lg::a(lg::b)

// When using log_scope/log_scope2 it is nice to have all output indented.
#define LOG_STREAM_INDENT(a,b) if (lg::a.dont_log(lg::b)) ; else lg::a(lg::b, true, true)


// Define a list of standard loggers.
#define DBG_GUI LOG_STREAM_INDENT(debug, gui)
#define LOG_GUI LOG_STREAM_INDENT(info, gui)
#define WRN_GUI LOG_STREAM_INDENT(warn, gui)
#define ERR_GUI LOG_STREAM_INDENT(err, gui)

#define DBG_G_P LOG_STREAM_INDENT(debug, gui_parse)
#define LOG_G_P LOG_STREAM_INDENT(info, gui_parse)
#define WRN_G_P LOG_STREAM_INDENT(warn, gui_parse)
#define ERR_G_P LOG_STREAM_INDENT(err, gui_parse)

#define DBG_G_L LOG_STREAM_INDENT(debug, gui_layout)
#define LOG_G_L LOG_STREAM_INDENT(info, gui_layout)
#define WRN_G_L LOG_STREAM_INDENT(warn, gui_layout)
#define ERR_G_L LOG_STREAM_INDENT(err, gui_layout)

#define DBG_G_D LOG_STREAM_INDENT(debug, gui_draw)
#define LOG_G_D LOG_STREAM_INDENT(info, gui_draw)
#define WRN_G_D LOG_STREAM_INDENT(warn, gui_draw)
#define ERR_G_D LOG_STREAM_INDENT(err, gui_draw)

#define DBG_G_E LOG_STREAM_INDENT(debug, gui_event)
#define LOG_G_E LOG_STREAM_INDENT(info, gui_event)
#define WRN_G_E LOG_STREAM_INDENT(warn, gui_event)
#define ERR_G_E LOG_STREAM_INDENT(err, gui_event)

#endif
