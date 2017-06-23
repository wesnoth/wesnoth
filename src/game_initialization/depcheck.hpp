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

#pragma once

#include <string>
#include <vector>
#include "config.hpp"
#include "gettext.hpp"
#include "utils/make_enum.hpp"

class CVideo;

namespace ng
{

namespace depcheck
{

enum component_type
{
	ERA,
	SCENARIO,
	MODIFICATION
};

MAKE_ENUM(component_availabilty,
	(SP, "sp")
	(MP, "mp")
	(HYBRID, "hybrid")
)
/**
 * Note to all triers:
 * It's not guaranteed that the specified component will be selected
 * (if the user denies to perform dependency resolution, all changes
 * will be reverted). Consequently, it's essential to check the
 * selected values after calling any trier.
 *
 * Note to ctor & insert_element:
 * Please note that the ctor collects data for scenario elements from
 * "multiplayer" nodes, while insert_element from "scenario" nodes.
 */
class manager
{
public:
	manager(const config& gamecfg, bool mp, CVideo& video);

	/**
	 * Tries to set the selected era
	 *
	 * @param id 		the id of the era
	 * @param force 	whether to skip dependency check
	 */
	void try_era(const std::string& id, bool force = false);

	/**
	 * Tries to set the selected scenario
	 *
	 * @param id 		the id of the scenario
	 * @param force 	whether to skip dependency check
	 */
	void try_scenario(const std::string& id, bool force = false);

	/**
	 * Tries to set the enabled modifications
	 *
	 * @param ids 		the ids of the modifications
	 * @param force 	whether to skip dependency check
	 */
	void try_modifications(const std::vector<std::string>& ids,
						   bool force = false	);

	/**
	 * Tries to enable/disable a specific modification
	 *
	 * @param index     the index of the modification
	 * @param activate  activate or deactivate
	 * @param force     whether to skip dependency check
	 */
	void try_modification_by_index(int index, bool activate, bool force = false);

	/**
	 * Tries to set the selected era
	 *
	 * @param index 	the index of the era
	 * @param force 	whether to skip dependency check
	 */
	void try_era_by_index(int index, bool force = false);

	/**
	 * Tries to set the selected scenario
	 *
	 * @param index 	the index of the scenario
	 * @param force 	whether to skip dependency check
	 */
	void try_scenario_by_index(int index, bool force = false);

	/**
	 * Returns the selected era
	 *
	 * @return the id of the era
	 */
	const std::string& get_era() const { return era_; }

	/**
	 * Returns the selected scenario
	 *
	 * @return the id of the scenario
	 */
	const std::string& get_scenario() const { return scenario_; }

	/**
	 * Returns the enabled modifications
	 *
	 * @return the ids of the modifications
	 */
	const std::vector<std::string>& get_modifications() const { return mods_; }

	/**
	 * Tells whether a certain mod is activated.
	 *
	 * @param index the index of the mod
	 *
	 * @return true if activated, false is not
	 */
	bool is_modification_active(int index) const;

	/**
	 * Tells whether a certain mod is activated.
	 *
	 * @param string the string id of the mod
	 *
	 * @return true if activated, false is not
	 */
	bool is_modification_active(const std::string id) const;

	/**
	 * Returns the selected era
	 *
	 * @return the index of the era
	 */
	int get_era_index() const;

	/**
	 * Returns the selected scenario
	 *
	 * @return the index of the scenario
	 */
	int get_scenario_index() const;

	/**
	 * Adds a new element to the manager's database
	 *
	 * @param type 		the type of the element
	 * @param data 		a config object containing the dependency info for the
	 * 					element
	 * @param index 	where to insert the element
	 */
	void insert_element(component_type type, const config& data, int index = 0);

private:

	/** represents a component (era, modification or scenario)*/
	struct elem {
		elem(const std::string& _id, const std::string& _type)
			: id(_id)
			, type(_type)
		{}

		std::string id;
		std::string type;

		bool operator ==(const elem& e) const
				{ return id == e.id && type == e.type; }

		bool operator !=(const elem& e) const { return !(*this == e); }
	};

	/** the screen to display dialogs on */
	CVideo& video_;

	/** holds all required info about the components and their dependencies */
	config depinfo_;

	/** the id of the currently selected era */
	std::string era_;

	/** the id of the currently selected scenario */
	std::string scenario_;

	/** the ids of the currently selected modifications */
	std::vector<std::string> mods_;

	/** used by save_state() and revert() to backup/restore era_ */
	std::string prev_era_;

	/** used by save_state() and revert() to backup/restore scenario_ */
	std::string prev_scenario_;

	/** used by save_state() and revert() to backup/restore mods_ */
	std::vector<std::string> prev_mods_;

	/** saves the current values of era_, scenarios_ and mods_ */
	void save_state();

	/** restores the lastly saved values of era_, scenarios_ and mods_ */
	void revert();

	/**
	 * Attempts to change the selected scenario.
	 *
	 * @param id 	the scenario's id
	 * @return 		true if the selection was changed; false if not
	 */
	bool change_scenario(const std::string& id);

	/**
	 * Attempts to change the selected era.
	 *
	 * @param id 	the era's id
	 * @return 		true if the selection was changed; false if not
	 */
	bool change_era(const std::string& id);

	/**
	 * Attempts to change the selected modifications.
	 *
	 * @param modifications 	the list of the modifications' ids
	 * @return 					true if the selection was changed; false if not
	 */
	bool change_modifications(const std::vector<std::string>& modifications);

	/**
	 * Decides if two components are conflicting or not
	 *
	 * @param elem1 			the first component
	 * @param elem2 			the second component
	 * @param directonly 		whether the function should ignore any possible
	 * 							conflicts between the components' dependencies.
	 *
	 * @return 					true if e1 and e2 conflict, false if not
	 */
	bool conflicts(const elem& elem1, const elem& elem2, bool directonly=false) const;

	/**
	 * Decides whether e1 requires e2
	 *
	 * @param elem1 	a component; by definition, passing a modification here
	 * 					makes no sense
	 * @param elem2 	another component; by definition, passing anything else
	 * 					than a modification here makes no sense
	 *
	 * @return 			true if e2 is required by e1, false if not
	 */
	bool requires(const elem& elem1, const elem& elem2) const;

	/**
	 * Get the list of modifications required by a certain component
	 *
	 * @param e 	the component
	 *
	 * @return 		the list of the modifications' ids
	 */
	std::vector<std::string> get_required(const elem& e) const;

	/**
	 * Get the list of modifications which are required by a certain
	 * component, but aren't currently enabled
	 *
	 * @param e 	the component
	 *
	 * @return 		the list of the modifications' ids
	 */
	std::vector<std::string> get_required_not_enabled(const elem& e) const;

	/**
	 * Get the list of modifications which are conflicting a certain
	 * component and are currently enabled
	 *
	 * @param e 	the component
	 *
	 * @return 		the list of the modifications' ids
	 */
	std::vector<std::string> get_conflicting_enabled(const elem& e) const;

	/**
	 * Get the list of modifications which are required by a certain
	 * component, but currently unavailable on the computer
	 *
	 * @param e 	the component
	 *
	 * @return 		the list of the modifications' ids
	 */
	std::vector<std::string> get_required_not_installed(const elem& e) const;

	/**
	 * Display a dialog requesting confirmation for enabling some
	 * modifications
	 *
	 * @param mods 		the list of modifications to be enabled
	 * @param requester	the add-on's name which requests the action to be done
	 *
	 * @return 			true, if the user accepted the change, false if not
	 */
	bool enable_mods_dialog(const std::vector<std::string>& mods,
							const std::string& requester = _("A component"));

	/**
	 * Display a dialog requesting confirmation for disabling some
	 * modifications
	 *
	 * @param mods 		the list of modifications to be disabled
	 * @param requester	the add-on's name which requests the action to be done
	 *
	 * @return 			true, if the user accepted the change, false if not
	 */
	bool disable_mods_dialog(const std::vector<std::string>& mods,
							 const std::string& requester = _("A component"));

	/**
	 * Display a dialog requesting the user to select a new era
	 *
	 * @param eras 		the possible options (ids)
	 *
	 * @return 			the selected era's id or empty string if the user
	 * 					refused to select any
	 */
	std::string change_era_dialog(const std::vector<std::string>& eras);

	/**
	 * Display a dialog requesting the user to select a new scenario
	 *
	 * @param scenarios the possible options (ids)
	 *
	 * @return 			the selected scenario's id or empty string if the user
	 * 					refused to select any
	 */
	std::string change_scenario_dialog
				(const std::vector<std::string>& scenarios);

	/**
	 * Shows an error message
	 *
	 * @param msg the message to be displayed
	 */
	void failure_dialog(const std::string& msg);

	/**
	 * Decides whether a certain component is installed or not
	 *
	 * @param e 	the component
	 *
	 * @return 		true if the component exists false if not
	 */
	bool exists(const elem& e) const;

	/**
	 * Look up the name of a given component.
	 *
	 * @param e		the component
	 *
	 * @return		the name of the component
	 */
	std::string find_name_for(const elem& e) const;

};

} //namespace depcheck

} //namespace ng
