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
#include "gettext.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/widget.hpp"
#include "gui/widgets/window_builder.hpp"
#include "log.hpp"
#include "serialization/parser.hpp"
#include "serialization/preprocessor.hpp"
#include "util.hpp"
#include "video.hpp"
#include "wml_exception.hpp"

#include <cassert>

#define DBG_GUI LOG_STREAM(debug, widget)
#define LOG_GUI LOG_STREAM(info, widget)
#define WRN_GUI LOG_STREAM(warn, widget)
#define ERR_GUI LOG_STREAM(err, widget)

namespace gui2 {

unsigned screen_width = 0;
unsigned screen_height = 0;

namespace {

	//! Map with all known windows, (the builder class builds a window).
	std::map<std::string, twindow_builder> windows;

	//! Map with all known guis.
	std::map<std::string, tgui_definition> guis;

	//! Points to the current gui.
	std::map<std::string, tgui_definition>::const_iterator current_gui = guis.end();

	//! Vector with all known windows, these are validated on existance on startup.
	//! The enum twindow_type is the index of the array.
	std::vector<std::string> window_type_list(DUMMY);

} // namespace 

static void fill_window_types() 
{
	window_type_list[ADDON_CONNECT] = "addon_connect";
}

const std::string& get_id(const twindow_type window_type)
{
	assert(window_type >= 0 && window_type < DUMMY);

	return window_type_list[window_type];
}

void load_settings() 
{
	LOG_GUI << "Init gui\n";

	fill_window_types();

	const SDL_Rect rect = screen_area();
	screen_width = rect.w;
	screen_height = rect.h;

	config cfg;
	const std::string& filename = "data/gui/default.cfg";
	try {
		scoped_istream stream = preprocess_file(filename);
		read(cfg, *stream);
	} catch(config::error&) {
		ERR_GUI << "Could not read file '" << filename << "'\n";
	}

	const config::child_list& gui_cfgs = cfg.get_children("gui");
	for(std::vector<config*>::const_iterator itor = gui_cfgs.begin();
			itor != gui_cfgs.end(); ++itor) {

		std::pair<std::string, tgui_definition> child;
		child.first = child.second.read(**itor);
		guis.insert(child);
	}

	VALIDATE(guis.find("default") != guis.end(), _ ("No default gui defined."));

	current_gui = guis.find("default");
}

const std::string& tgui_definition::read(const config& cfg)
{
/*WIKI
 * [gui]
 * The gui class contains the definitions of all widgets and windows used in
 * the game. The standard gui has the id 'default' and used as fallback it
 * another gui doesn't define a specific item. NOTE things might look odd when
 * that happens.
 *
 *
 *     id = (string = "")            Unique id for this gui (theme).
 *     description = (t_string = "") Unique translatable name for this gui.
 *
 *     [button_definition]           The definitions of the buttons in this gui.
 * [/gui]
 */
	id = cfg["id"];
	description = cfg["description"];

	VALIDATE(!id.empty(), missing_mandatory_wml_key("gui", "id"));
	VALIDATE(!description.empty(), missing_mandatory_wml_key("gui", "description"));

	std::cerr << "Parsing gui " << id << '\n';

	/***** Window definitions *****/
	const config::child_list& window_cfgs = cfg.get_children("window_definition");
	for(std::vector<config*>::const_iterator itor = window_cfgs.begin();
			itor != window_cfgs.end(); ++itor) {

		std::pair<std::string, twindow_definition> child;
		child.first = child.second.read(**itor);
		windows.insert(child);
	}

	VALIDATE(windows.find("default") != windows.end(), _ ("No default window defined."));

	/***** Button definitions *****/
	const config::child_list& button_cfgs = cfg.get_children("button_definition");
	for(std::vector<config*>::const_iterator itor = button_cfgs.begin();
			itor != button_cfgs.end(); ++itor) {

		std::pair<std::string, tbutton_definition> child;
		child.first = child.second.read(**itor);
		buttons.insert(child);
	}

	VALIDATE(buttons.find("default") != buttons.end(), _ ("No default button defined."));

	/***** Window types *****/
	const config::child_list& window_instance_cfgs = cfg.get_children("window");
	for(std::vector<config*>::const_iterator itor = window_instance_cfgs.begin();
			itor != window_instance_cfgs.end(); ++itor) {

		std::pair<std::string, twindow_builder> child;
		child.first = child.second.read(**itor);
		window_types.insert(child);
	}

	if(id == "default") {
		// The default gui needs to define all window types since we're the 
		// fallback in case another gui doesn't define the window type.
		for(std::vector<std::string>::const_iterator itor = window_type_list.begin();
				itor != window_type_list.end(); ++itor) {

			VALIDATE(window_types.find(*itor) != window_types.end(), _("Window not defined.")); 
		}
	}

	return id;
}

const std::string& tbutton_definition::read(const config& cfg)
{
/*WIKI
 * [button_definition]
 * The definition of a normal push button.
 *
 *     id = (string = "")            Unique id for this gui (theme).
 *     description = (t_string = "") Unique translatable name for this gui.
 *
 *     [resolution]                  The definitions of the button in various
 *                                   resolutions.
 * [/button_definition]
 */
	id = cfg["id"];
	description = cfg["description"];

	VALIDATE(!id.empty(), missing_mandatory_wml_key("gui", "id"));
	VALIDATE(!description.empty(), missing_mandatory_wml_key("gui", "description"));

	std::cerr << "Parsing button " << id << '\n';

	const config::child_list& cfgs = cfg.get_children("resolution");
	VALIDATE(!cfgs.empty(), _("No resolution defined."));
	for(std::vector<config*>::const_iterator itor = cfgs.begin();
			itor != cfgs.end(); ++itor) {

		resolutions.push_back(tresolution(**itor));
	}

	return id;
}

tbutton_definition::tresolution::tresolution(const config& cfg) :
	window_width(lexical_cast_default<unsigned>(cfg["window_width"])),
	window_height(lexical_cast_default<unsigned>(cfg["window_height"])),
	min_width(lexical_cast_default<unsigned>(cfg["min_width"])),
	min_height(lexical_cast_default<unsigned>(cfg["min_height"])),
	default_width(lexical_cast_default<unsigned>(cfg["default_width"])),
	default_height(lexical_cast_default<unsigned>(cfg["default_height"])),
	max_width(lexical_cast_default<unsigned>(cfg["max_width"])),
	max_height(lexical_cast_default<unsigned>(cfg["max_height"])),
	text_extra_width(lexical_cast_default<unsigned>(cfg["text_extra_width"])),
	text_extra_height(lexical_cast_default<unsigned>(cfg["text_extra_height"])),
	text_font_size(lexical_cast_default<unsigned>(cfg["text_font_size"])),
	enabled(cfg.child("state_enabled")),
	disabled(cfg.child("state_disabled")),
	pressed(cfg.child("state_pressed")),
	focussed(cfg.child("state_focussed"))
{
/*WIKI
 * [resolution]
 * Depending on the resolution a widget can look different. Resolutions are
 * evaluated in order of appearance. The ''window_width'' and ''window_height''
 * are the upper limit this resolution is valid for. When one of the sizes
 * gets above the limit, the next resolution is selected. There's one special
 * case where both values are ''0''. This resolution always matches. (Resolution
 * definitions behind that one will never be picked.) This resolution can be
 * used as upper limit or if there's only one resolution.
 *
 * The default (and also minimum) size of a button is determined by two items, 
 * the wanted default size and the size needed for the text. The size of the
 * text differs per used widget so needs to be determined per button. 
 *
 *     window_width = (unsigned = 0) Width of the application window.
 *     window_height = (unsigned = 0) 
 *                                   Height of the application window.
 *     min_width = (unsigned = 0)    The minimum width of the widget.
 *     min_height = (unsigned = 0)   The minimum height of the widget.
 *
 *     default_width = (unsigned = 0)
 *                                   The default width of the widget.
 *     default_height = (unsigned = 0)
 *                                   The default height of the widget.
 *
 *     max_width = (unsigned = 0)    The maximum width of the widget.
 *     max_height = (unsigned = 0)   The maximum height of the widget.
 *
 *     text_extra_width = (unsigned = 0)
 *                                   The extra width needed to determine the 
 *                                   minimal size for the text.
 *     text_extra_height = (unsigned = 0)
 *                                   The extra height needed to determine the
 *                                   minimal size for the text.
 *     text_font_size = (unsigned =0)
 *                                   The font size, which needs to be used to 
 *                                   determine the minimal size for the text.
 *
 *     [state_enabled]               Settings for the enabled state.
 *     [state_disabled]              Settings for the disabled state.
 *     [state_pressed]               Settings for the pressed state.
 *     [state_focussed]              Settings for the focussed state (happens
 *                                   when the mouse moves over the button).
 *
 *
 * [/resolution]
 */

	std::cerr << "Parsing resolution " 
		<< window_width << ", " << window_height << '\n';
}

tbutton_definition::tresolution::tstate::tstate(const config* cfg) :
	canvas()
{
	const config* draw = cfg ? cfg->child("draw") : 0;

	VALIDATE(draw, _("No state or draw section defined."));

	canvas.set_cfg(*draw);
}

const std::string& twindow_definition::read(const config& cfg)
{
/*WIKI
 * [window_definition]
 * The definition of a normal push window.
 *
 *     id = (string = "")            Unique id for this gui (theme).
 *     description = (t_string = "") Unique translatable name for this gui.
 *
 *     [resolution]                  The definitions of the window in various
 *                                   resolutions.
 * [/window_definition]
 */
	id = cfg["id"];
	description = cfg["description"];

	VALIDATE(!id.empty(), missing_mandatory_wml_key("gui", "id"));
	VALIDATE(!description.empty(), missing_mandatory_wml_key("gui", "description"));

	std::cerr << "Parsing window " << id << '\n';

	const config::child_list& cfgs = cfg.get_children("resolution");
	VALIDATE(!cfgs.empty(), _("No resolution defined."));
	for(std::vector<config*>::const_iterator itor = cfgs.begin();
			itor != cfgs.end(); ++itor) {

		resolutions.push_back(tresolution(**itor));
	}

	return id;
}

twindow_definition::tresolution::tresolution(const config& cfg) :
	window_width(lexical_cast_default<unsigned>(cfg["window_width"])),
	window_height(lexical_cast_default<unsigned>(cfg["window_height"])),
	top_border(lexical_cast_default<unsigned>(cfg["top_border"])),
	bottom_border(lexical_cast_default<unsigned>(cfg["bottom_border"])),
	left_border(lexical_cast_default<unsigned>(cfg["left_border"])),
	right_border(lexical_cast_default<unsigned>(cfg["right_border"])),
	min_width(lexical_cast_default<unsigned>(cfg["min_width"])),
	min_height(lexical_cast_default<unsigned>(cfg["min_height"])),
	background(cfg.child("background")),
	foreground(cfg.child("foreground"))
{
/*WIKI
 * [resolution]
 *     window_width = (unsigned = 0) Width of the application window.
 *     window_height = (unsigned = 0) 
 *                                   Height of the application window.
 *     min_width = (unsigned = 0)    The minimum width of the windows.
 *     min_height = (unsigned = 0)   The minimum height of the windows.
 *
 *     top_border = (unsigned = 0)   The size which isn't used for the client area.
 *     bottom_border = (unsigned = 0)The size which isn't used for the client area.
 *     left_border = (unsigned = 0)  The size which isn't used for the client area.
 *     right_border = (unsigned = 0) The size which isn't used for the client area.
 *
 *     [background]                  The things drawn on the window before
 *                                   the widgets are drawn.
 *     [foreground]                  The things drawn on the window on top
 *                                   of the widgets.
 *
 * [/resolution]
 */

	std::cerr << "Parsing resolution " 
		<< window_width << ", " << window_height << '\n';
}

twindow_definition::tresolution::tlayer::tlayer(const config* cfg) :
	canvas()
{
	const config* draw = cfg ? cfg->child("draw") : 0;

	VALIDATE(draw, _("No layer or draw section defined."));

	canvas.set_cfg(*draw);
}

std::vector<tbutton_definition::tresolution>::const_iterator get_button(const std::string& definition)
{
	std::map<std::string, tbutton_definition>::const_iterator 
		button = current_gui->second.buttons.find(definition);

	if(button == current_gui->second.buttons.end()) {
		button = current_gui->second.buttons.find("default");
		assert(button != current_gui->second.buttons.end());
	}

	for(std::vector<tbutton_definition::tresolution>::const_iterator 
			itor = button->second.resolutions.begin(),
			end = button->second.resolutions.end();
			itor != end;
			++itor) {

		if(screen_width <= itor->window_width &&
				screen_height <= itor->window_height) {

			return itor;
		} else if (itor == end - 1) {
			return itor;
		}
	}

	assert(false);
}

std::vector<twindow_definition::tresolution>::const_iterator get_window(const std::string& definition)
{
	std::map<std::string, twindow_definition>::const_iterator 
		window = current_gui->second.windows.find(definition);

	if(window == current_gui->second.windows.end()) {
		window = current_gui->second.windows.find("default");
		assert(window != current_gui->second.windows.end());
	}

	for(std::vector<twindow_definition::tresolution>::const_iterator 
			itor = window->second.resolutions.begin(),
			end = window->second.resolutions.end();
			itor != end;
			++itor) {

		if(screen_width <= itor->window_width &&
				screen_height <= itor->window_height) {

			return itor;
		} else if (itor == end - 1) {
			return itor;
		}
	}

	assert(false);
}

std::vector<twindow_builder::tresolution>::const_iterator get_window_builder(const std::string& type)
{
	std::map<std::string, twindow_builder>::const_iterator 
		window = current_gui->second.window_types.find(type);

	if(true) { // FIXME Test for default gui.
		assert(window != current_gui->second.window_types.end());
	} else {
		// FIXME Get the defintion in the default gui and do an assertion test.
	}

	for(std::vector<twindow_builder::tresolution>::const_iterator 
			itor = window->second.resolutions.begin(),
			end = window->second.resolutions.end();
			itor != end;
			++itor) {

		if(screen_width <= itor->window_width &&
				screen_height <= itor->window_height) {

			return itor;
		} else if (itor == end - 1) {
			return itor;
		}
	}

	assert(false);
}


} // namespace gui2
