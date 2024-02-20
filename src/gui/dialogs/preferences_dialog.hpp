/*
	Copyright (C) 2016 - 2024
	by Charles Dang <exodia339gmail.com>
	Copyright (C) 2011, 2015 by Iris Morelle <shadowm2006@gmail.com>
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

#include "config.hpp"
#include "gui/dialogs/modal_dialog.hpp"
#include "gui/widgets/group.hpp"
#include "gui/widgets/text_box.hpp"
#include "hotkey/hotkey_command.hpp"
#include "preferences/advanced.hpp"
#include "preferences/game.hpp"
#include "theme.hpp"

// This file is not named preferences.hpp in order -I conflicts with
// src/preferences.hpp.

namespace hotkey {
	struct hotkey_command;
}

struct point;
//struct theme_info;

namespace preferences {
	enum PREFERENCE_VIEW {
		VIEW_DEFAULT,
		VIEW_FRIENDS
	};

	/**
	 * Map containing page mappings that can be used to set the initially displayed page
	 * of the dialog. The pair is in an 0-indexed toplevel stack/substack format, where
	 * the first is the list of main Preference categories (such as General and Display)
	 * and the second is any sub-stack found on that page.
	 *
	 * TODO: this isn't the most optimal solution, since if the order or number of pages
	 * in either stack changes, this map needs to be updated. Optimally the stacked_widget
	 * widget would allow specifying page by string id, but that would require changes to
	 * generator. It's something to look into, however.
	 */
	static std::map<PREFERENCE_VIEW, std::pair<int,int>> pef_view_map {
		{VIEW_DEFAULT, {0,0}},
		{VIEW_FRIENDS, {4,1}}
	};
}

namespace gui2
{

class listbox;
class menu_button;
class slider;
class text_box;

namespace dialogs
{

class preferences_dialog : public modal_dialog
{
public:
	preferences_dialog(const preferences::PREFERENCE_VIEW initial_view = preferences::VIEW_DEFAULT);

	/** The display function -- see @ref modal_dialog for more information. */
	DEFINE_SIMPLE_DISPLAY_WRAPPER(preferences_dialog)

private:
	virtual const std::string& window_id() const override;

	virtual void pre_show(window& window) override;
	virtual void post_show(window& /*window*/) override;

	/** Initializers */
	void initialize_callbacks();
	void initialize_tabs(listbox& selector);
	void set_resolution_list(menu_button& res_list);
	void set_theme_list(menu_button& theme_list);
	listbox& setup_hotkey_list();

	template<bool(*toggle_getter)(), bool(*toggle_setter)(bool), int(*vol_getter)(), void(*vol_setter)(int)>
	void initialize_sound_option_group(const std::string& id_suffix);

	void apply_pixel_scale();

	widget_data get_friends_list_row_data(const preferences::acquaintance& entry);

	void add_friend_list_entry(const bool is_friend, text_box& textbox);
	void remove_friend_list_entry(listbox& friends_list, text_box& textbox);

	void on_friends_list_select(listbox& list, text_box& textbox);
	void update_friends_list_controls(listbox& list);

	void set_visible_page(unsigned int page, const std::string& pager_id);

	/** Callback for selection changes */
	void on_page_select();
	void on_tab_select();
	void on_advanced_prefs_list_select(listbox& tree);

	/** Special callback functions */
	void handle_res_select();
	void handle_theme_select();
	void fullscreen_toggle_callback();
	void add_hotkey_callback(listbox& hotkeys);
	void remove_hotkey_callback(listbox& hotkeys);
	void default_hotkey_callback();
	void hotkey_filter_callback();

	group<preferences::lobby_joins> lobby_joins_group;

	const preferences::advanced_pref_list& adv_preferences_;

	std::vector<point> resolutions_;
	std::vector<theme_info> themes_;

	int last_selected_item_;

	std::vector<double> accl_speeds_;

	std::vector<const hotkey::hotkey_command*> visible_hotkeys_;

	std::set<hotkey::HOTKEY_CATEGORY> visible_categories_;

	// The page/tab index pairs for setting visible pages
	const std::pair<int, int>& initial_index_;
};

} // namespace dialogs
} // namespace gui2
