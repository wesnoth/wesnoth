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
#include "make_enum.hpp"
#include "gui/dialogs/dialog.hpp"
#include "gui/widgets/group.hpp"

// This file is not named preferences.hpp in order -I conflicts with
// src/preferences.hpp.

namespace hotkey {
	struct hotkey_command;
}
namespace gui2
{

class tlistbox;
class tcombobox;
class tcontrol;
class tslider;
class ttext_box;
class ttoggle_button;

class tpreferences : public tdialog
{
public:
	/**
	 * Constructor.
	 */
	tpreferences(CVideo& video, const config& game_cfg);

	/** The display function -- see @ref tdialog for more information. */
	static bool display(CVideo& video, const config& game_cfg)
	{
		tpreferences(video, game_cfg).show(video);
		return true;
	}

	typedef std::vector<const hotkey::hotkey_command*> t_visible_hotkeys;

	void set_selected_index(std::pair<int, int> index)
	{
		index_ = index;
	}

private:
	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);
	void post_show(twindow& /*window*/);

	/** Initializers */
	void initialize_members(twindow& window);
	void initialize_tabs(twindow& window, tlistbox& selector, const int index);
	void setup_friends_list(twindow& window);
	void setup_hotkey_list(twindow& window);

	void add_friend_list_entry(const bool is_friend,
		ttext_box& textbox, twindow& window);

	void edit_friend_list_entry(tlistbox& friends, ttext_box& textbox);

	void remove_friend_list_entry(tlistbox& friends_list,
		ttext_box& textbox, twindow& window);

	void add_tab(tlistbox& tab_bar, const std::string& label);
	void add_pager_row(tlistbox& selector, const std::string& icon, const std::string& label);
	void set_visible_page(twindow& window, unsigned int page, const std::string& pager_id);

	/** Callback for selection changes */
	void on_page_select(twindow& window);
	void on_tab_select(twindow& window);
	void on_advanced_prefs_list_select(tlistbox& tree, twindow& window);

	/** Special callback functions */
	void handle_res_select(twindow& window);
	void fullscreen_toggle_callback(twindow& window);
	void accl_speed_slider_callback(tslider& slider);
	void max_autosaves_slider_callback(tslider& slider, tcontrol& status_label);
	void font_scaling_slider_callback(tslider& slider);
	void add_hotkey_callback(tlistbox& hotkeys);
	void remove_hotkey_callback(tlistbox& hotkeys);
	void default_hotkey_callback(twindow& window);

	/**
	 * Sets the initial state and callback for a simple bool-state toggle button
	 * In the callback, the bool value of the widget is passeed to the setter
	 */
	void setup_single_toggle(
		const std::string& widget_id,
		const bool start_value,
		boost::function<void(bool)> callback,
		twidget& find_in,
		const bool inverted = false);

	void single_toggle_callback(const ttoggle_button& widget,
		boost::function<void(bool)> setter,
		const bool inverted);

	/**
	 * Sets the initial state and callback for a bool-state toggle button/slider
	 * pair. In the callback, the slider is set to active/inactive based on the
	 * button's value, which is also passed to the setter.
	 */
	void setup_toggle_slider_pair(
		const std::string& toggle_widget,
		const std::string& slider_widget,
		const bool toggle_start_value,
		const int slider_state_value,
		boost::function<void(bool)> toggle_callback,
		boost::function<void(int)> slider_callback,
		twidget& find_in);

	void toggle_slider_pair_callback(const ttoggle_button& toggle_widget,
		tslider& slider_widget, boost::function<void(bool)> setter);

	/**
	 * Sets the initial state and callback for a standalone slider
	 * In the callback, int value of the widget is passeed to the setter
	 */
	void setup_single_slider(
		const std::string& widget_id,
		const int start_value,
		boost::function<void(int)> slider_callback,
		twidget& find_in);

	void single_slider_callback(const tslider& widget,
		boost::function<void(int)> setter);

	typedef std::pair<std::vector<std::string>, std::vector<std::string> > combo_data;

	/**
	 * Sets the initial state and callback for a combobox
	 */
	void setup_combobox(
		const std::string& widget_id,
		const combo_data& options,
		const unsigned start_value,
		boost::function<void(std::string)> callback,
		twidget& find_in);

	void simple_combobox_callback(const tcombobox& widget,
		boost::function<void(std::string)> setter, std::vector<std::string>& vec);

	/**
	 * Sets the a radio button group
	 * Since (so far) there is only one group of these, the code is speciliazed.
	 * If (at a later date) more groups need to be added, this will have to be
	 * generalized.
	 */
	tgroup<preferences::LOBBY_JOINS> lobby_joins_group;

	template <typename T>
	void setup_radio_toggle(
		const std::string& toggle_id,
		const T& enum_value,
		const int start_value,
		tgroup<T>& group,
		boost::function<void(int)> callback,
		twindow& window);

	template <typename T>
	void toggle_radio_callback(
		tgroup<T>& group,
		boost::function<void(int)> setter);

	/**
	 * Sets up a label that always displays the value of another widget.
	 */
	template <typename T>
	void bind_status_label(
		T& parent,
		const std::string& label_id,
		twidget& find_in);

	void bind_status_label(
		tslider& parent,
		const std::string& label_id,
		twidget& find_in,
		const std::string& suffix = "");

	template <typename T>
	void status_label_callback(T& parent_widget,
		tcontrol& label_widget, const std::string& suffix = "");

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

	// Special variable to keep the value of the scaling slider,
	// to be used in post_show
	int font_scaling_;

	// The page/tab index pairs for setting visible pages
	std::pair<int, int> index_;
};

} // namespace gui2

#endif /* ! GUI_DIALOGS_PREFERENCES_DIALOG_HPP_INCLUDED */
