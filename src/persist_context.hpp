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
	// TODO: transaction support (needed for MP)
	typedef std::map<std::string,persist_context*> child_map;
	// TODO: parent and child members (needed for namespace embeddeding)
	std::string namespace_;
	config cfg_;
	persist_context *parent_;
	child_map children_;
	bool valid_;
	bool dirty_;
	void load();
	void init(const std::string &name_space);
	bool save_context();
	void update_configs();
public:
	persist_context(const std::string &);
	persist_context(const std::string &, persist_context &);
	~persist_context();
	bool clear_var(std::string &);
	config get_var(const std::string &);
	bool set_var(const std::string &, const config &);
	bool valid() const { return valid_; };
	bool dirty() const { 
		bool dirt = dirty_;
		child_map::const_iterator i = children_.begin();
		while ((!dirt) && (i != children_.end())) {
			dirt |= i->second->dirty();
			i++;
		}
		return dirt; 
	};
};
#endif
