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

#include "filesystem.hpp"
#include "gettext.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/helper.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/widget.hpp"
#include "gui/widgets/window_builder.hpp"
#include "log.hpp"
#include "serialization/parser.hpp"
#include "serialization/preprocessor.hpp"
#include "util.hpp"
#include "video.hpp"
#include "wml_exception.hpp"

#include <cassert>

#define DBG_G LOG_STREAM_INDENT(debug, gui)
#define LOG_G LOG_STREAM_INDENT(info, gui)
#define WRN_G LOG_STREAM_INDENT(warn, gui)
#define ERR_G LOG_STREAM_INDENT(err, gui)

#define DBG_G_D LOG_STREAM_INDENT(debug, gui_draw)
#define LOG_G_D LOG_STREAM_INDENT(info, gui_draw)
#define WRN_G_D LOG_STREAM_INDENT(warn, gui_draw)
#define ERR_G_D LOG_STREAM_INDENT(err, gui_draw)

#define DBG_G_E LOG_STREAM_INDENT(debug, gui_event)
#define LOG_G_E LOG_STREAM_INDENT(info, gui_event)
#define WRN_G_E LOG_STREAM_INDENT(warn, gui_event)
#define ERR_G_E LOG_STREAM_INDENT(err, gui_event)

#define DBG_G_P LOG_STREAM_INDENT(debug, gui_parse)
#define LOG_G_P LOG_STREAM_INDENT(info, gui_parse)
#define WRN_G_P LOG_STREAM_INDENT(warn, gui_parse)
#define ERR_G_P LOG_STREAM_INDENT(err, gui_parse)

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
	LOG_G << "Setting: init gui.\n";

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
		ERR_G_P << "Setting: could not read file '" << filename << "'.\n";
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
 * @page = GUIToolkitWML
 * @order = 1
 *
 * = GUI =
 *
 * The gui class contains the definitions of all widgets and windows used in
 * the game. This can be seen as a skin and it allows the user to define the
 * visual aspect of the various items. The visual aspect can be determined
 * depending on the size of the game window.
 * 
 * Widgets have a definition and an instance, the definition contains the 
 * general info/looks of a widget and the instance the actual looks. Eg the
 * where the button text is placed is the same for every button, but the
 * text of every button might differ. 
 *
 * The default gui has the id ''default'' and must exist, in the default gui
 * there must a definition of every widget with the id ''default'' and every
 * window needs to be defined. If the definition of a widget with a certain
 * id doesn't exist it will fall back to default in the current gui, if it's
 * not defined there either it will fall back to the default widget in the
 * default theme. That way it's possible to slowly create your own gui and
 * test it.
 *
 * The gui has the following data:
 * @start_table = config
 *     id (string)                   Unique id for this gui (theme).
 *     description (t_string)        Unique translatable name for this gui.
 *
 *     widget_definitions (section)  The defintions of all 
 *                                   [[#widget_list|widgets]].
 *     window (section)              The defintions of all 
 *                                   [[#window_list|windows]].
 * @end_table
 *
 * <span id="widget_list"></span>List of available widgets:
 * @start_table = widget_definition
 *     button_definition             A push button.
 *     label_definition              A label.
 *     text_box_definition           A single line text box.
 *     tooltip_definition            A small tooltip with help.
 *     window_definition             A window.
 * @end_table
 *
 * <span id="window_list"></span>List of available windows:
 * @start_table = window_definition
 *     addon_connect                 The dialog to connect to the addon server
 *                                   and maintain locally installed addons.
 * @end_table
 *
 */
	id = cfg["id"];
	description = cfg["description"];

	VALIDATE(!id.empty(), missing_mandatory_wml_key("gui", "id"));
	VALIDATE(!description.empty(), missing_mandatory_wml_key("gui", "description"));

	DBG_G_P << "Parsing gui " << id << '\n';

	/***** Window definitions *****/
	const config::child_list& window_cfgs = cfg.get_children("window_definition");
	for(std::vector<config*>::const_iterator itor = window_cfgs.begin();
			itor != window_cfgs.end(); ++itor) {

		std::pair<std::string, twindow_definition> child;
		child.first = child.second.read(**itor);
		windows.insert(child);
	}

	VALIDATE(windows.find("default") != windows.end(), _ ("No default window defined."));

	/***** Control definitions *****/
	load_definitions<tbutton_definition>("button", cfg.get_children("button_definition"));
	load_definitions<tlabel_definition>("label", cfg.get_children("label_definition"));
	load_definitions<ttext_box_definition>("text_box", cfg.get_children("text_box_definition"));
	load_definitions<ttooltip_definition>("tooltip", cfg.get_children("tooltip_definition"));

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

template<class T>
void tgui_definition::load_definitions(const std::string& definition_type, const config::child_list& definition_list)
{
	for(std::vector<config*>::const_iterator itor = definition_list.begin();
			itor != definition_list.end(); ++itor) {

		T* def = new T(**itor);

		control_definition[definition_type].insert(std::make_pair(def->id, def));
	}

	// FIXME use proper control name
	VALIDATE(control_definition[definition_type].find("default") != control_definition[definition_type].end(), _ ("No default button defined."));
}

tcontrol_definition::tcontrol_definition(const config& cfg) :
	id(cfg["id"]),
	description(cfg["description"]),
	resolutions()
{
/*WIKI
 * @page = GUIToolkitWML
 * @order = 1_widget
 *
 * = Widget defintion =
 *
 * Every widget has some parts in common, first of all every definition has the
 * following fields.
 *
 * @start_table = config
 *     id (string)                   Unique id for this gui (theme).
 *     description (t_string)        Unique translatable name for this gui.
 *
 *     resolution (section)          The definitions of the widget in various
 *                                   resolutions.
 * @end_table
 *
 */

	VALIDATE(!id.empty(), missing_mandatory_wml_key("gui", "id"));
	VALIDATE(!description.empty(), missing_mandatory_wml_key("gui", "description"));

}

tresolution_definition_::tresolution_definition_(const config& cfg) :
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
	text_font_style(decode_font_style(cfg["text_font_style"])),
	state()
{
/*WIKI
 * @page = GUIToolkitWML
 * @order = 1_widget
 *
 * == Resolution ==
 *
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
 * @start_table = config
 *     window_width (unsigned = 0)   Width of the application window.
 *     window_height (unsigned = 0) 
 *                                   Height of the application window.
 *     min_width (unsigned = 0)      The minimum width of the widget.
 *     min_height (unsigned = 0)     The minimum height of the widget.
 *
 *     default_width (unsigned = 0)  The default width of the widget.
 *     default_height (unsigned = 0) The default height of the widget.
 *
 *     max_width (unsigned = 0)      The maximum width of the widget.
 *     max_height (unsigned = 0)     The maximum height of the widget.
 *
 *     text_extra_width (unsigned = 0)
 *                                   The extra width needed to determine the 
 *                                   minimal size for the text.
 *     text_extra_height (unsigned = 0)
 *                                   The extra height needed to determine the
 *                                   minimal size for the text.
 *     text_font_size (unsigned)     The font size, which needs to be used to 
 *                                   determine the minimal size for the text.
 *     text_font_style (font_style = "")  
 *                                   The font style, which needs to be used to
 *                                   determine the minimal size for the text.
 *
 *     state (section)               Every widget has one or more state sections.
 *                                   Note they aren't called state but state_xxx
 *                                   the exact names are listed per widget.
 * @end_table
 *
 */

	VALIDATE(text_font_size, missing_mandatory_wml_key("resolution", "text_font_size"));

	DBG_G_P << "Parsing resolution " 
		<< window_width << ", " << window_height << '\n';
}

template<class T>
void tcontrol_definition::load_resolutions(const config::child_list& resolution_list)
{

	VALIDATE(!resolution_list.empty(), _("No resolution defined."));
	for(std::vector<config*>::const_iterator itor = resolution_list.begin();
			itor != resolution_list.end(); ++itor) {

		resolutions.push_back(new T(**itor));
	}
}

tstate_definition::tstate_definition(const config* cfg) :
	full_redraw(cfg ? utils::string_bool((*cfg)["full_redraw"]) : false),
	canvas()
{
/*WIKI
 * @page = GUIToolkitWML
 * @order = 1_widget
 *
 * == State ==
 *
 * Definition of a state. A state contains the info what to do in a state.
 * Atm this is rather focussed on the drawing part, might change later.
 * Keys: 
 * @start_table = config
 *     full_redraw (bool = false)      Does this state need a full redraw when
 *                                     it's being drawn? Normally only required
 *                                     if the widget is (partly) transparent.
 *     draw (section)                  Section with drawing directions for a canvas.
 * @end_table
 *
 */

	const config* draw = cfg ? cfg->child("draw") : 0;

	VALIDATE(draw, _("No state or draw section defined."));

	canvas.set_cfg(*draw);
}

/*WIKI
 * @page = GUIToolkitWML
 * @order = 1_widget
 *
 * = Example =
 *
 * FIXME add a example here.
 * 
 * = List of widgets =
 *
 * Below the list of available widgets.
 *
 */

tbutton_definition::tbutton_definition(const config& cfg) :
	tcontrol_definition(cfg)
{
	DBG_G_P << "Parsing button " << id << '\n';

	load_resolutions<tresolution>(cfg.get_children("resolution"));
}

tbutton_definition::tresolution::tresolution(const config& cfg) :
	tresolution_definition_(cfg)
{
/*WIKI
 * @page = GUIToolkitWML
 * @order = 1_widget_button
 *
 * == Button ==
 *
 * The definition of a normal push button.
 *
 * The following states exist:
 * * state_enabled, the button is enabled.
 * * state_disabled, the button is disabled.
 * * state_pressed, the left mouse button is down.
 * * state_focussed, the mouse is over the button.
 *
 */

	// Note the order should be the same as the enum tstate is button.hpp.
	state.push_back(tstate_definition(cfg.child("state_enabled")));
	state.push_back(tstate_definition(cfg.child("state_disabled")));
	state.push_back(tstate_definition(cfg.child("state_pressed")));
	state.push_back(tstate_definition(cfg.child("state_focussed")));
}

tlabel_definition::tlabel_definition(const config& cfg) :
	tcontrol_definition(cfg)
{
	DBG_G_P << "Parsing label " << id << '\n';

	load_resolutions<tresolution>(cfg.get_children("resolution"));
}


tlabel_definition::tresolution::tresolution(const config& cfg) :
	tresolution_definition_(cfg)
{
/*WIKI
 * @page = GUIToolkitWML
 * @order = 1_widget_label
 *
 * == Label ==
 *
 * The definition of a normal label.
 * 
 * The following states exist:
 * * state_enabled, the label is enabled.
 * * state_disabled, the label is disabled.
 *
 */

	// Note the order should be the same as the enum tstate is label.hpp.
	state.push_back(tstate_definition(cfg.child("state_enabled")));
	state.push_back(tstate_definition(cfg.child("state_disabled")));
}

ttext_box_definition::ttext_box_definition(const config& cfg) :
	tcontrol_definition(cfg)
{
	DBG_G_P << "Parsing text_box " << id << '\n';

	load_resolutions<tresolution>(cfg.get_children("resolution"));
}

ttext_box_definition::tresolution::tresolution(const config& cfg) :
	tresolution_definition_(cfg),
	text_x_offset(cfg["text_x_offset"]),
	text_y_offset(cfg["text_y_offset"])
{
/*WIKI
 * @page = GUIToolkitWML
 * @order = 1_widget_text_box
 *
 * == Text box ==
 *
 * The definition of a text box.
 *
 * The resolution for a text box also contains the following keys:
 * @start_table = config
 *     text_x_offset (f_unsigned = "") The x offset of the text in the text
 *                                     box. This is needed for the code to 
 *                                     determine where in the text the mouse 
 *                                     clicks, so it can set the cursor
 *                                     properly.
 *     text_y_offset (f_unsigned = "") The y offset of the text in the text
 *                                     box.
 * @end_table
 *
 * The following states exist:
 * * state_enabled, the text box is enabled.
 * * state_disabled, the text box is disabled.
 * * state_focussed, the text box has the focus of the keyboard.
 *
 */

	// Note the order should be the same as the enum tstate is text_box.hpp.
	state.push_back(tstate_definition(cfg.child("state_enabled")));
	state.push_back(tstate_definition(cfg.child("state_disabled")));
	state.push_back(tstate_definition(cfg.child("state_focussed")));
}

ttooltip_definition::ttooltip_definition(const config& cfg) : 
	tcontrol_definition(cfg)
{
	DBG_G_P << "Parsing tooltip " << id << '\n';

	load_resolutions<tresolution>(cfg.get_children("resolution"));
}

ttooltip_definition::tresolution::tresolution(const config& cfg) :
	tresolution_definition_(cfg)
{
/*WIKI
 * @page = GUIToolkitWML
 * @order = 1_widget_tooltip
 *
 * == Tooltip ==
 *
 * The definition of a tooltip.
 *
 * The following states exist:
 * * state_enabled, the tooltip has only one state, it's either shown or hidden.
 *
 */

	// Note only one state for a tooltip.
	state.push_back(tstate_definition(cfg.child("state_enabled")));
}

const std::string& twindow_definition::read(const config& cfg)
{
/*WIKI (FIXME cleanup)
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

	DBG_G_P << "Parsing window " << id << '\n';

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
/*WIKI (FIXME cleanup)
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

	DBG_G_P << "Parsing resolution " 
		<< window_width << ", " << window_height << '\n';
}

twindow_definition::tresolution::tlayer::tlayer(const config* cfg) :
	canvas()
{
	const config* draw = cfg ? cfg->child("draw") : 0;

	VALIDATE(draw, _("No layer or draw section defined."));

	canvas.set_cfg(*draw);
}

tresolution_definition_* get_control(const std::string& control_type, const std::string& definition)
{
	const tgui_definition::tcontrol_definition_map::const_iterator	
		control_definition = current_gui->second.control_definition.find(control_type);

	assert(control_definition != current_gui->second.control_definition.end());

	std::map<std::string, tcontrol_definition*>::const_iterator 
		control = control_definition->second.find(definition);

	if(control == control_definition->second.end()) {
		LOG_G << "Control: type '" << control_type << "' definition '" 
			<< definition << "' not found, falling back to 'default'.\n";
		control = control_definition->second.find("default");
		assert(control != control_definition->second.end());
	}

	for(std::vector<tresolution_definition_*>::const_iterator 
			itor = (*control->second).resolutions.begin(),
			end = (*control->second).resolutions.end();
			itor != end;
			++itor) {

		if(screen_width <= (**itor).window_width &&
				screen_height <= (**itor).window_height) {

			return *itor;
		} else if (itor == end - 1) {
			return *itor;
		}
	}

	assert(false);
}

std::vector<twindow_definition::tresolution>::const_iterator get_window(const std::string& definition)
{
	std::map<std::string, twindow_definition>::const_iterator 
		window = current_gui->second.windows.find(definition);

	if(window == current_gui->second.windows.end()) {
		LOG_G << "Window: definition '" 
			<< definition << "' not found, falling back to 'default'.\n";
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
