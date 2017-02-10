/*
   Copyright (C) 2008 - 2016 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_ADDON_LIST_HPP_INCLUDED
#define GUI_DIALOGS_ADDON_LIST_HPP_INCLUDED

#include "addon/client.hpp"
#include "addon/info.hpp"
#include "addon/state.hpp"

#include "gui/dialogs/modal_dialog.hpp"
#include "gui/widgets/pane.hpp"

#include "config.hpp" // needed for config::const_child_itors

namespace gui2
{
class listbox;
class text_box_base;
class text_box;
class pane;
class selectable_item;
namespace dialogs
{

/** Shows the list of addons on the server. */
class addon_manager : public modal_dialog
{
public:
	explicit addon_manager(addons_client& client);

private:
	void on_filtertext_changed(text_box_base* textbox, const std::string& text);

	std::vector<selectable_item*> orders_;

	void on_addon_select(window& window);
	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from modal_dialog. */
	void pre_show(window& window);

	void load_addon_list(window& window);

	/** Config which contains the list with the campaigns. */
	config cfg_;

	/**
	 * Debug iterators for testing with --new-widgets
	 */
	config::const_child_itors cfg_iterators_;

	addons_client& client_;

	addons_list addons_;

	addons_tracking_list tracking_info_;

	std::vector<std::pair<ADDON_STATUS_FILTER, std::string>> status_filter_types_;

	void install_selected_addon(window& window);
	void install_addon(addon_info addon, window& window);
	void uninstall_selected_addon(window& window);
	void uninstall_addon(addon_info addon, window& window);
	void browse_url_callback(text_box& url_box);
	void copy_url_callback(text_box& url_box);
	void options_button_callback(window& window);
	void status_filter_callback(window& window);
	void show_help(window& window);
};

} // namespace dialogs
} // namespace gui2

#endif
