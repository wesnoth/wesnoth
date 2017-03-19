/*
   Copyright (C) 2016 - 2017 by the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_WIDGETS_ADDON_LIST_HPP_INCLUDED
#define GUI_WIDGETS_ADDON_LIST_HPP_INCLUDED

#include "addon/info.hpp"
#include "addon/manager.hpp"
#include "addon/state.hpp"
#include "gui/widgets/container_base.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/widget.hpp"

#include <boost/dynamic_bitset.hpp>
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
	addon_list()
		: container_base(1)
		, addon_vector_()
		, install_status_visibility_(visibility::visible)
		, install_buttons_visibility_(visibility::invisible)
		, install_function_()
		, uninstall_function_()
		, publish_function_()
		, delete_function_()
	{}

	/** Special retval for the toggle panels in the addons list */
	static const int INSTALL_ADDON_RETVAL = 200;

	/** Sets the add-ons to show. */
	void set_addons(const addons_list& addons);

	/** Sets up a callback that will be called when the player selects an add-on. */
	void set_callback_value_change(const std::function<void(widget&)>& callback)
	{
		get_listbox().set_callback_value_change(callback);
	}

	/** Returns the selected add-on. */
	const addon_info* get_selected_addon() const;

	/** Returns the selected add-on id, for use with remote publish/delete ops. */
	std::string get_remote_addon_id();

	/** Selects the add-on with the given ID. */
	void select_addon(const std::string& id);

	/** Sets the function to call when the player clicks the install button. */
	void set_install_function(std::function<void(const addon_info&)> function)
	{
		install_function_ = function;
	}

	/** Sets the function to call when the player clicks the uninstall button. */
	void set_uninstall_function(std::function<void(const addon_info&)> function)
	{
		uninstall_function_ = function;
	}

	/** Sets the function to call when the player clicks the update button. */
	void set_update_function(std::function<void(const addon_info&)> function)
	{
		update_function_ = function;
	}

	/** Sets the function to upload an addon to the addons server. */
	void set_publish_function(std::function<void(const addon_info&)> function)
	{
		publish_function_ = function;
	}

	/** Sets the function to install an addon from the addons server. */
	void set_delete_function(std::function<void(const addon_info&)> function)
	{
		delete_function_ = function;
	}

	/** Filters which add-ons are visible. 1 = visible, 0 = hidden. */
	void set_addon_shown(boost::dynamic_bitset<>& shown)
	{
		get_listbox().set_row_shown(shown);
	}

	/**
	 * Changes the color of an add-on state string (installed, outdated, etc.) according to the state itself.
	 * This function is here because the add-on list widget itself needs it.
	 */
	static std::string colorify_addon_state_string(const std::string& str, ADDON_STATUS state, bool verbose = false);

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

	std::function<void(const addon_info&)> install_function_;
	std::function<void(const addon_info&)> uninstall_function_;
	std::function<void(const addon_info&)> update_function_;

	std::function<void(const addon_info&)> publish_function_;
	std::function<void(const addon_info&)> delete_function_;

	static std::string describe_status(const addon_tracking_info& info);

	/** Returns the underlying list box. */
	listbox& get_listbox();

	void finalize_setup();

	/** See @ref control::get_control_type. */
	const std::string& get_control_type() const override
	{
		static const std::string control_type = "addon_list";
		return control_type;
	}

	/** See @ref container_::set_self_active. */
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
public:
	explicit builder_addon_list(const config& cfg);

	using builder_styled_widget::build;

	widget* build() const;

private:
	widget::visibility install_status_visibility_;
	widget::visibility install_buttons_visibility_;
};
}
}

#endif
