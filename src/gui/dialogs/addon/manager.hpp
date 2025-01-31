/*
	Copyright (C) 2008 - 2024
	by Mark de Wever <koraq@xs4all.nl>
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

#include "addon/client.hpp"
#include "addon/info.hpp"
#include "addon/state.hpp"

#include "gui/dialogs/modal_dialog.hpp"
#include "gui/widgets/addon_list.hpp"


namespace gui2
{
class selectable_item;
class button;
class stacked_widget;
namespace dialogs
{

class addon_manager : public modal_dialog
{
public:
	explicit addon_manager(addons_client& client);

	bool get_need_wml_cache_refresh() const
	{
		return need_wml_cache_refresh_;
	}

private:
	struct addon_order
	{
		std::string label;
		/** The value used in the preferences file */
		std::string as_preference;
		int column_index; // -1 if there is no such column
		addon_list::addon_sort_func sort_func_asc;
		addon_list::addon_sort_func sort_func_desc;

		addon_order(std::string label_, std::string as_preference_, int column, addon_list::addon_sort_func sort_func_asc_, addon_list::addon_sort_func sort_func_desc_)
			: label(label_)
			, as_preference(as_preference_)
			, column_index(column)
			, sort_func_asc(sort_func_asc_)
			, sort_func_desc(sort_func_desc_)
		{}
	};

	std::vector<selectable_item*> orders_;

	void on_addon_select();
	void toggle_details(button& btn, stacked_widget& stk);

	virtual const std::string& window_id() const override;

	virtual void pre_show() override;

	void fetch_addons_list();
	void load_addon_list();

	void reload_list_and_reselect_item(const std::string& id);

	/** Config which contains the list with the campaigns. */
	config cfg_;

	addons_client& client_;

	addons_list addons_;

	addons_tracking_list tracking_info_;

	static const std::vector<std::pair<ADDON_STATUS_FILTER, std::string>> status_filter_types_;
	static const std::vector<std::pair<ADDON_TYPE, std::string>> type_filter_types_;
	std::vector<std::pair<int, std::string>> language_filter_types_;
	static const std::vector<addon_order> all_orders_;

	bool need_wml_cache_refresh_;

	template<void(addon_manager::*fptr)(const addon_info& addon)>
	void execute_action_on_selected_addon();

	void install_addon(const addon_info& addon);
	void install_selected_addon()
	{
		execute_action_on_selected_addon<&addon_manager::install_addon>();
	}

	void uninstall_addon(const addon_info& addon);
	void uninstall_selected_addon()
	{
		execute_action_on_selected_addon<&addon_manager::uninstall_addon>();
	}

	void update_addon(const addon_info& addon);
	void update_selected_addon()
	{
		execute_action_on_selected_addon<&addon_manager::update_addon>();
	}

	void publish_addon(const addon_info& addon);
	void publish_selected_addon()
	{
		execute_action_on_selected_addon<&addon_manager::publish_addon>();
	}

	void delete_addon(const addon_info& addon);
	void delete_selected_addon()
	{
		execute_action_on_selected_addon<&addon_manager::delete_addon>();
	}

	void execute_default_action(const addon_info& addon);
	void execute_default_action_on_selected_addon()
	{
		execute_action_on_selected_addon<&addon_manager::execute_default_action>();
	}

	void update_all_addons();
	void info();

	void apply_filters();
	void order_addons();
	void on_order_changed(unsigned int sort_column, sort_order::type order);
	void show_help();

	boost::dynamic_bitset<> get_name_filter_visibility() const;
	boost::dynamic_bitset<> get_status_filter_visibility() const;
	boost::dynamic_bitset<> get_tag_filter_visibility() const;
	boost::dynamic_bitset<> get_type_filter_visibility() const;
	boost::dynamic_bitset<> get_lang_filter_visibility() const;

	void on_selected_version_change();
	bool exit_hook();
};

} // namespace dialogs
} // namespace gui2
