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

#include "filesystem.hpp"
#include "persist_context.hpp"
#include "persist_manager.hpp"
#include "serialization/binary_or_text.hpp"

static std::string get_persist_cfg_name(const std::string &name_space) {
	return (get_dir(get_user_data_dir() + "/persist/") + name_space + ".cfg");
}

bool persist_manager::save_data(const std::string &name_space, config &cfg) {
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

bool persist_manager::load_data(const std::string &name_space, config &cfg, const bool create_if_missing) {
	bool success = false;
	std::string cfg_dir = get_dir(get_user_data_dir() + "/persist");
	create_directory_if_missing(cfg_dir);

	std::string cfg_name = get_persist_cfg_name(name_space);
	if (!cfg_name.empty()) {
		scoped_istream file_stream = istream_file(cfg_name);
		if (file_stream->fail()) {
			if (create_if_missing) {
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
persist_context &persist_manager::get_context(const std::string &ns) 
{
	persist_context::name_space name(ns,true);
	std::string key(name.root_);
	context_map::iterator i = contexts_.find(key);
	if (i == contexts_.end()) {
		contexts_[key] = new persist_context(key);
	}
	persist_context *ret = contexts_[key];
	if (ret->get_node() != ns)
		ret->set_node(name.descendants_);
	return *ret;
};
