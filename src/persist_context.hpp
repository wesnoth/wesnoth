/*
   Copyright (C) 2010 - 2018 by Jody Northup
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "config.hpp"
#include "log.hpp"
static lg::log_domain log_persist("engine/persistence");

#define LOG_PERSIST LOG_STREAM(info, log_persist)
#define ERR_PERSist LOG_STREAM(err, log_persist)
config pack_scalar(const std::string &,const t_string &);
class persist_context {
public:

	virtual ~persist_context() {}

	struct name_space {
		std::string namespace_;
		std::string root_;
		std::string node_;
		std::string lineage_;
		std::string descendants_;
		bool valid_;

		bool valid() const {
			return valid_;
		}
		void parse() {
			while (namespace_.find_first_of("^") < namespace_.size()) {
				if (namespace_[0] == '^') {
					//TODO: Throw a WML error
					namespace_ = "";
					break;
				}
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
		name_space next() const {
			return name_space(descendants_);
		}
		name_space prev() const {
			return name_space(lineage_);
		}
		operator bool () const { return valid_; }
		name_space()
			: namespace_()
			, root_()
			, node_()
			, lineage_()
			, descendants_()
			, valid_(false)
		{
		}

		name_space(const std::string &ns, bool doParse = false)
			: namespace_(ns)
			, root_()
			, node_()
			, lineage_()
			, descendants_()
			, valid_(false)
		{
			if (doParse)
				parse();
			valid_ = ((namespace_.find_first_not_of("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_.") > namespace_.length()) && !namespace_.empty());
			root_ = namespace_.substr(0,namespace_.find_first_of("."));
			node_ = namespace_.substr(namespace_.find_last_of(".") + 1);
			if (namespace_.find_last_of(".") <= namespace_.length())
				lineage_ = namespace_.substr(0,namespace_.find_last_of("."));
			if (namespace_.find_first_of(".") <= namespace_.length())
				descendants_ = namespace_.substr(namespace_.find_first_of(".") + 1);
		}
	};
protected:
	config cfg_;
	name_space namespace_;
	bool valid_;
	bool in_transaction_;

	persist_context()
		: cfg_()
		, namespace_()
		, valid_(false)
		, in_transaction_(false)
	{}

	persist_context(const std::string &name_space)
		: cfg_()
		, namespace_(name_space,true)
		, valid_(namespace_.valid())
		, in_transaction_(false)
	{}

	config *get_node(config &cfg, name_space &ns, bool force = false) {
		name_space next = ns.next();
		if (next) {
			if (force)
				return get_node(cfg.child_or_add(next.root_), next, true);
			else if (cfg.has_child(next.root_))
				return get_node(cfg.child(next.root_), next);
			else
				return nullptr;
		}
		else
			return &cfg;
	}

	const config *get_node(const config &cfg, const name_space &ns) const {
		name_space next = ns.next();
		if (next) {
			if (cfg.has_child(next.root_))
				return get_node(cfg.child(next.root_), next);
			else
				return nullptr;
		}
		else
			return &cfg;
	}

public:
	virtual bool clear_var(const std::string &, bool immediate = false) = 0;
	virtual config get_var(const std::string &) const = 0;
	virtual bool set_var(const std::string &, const config &, bool immediate = false) = 0;
	virtual bool start_transaction () = 0;
	virtual bool end_transaction () = 0;
	virtual bool cancel_transaction () = 0;
	std::string get_node() const;
	void set_node(const std::string &);
	bool valid() const { return valid_; }
	bool dirty() const {
		return true;
	}
	operator bool() const { return valid_; }
};

class persist_file_context : public persist_context {
private:
	void load();
	void init();
	bool save_context();

public:
	persist_file_context(const std::string &name_space);
	bool clear_var(const std::string &, bool immediate = false);
	config get_var(const std::string &) const;
	bool set_var(const std::string &, const config &, bool immediate = false);

	bool start_transaction () {
		if (in_transaction_)
			return false;
		in_transaction_ = true;
		return true;
	}
	bool end_transaction () {
		if (!in_transaction_)
			return false;
		in_transaction_ = false;
		save_context();
		return true;
	}
	bool cancel_transaction () {
		if (!in_transaction_)
			return false;
		load();
		in_transaction_ = false;
		return true;
	}
};
