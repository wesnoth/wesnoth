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

#include "global.hpp"

#include "filesystem.hpp"
#include "util.hpp"
#include "log.hpp"
#include "persist_context.hpp"
#include "resources.hpp"
#include "serialization/binary_or_text.hpp"

static bool is_valid_namespace(const std::string &name_space) {
	return (name_space.find_first_not_of("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_") > name_space.length());
}

static std::string get_namespace_ancestry(const std::string &name_space) {
	return name_space.substr(0,name_space.find_last_of("."));
}
static std::string get_namespace_leaf(const std::string &name_space) {
	return name_space.substr(name_space.find_last_of(".") + 1);
}
static std::string get_namespace_root(const std::string &name_space) {
	return name_space.substr(0,name_space.find_first_of("."));
}

static std::string get_persist_cfg_name(const std::string &name_space) {
	// TODO: get namespace base name
	if (is_valid_namespace(get_namespace_root(name_space)))
		return (get_dir(get_user_data_dir() + "/persist/") + get_namespace_root(name_space) + ".cfg");
	else
		return "";
}

static bool load_persist_data(const std::string &name_space, config &cfg, const bool create_if_missing = true)
{
	bool success = false;
	std::string cfg_dir = get_dir(get_user_data_dir() + "/persist");
	create_directory_if_missing(cfg_dir);

	std::string cfg_name = get_persist_cfg_name(name_space);
	if (!cfg_name.empty()) {
		scoped_istream file_stream = istream_file(cfg_name);
		if (file_stream->fail()) {
			if (create_if_missing) {
				// TODO: create proper file structure.
				success = true;
			}
		} else {
			try {
				detect_format_and_read(cfg,*file_stream);
				success = true;
			} catch (config::error &err) {
				LOG_SAVE << err.message;
				success = false;
			}
		}
	}
	return success;
}

static bool save_persist_data(std::string &name_space, config &cfg)
{
	bool success = false;

	std::string cfg_name = get_persist_cfg_name(name_space);
	if (!cfg_name.empty()) {
		if (cfg.empty()) {
			success = delete_directory(cfg_name);
		} else {
			scoped_ostream out = ostream_file(cfg_name);
			if (!out->fail())
			{
				config_writer writer(*out,false);
				try {
					writer.write(cfg);
					success = true;
				} catch(config::error &err) {
					LOG_SAVE << err.message;
					success = false;
				}
			}
		}
	}
	return success;
}

config pack_scalar(const std::string &name, const t_string &val)
{
	config cfg;
	cfg[name] = val;
	return cfg;
}

void persist_context::load() {
	if (parent_ == NULL)
		load_persist_data(namespace_,cfg_,false);
	for (config::all_children_iterator i = cfg_.ordered_begin(); i != cfg_.ordered_end(); i++) {
		if (i->key != "variables") {
			if (children_.find(i->key) == children_.end()) {
				persist_context *child = new persist_context(i->key);
				children_[i->key] = child;
			}
			children_[i->key]->parent_ = this;
			children_[i->key]->cfg_ = cfg_.child_or_add(i->key);
		}
	}
}

void persist_context::init(const std::string &name_space) {
	if (name_space.find_last_of(".") < name_space.length()) {
		std::string ancestry = get_namespace_ancestry(name_space);
		std::string parent_name = get_namespace_leaf(ancestry);
		parent_ = new persist_context(ancestry,*this);
	}
	load();
	if (!cfg_.child("variables"))
		cfg_.add_child("variables");
}

persist_context::persist_context(const std::string &name_space) : 
namespace_(get_namespace_leaf(name_space)), 
cfg_(),
parent_(NULL),
children_(),
valid_(is_valid_namespace(namespace_)),
dirty_(false)
{
	init(name_space);
}

persist_context::persist_context(const std::string &name_space, persist_context &child) : 
namespace_(get_namespace_leaf(name_space)), 
cfg_(),
parent_(NULL),
children_(),
valid_(is_valid_namespace(namespace_)),
dirty_(false)
{
	children_[child.namespace_] = &child;
	init(name_space);
	if (!cfg_.child(child.namespace_))
		cfg_.add_child(child.namespace_);
}

persist_context::~persist_context() {
	for (persist_context::child_map::iterator i = children_.begin(); i != children_.end(); i++)
		delete i->second;
}

bool persist_context::clear_var(std::string &global)
{
	if (cfg_.empty()) {
		load_persist_data(namespace_,cfg_);
	}

	config &cfg = cfg_.child("variables");
	bool exists = cfg.has_attribute(global);
	bool ret;
	if (!exists) {
		if (cfg.child(global)) {
			exists = true;
			std::string::iterator index_start = std::find(global.begin(),global.end(),'[');
			if (index_start != global.end())
			{
				const std::string::iterator index_end = std::find(global.begin(),global.end(),']');
				const std::string index_str(index_start+1,index_end);
				size_t index = static_cast<size_t>(lexical_cast_default<int>(index_str));
				cfg.remove_child(global,index);
			} else {
				cfg.clear_children(global);
			}
		}
	}
	if (exists) {
		cfg.remove_attribute(global);
		if (cfg.empty()) {
			cfg_.clear_children("variables");
			cfg_.remove_attribute("variables");
		}
		dirty_ = true;
		ret = save_context();
	} else {
		ret = exists;
	}
	return ret;
}

config persist_context::get_var(const std::string &global)
{
	config ret;
	if (cfg_.empty()) {
		load_persist_data(namespace_,cfg_,false);
	}

	config &cfg = cfg_.child("variables");
	if (cfg.child(global)) {
		ret.add_child(global,cfg.child(global));
	} else {
		ret = pack_scalar(global,cfg[global]);
	}
	return ret;
}
void persist_context::update_configs() {
	for (child_map::iterator i = children_.begin(); i != children_.end(); i++) {
		if (i->second->dirty()) {
			i->second->update_configs();
			cfg_.clear_children(i->second->namespace_);
			cfg_.remove_attribute(i->second->namespace_);
			cfg_.add_child(i->second->namespace_,i->second->cfg_);
			i->second->dirty_ = false;
		}
	}
}
bool persist_context::save_context() {
	if (parent_ != NULL)
		return parent_->save_context();
	update_configs();
	return save_persist_data(namespace_,cfg_);
}
bool persist_context::set_var(const std::string &global,const config &val)
{
	if (cfg_.empty()) {
		load_persist_data(namespace_,cfg_);
	}

	config &cfg = cfg_.child("variables");
	if (val.has_attribute(global)) {
		cfg[global] = val[global];
	} else {
		cfg.add_child(global,val.child(global));
	}
	dirty_ = true;
	return save_context();
}
