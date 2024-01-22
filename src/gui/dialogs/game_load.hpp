/*
	Copyright (C) 2008 - 2024
	by Jörg Hinrichs <joerg.hinrichs@alice-dsl.de>
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

#include "gettext.hpp"
#include "gui/dialogs/modal_dialog.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "gui/widgets/menu_button.hpp"
#include "save_index.hpp"
#include "savegame.hpp"

#include <SDL2/SDL_keycode.h>

namespace gui2
{
class text_box_base;

namespace dialogs
{
/**
 * @ingroup GUIWindowDefinitionWML
 *
 * This shows the dialog to select and load a savegame file.
 * Key               |Type          |Mandatory|Description
 * ------------------|--------------|---------|-----------
 * txtFilter         | text         |yes      |The filter for the listbox items.
 * savegame_list     | @ref listbox |yes      |List of savegames.
 * filename          | control      |yes      |Name of the savegame.
 * date              | control      |no       |Date the savegame was created.
 * preview_pane      | widget       |yes      |Container widget or grid that contains the items for a preview. The visible status of this container depends on whether or not something is selected.
 * minimap           | @ref minimap |yes      |Minimap of the selected savegame.
 * imgLeader         | @ref image   |yes      |The image of the leader in the selected savegame.
 * lblScenario       | @ref label   |yes      |The name of the scenario of the selected savegame.
 * lblSummary        | @ref label   |yes      |Summary of the selected savegame.
 */
class game_load : public modal_dialog
{
public:
	game_load(const game_config_view& cache_config, savegame::load_game_metadata& data);

	static bool execute(const game_config_view& cache_config, savegame::load_game_metadata& data);

private:
	virtual void pre_show(window& window) override;

	virtual const std::string& window_id() const override;

	void set_save_dir_list(menu_button& dir_list);

	/** Update (both internally and visually) the list of games. */
	void populate_game_list();

	void filter_text_changed(const std::string& text);
	void browse_button_callback();
	void delete_button_callback();
	void handle_dir_select();

	/** Part of display_savegame that might throw a config::error if the savegame data is corrupt. */
	void display_savegame_internal(const savegame::save_info& game);
	void display_savegame();
	void evaluate_summary_string(std::stringstream& str, const config& cfg_summary);

	void key_press_callback(const SDL_Keycode key);

	std::string& filename_;
	std::shared_ptr<savegame::save_index_class>& save_index_manager_;

	field_bool* change_difficulty_;
	field_bool* show_replay_;
	field_bool* cancel_orders_;

	config& summary_;

	std::vector<savegame::save_info> games_;
	const game_config_view& cache_config_;

	std::vector<std::string> last_words_;
};
} // namespace dialogs
} // namespace gui2
