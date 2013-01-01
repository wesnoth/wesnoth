/* $Id$ */
/*
   Copyright (C) 2012 - 2013 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
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

#include "gui/dialogs/dialog.hpp"

#include "addon/validation.hpp"
#include "addon/state.hpp"

#include <boost/array.hpp>

namespace gui2 {

class taddon_filter_options : public tdialog
{
public:

    taddon_filter_options();

	std::vector<bool> displayed_types() const {
		return std::vector<bool>(displayed_types_.begin(), displayed_types_.end());
	}

	void set_displayed_types(const std::vector<bool>& types) {
		read_types_vector(types);
	}

	ADDON_STATUS_FILTER displayed_status() const {
		return displayed_status_;
	}

	void set_displayed_status(ADDON_STATUS_FILTER status) {
		displayed_status_ = status;
	}

private:

	ADDON_STATUS_FILTER displayed_status_;
	boost::array<bool, ADDON_TYPES_COUNT> displayed_types_;
	std::vector<tfield_bool*> displayed_types_fields_;

	void register_displayed_type_field(const std::string& field_id, ADDON_TYPE addon_type);

	void read_types_vector(const std::vector<bool>& v);

	void toggle_all_displayed_types_button_callback(twindow& window);

	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);

	static std::string status_label(ADDON_STATUS_FILTER s);
};


} // end namespace gui2

#endif

