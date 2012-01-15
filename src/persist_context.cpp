/* $Id$ */
/*
   Copyright (C) 2010 - 2012 by Jody Northup
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include "filesystem.hpp"
#include "log.hpp"
#include "persist_context.hpp"
#include "persist_manager.hpp"
#include "serialization/binary_or_text.hpp"
#include "serialization/parser.hpp"
#include "util.hpp"

config pack_scalar(const std::string &name, const t_string &val)
{
	config cfg;
	cfg[name] = val;
	return cfg;
}

static std::string get_persist_cfg_name(const std::string &name_space) {
	return (get_dir(get_user_data_dir() + "/persist/") + name_space + ".cfg");
}

void persist_file_context::load() {
	std::string cfg_dir = get_dir(get_user_data_dir() + "/persist");
	create_directory_if_missing(cfg_dir);

	std::string cfg_name = get_persist_cfg_name(namespace_.root_);
	if (file_exists(cfg_name) && !is_directory(cfg_name)) {
		scoped_istream file_stream = istream_file(cfg_name);
		if (!(file_stream->fail())) {
			try {
				read(cfg_,*file_stream);
			} catch (config::error &err) {
				LOG_PERSIST << err.message;
			}
		}
	}
}

persist_file_context::persist_file_context(const std::string &name_space)
	: persist_context(name_space)
{
	load();
	root_node_.init();
	active_ = &(root_node_.child(namespace_.next()));
}

bool persist_file_context::clear_var(const std::string &global, bool immediate)
{
//	if (cfg_.empty()) {
//		load_persist_data(namespace_.root(),cfg_);
//	}

	config bak;
	config bactive;
	if (immediate) {
		bak = cfg_;
		bactive = active_->cfg_.child_or_add("variables");
		load();
		root_node_.init();
	}
	config &cfg = active_->cfg_.child_or_add("variables");
	bool ret(cfg);
	if (ret) {
		bool exists = cfg.has_attribute(global);
		if (!exists) {
			if (cfg.child(global)) {
				exists = true;
				std::string::const_iterator index_start = std::find(global.begin(),global.end(),'[');
				if (index_start != global.end())
				{
					const std::string::const_iterator index_end = std::find(global.begin(),global.end(),']');
					const std::string index_str(index_start+1,index_end);
					size_t index = static_cast<size_t>(lexical_cast_default<int>(index_str));
					cfg.remove_child(global,index);
					if (immediate) bactive.remove_child(global,index);
				} else {
					cfg.clear_children(global);
					if (immediate) bactive.clear_children(global);
				}
			}
		}
		if (exists) {
			cfg.remove_attribute(global);
			if (immediate) bactive.remove_attribute(global);
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
			if (!in_transaction_)
				ret = save_context();
			else if (immediate) {
				ret = save_context();
				cfg_ = bak;
				root_node_.init();
				active_->cfg_.clear_children("variables");
				active_->cfg_.remove_attribute("variables");
				active_->cfg_.add_child("variables",bactive);
				config &cfg = active_->cfg_.child("variables");
				if (cfg.empty()) {
					active_->cfg_.clear_children("variables");
					active_->cfg_.remove_attribute("variables");
					while ((active_->cfg_.empty()) && (active_->parent_ != NULL)) {
						active_ = active_->parent_;
						active_->remove_child(namespace_.node_);
						namespace_ = namespace_.prev();
					}
				}
			} else
				ret = true;
		} else {
			ret = exists;
		}
	}
	return ret;
}

config persist_file_context::get_var(const std::string &global) const
{
	config ret;
//	if (cfg_.empty()) {
//		load_persist_data(namespace_,cfg_,false);
//	}

	config &cfg = active_->cfg_.child("variables");
	if (cfg) {
		size_t arrsize = cfg.child_count(global);
		if (arrsize > 0) {
			for (size_t i = 0; i < arrsize; i++)
				ret.add_child(global,cfg.child(global,i));
		} else {
			ret = pack_scalar(global,cfg[global]);
		}
	} else {
		ret = pack_scalar(global,"");
	}
	return ret;
}
bool persist_file_context::save_context() {
	bool success = false;

	std::string cfg_name = get_persist_cfg_name(namespace_.root_);
	if (!cfg_name.empty()) {
		if (cfg_.empty()) {
			success = delete_directory(cfg_name);
		} else {
			scoped_ostream out = ostream_file(cfg_name);
			if (!out->fail())
			{
				config_writer writer(*out,false);
				try {
					writer.write(cfg_);
					success = true;
				} catch(config::error &err) {
					LOG_PERSIST << err.message;
					success = false;
				}
			}
		}
	}
	return success;
}
bool persist_file_context::set_var(const std::string &global,const config &val, bool immediate)
{
//	if (cfg_.empty()) {
//		load_persist_data(namespace_,cfg_);
//	}
	config bak;
	config bactive;
	if (immediate) {
		bak = cfg_;
		bactive = active_->cfg_.child_or_add("variables");
		load();
		root_node_.init();
	}

	config &cfg = active_->cfg_.child_or_add("variables");
	if (val.has_attribute(global)) {
		if (val[global].empty()) {
			clear_var(global,immediate);
		} else {
			cfg[global] = val[global];
			if (immediate) bactive[global] = val[global];
		}
	} else {
		cfg.clear_children(global);
		cfg.append(val);
		if (immediate) {
			bactive.clear_children(global);
			bactive.append(val);
		}
	}
//	dirty_ = true;
	if (!in_transaction_)
		return save_context();
	else if (immediate) {
		bool ret = save_context();
		cfg_ = bak;
		root_node_.init();
		active_->cfg_.clear_children("variables");
		active_->cfg_.remove_attribute("variables");
		active_->cfg_.add_child("variables",bactive);
		return ret;
	} else
		return true;
}
void persist_context::set_node(const std::string &name) {
	active_ = &(root_node_.child(name));
	namespace_ = name_space(namespace_.namespace_ + "." + name);
}

std::string persist_context::get_node() const
{
	return namespace_.namespace_;
}

