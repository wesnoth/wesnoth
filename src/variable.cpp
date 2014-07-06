/*
   Copyright (C) 2003 by David White <dave@whitevine.net>
   Copyright (C) 2005 - 2014 by Philippe Plantier <ayin@anathas.org>

   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 *  @file
 *  Manage WML-variables.
 */

#include "global.hpp"

#include "variable.hpp"

#include "formula_string_utils.hpp"
#include "game_data.hpp"
#include "log.hpp"
#include "resources.hpp"
#include "unit.hpp"
#include "unit_map.hpp"
#include "team.hpp"

#include <boost/foreach.hpp>
#include <boost/variant/static_visitor.hpp>

static lg::log_domain log_engine("engine");
#define LOG_NG LOG_STREAM(info, log_engine)
#define WRN_NG LOG_STREAM(warn, log_engine)
#define ERR_NG LOG_STREAM(err, log_engine)

vconfig::vconfig() :
	cache_(), cfg_(NULL)
{
}

vconfig::vconfig(const config & cfg, const boost::shared_ptr<config> & cache) :
	cache_(cache), cfg_(&cfg)
{
}

/**
 * Constructor from a config, with an option to manage memory.
 * @param[in] cfg           The "WML source" of the vconfig being constructed.
 * @param[in] manage_memory If true, a copy of @a cfg will be made, allowing the
 *                          vconfig to safely persist after @a cfg is destroyed.
 *                          If false, no copy is made, so @a cfg must be
 *                          guaranteed to persist as long as the vconfig will.
 *                          If in doubt, set to true; it is less efficient, but safe.
 * See also make_safe().
 */
vconfig::vconfig(const config &cfg, bool manage_memory) :
	cache_(manage_memory ? new config(cfg) : NULL),
	cfg_(manage_memory ? cache_.get() : &cfg)
{
}

/**
 * Default destructor, but defined here for possibly faster compiles
 * (templates sometimes can be rough on the compiler).
 */
vconfig::~vconfig()
{
}

vconfig vconfig::empty_vconfig()
{
	static const config empty_config;
	return vconfig(empty_config, false);
}

/**
 * This is just a wrapper for the default constructor; it exists for historical
 * reasons and to make it clear that default construction cannot be dereferenced
 * (in contrast to an empty vconfig).
 */
vconfig vconfig::unconstructed_vconfig()
{
	return vconfig();
}

/**
 * Ensures that *this manages its own memory, making it safe for *this to
 * outlive the config it was ultimately constructed from.
 * It is perfectly safe to call this for a vconfig that already manages its memory.
 * This does not work on a null() vconfig.
 */
void vconfig::make_safe() const
{
	// Nothing to do if we already manage our own memory.
	if ( memory_managed() )
		return;

	// Make a copy of our config.
	cache_.reset(new config(*cfg_));
	// Use our copy instead of the original.
	cfg_ = cache_.get();
}

config vconfig::get_parsed_config() const
{
	// Keeps track of insert_tag variables.
	static std::set<std::string> vconfig_recursion;

	config res;

	BOOST_FOREACH(const config::attribute &i, cfg_->attribute_range()) {
		res[i.first] = expand(i.first);
	}

	BOOST_FOREACH(const config::any_child &child, cfg_->all_children_range())
	{
		if (child.key == "insert_tag") {
			vconfig insert_cfg(child.cfg);
			const t_string& name = insert_cfg["name"];
			const t_string& vname = insert_cfg["variable"];
			if(!vconfig_recursion.insert(vname).second) {
				throw recursion_error("vconfig::get_parsed_config() infinite recursion detected, aborting");
			}
			try {
				variable_access_const vinfo = resources::gamedata->get_variable_access_read(vname);
				if(vinfo.explicit_index()) {
					res.add_child(name, vconfig(vinfo.as_container()).get_parsed_config());
				} else {
					config::const_child_itors range = vinfo.as_array();
					if(range.first == range.second) {
						res.add_child(name); //add empty tag
					}
					while(range.first != range.second) {
						res.add_child(name, vconfig(*range.first++).get_parsed_config());
					}
				}
			} 
			catch(const invalid_variablename_exception&)
			{
				res.add_child(name);
			}
			catch(recursion_error &err) {
				vconfig_recursion.erase(vname);
				WRN_NG << err.message << std::endl;
				if(vconfig_recursion.empty()) {
					res.add_child("insert_tag", insert_cfg.get_config());
				} else {
					// throw to the top [insert_tag] which started the recursion
					throw;
				}
			}
			vconfig_recursion.erase(vname);
		} else {
			res.add_child(child.key, vconfig(child.cfg).get_parsed_config());
		}
	}
	return res;
}

vconfig::child_list vconfig::get_children(const std::string& key) const
{
	vconfig::child_list res;

	BOOST_FOREACH(const config::any_child &child, cfg_->all_children_range())
	{
		if (child.key == key) {
			res.push_back(vconfig(child.cfg, cache_));
		} else if (child.key == "insert_tag") {
			vconfig insert_cfg(child.cfg);
			if(insert_cfg["name"] == key)
			{
				try
				{
					variable_access_const vinfo = resources::gamedata->get_variable_access_read(insert_cfg["variable"]);
					if(vinfo.explicit_index()) {
						res.push_back(vconfig(vinfo.as_container(), true));
					} else {
						config::const_child_itors range = vinfo.as_array();
						if(range.first == range.second) {
							//push back an empty tag
							res.push_back(empty_vconfig());
						}
						while(range.first != range.second)
						{
							res.push_back(vconfig(*range.first++, true));
						}
					}
				}
				catch(const invalid_variablename_exception&)
				{
					res.push_back(empty_vconfig());
				}
			}
		}
	}
	return res;
}

/**
 * Returns a child of *this whose key is @a key.
 * If no such child exists, returns an unconstructed vconfig (use null() to test
 * for this).
 */
vconfig vconfig::child(const std::string& key) const
{
	if (const config &natural = cfg_->child(key)) {
		return vconfig(natural, cache_);
	}
	BOOST_FOREACH(const config &ins, cfg_->child_range("insert_tag"))
	{
		vconfig insert_cfg(ins);
		if(insert_cfg["name"] == key) 
		{
			try
			{
				variable_access_const vinfo = resources::gamedata->get_variable_access_read(insert_cfg["variable"]);
				return vconfig(vinfo.as_container(), true);
			}
			catch(const invalid_variablename_exception&)
			{
				return empty_vconfig();
			}
		}
	}
	return unconstructed_vconfig();
}

/**
 * Returns whether or not *this has a child whose key is @a key.
 */
bool vconfig::has_child(const std::string& key) const
{
	if (cfg_->child(key)) {
		return true;
	}
	BOOST_FOREACH(const config &ins, cfg_->child_range("insert_tag"))
	{
		vconfig insert_cfg(ins);
		if(insert_cfg["name"] == key) {
			return true;
		}
	}
	return false;
}

namespace {
	struct vconfig_expand_visitor : boost::static_visitor<void>
	{
		config::attribute_value &result;

		vconfig_expand_visitor(config::attribute_value &r): result(r) {}
		template<typename T> void operator()(T const &) const {}
		void operator()(const std::string &s) const
		{
			result = utils::interpolate_variables_into_string(s, *(resources::gamedata));
		}
		void operator()(const t_string &s) const
		{
			result = utils::interpolate_variables_into_tstring(s, *(resources::gamedata));
		}
	};
}//unnamed namespace

config::attribute_value vconfig::expand(const std::string &key) const
{
	config::attribute_value val = (*cfg_)[key];
	if (resources::gamedata)
		val.apply_visitor(vconfig_expand_visitor(val));
	return val;
}

vconfig::all_children_iterator::all_children_iterator(const Itor &i) :
	i_(i), inner_index_(0), cache_()
{
}

vconfig::all_children_iterator::all_children_iterator(const Itor &i, const boost::shared_ptr<config> & cache) :
	i_(i), inner_index_(0), cache_(cache)
{
}

vconfig::all_children_iterator& vconfig::all_children_iterator::operator++()
{
	if (inner_index_ >= 0 && i_->key == "insert_tag")
	{
		try
		{
			variable_access_const vinfo = resources::gamedata->get_variable_access_read(vconfig(i_->cfg)["variable"]);
			if(!vinfo.explicit_index()) {
				config::const_child_itors range = vinfo.as_array();
				if (++inner_index_ < std::distance(range.first, range.second)) {
					return *this;
				}
				inner_index_ = 0;
			}
		}
		catch(const invalid_variablename_exception&)
		{
		}
	}
	++i_;
	return *this;
}

vconfig::all_children_iterator vconfig::all_children_iterator::operator++(int)
{
	vconfig::all_children_iterator i = *this;
	this->operator++();
	return i;
}

vconfig::all_children_iterator::reference vconfig::all_children_iterator::operator*() const
{
	return value_type(get_key(), get_child());
}

vconfig::all_children_iterator::pointer vconfig::all_children_iterator::operator->() const
{
	pointer_proxy p = { value_type(get_key(), get_child()) };
	return p;
}


std::string vconfig::all_children_iterator::get_key() const
{
	const std::string &key = i_->key;
	if (inner_index_ >= 0 && key == "insert_tag") {
		return vconfig(i_->cfg)["name"];
	}
	return key;
}

vconfig vconfig::all_children_iterator::get_child() const
{
	if (inner_index_ >= 0 && i_->key == "insert_tag")
	{
		try
		{
			variable_access_const vinfo = resources::gamedata->get_variable_access_read(vconfig(i_->cfg)["variable"]);
			if(inner_index_ == 0) {
				return vconfig(vinfo.as_container(), true);
			} else {
				config::const_child_itors r = vinfo.as_array();
				std::advance(r.first, inner_index_);
				return vconfig(*r.first, true);
			}
		}
		catch(const invalid_variablename_exception&)
		{
			return empty_vconfig();
		}
	}
	return vconfig(i_->cfg, cache_);
}

bool vconfig::all_children_iterator::operator==(const all_children_iterator &i) const
{
	return i_ == i.i_ && inner_index_ == i.inner_index_;
}

vconfig::all_children_iterator vconfig::ordered_begin() const
{
	return all_children_iterator(cfg_->ordered_begin(), cache_);
}

vconfig::all_children_iterator vconfig::ordered_end() const
{
	return all_children_iterator(cfg_->ordered_end(), cache_);
}

scoped_wml_variable::scoped_wml_variable(const std::string& var_name) :
	previous_val_(),
	var_name_(var_name),
	activated_(false)
{
	if (resources::gamedata)
		resources::gamedata->scoped_variables.push_back(this);
}

config &scoped_wml_variable::store(const config &var_value)
{
	try
	{
		BOOST_FOREACH(const config &i, resources::gamedata->get_variables().child_range(var_name_)) {
			previous_val_.add_child(var_name_, i);
		}
		resources::gamedata->clear_variable_cfg(var_name_);
		config &res = resources::gamedata->add_variable_cfg(var_name_, var_value);
		LOG_NG << "scoped_wml_variable: var_name \"" << var_name_ << "\" has been auto-stored.\n";
		activated_ = true;
		return res;
	}
	catch(const invalid_variablename_exception&)
	{
		assert(false && "invalid variable name of autostored varaible");
		throw "assertion ignored";
	}

}

scoped_wml_variable::~scoped_wml_variable()
{
	if(activated_) {
		resources::gamedata->clear_variable_cfg(var_name_);
		BOOST_FOREACH(const config &i, previous_val_.child_range(var_name_)) 
		{
			try
			{
				resources::gamedata->add_variable_cfg(var_name_, i);
			}
			catch(const invalid_variablename_exception&)
			{
			}
		}
		LOG_NG << "scoped_wml_variable: var_name \"" << var_name_ << "\" has been reverted.\n";
	}
	if (resources::gamedata) {
		assert(resources::gamedata->scoped_variables.back() == this);
		resources::gamedata->scoped_variables.pop_back();
	}
}

void scoped_xy_unit::activate()
{
	map_location loc = map_location(x_, y_);
	unit_map::const_iterator itor = umap_.find(loc);
	if(itor != umap_.end()) {
		config &tmp_cfg = store();
		itor->write(tmp_cfg);
		tmp_cfg["x"] = x_ + 1;
		tmp_cfg["y"] = y_ + 1;
		LOG_NG << "auto-storing $" << name() << " at (" << loc << ")\n";
	} else {
		ERR_NG << "failed to auto-store $" << name() << " at (" << loc << ")" << std::endl;
	}
}

void scoped_weapon_info::activate()
{
	if (data_) {
		store(data_);
	}
}

void scoped_recall_unit::activate()
{
	const std::vector<team>& teams = teams_manager::get_teams();
	std::vector<team>::const_iterator team_it;
	for (team_it = teams.begin(); team_it != teams.end(); ++team_it) {
		if (team_it->save_id() == player_ )
			break;
	}

	if(team_it != teams.end()) {
		if(team_it->recall_list().size() > recall_index_) {
			config &tmp_cfg = store();
			team_it->recall_list()[recall_index_]->write(tmp_cfg);
			tmp_cfg["x"] = "recall";
			tmp_cfg["y"] = "recall";
			LOG_NG << "auto-storing $" << name() << " for player: " << player_
				<< " at recall index: " << recall_index_ << '\n';
		} else {
			ERR_NG << "failed to auto-store $" << name() << " for player: " << player_
				<< " at recall index: " << recall_index_ << '\n';
		}
	} else {
		ERR_NG << "failed to auto-store $" << name() << " for player: " << player_ << '\n';
	}
}


