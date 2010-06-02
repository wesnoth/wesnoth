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
	struct name_space {
		std::string namespace_;
		std::string root_;
		std::string node_;
		std::string lineage_;
		bool valid_;
		bool valid() const {
			return valid_;
		}
		std::string lineage() const {
			return lineage_;
		}
		std::string node() const {
			return node_;
		}
		std::string root() const {
			return root_;
		}
		void parse() {
			while (namespace_.find_first_of("^") < namespace_.size()) {
				std::string infix = namespace_.substr(namespace_.find_first_of("^"));
				size_t end = infix.find_first_not_of("^");
				if (!((end >= infix.length()) || (infix[end] == '.'))) {
					//TODO: Throw a WML error
					namespace_ = "";
					break;
				}
				infix = infix.substr(0,end);
				std::string suffix = namespace_.substr(namespace_.find_first_of("^") + infix.length());
				while (!infix.empty())
				{
					std::string body = namespace_.substr(0,namespace_.find_first_of("^"));
					body = body.substr(0,body.find_last_of("."));
					infix = infix.substr(1);
					namespace_ = body + infix + suffix;
				}
			}
		}
		name_space(const std::string &ns) : namespace_(ns) {
			parse();
			valid_ = ((namespace_.find_first_not_of("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_.") > namespace_.length()) && !namespace_.empty());
			root_ = namespace_.substr(0,namespace_.find_first_of("."));
			node_ = namespace_.substr(namespace_.find_last_of(".") + 1);
			if (namespace_.find_last_of(".") <= namespace_.length())
				lineage_ = namespace_.substr(0,namespace_.find_last_of("."));
		}
	};
	// TODO: transaction support (needed for MP)
	typedef std::map<std::string,persist_context*> child_map;
	// TODO: parent and child members (needed for namespace embeddeding)
	name_space namespace_;
	config cfg_;
	persist_context *parent_;
	child_map children_;
	bool valid_;
	bool dirty_;
	bool collected_;
	void load();
	void init();
	bool save_context();
	void update_configs();
public:
	persist_context(const std::string &);
	persist_context(const std::string &, persist_context &);
	persist_context(const std::string &, persist_context *);
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
