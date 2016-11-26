/*
   Copyright (C) 2012 - 2016 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_ADDON_FILTER_OPTIONS_HPP_INCLUDED
#define GUI_DIALOGS_ADDON_FILTER_OPTIONS_HPP_INCLUDED

#include "gui/dialogs/modal_dialog.hpp"
#include "gui/widgets/group.hpp"

#include "addon/validation.hpp"
#include "addon/state.hpp"

#include <array>
#include <boost/dynamic_bitset.hpp>

namespace gui2
{

class toggle_button;

namespace dialogs
{

class addon_filter_options : public modal_dialog
{
public:
	addon_filter_options();

	boost::dynamic_bitset<> displayed_types() const
	{
		return boost::dynamic_bitset<>(displayed_types_.begin(), displayed_types_.end());
	}

	void set_displayed_types(const boost::dynamic_bitset<>& types)
	{
		read_types_vector(types);
	}

	ADDON_STATUS_FILTER displayed_status() const
	{
		return displayed_status_;
	}

	void set_displayed_status(ADDON_STATUS_FILTER status)
	{
		displayed_status_ = status;
	}

	ADDON_SORT sort() const
	{
		return sort_;
	}

	void set_sort(ADDON_SORT sort)
	{
		sort_ = sort;
	}

	ADDON_SORT_DIRECTION direction() const
	{
		return dir_;
	}

	void set_direction(ADDON_SORT_DIRECTION direction)
	{
		dir_ = direction;
	}

private:
	ADDON_STATUS_FILTER displayed_status_;
	std::array<bool, ADDON_TYPES_COUNT> displayed_types_;
	std::vector<field_bool*> displayed_types_fields_;

	ADDON_SORT sort_;
	ADDON_SORT_DIRECTION dir_;

	typedef std::pair<toggle_button*, ADDON_SORT> sort_toggle;
	typedef std::pair<toggle_button*, ADDON_SORT_DIRECTION> dir_toggle;

	// Dialog display state variables.
	group<ADDON_SORT> sort_tgroup_;
	group<ADDON_SORT_DIRECTION> dir_tgroup_;

	void register_displayed_type_field(const std::string& field_id,
									   ADDON_TYPE addon_type);

	void read_types_vector(const boost::dynamic_bitset<>& v);

	void toggle_all_displayed_types_button_callback(window& window);

	void register_sort_toggle(window& window,
							  const std::string& toggle_id,
							  ADDON_SORT value);
	void register_dir_toggle(window& window,
							 const std::string& toggle_id,
							 ADDON_SORT_DIRECTION value);

	void toggle_sort_callback();
	void toggle_dir_callback();

	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from modal_dialog. */
	void pre_show(window& window);

	/** Inherited from modal_dialog. */
	void post_show(window& window);

	static std::string status_label(ADDON_STATUS_FILTER s);
};

} // namespace dialogs
} // end namespace gui2

#endif
