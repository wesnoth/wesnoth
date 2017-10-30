/*
   Copyright (C) 2012 - 2017 by Boldizsár Lipka <lipkab@zoho.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "game_initialization/depcheck.hpp"

#include <algorithm>

#include "formula/string_utils.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "utils/general.hpp"

#include "gui/dialogs/depcheck_confirm_change.hpp"
#include "gui/dialogs/depcheck_select_new.hpp"
#include "gui/dialogs/message.hpp"

static lg::log_domain log_mp_create_depcheck("mp/create/depcheck");
#define DBG_MP LOG_STREAM(debug, log_mp_create_depcheck)

namespace
{
// helper function
void copy_keys(config& out, const config& in, const std::string& type, bool copy_force_key = false)
{
	if(in.has_attribute("allow_" + type)) {
		out["allow_" + type] = in["allow_" + type];
	} else if(in.has_attribute("disallow_" + type)) {
		out["disallow_" + type] = in["disallow_" + type];
	}

	if(in.has_attribute("ignore_incompatible_" + type)) {
		out["ignore_incompatible_" + type] = in["ignore_incompatible_" + type];
	}

	if(copy_force_key) {
		if(in.has_attribute("force_" + type)) {
			out["force_" + type] = in["force_" + type];
		}
	}
}
} // anonymous namespace

namespace ng
{
namespace depcheck
{
manager::manager(const config& gamecfg, bool mp, CVideo& video)
	: video_(video)
	, depinfo_()
	, era_()
	, scenario_()
	, mods_()
	, prev_era_()
	, prev_scenario_()
	, prev_mods_()
{
	DBG_MP << "Initializing the dependency manager" << std::endl;

	for(const config& cfg : gamecfg.child_range("modification")) {
		component_availability type = cfg["type"].to_enum<component_availability>(component_availability::HYBRID);

		if((type != component_availability::MP || mp) && (type != component_availability::SP || !mp)) {
			config info;
			info["id"] = cfg["id"];
			info["name"] = cfg["name"];

			copy_keys(info, cfg, "scenario");
			copy_keys(info, cfg, "era");
			copy_keys(info, cfg, "modification");

			depinfo_.add_child("modification", info);
		}
	}

	for(const config& cfg : gamecfg.child_range("era")) {
		component_availability type = cfg["type"].to_enum<component_availability>(component_availability::MP);

		if((type != component_availability::MP || mp) && (type != component_availability::SP || !mp)) {
			config info;
			info["id"] = cfg["id"];
			info["name"] = cfg["name"];

			copy_keys(info, cfg, "scenario");
			copy_keys(info, cfg, "modification", true);

			depinfo_.add_child("era", info);
		}
	}

	for(const config& cfg : gamecfg.child_range("multiplayer")) {
		if(cfg["allow_new_game"].to_bool(true)) {
			config info;
			info["id"] = cfg["id"];
			info["name"] = cfg["name"];

			copy_keys(info, cfg, "era");
			copy_keys(info, cfg, "modification", true);

			depinfo_.add_child("scenario", info);
		}
	}

	for(const config& cfg : gamecfg.child_range("campaign")) {
		config info;
		info["id"] = cfg["id"];
		info["name"] = cfg["name"];
		info["allow_era_choice"] = cfg["allow_era_choice"];

		copy_keys(info, cfg, "era");
		copy_keys(info, cfg, "modification", true);

		depinfo_.add_child("scenario", info);
	}
}

void manager::save_state()
{
	DBG_MP << "Saving current state" << std::endl;
	prev_era_ = era_;
	prev_scenario_ = scenario_;
	prev_mods_ = mods_;
}

void manager::revert()
{
	DBG_MP << "Restoring previous state" << std::endl;
	era_ = prev_era_;
	scenario_ = prev_scenario_;
	mods_ = prev_mods_;
}

bool manager::exists(const elem& e) const
{
	for(const config& cfg : depinfo_.child_range(e.type)) {
		if(cfg["id"] == e.id) {
			return true;
		}
	}

	return false;
}

std::string manager::find_name_for(const elem& e) const
{
	const config& cfg = depinfo_.find_child(e.type, "id", e.id);
	return cfg["name"];
}

std::vector<std::string> manager::get_required_not_installed(const elem& e) const
{
	std::vector<std::string> result;

	std::vector<std::string> items = get_required(e);

	for(const std::string& str : items) {
		if(!exists(elem(str, "modification"))) {
			result.push_back(str);
		}
	}

	return result;
}

std::vector<std::string> manager::get_required(const elem& e) const
{
	std::vector<std::string> result;

	if(e.type == "modification") {
		return result;
	}

	config data = depinfo_.find_child(e.type, "id", e.id);

	if(data.has_attribute("force_modification")) {
		result = utils::split(data["force_modification"], ',');
	}

	return result;
}

std::vector<std::string> manager::get_required_not_enabled(const elem& e) const
{
	std::vector<std::string> required = get_required(e);
	std::vector<std::string> result;

	for(std::string str : required) {
		if(!utils::contains(mods_, str)) {
			result.push_back(str);
		}
	}

	return result;
}

std::vector<std::string> manager::get_conflicting_enabled(const elem& e) const
{
	std::vector<std::string> result;

	for(const std::string& mod : mods_) {
		if(conflicts(elem(mod, "modification"), e)) {
			result.push_back(mod);
		}
	}

	return result;
}

bool manager::conflicts(const elem& elem1, const elem& elem2, bool directonly) const
{
	if(elem1 == elem2) {
		return false;
	}

	// We ignore nonexistant elements at this point, they will generate
	// errors in change_era()/change_scenario() anyways.
	if(!exists(elem1) || !exists(elem2)) {
		return false;
	}

	config data1 = depinfo_.find_child(elem1.type, "id", elem1.id);
	config data2 = depinfo_.find_child(elem2.type, "id", elem2.id);

	// Whether we should skip the check entirely
	if(data1.has_attribute("ignore_incompatible_" + elem2.type)) {
		std::vector<std::string> ignored = utils::split(data1["ignore_incompatible_" + elem2.type]);

		if(utils::contains(ignored, elem2.id)) {
			return false;
		}
	}

	if(data2.has_attribute("ignore_incompatible_" + elem1.type)) {
		std::vector<std::string> ignored = utils::split(data2["ignore_incompatible_" + elem1.type]);

		if(utils::contains(ignored, elem1.id)) {
			return false;
		}
	}

	if((elem1.type == "era" && data2["allow_era_choice"].to_bool(false)) ||(elem2.type == "era" && data1["allow_era_choice"].to_bool(false))) {
		return false;
	}

	bool result = false;

	// Checking for direct conflicts between elem1 and elem2
	if(data1.has_attribute("allow_" + elem2.type)) {
		std::vector<std::string> allowed = utils::split(data1["allow_" + elem2.type]);

		result = !utils::contains(allowed, elem2.id) && !requires(elem1, elem2);
	} else if(data1.has_attribute("disallow_" + elem2.type)) {
		std::vector<std::string> disallowed = utils::split(data1["disallow_" + elem2.type]);

		result = utils::contains(disallowed, elem2.id);
	}

	if(data2.has_attribute("allow_" + elem1.type)) {
		std::vector<std::string> allowed = utils::split(data2["allow_" + elem1.type]);

		result = result || (!utils::contains(allowed, elem1.id) && !requires(elem2, elem1));
	} else if(data2.has_attribute("disallow_" + elem1.type)) {
		std::vector<std::string> disallowed = utils::split(data2["disallow_" + elem1.type]);

		result = result || utils::contains(disallowed, elem1.id);
	}

	if(result) {
		return true;
	}

	// Checking for indirect conflicts (i.e. conflicts between dependencies)
	if(!directonly) {
		std::vector<std::string> req1 = get_required(elem1), req2 = get_required(elem2);

		for(const std::string& s : req1) {
			elem m(s, "modification");

			if(conflicts(elem2, m, true)) {
				return true;
			}
		}

		for(const std::string& s : req2) {
			elem m(s, "modification");

			if(conflicts(elem1, m, true)) {
				return true;
			}
		}

		for(const std::string& id1 : req1) {
			elem m1(id1, "modification");

			for(const std::string& id2 : req2) {
				elem m2(id2, "modification");

				if(conflicts(m1, m2)) {
					return true;
				}
			}
		}
	}

	return false;
}

bool manager::requires(const elem& elem1, const elem& elem2) const
{
	if(elem2.type != "modification") {
		return false;
	}

	config data = depinfo_.find_child(elem1.type, "id", elem1.id);

	if(data.has_attribute("force_modification")) {
		std::vector<std::string> required = utils::split(data["force_modification"]);

		return utils::contains(required, elem2.id);
	}

	return false;
}

void manager::try_era(const std::string& id, bool force)
{
	save_state();

	if(force) {
		era_ = id;
	} else if(!change_era(id)) {
		revert();
	}
}

void manager::try_scenario(const std::string& id, bool force)
{
	save_state();

	if(force) {
		scenario_ = id;
	} else if(!change_scenario(id)) {
		revert();
	}
}

void manager::try_modifications(const std::vector<std::string>& ids, bool force)
{
	save_state();

	if(force) {
		mods_ = ids;
	} else if(!change_modifications(ids)) {
		revert();
	}
}

void manager::try_modification_by_index(int index, bool activate, bool force)
{
	std::string id = depinfo_.child("modification", index)["id"];
	std::vector<std::string> mods_copy = mods_;

	if(activate) {
		if(std::find(mods_copy.begin(), mods_copy.end(), id) == mods_copy.end()) {
			mods_copy.push_back(id);
		}
	} else {
		std::vector<std::string>::iterator pos = std::find(mods_copy.begin(), mods_copy.end(), id);
		if(pos != mods_copy.end()) {
			mods_copy.erase(pos);
		}
	}

	try_modifications(mods_copy, force);
}

void manager::try_era_by_index(int index, bool force)
{
	try_era(depinfo_.child("era", index)["id"], force);
}

void manager::try_scenario_by_index(int index, bool force)
{
	try_scenario(depinfo_.child("scenario", index)["id"], force);
}

int manager::get_era_index() const
{
	int result = 0;
	for(const config& i : depinfo_.child_range("era")) {
		if(i["id"] == era_) {
			return result;
		}

		result++;
	}

	return -1;
}

int manager::get_scenario_index() const
{
	int result = 0;

	for(const config& i : depinfo_.child_range("scenario")) {
		if(i["id"] == scenario_) {
			return result;
		}

		result++;
	}

	return -1;
}

bool manager::is_modification_active(int index) const
{
	std::string id = depinfo_.child("modification", index)["id"];
	return std::find(mods_.begin(), mods_.end(), id) != mods_.end();
}

bool manager::is_modification_active(const std::string id) const
{
	return std::find(mods_.begin(), mods_.end(), id) != mods_.end();
}

bool manager::enable_mods_dialog(const std::vector<std::string>& mods, const std::string& requester)
{
	std::vector<std::string> items;
	for(const std::string& mod : mods) {
		items.push_back(depinfo_.find_child("modification", "id", mod)["name"]);
	}

	gui2::dialogs::depcheck_confirm_change dialog(true, items, requester);
	return dialog.show(video_);
}

bool manager::disable_mods_dialog(const std::vector<std::string>& mods, const std::string& requester)
{
	std::vector<std::string> items;
	for(const std::string& mod : mods) {
		items.push_back(depinfo_.find_child("modification", "id", mod)["name"]);
	}

	gui2::dialogs::depcheck_confirm_change dialog(false, items, requester);
	return dialog.show(video_);
}

std::string manager::change_era_dialog(const std::vector<std::string>& eras)
{
	std::vector<std::string> items;
	for(const std::string& era : eras) {
		items.push_back(depinfo_.find_child("era", "id", era)["name"]);
	}

	gui2::dialogs::depcheck_select_new dialog(ERA, items);

	if(dialog.show(video_)) {
		return eras[dialog.result()];
	}

	return "";
}

std::string manager::change_scenario_dialog(const std::vector<std::string>& scenarios)
{
	std::vector<std::string> items;
	for(const std::string& scenario : scenarios) {
		items.push_back(depinfo_.find_child("scenario", "id", scenario)["name"]);
	}

	gui2::dialogs::depcheck_select_new dialog(SCENARIO, items);
	if(dialog.show(video_)) {
		return scenarios[dialog.result()];
	}

	return "";
}

void manager::failure_dialog(const std::string& msg)
{
	gui2::show_message(video_, _("Failed to resolve dependencies"), msg, _("OK"));
}

void manager::insert_element(component_type type, const config& data, int index)
{
	std::string type_str;

	switch(type) {
	case ERA:
		type_str = "era";
		break;
	case SCENARIO:
		type_str = "scenario";
		break;
	case MODIFICATION:
		type_str = "modification";
	}

	depinfo_.add_child_at(type_str, data, index);
}

bool manager::change_scenario(const std::string& id)
{
	// Checking for missing dependencies
	if(!get_required_not_installed(elem(id, "scenario")).empty()) {
		std::string msg = _("Scenario can't be activated. Some dependencies are missing: ");

		msg += utils::join(get_required_not_installed(elem(id, "scenario")), ", ");

		failure_dialog(msg);
		return false;
	}

	scenario_ = id;

	elem scen = elem(id, "scenario");
	std::string scen_name = find_name_for(scen);

	// Firstly, we check if we have to enable/disable any mods
	std::vector<std::string> req = get_required_not_enabled(scen);
	std::vector<std::string> con = get_conflicting_enabled(scen);

	if(!req.empty()) {
		if(!enable_mods_dialog(req, scen_name)) {
			return false;
		}
	}

	if(!con.empty()) {
		if(!disable_mods_dialog(con, scen_name)) {
			return false;
		}
	}

	std::vector<std::string> newmods = req;
	for(const std::string& i : mods_) {
		if(!utils::contains(con, i)) {
			newmods.push_back(i);
		}
	}

	mods_ = newmods;

	// Now checking if the currently selected era conflicts the scenario
	// and changing era if necessary
	if(!conflicts(scen, elem(era_, "era"))) {
		return true;
	}

	std::vector<std::string> compatible;
	for(const config& i : depinfo_.child_range("era")) {
		if(!conflicts(scen, elem(i["id"], "era"))) {
			compatible.push_back(i["id"]);
		}
	}

	if(!compatible.empty()) {
		era_ = change_era_dialog(compatible);
	} else {
		failure_dialog(_("No compatible eras found."));
		return false;
	}

	if(era_.empty()) {
		return false;
	}

	return change_era(era_);
}

bool manager::change_era(const std::string& id)
{
	// Checking for missing dependencies
	if(!get_required_not_installed(elem(id, "era")).empty()) {
		std::string msg = _("Era can't be activated. Some dependencies are missing: ");

		msg += utils::join(get_required_not_installed(elem(id, "era")), ", ");
		failure_dialog(msg);
		return false;
	}

	era_ = id;

	elem era = elem(id, "era");
	std::string era_name = find_name_for(era);

	std::vector<std::string> req = get_required_not_enabled(era);
	std::vector<std::string> con = get_conflicting_enabled(era);

	// Firstly, we check if we have to enable/disable any mods
	if(!req.empty()) {
		if(!enable_mods_dialog(req, era_name)) {
			return false;
		}
	}

	if(!con.empty()) {
		if(!disable_mods_dialog(con, era_name)) {
			return false;
		}
	}

	std::vector<std::string> newmods = req;
	for(const std::string& i : mods_) {
		if(!utils::contains(con, i)) {
			newmods.push_back(i);
		}
	}

	mods_ = newmods;

	// Now checking if the currently selected scenario conflicts the era
	// and changing scenario if necessary
	if(!conflicts(era, elem(scenario_, "scenario"))) {
		return true;
	}

	std::vector<std::string> compatible;
	for(const config& i : depinfo_.child_range("scenario")) {
		if(!conflicts(era, elem(i["id"], "scenario"))) {
			compatible.push_back(i["id"]);
		}
	}

	if(!compatible.empty()) {
		scenario_ = change_scenario_dialog(compatible);
	} else {
		failure_dialog(_("No compatible scenarios found."));
		return false;
	}

	if(scenario_.empty()) {
		return false;
	}

	return change_scenario(scenario_);
}

bool manager::change_modifications(const std::vector<std::string>& modifications)
{
	// Checking if the selected combination of mods is valid at all
	std::vector<std::string> filtered;
	for(const std::string& i : modifications) {
		bool ok = true;
		elem ei(i, "modification");

		for(const std::string& j : filtered) {
			ok = ok && !conflicts(ei, elem(j, "modification"));
		}

		if(ok) {
			filtered.push_back(i);
		}
	}

	if(filtered.size() != modifications.size()) {
		failure_dialog(_("Not all of the chosen modifications are compatible."
						 " Some of them will be disabled."));
	}

	mods_ = filtered;

	// Checking if the currently selected era is compatible with the set
	// modifications, and changing era if necessary
	std::vector<std::string> compatible;
	for(const config& c : depinfo_.child_range("era")) {
		elem era(c["id"], "era");
		bool ok = true;

		for(const std::string& s : mods_) {
			ok = ok && !conflicts(era, elem(s, "modification"));
		}

		if(ok) {
			compatible.push_back(era.id);
		}
	}

	if(!utils::contains(compatible, era_)) {
		if(!compatible.empty()) {
			era_ = change_era_dialog(compatible);
		} else {
			failure_dialog(_("No compatible eras found."));
			return false;
		}

		if(era_.empty()) {
			return false;
		}

		if(!change_era(era_)) {
			return false;
		}
	} else {
		if(!change_era(era_)) {
			return false;
		}
	}

	compatible.clear();

	// Checking if the currently selected scenario is compatible with
	// the set modifications, and changing scenario if necessary
	for(const config& c : depinfo_.child_range("scenario")) {
		elem scen(c["id"], "scenario");
		bool ok = true;
		for(const std::string& s : mods_) {
			ok = ok && !conflicts(scen, elem(s, "modification"));
		}

		if(ok) {
			compatible.push_back(scen.id);
		}
	}

	if(!utils::contains(compatible, scenario_)) {
		if(!compatible.empty()) {
			scenario_ = change_scenario_dialog(compatible);
		} else {
			failure_dialog(_("No compatible scenarios found."));
			return false;
		}

		if(scenario_.empty()) {
			return false;
		}

		return change_scenario(scenario_);
	} else {
		if(!change_scenario(scenario_)) {
			return false;
		}
	}

	return true;
}

} // namespace depcheck

} // namespace ng
