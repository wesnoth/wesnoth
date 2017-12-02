/*
   Copyright (C) 2008 - 2017 by Mark de Wever <koraq@xs4all.nl>
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

#include "addon/client.hpp"
#include "addon/info.hpp"
#include "addon/state.hpp"

#include "gui/dialogs/modal_dialog.hpp"
#include "gui/widgets/pane.hpp"

#include <boost/dynamic_bitset.hpp>

namespace gui2
{
class text_box_base;
class text_box;
class pane;
class selectable_item;
class button;
class stacked_widget;
namespace dialogs
{

/** Shows the list of addons on the server. */
class addon_manager : public modal_dialog
{
public:
	explicit addon_manager(addons_client& client);

	bool get_need_wml_cache_refresh() const
	{
		return need_wml_cache_refresh_;
	}

private:
	void on_filtertext_changed(text_box_base* textbox);

	std::vector<selectable_item*> orders_;

	void on_addon_select(window& window);
	void toggle_details(button& btn, stacked_widget& stk);

	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const override;

	/** Inherited from modal_dialog. */
	virtual void pre_show(window& window) override;

	void fetch_addons_list(window& window);
	void load_addon_list(window& window);

	void reload_list_and_reselect_item(const std::string id, window& window);

	/** Config which contains the list with the campaigns. */
	config cfg_;

	addons_client& client_;

	addons_list addons_;

	addons_tracking_list tracking_info_;

	static const std::vector<std::pair<ADDON_STATUS_FILTER, std::string>> status_filter_types_;
	static const std::vector<std::pair<ADDON_TYPE, std::string>> type_filter_types_;

	bool need_wml_cache_refresh_;

	template<void(addon_manager::*fptr)(const addon_info& addon, window& window)>
	void execute_action_on_selected_addon(window& window);

	void install_addon(const addon_info& addon, window& window);
	void install_selected_addon(window& window)
	{
		execute_action_on_selected_addon<&addon_manager::install_addon>(window);
	}

	void uninstall_addon(const addon_info& addon, window& window);
	void uninstall_selected_addon(window& window)
	{
		execute_action_on_selected_addon<&addon_manager::uninstall_addon>(window);
	}

	void update_addon(const addon_info& addon, window& window);
	void update_selected_addon(window& window)
	{
		execute_action_on_selected_addon<&addon_manager::update_addon>(window);
	}

	void publish_addon(const addon_info& addon, window& window);
	void publish_selected_addon(window& window)
	{
		execute_action_on_selected_addon<&addon_manager::publish_addon>(window);
	}

	void delete_addon(const addon_info& addon, window& window);
	void delete_selected_addon(window& window)
	{
		execute_action_on_selected_addon<&addon_manager::delete_addon>(window);
	}

	void execute_default_action(const addon_info& addon, window& window);
	void execute_default_action_on_selected_addon(window& window)
	{
		execute_action_on_selected_addon<&addon_manager::execute_default_action>(window);
	}

	void update_all_addons(window& window);

	void browse_url_callback(text_box& url_box);
	void copy_url_callback(text_box& url_box);

	void apply_filters(window& window);
	void show_help(window& window);

	boost::dynamic_bitset<> get_name_filter_visibility(const window& window) const;
	boost::dynamic_bitset<> get_status_filter_visibility(const window& window) const;
	boost::dynamic_bitset<> get_type_filter_visibility(const window& window) const;

	bool exit_hook(window& window);
};

} // namespace dialogs
} // namespace gui2
