/*
   Copyright (C) 2003 by David White <dave@whitevine.net>
   Copyright (C) 2005 - 2018 by Philippe Plantier <ayin@anathas.org>

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

#include "variable.hpp"

#include "formula/string_utils.hpp"
#include "game_board.hpp"
#include "game_data.hpp"
#include "log.hpp"
#include "resources.hpp"
#include "units/unit.hpp"
#include "units/map.hpp"
#include "team.hpp"

#include <boost/variant/static_visitor.hpp>

static lg::log_domain log_engine("engine");
#define LOG_NG LOG_STREAM(info, log_engine)
#define WRN_NG LOG_STREAM(warn, log_engine)
#define ERR_NG LOG_STREAM(err, log_engine)
namespace
{
	const config as_nonempty_range_default("_");
	config::const_child_itors as_nonempty_range(const std::string& varname)
	{
		config::const_child_itors range = as_nonempty_range_default.child_range("_");

		if(resources::gamedata) {
			config::const_child_itors temp_range = resources::gamedata->get_variable_access_read(varname).as_array();

			if(!temp_range.empty()) {
				range = temp_range;
			}
		}

		return range;
	}
}

config_attribute_value config_variable_set::get_variable_const(const std::string &id) const {
	try {
		variable_access_const variable(id, cfg_);
		return variable.as_scalar();
	} catch(const invalid_variablename_exception&) {
		ERR_NG << "invalid variablename " << id << "\n";
		return config::attribute_value();
	}
}

const config vconfig::default_empty_config = config();


vconfig::vconfig() :
	cache_(), cfg_(&default_empty_config)
{
}

vconfig::vconfig(const config & cfg, const std::shared_ptr<const config> & cache) :
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
	cache_(manage_memory ? new config(cfg) : nullptr),
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
const vconfig& vconfig::make_safe() const
{
	// Nothing to do if we already manage our own memory.
	if ( memory_managed() )
		return *this;

	// Make a copy of our config.
	cache_.reset(new config(*cfg_));
	// Use our copy instead of the original.
	cfg_ = cache_.get();
	return *this;
}

config vconfig::get_parsed_config() const
{
	// Keeps track of insert_tag variables.
	static std::set<std::string> vconfig_recursion;

	config res;

	for (const config::attribute &i : cfg_->attribute_range()) {
		res[i.first] = expand(i.first);
	}

	for (const config::any_child &child : cfg_->all_children_range())
	{
		if (child.key == "insert_tag") {
			vconfig insert_cfg(child.cfg);
			std::string name = insert_cfg["name"];
			std::string vname = insert_cfg["variable"];
			if(!vconfig_recursion.insert(vname).second) {
				throw recursion_error("vconfig::get_parsed_config() infinite recursion detected, aborting");
			}
			try
			{
				config::const_child_itors range = as_nonempty_range(vname);
				for (const config& ch : range)
				{
					res.add_child(name, vconfig(ch).get_parsed_config());
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

	for (const config::any_child &child : cfg_->all_children_range())
	{
		if (child.key == key) {
			res.push_back(vconfig(child.cfg, cache_));
		} else if (child.key == "insert_tag") {
			vconfig insert_cfg(child.cfg);
			if(insert_cfg["name"] == key)
			{
				try
				{
					config::const_child_itors range = as_nonempty_range(insert_cfg["variable"]);
					for (const config& ch : range)
					{
						res.push_back(vconfig(ch, true));
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

size_t vconfig::count_children(const std::string& key) const
{
	size_t n = 0;

	for (const config::any_child &child : cfg_->all_children_range())
	{
		if (child.key == key) {
			n++;
		} else if (child.key == "insert_tag") {
			vconfig insert_cfg(child.cfg);
			if(insert_cfg["name"] == key)
			{
				try
				{
					config::const_child_itors range = as_nonempty_range(insert_cfg["variable"]);
					n += range.size();
				}
				catch(const invalid_variablename_exception&)
				{
					n++;
				}
			}
		}
	}
	return n;
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
	for (const config &ins : cfg_->child_range("insert_tag"))
	{
		vconfig insert_cfg(ins);
		if(insert_cfg["name"] == key)
		{
			try
			{
				config::const_child_itors range = as_nonempty_range(insert_cfg["variable"]);
				return vconfig(range.front(), true);
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
	for (const config &ins : cfg_->child_range("insert_tag"))
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
		template<typename T> void operator()(const T&) const {}
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

vconfig::attribute_iterator::reference vconfig::attribute_iterator::operator*() const
{
	config::attribute val = *i_;
	if(resources::gamedata) {
		val.second.apply_visitor(vconfig_expand_visitor(val.second));
	}
	return val;
}

vconfig::attribute_iterator::pointer vconfig::attribute_iterator::operator->() const
{
	config::attribute val = *i_;
	if(resources::gamedata) {
		val.second.apply_visitor(vconfig_expand_visitor(val.second));
	}
	pointer_proxy p {val};
	return p;
}

vconfig::all_children_iterator::all_children_iterator(const Itor &i) :
	i_(i), inner_index_(0), cache_()
{
}

vconfig::all_children_iterator::all_children_iterator(const Itor &i, const std::shared_ptr<const config> & cache) :
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

			config::const_child_itors range = vinfo.as_array();

			if (++inner_index_ < int(range.size()))
			{
				return *this;
			}

		}
		catch(const invalid_variablename_exception&)
		{
		}
		inner_index_ = 0;
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

vconfig::all_children_iterator& vconfig::all_children_iterator::operator--()
{
	if(inner_index_ >= 0 && i_->key == "insert_tag") {
		if(--inner_index_ >= 0) {
			return *this;
		}
		inner_index_ = 0;
	}
	--i_;
	return *this;
}

vconfig::all_children_iterator vconfig::all_children_iterator::operator--(int)
{
	vconfig::all_children_iterator i = *this;
	this->operator--();
	return i;
}

vconfig::all_children_iterator::reference vconfig::all_children_iterator::operator*() const
{
	return value_type(get_key(), get_child());
}

vconfig::all_children_iterator::pointer vconfig::all_children_iterator::operator->() const
{
	pointer_proxy p { value_type(get_key(), get_child()) };
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
			config::const_child_itors range = as_nonempty_range(vconfig(i_->cfg)["variable"]);

			range.advance_begin(inner_index_);
			return vconfig(range.front(), true);
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
		for (const config &i : resources::gamedata->get_variables().child_range(var_name_)) {
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
	if(!resources::gamedata) {
		return;
	}

	if(activated_) {
		resources::gamedata->clear_variable_cfg(var_name_);
		for(const config &i : previous_val_.child_range(var_name_))
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

	assert(resources::gamedata->scoped_variables.back() == this);
	resources::gamedata->scoped_variables.pop_back();
}

void scoped_xy_unit::activate()
{
	unit_map::const_iterator itor = umap_.find(loc_);
	if(itor != umap_.end()) {
		config &tmp_cfg = store();
		itor->write(tmp_cfg);
		tmp_cfg["x"] = loc_.wml_x();
		tmp_cfg["y"] = loc_.wml_y();
		LOG_NG << "auto-storing $" << name() << " at (" << loc_ << ")\n";
	} else {
		ERR_NG << "failed to auto-store $" << name() << " at (" << loc_ << ")" << std::endl;
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
	assert(resources::gameboard);

	const std::vector<team>& teams = resources::gameboard->teams();

	std::vector<team>::const_iterator team_it = std::find_if(teams.begin(), teams.end(), [&](const team& t) { return t.save_id() == player_; });

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


