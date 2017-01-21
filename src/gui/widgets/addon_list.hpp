/*
   Copyright (C) 2017 by the Battle for Wesnoth Project http://www.wesnoth.org/

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
#include "addon/state.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/widgets/container_base.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/widget.hpp"
#include <string>

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
		, install_status_visibility_(visibility::visible)
		, install_buttons_visibility_(visibility::invisible)
	{}

	/** Sets the add-ons to show. */
	void set_addons(const addons_list& addons);

	/** Sets up a callback that will be called when the player selects an add-on. */
	void set_callback_value_change(const std::function<void(widget&)>& callback)
	{
		get_listbox().set_callback_value_change(callback);
	}

	/** Returns the index of the selected row. */
	int get_selected_row() const
	{
		const listbox& list = find_widget<const listbox>(&get_grid(), "addons", false);
		return list.get_selected_row();
	}

	/** Returns the underlying list box. */
	listbox& get_listbox()
	{
		return find_widget<listbox>(&get_grid(), "addons", false);
	}

	/** Changes the color of an add-on state string (installed, outdated, etc.)
	according to the state itself.
	This function is here because the add-on list widget itself needs it. */
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
	visibility install_status_visibility_;
	visibility install_buttons_visibility_;

	static std::string describe_status(const addon_tracking_info& info);

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
