/* $Id$ */
/*
   Copyright (C) 2008 by Pauli Nieminen <paniemin@cc.hut.fi>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef CONFIG_CACHE_HPP_INCLUDED
#define CONFIG_CACHE_HPP_INCLUDED

#include <boost/utility.hpp>
#include "config.hpp"
#include "serialization/preprocessor.hpp"

namespace game_config {


	/**
	 * Singleton object to manage game configs
	 * and cache reading.
	 **/
class config_cache : private boost::noncopyable {
	static config_cache cache_;

	config game_config_;
	bool force_valid_cache_, use_cache_, dirty_;
	std::string config_root_, user_config_root_;
	preproc_map defines_map_;

	void read_configs(config&, std::string&);

	protected:
	config_cache();


	std::string get_config_root() const;
	std::string get_user_config_root() const;
	const preproc_map& get_preproc_map() const;
	void load_configs(config& cfg, bool recheck_cache);

	public:
	static config_cache& instance();

	void set_config_root(const std::string&);
	void set_user_config_root(const std::string&);

	config& get_config(bool recheck_cache = false);
	
	void clear_defines();
	void add_define(const std::string& define);

	void reload_translations();

	void set_use_cache(bool use);
};

}
#endif
