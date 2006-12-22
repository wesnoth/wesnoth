/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@verizon.net>
   Copyright (C) 2005 by Philippe Plantier <ayin@anathas.org>

   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include "variable.hpp"
#include "gamestatus.hpp"
#include "config.hpp"
#include <iostream>

namespace
{
	// FIXME: the variable repository should be a class of variable.hpp,
	// and not the game_state.
	game_state* repos = NULL;
}

vconfig::vconfig() :
	cfg_(NULL)
{
}

vconfig::vconfig(const config* cfg) :
	cfg_(cfg)
{
}

vconfig& vconfig::operator=(const vconfig cfg)
{
	cfg_ = cfg.cfg_;
	return *this;
}

vconfig& vconfig::operator=(const config* cfg)
{
	cfg_ = cfg;
	return *this;
}

bool vconfig::null() const
{
	return cfg_ == NULL;
}

const config& vconfig::get_config() const
{
	return *cfg_;
}

const config vconfig::get_parsed_config() const
{
	config res;

	for(string_map::const_iterator itor = cfg_->values.begin();
			itor != cfg_->values.end(); ++itor) {

		res[itor->first] = expand(itor->first);
	}

	for(config::all_children_iterator child = cfg_->ordered_begin();
			child != cfg_->ordered_end(); ++child) {
			vconfig tmp_vconf= (*child).second;
			tmp_vconf.local_vars_ = local_vars_;


		res.add_child(*(*child).first, tmp_vconf.get_parsed_config());
	}
	return res;
}

vconfig::child_list vconfig::get_children(const std::string& key) const
{
	const config::child_list& list = cfg_->get_children(key);
	vconfig::child_list res(list.size());
	std::copy(list.begin(), list.end(),res.begin());
	for(vconfig::child_list::iterator itor = res.begin(); itor != res.end() ;itor++) {
		itor->local_vars_ = local_vars_;
	}
	return res;
}

vconfig vconfig::child(const std::string& key) const
{
	vconfig tmp = cfg_->child(key);
	tmp.local_vars_ = local_vars_;
	return tmp;
}

const t_string vconfig::operator[](const std::string& key) const
{
	return expand(key);
}

const t_string vconfig::expand(const std::string& key) const
{
	const t_string& val = (*cfg_)[key];

	if(!val.str().empty() && val.str()[0] == '$') {
		std::string tmp = val.str();
		// stupid const
		config grr = local_vars_;
		// first expand local variables
		tmp = utils::interpolate_variables_into_string(val.str(),grr);
		if(tmp.empty()) tmp = val.str();
		// now expand global variables
		if(repos != NULL && !tmp.empty() && tmp[0] == '$') {
			tmp =  repos->get_variable(tmp.substr(1));
			if(tmp.empty()) tmp = val.str();
		}
		return tmp;
	} else {
		return val;
	}
}

const t_string vconfig::get_attribute(const std::string& key) const
{
	return (*cfg_)[key];
}

void vconfig::add_local_var(std::string var_name, config& var)
{
	local_vars_.add_child(var_name,var);
}
void vconfig::rem_local_var(std::string var_name)
{
	local_vars_.clear_children(var_name);
}
namespace variable
{
	manager::manager(game_state* repository)
	{
		repos = repository;
	}

	manager::~manager()
	{
		repos = NULL;
	}
}

