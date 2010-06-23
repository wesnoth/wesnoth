/* $Id$ */
/*
   Copyright (C) 2010 by Jody Northup
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include "log.hpp"
#include "persist_context.hpp"
#include "persist_manager.hpp"
#include "resources.hpp"
#include "util.hpp"

config pack_scalar(const std::string &name, const t_string &val)
{
	config cfg;
	cfg[name] = val;
	return cfg;
}
persist_context &persist_context::add_child(const std::string& /*key*/)  {
//	children_[key] = new persist_context(namespace_.namespace_ + "." + key,this);
//	children_[key]->cfg_.child_or_add(key);
	return *this;//(children_[key]);
}
void persist_context::load() {
	resources::persist->load_data(namespace_.root_,cfg_,false);
}

persist_context::persist_context(const std::string &name_space) 
	: cfg_()
	, namespace_(name_space,true)
	, root_node_(namespace_.root_,this,cfg_)
	, active_(&root_node_)
	, valid_(namespace_.valid())
{
	load();
	root_node_.init();
	active_ = &(root_node_.child(namespace_.next()));
}

bool persist_context::clear_var(std::string &global)
{
//	if (cfg_.empty()) {
//		load_persist_data(namespace_.root(),cfg_);
//	}

	config &cfg = active_->cfg_.child("variables");
	bool ret = cfg;
	if (ret) {
		bool exists = cfg.has_attribute(global);
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
				active_->cfg_.clear_children("variables");
				active_->cfg_.remove_attribute("variables");
				while ((active_->cfg_.empty()) && (active_->parent_ != NULL)) {
					active_ = active_->parent_;
					active_->remove_child(namespace_.node_);
					namespace_ = namespace_.prev();
				}
			}
	//		dirty_ = true;
			ret = save_context();
		} else {
			ret = exists;
		}
	}
	return ret;
}

config persist_context::get_var(const std::string &global) const
{
	config ret;
//	if (cfg_.empty()) {
//		load_persist_data(namespace_,cfg_,false);
//	}

	config &cfg = active_->cfg_.child("variables");
	if (cfg) {
		if (cfg.child(global)) {
			ret.add_child(global,cfg.child(global));
		} else {
			ret = pack_scalar(global,cfg[global]);
		}
	} else {
		ret = pack_scalar(global,"");
	}
	return ret;
}
bool persist_context::save_context() {
	return resources::persist->save_data(namespace_.root_,cfg_);
}
bool persist_context::set_var(const std::string &global,const config &val)
{
//	if (cfg_.empty()) {
//		load_persist_data(namespace_,cfg_);
//	}

	config &cfg = active_->cfg_.child_or_add("variables");
	if (val.has_attribute(global)) {
		cfg[global] = val[global];
	} else {
		cfg.add_child(global,val.child(global));
	}
//	dirty_ = true;
	return save_context();
}
void persist_context::set_node(const std::string &name) {
	active_ = &(root_node_.child(name));
	namespace_ = name_space(namespace_.namespace_ + "." + name);
}
std::string persist_context::get_node() {
	return namespace_.namespace_;
}