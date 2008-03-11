/* $Id$ */
/*
   Copyright (C) 2007 - 2008 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file setting.cpp
//! Implementation of settings.hpp.

#include "gui/widgets/settings.hpp"

#include "config.hpp"
#include "filesystem.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/widget.hpp"
#include "log.hpp"
#include "serialization/parser.hpp"
#include "serialization/preprocessor.hpp"


#define DBG_GUI LOG_STREAM(debug, widget)
#define LOG_GUI LOG_STREAM(info, widget)
#define WRN_GUI LOG_STREAM(warn, widget)
#define ERR_GUI LOG_STREAM(err, widget)

namespace gui2 {

namespace {

//	config gui_;
} // namespace

unsigned tbutton::default_width_ = 10;
unsigned tbutton::default_height_ = 10;
config tbutton::default_enabled_draw_ = config();


void load_settings() 
{
	LOG_GUI << "Init gui\n";
	config file;
	const std::string& filename = "data/hardwired/gui.cfg";
	try {
		scoped_istream stream = preprocess_file(filename);
		read(file, *stream);
	} catch(config::error&) {
		ERR_GUI << "Could not read file '" << filename << "'\n";
	}

	config* gui = file.child("gui");
	if(gui) {
/*
		config* settings = gui->child("setting");
		if(settings) {

		} else {
			ERR_GUI << "[settings] not found in [gui].\n";
		}
*/
		config* button = gui->child("button");
		if(button) {

			tbutton::default_width_ = lexical_cast_default<unsigned>((*button)["width"]);
			tbutton::default_height_ = lexical_cast_default<unsigned>((*button)["height"]);

			
			config* state_enabled = button->child("state_enabled");
			if(state_enabled) {

				config* draw = state_enabled->child("draw");

				if(draw) {
					tbutton::default_enabled_draw_ = config(*draw);
				} else {
					ERR_GUI << "[draw] not found in [gui][button][state_enabled].\n";
				}
			} else {
				ERR_GUI << "[state_enabled] not found in [gui][button].\n";
			}
		} else {
			ERR_GUI << "[button] not found in [gui].\n";
		}
	} else {
		ERR_GUI << "[gui] not found.\n";
	}
}

} // namespace gui2
