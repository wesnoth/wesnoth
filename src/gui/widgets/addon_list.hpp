/*
	Copyright (C) 2016 - 2024
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

#include "addon/info.hpp"
#include "addon/state.hpp"
#include "gui/widgets/container_base.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/widget.hpp"

#include <functional>
#include <string>
#include <vector>

namespace gui2
{

namespace implementation
{
	struct builder_addon_list;
}

class addon_list : public container_base
{
	friend struct implementation::builder_addon_list;

public:
	using addon_sort_func = std::function<bool(const addon_info&, const addon_info&)>;

	explicit addon_list(const implementation::builder_addon_list& builder);

	/** Special retval for the toggle panels in the addons list */
	static const int DEFAULT_ACTION_RETVAL = 200;

	const std::string display_title_full_shift(const addon_info& addon) const;

	/** Sets the add-ons to show. */
	void set_addons(const addons_list& addons);

	/** Sets up a callback that will be called when the player selects an add-on. */
	void set_modified_signal_handler(const std::function<void()>& callback)
	{
		connect_signal_notify_modified(get_listbox(), std::bind(callback));
	}

	/** Returns the selected add-on. */
	const addon_info* get_selected_addon() const;

	/** Returns the selected add-on id, for use with remote publish/delete ops. */
	std::string get_remote_addon_id();

	/** Selects the add-on with the given ID. */
	void select_addon(const std::string& id);

	using addon_op_func_t = std::function<void(const addon_info&)>;

	/**
	 * Helper to wrap the execution of any of the addon operation functions.
	 * It catches addons_client::user_exit exceptions and halts GUI2 event execution
	 * after calling the given function.
	 */
	void addon_action_wrapper(addon_op_func_t& func, const addon_info& addon, bool& handled, bool& halt);

	/** Sets the function to call when the player clicks the install button. */
	void set_install_function(addon_op_func_t function)
	{
		install_function_ = function;
	}

	/** Sets the function to call when the player clicks the uninstall button. */
	void set_uninstall_function(addon_op_func_t function)
	{
		uninstall_function_ = function;
	}

	/** Sets the function to call when the player clicks the update button. */
	void set_update_function(addon_op_func_t function)
	{
		update_function_ = function;
	}

	/** Sets the function to upload an addon to the addons server. */
	void set_publish_function(addon_op_func_t function)
	{
		publish_function_ = function;
	}

	/** Sets the function to install an addon from the addons server. */
	void set_delete_function(addon_op_func_t function)
	{
		delete_function_ = function;
	}

	/** Filters which add-ons are visible. 1 = visible, 0 = hidden. */
	void set_addon_shown(boost::dynamic_bitset<>& shown)
	{
		get_listbox().set_row_shown(shown);
	}

	void set_addon_order(const addon_sort_func& func);

	/**
	 * Changes the color of an add-on state string (installed, outdated, etc.) according to the state itself.
	 * This function is here because the add-on list widget itself needs it.
	 */
	static std::string colorize_addon_state_string(const std::string& str, ADDON_STATUS state, bool verbose = false);

	/** Determines if install status of each widget is shown. */
	void set_install_status_visibility(visibility visibility)
	{
		install_status_visibility_ = visibility;
	}

	/** Determines if install/uninstall buttons are shown for each widget. */
	void set_install_buttons_visibility(visibility visibility)
	{
		install_buttons_visibility_ = visibility;
	}

	/** Adds the internal listbox to the keyboard event chain. */
	void add_list_to_keyboard_chain();

	/** Sets up a callback that will be called when the player changes the sorting order. */
	void set_callback_order_change(std::function<void(unsigned, sort_order::type)> callback) {
		get_listbox().set_callback_order_change(callback);
	}

	/** See @ref styled_widget::set_active. */
	virtual void set_active(const bool) override
	{
		// DO NOTHING
	}

	/** See @ref styled_widget::get_active. */
	virtual bool get_active() const override
	{
		return true;
	}

	/** See @ref styled_widget::get_state. */
	virtual unsigned get_state() const override
	{
		return 0;
	}

private:
	std::vector<const addon_info*> addon_vector_;

	visibility install_status_visibility_;
	visibility install_buttons_visibility_;

	addon_op_func_t install_function_;
	addon_op_func_t uninstall_function_;
	addon_op_func_t update_function_;

	addon_op_func_t publish_function_;
	addon_op_func_t delete_function_;

	static std::string describe_status(const addon_tracking_info& info);

	/** Returns the underlying list box. */
	listbox& get_listbox();

	void finalize_setup();

public:
	/** Choose the item at the top of the list (taking account of sort order). */
	void select_first_addon();

	/** Static type getter that does not rely on the widget being constructed. */
	static const std::string& type();

private:
	/** Inherited from styled_widget, implemented by REGISTER_WIDGET. */
	virtual const std::string& get_control_type() const override;

	/** See @ref container_base::set_self_active */
	void set_self_active(const bool) override
	{
		// DO NOTHING
	}
};

struct addon_list_definition : public styled_widget_definition
{
	explicit addon_list_definition(const config& cfg);

	struct resolution : public resolution_definition
	{
		explicit resolution(const config& cfg);

		builder_grid_ptr grid;
	};
};

namespace implementation
{

struct builder_addon_list : public builder_styled_widget
{
	explicit builder_addon_list(const config& cfg);

	using builder_styled_widget::build;

	virtual std::unique_ptr<widget> build() const override;

	widget::visibility install_status_visibility;
	widget::visibility install_buttons_visibility;
};
}
}
