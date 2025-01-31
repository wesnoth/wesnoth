/*
	Copyright (C) 2023 - 2024
	by Subhraman Sarkar (babaissarkar) <suvrax@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

#include "game_config_view.hpp"
#include "gui/dialogs/modal_dialog.hpp"
#include "gui/widgets/group.hpp"
#include "gui/widgets/combobox.hpp"
#include "gui/widgets/menu_button.hpp"
#include "serialization/preprocessor.hpp"

#include <boost/dynamic_bitset.hpp>

namespace gui2::dialogs
{
/**
 * Dialog that allows user to create custom unit types.
 */
class editor_edit_unit : public modal_dialog
{
public:
	editor_edit_unit(const game_config_view& game_config, const std::string& addon_id);

	/** The execute function. See @ref modal_dialog for more information. */
	DEFINE_SIMPLE_EXECUTE_WRAPPER(editor_edit_unit);

	/** Write the cfg file */
	void write();

private:
	const game_config_view& game_config_;
	const std::string& addon_id_;

	config type_cfg_;
	config resistances_, defenses_, movement_;
	preproc_map specials_map_, abilities_map_;

	/**
	 * Used to control checkboxes for various resistances, defences, etc.
	 * so that only specific values are overridden.
	 */
	boost::dynamic_bitset<> res_toggles_, def_toggles_, move_toggles_;

	std::vector<config> align_list_, race_list_, movetype_list_, defense_list_, resistances_list_, usage_type_list_;
	std::vector<config> specials_list_, abilities_list_;

	std::vector<std::pair<boost::dynamic_bitset<>, config>> attacks_;

	/** Need this because can't store macros in config */
	std::vector<std::string> sel_abilities_, sel_specials_;

	/** Generated WML */
	std::string generated_wml;

	/** 0 means there are no attacks. 1 is the first attack, and so on.*/
	unsigned int selected_attack_ = 0;

	virtual void pre_show() override;

	virtual const std::string& window_id() const override;

	/** Load Unit Type data from cfg */
	void load_unit_type();

	/** Save Unit Type data to cfg */
	void save_unit_type();

	/** Write macro to a stream at specified tab level */
	void write_macro(std::ostream& out, unsigned level, const std::string& macro_name);

	/** Update wml preview */
	void update_wml_view();

	/** Callback for loading movetype data in UI */
	void load_movetype();

	/** Callback for resistance list */
	void update_resistances();
	void store_resistances();
	void enable_resistances_slider();

	/** Callbacks for defense list */
	void update_defenses();
	void store_defenses();
	void enable_defense_slider();

	/** Callbacks for movement list */
	void update_movement_costs();
	void store_movement_costs();
	void enable_movement_slider();

	/** Callbacks for attack page */
	void store_attack();
	void update_attacks();
	void add_attack();
	void delete_attack();
	void update_index();
	void next_attack();
	void prev_attack();
	void select_attack();

	/** Callback when an tab item in the "page" listbox is selected */
	void on_page_select();

	/** Callback for file select button */
	void select_file(const std::string& default_dir, const std::string& id_stem);

	/** Callback for image update */
	void update_image(const std::string& id_stem);

	/** Callback to enable/disable OK button if ID/Name is invalid */
	void button_state_change();

	/** Quit confirmation */
	void quit_confirmation();

	/** Utility method to check if ID contains any invalid characters */
	bool check_id(const std::string& id);

	void set_selected_from_string(menu_button& list, std::vector<config> values, std::string item) {
		for (unsigned i = 0; i < values.size(); ++i) {
			if(values.at(i)["label"] == item) {
				list.set_selected(i);
				break;
			}
		}
	}

	void set_selected_from_string(combobox& list, std::vector<config> values, std::string item) {
		for (unsigned i = 0; i < values.size(); ++i) {
			if(values.at(i)["label"] == item) {
				list.set_selected(i);
				break;
			}
		}
		list.set_value(item);
	}

	/* signal handler for Ctrl+O shorcut */
	void signal_handler_sdl_key_down(
		const event::ui_event /*event*/,
		bool& handled,
		const SDL_Keycode key,
		SDL_Keymod modifier);
};

} // namespace gui2::dialogs
