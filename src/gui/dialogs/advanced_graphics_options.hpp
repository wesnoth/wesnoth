/*
   Copyright (C) 2012 - 2016 by Chris Beck <render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_ADVANCED_GRAPHICS_OPTIONS_HPP_INCLUDED
#define GUI_DIALOGS_ADVANCED_GRAPHICS_OPTIONS_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"
#include "make_enum.hpp"

namespace gui2
{
class tlabel;
class ttoggle_button;

class tadvanced_graphics_options : public tdialog
{
public:
	/** Constructor. */
	tadvanced_graphics_options();

	/**
	 * The display function.
	 *
	 * See @ref tdialog for more information.
	 */
	static void display(CVideo& video)
	{
		tadvanced_graphics_options().show(video);
	}

	// These names must match the infixes of the widget ids in advanced_graphics_options.cfg
	static const std::vector<std::string> scale_cases;

	// These names must match the suffixes of the widget ids in advanced_graphics_options.cfg
	MAKE_ENUM(SCALING_ALGORITHM,
		(LINEAR,		"linear")
		(NEAREST_NEIGHBOR, 	"nn")
		(XBRZ_LIN,		"xbrzlin")
		(XBRZ_NN,		"xbrznn")
		(LEGACY_LINEAR,		"legacy_lin")
	)

private:
	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);

	void setup_scale_case(const std::string &, twindow &);
	void setup_scale_button(const std::string &, SCALING_ALGORITHM, twindow &);
	void scale_button_callback(std::string, SCALING_ALGORITHM, twindow &);
};

} // end namespace gui2

#endif
