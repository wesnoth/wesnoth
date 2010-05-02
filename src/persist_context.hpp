/* $Id$ */
/*
   Copyright (C) 2003 - 2010 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef PERSIST_CONTEXT_H_INCLUDED
#define PERSIST_CONTEXT_H_INCLUDED
#include "config.hpp"
#include "log.hpp"
static lg::log_domain log_persist("engine/persistence");

#define LOG_SAVE LOG_STREAM(info, log_persist)
#define ERR_SAVE LOG_STREAM(err, log_persist)
config pack_scalar(const std::string &,const t_string &);
class persist_context {
private:
	// TODO: dirty marking
	// TODO: child persist_contexts for embedded namespaces?
	config cfg_;
	std::string namespace_;
	bool valid_;
public:
	persist_context(const std::string &);
	bool clear_var(std::string &);
	config get_var(const std::string &);
	bool set_var(const std::string &, const config &);
	bool valid() const { return valid_; };
};
#endif
