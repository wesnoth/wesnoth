/*
   Copyright (C) 2010 - 2016 by the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_GENERATOR_SETTINGS_HPP_INCLUDED
#define GUI_DIALOGS_GENERATOR_SETTINGS_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"
#include "generators/default_map_generator.hpp"

namespace gui2
{

class tlabel;
class tslider;

class tgenerator_settings : public tdialog
{
public:
	explicit tgenerator_settings(generator_data& data);

	/** The execute function see @ref tdialog for more information. */
	static bool execute(generator_data& data, CVideo& video)
	{
		return tgenerator_settings(data).show(video);
	}

private:
	void pre_show(twindow& window);

	void bind_status_label(twindow& window, const std::string& id, const std::string& suffix = "");
	void status_label_callback(tslider& slider, tlabel& label, const std::string& suffix = "");

	// TODO: find a more generic way to do this
	void bind_landform_status_label(twindow& window);
	void landform_status_label_callback(tslider& slider, tlabel& label);

	void adjust_minimum_size_by_players(twindow& window);

	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** We need to own these fields to access the underlying widget */
	tfield_integer* players_;
	tfield_integer* width_;
	tfield_integer* height_;
};

}

#endif /* ! GUI_DIALOGS_GENERATOR_SETTINGS_HPP_INCLUDED */
