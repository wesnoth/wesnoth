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

	//! Map with all known guis.
	std::map<std::string, tgui_definition> guis;

	//! Points to the current gui.
	std::map<std::string, tgui_definition>::const_iterator current_gui = guis.end();


} // namespace 

void load_settings() 
{
	LOG_GUI << "Init gui\n";


	const SDL_Rect rect = screen_area();
	screen_width = rect.w;
	screen_height = rect.h;

	config cfg;
	const std::string& filename = "data/hardwired/gui.cfg";
	try {
		scoped_istream stream = preprocess_file(filename);
		read(cfg, *stream);
	} catch(config::error&) {
		ERR_GUI << "Could not read file '" << filename << "'\n";
	}

	const config::child_list& cfgs = cfg.get_children("gui");
	for(std::vector<config*>::const_iterator itor = cfgs.begin();
			itor != cfgs.end(); ++itor) {

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

	const config::child_list& button_cfgs = cfg.get_children("button_definition");
	for(std::vector<config*>::const_iterator itor = button_cfgs.begin();
			itor != button_cfgs.end(); ++itor) {

		std::pair<std::string, tbutton_definition> child;
		child.first = child.second.read(**itor);
		buttons.insert(child);
	}

	VALIDATE(buttons.find("default") != buttons.end(), _ ("No default button defined."));

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

//tbutton_definition::tresolution* 
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
		} else if (itor == end) {
			return itor;
		}
	}

	assert(false);
}

} // namespace gui2
