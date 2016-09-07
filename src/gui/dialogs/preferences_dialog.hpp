/*
   Copyright (C) 2011, 2015 by Ignacio R. Morelle <shadowm2006@gmail.com>
   Copyright (C) 2016 by Charles Dang <exodia339gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_PREFERENCES_DIALOG_HPP_INCLUDED
#define GUI_DIALOGS_PREFERENCES_DIALOG_HPP_INCLUDED

#include "config.hpp"
#include "game_preferences.hpp"
#include "utils/make_enum.hpp"
#include "gui/dialogs/dialog.hpp"
#include "gui/widgets/group.hpp"

// This file is not named preferences.hpp in order -I conflicts with
// src/preferences.hpp.

namespace hotkey {
	struct hotkey_command;
}

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
	 * tgenerator. It's something to look into, however.
	 */
	static std::map<PREFERENCE_VIEW, std::pair<int,int>> pef_view_map = {
		{VIEW_DEFAULT, {0,0}},
		{VIEW_FRIENDS, {4,1}}
	};
}

namespace gui2
{

using namespace preferences;

class tlistbox;
class tmenu_button;
class tcontrol;
class tslider;
class ttext_box;
class ttoggle_button;

class tpreferences : public tdialog
{
public:
	tpreferences(CVideo& video, const config& game_cfg, const PREFERENCE_VIEW& initial_view);

	/** The display function -- see @ref tdialog for more information. */
	static void display(CVideo& video, const config& game_cfg, const PREFERENCE_VIEW initial_view = VIEW_DEFAULT)
	{
		tpreferences(video, game_cfg, initial_view).show(video);
	}

	typedef std::vector<const hotkey::hotkey_command*> t_visible_hotkeys;

private:
	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from tdialog. */
	void post_build(twindow& window);
	void pre_show(twindow& window);
	void post_show(twindow& /*window*/);

	/** Initializers */
	void initialize_tabs(twindow& window, tlistbox& selector);
	void set_resolution_list(tmenu_button& res_list, CVideo& video);
	void setup_friends_list(twindow& window);
	void setup_hotkey_list(twindow& window);

	void add_friend_list_entry(const bool is_friend,
		ttext_box& textbox, twindow& window);

	void on_friends_list_select(tlistbox& friends, ttext_box& textbox);

	void remove_friend_list_entry(tlistbox& friends_list,
		ttext_box& textbox, twindow& window);

	void set_visible_page(twindow& window, unsigned int page, const std::string& pager_id);

	/** Callback for selection changes */
	void on_page_select(twindow& window);
	void on_tab_select(twindow& window);
	void on_advanced_prefs_list_select(tlistbox& tree, twindow& window);

	/** Special callback functions */
	void handle_res_select(twindow& window);
	void fullscreen_toggle_callback(twindow& window);
	void animate_map_toggle_callback(ttoggle_button& toggle, ttoggle_button& toggle_water);
	void add_hotkey_callback(tlistbox& hotkeys);
	void remove_hotkey_callback(tlistbox& hotkeys);
	void default_hotkey_callback(twindow& window);

	tgroup<preferences::LOBBY_JOINS> lobby_joins_group;

	MAKE_ENUM(ADVANCED_PREF_TYPE,
		(TOGGLE,  "boolean")
		(SLIDER,  "int")
		(COMBO,   "combo")
		(SPECIAL, "custom")
	)

	std::vector<std::pair<int,int> > resolutions_;
	std::vector<config> adv_preferences_cfg_;
	std::vector<std::string> friend_names_;

	int last_selected_item_;

	std::vector<t_string> accl_speeds_;
	t_visible_hotkeys visible_hotkeys_;

	// The page/tab index pairs for setting visible pages
	const std::pair<int, int>& initial_index_;
};

} // namespace gui2

#endif /* ! GUI_DIALOGS_PREFERENCES_DIALOG_HPP_INCLUDED */
