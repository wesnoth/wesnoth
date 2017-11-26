/*
   Copyright (C) 2011, 2015 by Ignacio R. Morelle <shadowm2006@gmail.com>
   Copyright (C) 2016 - 2017 by Charles Dang <exodia339gmail.com>
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

#include "config.hpp"
#include "preferences/game.hpp"
#include "utils/make_enum.hpp"
#include "gui/dialogs/modal_dialog.hpp"
#include "gui/widgets/group.hpp"

#include <boost/dynamic_bitset.hpp>

// This file is not named preferences.hpp in order -I conflicts with
// src/preferences.hpp.

namespace hotkey {
	struct hotkey_command;
}

struct point;

namespace preferences {
	enum PREFERENCE_VIEW {
		VIEW_DEFAULT,
		VIEW_FRIENDS
	};

	/**
	 * Map containing page mappings that can be used to set the initally displayed page
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
class styled_widget;
class slider;
class text_box;
class toggle_button;

namespace dialogs
{

class preferences_dialog : public modal_dialog
{
public:
	preferences_dialog(const config& game_cfg, const preferences::PREFERENCE_VIEW& initial_view);

	/** The display function -- see @ref modal_dialog for more information. */
	static void display(const config& game_cfg, const preferences::PREFERENCE_VIEW initial_view = preferences::VIEW_DEFAULT)
	{
		preferences_dialog(game_cfg, initial_view).show();
	}

	typedef std::vector<const hotkey::hotkey_command*> t_visible_hotkeys;

private:
	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const override;

	/** Inherited from modal_dialog. */
	virtual void post_build(window& window) override;
	virtual void pre_show(window& window) override;
	virtual void post_show(window& /*window*/) override;

	/** Initializers */
	void initialize_tabs(window& window, listbox& selector);
	void set_resolution_list(menu_button& res_list, CVideo& video);
	listbox& setup_hotkey_list(window& window);

	std::map<std::string, string_map> get_friends_list_row_data(const preferences::acquaintance& entry);

	void add_friend_list_entry(const bool is_friend, text_box& textbox, window& window);
	void remove_friend_list_entry(listbox& friends_list, text_box& textbox, window& window);

	void on_friends_list_select(listbox& list, text_box& textbox);
	void update_friends_list_controls(window& window, listbox& list);

	void set_visible_page(window& window, unsigned int page, const std::string& pager_id);

	/** Callback for selection changes */
	void on_page_select(window& window);
	void on_tab_select(window& window);
	void on_advanced_prefs_list_select(listbox& tree);

	/** Special callback functions */
	void handle_res_select(window& window);
	void fullscreen_toggle_callback(window& window);
	void add_hotkey_callback(listbox& hotkeys);
	void remove_hotkey_callback(listbox& hotkeys);
	void default_hotkey_callback(window& window);
	void hotkey_type_filter_callback(window& window) const;

	group<preferences::LOBBY_JOINS> lobby_joins_group;

	MAKE_ENUM(ADVANCED_PREF_TYPE,
		(TOGGLE,  "boolean")
		(SLIDER,  "int")
		(COMBO,   "combo")
		(SPECIAL, "custom")
	)

	std::vector<point> resolutions_;
	std::vector<config> adv_preferences_cfg_;

	int last_selected_item_;

	std::vector<double> accl_speeds_;
	t_visible_hotkeys visible_hotkeys_;

	std::vector<t_string> cat_names_;

	// The page/tab index pairs for setting visible pages
	const std::pair<int, int>& initial_index_;
};

} // namespace dialogs
} // namespace gui2
