/* $Id$ */
/*
   Copyright (C) 2007 - 2010 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file src/gui/widgets/settings.cpp
 * Implementation of settings.hpp.
 */

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "asserts.hpp"
#include "config_cache.hpp"
#include "filesystem.hpp"
#include "foreach.hpp"
#include "gettext.hpp"
#include "gui/auxiliary/log.hpp"
#include "gui/auxiliary/widget_definition/button.hpp"
#include "gui/auxiliary/widget_definition/horizontal_scrollbar.hpp"
#include "gui/auxiliary/widget_definition/image.hpp"
#include "gui/auxiliary/widget_definition/label.hpp"
#include "gui/auxiliary/widget_definition/listbox.hpp"
#include "gui/auxiliary/widget_definition/menubar.hpp"
#include "gui/auxiliary/widget_definition/minimap.hpp"
#include "gui/auxiliary/widget_definition/multi_page.hpp"
#include "gui/auxiliary/widget_definition/panel.hpp"
#include "gui/auxiliary/widget_definition/vertical_scrollbar.hpp"
#include "gui/widgets/window.hpp"
#include "serialization/parser.hpp"
#include "serialization/preprocessor.hpp"
#include "formula_string_utils.hpp"

namespace gui2 {

bool new_widgets = false;

namespace settings {
	unsigned screen_width = 0;
	unsigned screen_height = 0;

	unsigned gamemap_width = 0;
	unsigned gamemap_height = 0;

	unsigned popup_show_delay = 0;
	unsigned popup_show_time = 0;
	unsigned help_show_time = 0;
	unsigned double_click_time = 0;
	unsigned repeat_button_repeat_time = 0;

	std::string sound_button_click = "";
	std::string sound_toggle_button_click = "";
	std::string sound_toggle_panel_click = "";
	std::string sound_slider_adjust = "";

} // namespace settings

namespace {

	/** Map with all known windows, (the builder class builds a window). */
	std::map<std::string, twindow_builder> windows;

	/** Map with all known guis. */
	std::map<std::string, tgui_definition> guis;

	/** Points to the current gui. */
	std::map<std::string, tgui_definition>::const_iterator current_gui = guis.end();

	/**
	 * Vector with all known windows, these are validated on existance on startup.
	 * The enum twindow_type is the index of the array.
	 */
	std::vector<std::string> window_type_list(COUNT);
} // namespace

static void fill_window_types()
{
	window_type_list[ADDON_CONNECT] = "addon_connect";
	window_type_list[ADDON_LIST] = "addon_list";
	window_type_list[CAMPAIGN_SELECTION] = "campaign_selection";
	window_type_list[LANGUAGE_SELECTION] = "language_selection";
	window_type_list[WML_MESSAGE_LEFT] = "wml_message_left";
	window_type_list[WML_MESSAGE_RIGHT] = "wml_message_right";
	window_type_list[MESSAGE] = "message";
	window_type_list[TRANSIENT_MESSAGE] = "transient_message";
	window_type_list[MP_CONNECT] = "mp_connect";
	window_type_list[MP_METHOD_SELECTION] = "mp_method_selection";
	window_type_list[MP_SERVER_LIST] = "mp_server_list";
	window_type_list[MP_LOGIN] = "mp_login";
	window_type_list[MP_CMD_WRAPPER] = "mp_cmd_wrapper";
	window_type_list[MP_CREATE_GAME] = "mp_create_game";
	window_type_list[TITLE_SCREEN] = "title_screen";
	window_type_list[GAME_LOAD] = "game_load";
	window_type_list[GAME_DELETE] = "game_delete";
	window_type_list[GAME_SAVE] = "game_save";
	window_type_list[GAME_SAVE_MESSAGE] = "game_save_message";
	window_type_list[GAME_SAVE_OOS] = "game_save_oos";
#ifndef DISABLE_EDITOR
	window_type_list[EDITOR_NEW_MAP] = "editor_new_map";
	window_type_list[EDITOR_GENERATE_MAP] = "editor_generate_map";
	window_type_list[EDITOR_RESIZE_MAP] = "editor_resize_map";
	window_type_list[EDITOR_SETTINGS] = "editor_settings";
#endif
	window_type_list[LOBBY_MAIN] = "lobby_main";
	window_type_list[LOBBY_PLAYER_INFO] = "lobby_player_info";
	window_type_list[UNIT_CREATE] = "unit_create";
	window_type_list[FORMULA_DEBUGGER] = "formula_debugger";
	window_type_list[GAMESTATE_INSPECTOR] = "gamestate_inspector";
}

const std::string& get_id(const twindow_type window_type)
{
	assert(window_type >= 0 && window_type < COUNT);

	return window_type_list[window_type];
}

void load_settings()
{
	LOG_GUI_G << "Setting: init gui.\n";

	// Init.
	fill_window_types();

	twindow::update_screen_size();

	// Read file.
	config cfg;
	try {
		preproc_map preproc(
				game_config::config_cache::instance().get_preproc_map());
		scoped_istream stream = preprocess_file(get_wml_location("gui/default.cfg"), &preproc);

		read(cfg, *stream);
	} catch(config::error&) {
		ERR_GUI_P << "Setting: could not read file 'data/gui/default.cfg'.\n";
	}

	// Parse guis
	foreach (const config &g, cfg.child_range("gui")) {
		std::pair<std::string, tgui_definition> child;
		child.first = child.second.read(g);
		guis.insert(child);
	}

	VALIDATE(guis.find("default") != guis.end(), _ ("No default gui defined."));

	current_gui = guis.find("default");
	current_gui->second.activate();
}

const std::string& tgui_definition::read(const config& cfg)
{
/*WIKI
 * @page = GUIToolkitWML
 * @order = 1
 *
 * THIS PAGE IS AUTOMATICALLY GENERATED, DO NOT MODIFY DIRECTLY !!!
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
 *     settings (section)            The settings for the gui.
 * @end_table
 *
 * <span id="widget_list"></span>List of available widgets:
 * @start_table = widget_overview
 *     Button                        @macro = button_description
 *     Image                         @macro = image_description
 *     Horizontal_listbox            @macro = horizontal_listbox_description
 *     Horizontal_scrollbar          @macro = horizontal_scrollbar_description
 *     Label                         @macro = label_description
 *     Listbox                       @macro = listbox_description
 *     Menubar                       A menubar which is used in menus and the
 *                                   tabbar in a tabcontrol. (NOTE this widget
 *                                   is still under heavy development.)
 *     Minimap                       @macro = minimap_description
 *     Multi_page                    @macro = multi_page_description
 *     Panel                         @macro = panel_description
 *     Repeating_button              @macro = repeating_button_description
 *     Scroll_label                  @macro = scroll_label_description
 *     Slider                        @macro = slider_description
 *     Spacer                        @macro = spacer_description
 *     Stacked_widget                A stacked widget is a control several
 *                                   widgets can be stacked on top of each
 *                                   other in the same space. This is mainly
 *                                   intended for over- and underlays. (The
 *                                   widget is still experimental.)
 *     Text_box                      A single line text box.
 *     Toggle_button                 A kind of button with two 'states' normal
 *                                   and selected. This is a more generic widget
 *                                   which is used for eg checkboxes and
 *                                   radioboxes.
 *     Toggle_panel                  Like a toggle button but then as panel so
 *                                   can hold multiple items in a grid.
 *     Tooltip                       A small tooltip with help.
 *     Vertical_scrollbar            A vertical scrollbar.
 *     Window                        A window.
 * @end_table
 *
 * <span id="window_list"></span>List of available windows:
 * @start_table = window_overview
 *     Addon_connect                 The dialog to connect to the addon server
 *                                   and maintain locally installed addons.
 *     Addon_list                    Shows the list of the addons to install or
 *                                   update.
 *     Campaign_selection            Shows the list of campaigns, to select one
 *                                   to play.
 *     Language_selection            The dialog to select the primairy language.
 *     WML_message_left              The ingame message dialog with a portrait
 *                                   on the left side. (Used for the WML messages.)
 *     WML_message_right             The ingame message dialog with a portrait
 *                                   on the right side. (Used for the WML
 *                                   messages.)
 *     Message                       A generic message dialog.
 *     MP_connect                    The dialog to connect to the MP server.
 *     MP_method_selection           The dialog to select the kind of MP game
 *                                   to play. Official server, local etc.
 *     MP_server_list                List of the 'official' MP servers.
 *     MP_login                      The dialog to provide a password for registered
 *                                   usernames, request a password reminder or
 *                                   choose a different username.
 *     MP_cmd_wrapper                Perform various actions on the selected user
 *                                   (e.g. whispering or kicking).
 *     MP_create_game                The dialog to select and create an MP game.
 *     Title_screen                  The title screen.
 *     Editor_new_map                Creates a new map in the editor.
 *     Editor_generate_map           Generates a random map in the editor.
 *     Editor_resize_map             Resizes a map in the editor.
 *     Editor_settings               The settings specific for the editor.
 * @end_table
 *
 */
	id = cfg["id"];
	description = cfg["description"];

	VALIDATE(!id.empty(), missing_mandatory_wml_key("gui", "id"));
	VALIDATE(!description.empty(), missing_mandatory_wml_key("gui", "description"));

	DBG_GUI_P << "Parsing gui " << id << '\n';

	/***** Control definitions *****/
	load_definitions<tbutton_definition>("button", cfg);
	// share the definition of the normal listbox.
	load_definitions<tlistbox_definition>("horizontal_listbox", cfg);
	load_definitions<thorizontal_scrollbar_definition>("horizontal_scrollbar", cfg);
	load_definitions<timage_definition>("image", cfg);
	load_definitions<tlabel_definition>("label", cfg);
	load_definitions<tlistbox_definition>("listbox", cfg);
	load_definitions<tmenubar_definition>("menubar", cfg);
	load_definitions<tminimap_definition>("minimap", cfg);
	load_definitions<tmulti_page_definition>("multi_page", cfg);
	load_definitions<tstacked_widget_definition>("stacked_widget", cfg);
	load_definitions<tpanel_definition>("panel", cfg);
	load_definitions<trepeating_button_definition>("repeating_button", cfg);
	load_definitions<tscroll_label_definition>("scroll_label", cfg);
	load_definitions<tscrollbar_panel_definition>("scrollbar_panel", cfg);
	load_definitions<tslider_definition>("slider", cfg);
	load_definitions<tspacer_definition>("spacer", cfg);
	load_definitions<ttext_box_definition>("text_box", cfg);
	// use the same definition for password boxes
	load_definitions<ttext_box_definition>("password_box", cfg, "text_box_definition");
	load_definitions<ttoggle_button_definition>("toggle_button", cfg);
	load_definitions<ttoggle_panel_definition>("toggle_panel", cfg);
	load_definitions<ttooltip_definition>("tooltip", cfg);
	load_definitions<tvertical_scrollbar_definition>("vertical_scrollbar", cfg);
	load_definitions<twindow_definition>("window", cfg);

	/***** Window types *****/
	foreach (const config &w, cfg.child_range("window")) {
		std::pair<std::string, twindow_builder> child;
		child.first = child.second.read(w);
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

	/***** settings *****/
/*WIKI
 * @page = GUIToolkitWML
 * @order = 1
 *
 * A setting section has the following variables:
 * @start_table = config
 *     popup_show_delay (unsigned = 0) The time it take before the popup shows
 *                                     if the mouse moves over the widget. 0
 *                                     means show directly.
 *     popup_show_time (unsigned = 0)  The time a shown popup remains visible.
 *                                     0 means until the mouse leaves the
 *                                     widget.
 *     help_show_time (unsigned = 0)   The time a shown help remains visible.
 *                                     0 means until the mouse leaves the
 *                                     widget.
 *     double_click_time (unsigned)    The time between two clicks to still be a
 *                                     double click.
 *     repeat_button_repeat_time (unsigned = 0)
 *                                     The time a repeating button waits before
 *                                     the next event is issued if the button
 *                                     is still pressed down.
 *
 *     sound_button_click (string = "")
 *                                     The sound played if a button is
 *                                     clicked.
 *     sound_toggle_button_click (string = "")
 *                                     The sound played if a toggle button is
 *                                     clicked.
 *     sound_toggle_panel_click (string = "")
 *                                     The sound played if a toggle panel is
 *                                     clicked. Normally the toggle panels
 *                                     are the items in a listbox. If a
 *                                     toggle button is in the listbox it's
 *                                     sound is played.
 *     sound_slider_adjust (string = "")
 *                                     The sound played if a slider is
 *                                     adjusted.
 * @end_table
 */

/**
 * @todo Regarding sounds:
 * Need to evaluate but probably we want the widget definition be able to:
 * - Override the default (and clear it). This will allow toggle buttons in a
 *   listbox to sound like a toggle panel.
 * - Override the default and above per instance of the widget, some buttons
 *   can give a different sound.
 */
	const config &settings = cfg.child("settings");

	popup_show_delay_ = lexical_cast_default<unsigned>(settings["popup_show_delay"]);
	popup_show_time_ = lexical_cast_default<unsigned>(settings["popup_show_time"]);
	help_show_time_ = lexical_cast_default<unsigned>(settings["help_show_time"]);
	double_click_time_ = lexical_cast_default<unsigned>(settings["double_click_time"]);

	repeat_button_repeat_time_ = lexical_cast_default<unsigned>(
				settings["repeat_button_repeat_time"]);

	VALIDATE(double_click_time_, missing_mandatory_wml_key("settings", "double_click_time"));

	sound_button_click_ = settings["sound_button_click"];
	sound_toggle_button_click_ = settings["sound_toggle_button_click"];
	sound_toggle_panel_click_ = settings["sound_toggle_panel_click"];
	sound_slider_adjust_ = settings["sound_slider_adjust"];

	return id;
}

void tgui_definition::activate() const
{
	settings::popup_show_delay = popup_show_delay_;
	settings::popup_show_time = popup_show_time_;
	settings::help_show_time = help_show_time_;
	settings::double_click_time = double_click_time_;
	settings::repeat_button_repeat_time = repeat_button_repeat_time_;
	settings::sound_button_click = sound_button_click_;
	settings::sound_toggle_button_click = sound_toggle_button_click_;
	settings::sound_toggle_panel_click = sound_toggle_panel_click_;
	settings::sound_slider_adjust = sound_slider_adjust_;
}

template<class T>
void tgui_definition::load_definitions(
	const std::string &definition_type, const config &cfg, const char *key)
{
	foreach (const config &d, cfg.child_range(key ? key : definition_type + "_definition"))
	{
		T* def = new T(d);

		// We assume all definitions are unique if not we would leak memory.
		assert(control_definition[definition_type].find(def->id)
			== control_definition[definition_type].end());

		control_definition[definition_type].insert(std::make_pair(def->id, def));
	}

	utils::string_map symbols;
	symbols["definition"] = definition_type;
	symbols["id"] = "default";
	t_string msg(vgettext(
		"Widget definition '$definition' doesn't contain the definition for '$id'.",
		symbols));
	VALIDATE(control_definition[definition_type].find("default")
		!= control_definition[definition_type].end(), msg);
}

tstate_definition::tstate_definition(const config &cfg) :
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
 *     draw (section)                  Section with drawing directions for a canvas.
 * @end_table
 *
 */

	const config &draw = *(cfg ? &cfg.child("draw") : &cfg);

	VALIDATE(draw, _("No state or draw section defined."));

	canvas.set_cfg(draw);
}


trepeating_button_definition::trepeating_button_definition(const config& cfg) :
	tcontrol_definition(cfg)
{
	DBG_GUI_P << "Parsing repeating button " << id << '\n';

	load_resolutions<tresolution>(cfg);
}

trepeating_button_definition::tresolution::tresolution(const config& cfg) :
	tresolution_definition_(cfg)
{
/*WIKI
 * @page = GUIWidgetDefinitionWML
 * @order = 1_repeating_button
 *
 * == Repeating button ==
 *
 * @macro = repeating_button_description
 *
 * The following states exist:
 * * state_enabled, the repeating_button is enabled.
 * * state_disabled, the repeating_button is disabled.
 * * state_pressed, the left mouse repeating_button is down.
 * * state_focussed, the mouse is over the repeating_button.
 *
 */
	// Note the order should be the same as the enum tstate in
	// repeating_button.hpp.
	state.push_back(tstate_definition(cfg.child("state_enabled")));
	state.push_back(tstate_definition(cfg.child("state_disabled")));
	state.push_back(tstate_definition(cfg.child("state_pressed")));
	state.push_back(tstate_definition(cfg.child("state_focussed")));
}

tscroll_label_definition::tscroll_label_definition(const config& cfg) :
	tcontrol_definition(cfg)
{
	DBG_GUI_P << "Parsing scroll label " << id << '\n';

	load_resolutions<tresolution>(cfg);
}

tscroll_label_definition::tresolution::tresolution(const config& cfg) :
	tresolution_definition_(cfg),
	grid(NULL)
{
/*WIKI
 * @page = GUIWidgetDefinitionWML
 * @order = 1_scroll_label
 *
 * == Scroll label ==
 *
 * @macro = scroll_label_description
 *
 * This widget is slower as a normal label widget so only use this widget
 * when the scrollbar is required (or expected to become required).
 *
 * @start_table = config
 *     grid (grid)                     A grid containing the widgets for main
 *                                     widget.
 * @end_table
 *
 * TODO we need one definition for a vertical scrollbar since this is the second
 * time we use it.
 *
 * @start_table = container
 *     _content_grid (grid)            A grid which should only contain one
 *                                     label widget.
 *     _scrollbar_grid (grid)          A grid for the scrollbar
 *                                     (Merge with listbox info.)
 * @end_table
 *
 * The following states exist:
 * * state_enabled, the scroll label is enabled.
 * * state_disabled, the scroll label is disabled.
 *
 */
	// Note the order should be the same as the enum tstate is scroll_label.hpp.
	state.push_back(tstate_definition(cfg.child("state_enabled")));
	state.push_back(tstate_definition(cfg.child("state_disabled")));

	const config &child = cfg.child("grid");
	VALIDATE(child, _("No grid defined."));

	grid = new tbuilder_grid(child);
}

tscrollbar_panel_definition::tscrollbar_panel_definition(const config& cfg)
	: tcontrol_definition(cfg)
{

	DBG_GUI_P << "Parsing scrollbar panel " << id << '\n';

	load_resolutions<tresolution>(cfg);
}

tscrollbar_panel_definition::tresolution::tresolution(const config& cfg)
	: tresolution_definition_(cfg)
	, grid()
{
/*WIKI
 * @page = GUIWidgetDefinitionWML
 * @order = 1_scrollbar_panel
 *
 * == Scrollbar panel ==
 *
 * The definition of a panel with scrollbars. A panel is a container hold
 * other elements in it's grid. A panel is always enabled and can't be
 * disabled. Instead it uses the states as layers to draw on.
 *
 * @start_table = config
 *     grid (grid)                     A grid containing the widgets for main
 *                                     widget.
 * @end_table
 *
 * The following layers exist:
 * * background, the background of the panel.
 * * foreground, the foreground of the panel.
 */

	// The panel needs to know the order.
	state.push_back(tstate_definition(cfg.child("background")));
	state.push_back(tstate_definition(cfg.child("foreground")));

	const config &child = cfg.child("grid");
	VALIDATE(child, _("No grid defined."));

	grid = new tbuilder_grid(child);
}

tslider_definition::tslider_definition(const config& cfg) :
	tcontrol_definition(cfg)
{
	DBG_GUI_P << "Parsing slider " << id << '\n';

	load_resolutions<tresolution>(cfg);
}

tslider_definition::tresolution::tresolution(const config& cfg) :
	tresolution_definition_(cfg),
	minimum_positioner_length(
		lexical_cast_default<unsigned>(cfg["minimum_positioner_length"])),
	maximum_positioner_length(
		lexical_cast_default<unsigned>(cfg["maximum_positioner_length"])),
	left_offset(lexical_cast_default<unsigned>(cfg["left_offset"])),
	right_offset(lexical_cast_default<unsigned>(cfg["right_offset"]))
{
/*WIKI
 * @page = GUIWidgetDefinitionWML
 * @order = 1_slider
 *
 * == Slider ==
 *
 * @macro = slider_description
 *
 * @start_table = config
 *     minimum_positioner_length (unsigned)
 *                                     The minumum size the positioner is
 *                                     allowed to be. The engine needs to know
 *                                     this in order to calculate the best size
 *                                     for the positioner.
 *     maximum_positioner_length (unsigned = 0)
 *                                     The maximum size the positioner is
 *                                     allowed to be. If minimum and maximum are
 *                                     the same value the positioner is fixed
 *                                     size. If the maximum is 0 (and the
 *                                     minimum not) there's no maximum.
 *     left_offset (unsigned = 0)      The number of pixels at the left side
 *                                     which can't be used by the positioner.
 *     right_offset (unsigned = 0)     The number of pixels at the right side
 *                                     which can't be used by the positioner.
 * @end_table
 *
 * The following states exist:
 * * state_enabled, the slider is enabled.
 * * state_disabled, the slider is disabled.
 * * state_pressed, the left mouse button is down on the positioner of the slider.
 * * state_focussed, the mouse is over the positioner of the slider.
 */

	VALIDATE(minimum_positioner_length,
		missing_mandatory_wml_key("resolution", "minimum_positioner_length"));

	// Note the order should be the same as the enum tstate is slider.hpp.
	state.push_back(tstate_definition(cfg.child("state_enabled")));
	state.push_back(tstate_definition(cfg.child("state_disabled")));
	state.push_back(tstate_definition(cfg.child("state_pressed")));
	state.push_back(tstate_definition(cfg.child("state_focussed")));
}

tspacer_definition::tspacer_definition(const config& cfg) :
	tcontrol_definition(cfg)
{
	DBG_GUI_P << "Parsing spacer " << id << '\n';

	load_resolutions<tresolution>(cfg);
}


tspacer_definition::tresolution::tresolution(const config& cfg) :
	tresolution_definition_(cfg)
{
/*WIKI
 * @page = GUIWidgetDefinitionWML
 * @order = 1_spacer
 *
 * == Spacer ==
 *
 * @macro = spacer_description
 *
 * A spacer has no states so nothing to load.
 */
}

tstacked_widget_definition::tstacked_widget_definition(const config& cfg) :
	tcontrol_definition(cfg)
{
	DBG_GUI_P << "Parsing stacked widget " << id << '\n';

	load_resolutions<tresolution>(cfg);
}

tstacked_widget_definition::tresolution::tresolution(const config& cfg) :
	tresolution_definition_(cfg),
	grid(NULL)
{
/*WIKI
 * @page = GUIWidgetDefinitionWML
 * @order = 1_stacked_widget
 *
 * == Multi page ==
 *
 * The documentation is not written yet.
 */

	// Add a dummy state since every widget needs a state.
	static config dummy ("draw");
	state.push_back(tstate_definition(dummy));

	const config &child = cfg.child("grid");
	VALIDATE(child, _("No grid defined."));

	grid = new tbuilder_grid(child);
}

ttext_box_definition::ttext_box_definition(const config& cfg) :
	tcontrol_definition(cfg)
{
	DBG_GUI_P << "Parsing text_box " << id << '\n';

	load_resolutions<tresolution>(cfg);
}

ttext_box_definition::tresolution::tresolution(const config& cfg) :
	tresolution_definition_(cfg),
	text_x_offset(cfg["text_x_offset"]),
	text_y_offset(cfg["text_y_offset"])
{
/*WIKI
 * @page = GUIWidgetDefinitionWML
 * @order = 1_text_box
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

ttoggle_button_definition::ttoggle_button_definition(const config& cfg) :
	tcontrol_definition(cfg)
{
	DBG_GUI_P << "Parsing toggle button " << id << '\n';

	load_resolutions<tresolution>(cfg);
}

ttoggle_button_definition::tresolution::tresolution(const config& cfg) :
	tresolution_definition_(cfg)
{
/*WIKI
 * @page = GUIWidgetDefinitionWML
 * @order = 1_toggle_button
 *
 * == Toggle button ==
 *
 * The definition of a toggle button.
 *
 * The following states exist:
 * * state_enabled, the button is enabled and not selected.
 * * state_disabled, the button is disabled and not selected.
 * * state_focussed, the mouse is over the button and not selected.
 *
 * * state_enabled_selected, the button is enabled and selected.
 * * state_disabled_selected, the button is disabled and selected.
 * * state_focussed_selected, the mouse is over the button and selected.
 *
 */

	// Note the order should be the same as the enum tstate is toggle_button.hpp.
	state.push_back(tstate_definition(cfg.child("state_enabled")));
	state.push_back(tstate_definition(cfg.child("state_disabled")));
	state.push_back(tstate_definition(cfg.child("state_focussed")));

	state.push_back(tstate_definition(cfg.child("state_enabled_selected")));
	state.push_back(tstate_definition(cfg.child("state_disabled_selected")));
	state.push_back(tstate_definition(cfg.child("state_focussed_selected")));
}

ttoggle_panel_definition::ttoggle_panel_definition(const config& cfg) :
	tcontrol_definition(cfg)
{
	DBG_GUI_P << "Parsing toggle panel " << id << '\n';

	load_resolutions<tresolution>(cfg);
}

ttoggle_panel_definition::tresolution::tresolution(const config& cfg) :
	tresolution_definition_(cfg),
	top_border(lexical_cast_default<unsigned>(cfg["top_border"])),
	bottom_border(lexical_cast_default<unsigned>(cfg["bottom_border"])),
	left_border(lexical_cast_default<unsigned>(cfg["left_border"])),
	right_border(lexical_cast_default<unsigned>(cfg["right_border"]))
{
/*WIKI
 * @page = GUIWidgetDefinitionWML
 * @order = 1_toggle_panel
 *
 * == Toggle panel ==
 *
 * The definition of a toggle panel. A toggle panel is like a toggle button, but
 * instead of being a button it's a panel. This means it can hold multiple child
 * items.
 *
 * The resolution for a toggle panel also contains the following keys:
 * @start_table = config
 *     top_border (unsigned = 0)     The size which isn't used for the client area.
 *     bottom_border (unsigned = 0)  The size which isn't used for the client area.
 *     left_border (unsigned = 0)    The size which isn't used for the client area.
 *     right_border (unsigned = 0)   The size which isn't used for the client area.
 * @end_table
 *
 * The following layers exist:
 *
 * The following states exist:
 * * state_enabled, the panel is enabled and not selected.
 * * state_disabled, the panel is disabled and not selected.
 * * state_focussed, the mouse is over the panel and not selected.
 *
 * * state_enabled_selected, the panel is enabled and selected.
 * * state_disabled_selected, the panel is disabled and selected.
 * * state_focussed_selected, the mouse is over the panel and selected.
 *
 */

	// Note the order should be the same as the enum tstate is toggle_panel.hpp.
	state.push_back(tstate_definition(cfg.child("state_enabled")));
	state.push_back(tstate_definition(cfg.child("state_disabled")));
	state.push_back(tstate_definition(cfg.child("state_focussed")));

	state.push_back(tstate_definition(cfg.child("state_enabled_selected")));
	state.push_back(tstate_definition(cfg.child("state_disabled_selected")));
	state.push_back(tstate_definition(cfg.child("state_focussed_selected")));
}

ttooltip_definition::ttooltip_definition(const config& cfg) :
	tcontrol_definition(cfg)
{
	DBG_GUI_P << "Parsing tooltip " << id << '\n';

	load_resolutions<tresolution>(cfg);
}

ttooltip_definition::tresolution::tresolution(const config& cfg) :
	tresolution_definition_(cfg)

{
/*WIKI
 * @page = GUIWidgetDefinitionWML
 * @order = 1_tooltip
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

twindow_definition::twindow_definition(const config& cfg) :
	tcontrol_definition(cfg)
{
/*WIKI
 * @page = GUIWidgetDefinitionWML
 * @order = 1_window
 *
 * == Window ==
 *
 * The definition of a window. A window is a kind of panel see the panel for
 * which fields exist
 *
 */

	DBG_GUI_P << "Parsing window " << id << '\n';

	load_resolutions<tresolution>(cfg);
}

twindow_definition::tresolution::tresolution(const config& cfg)
	: tpanel_definition::tresolution(cfg)
	, grid(NULL)
{
	const config &child = cfg.child("grid");
//	VALIDATE(child, _("No grid defined."));

	/** @todo Evaluate whether the grid should become mandatory. */
	if(child) {
		grid = new tbuilder_grid(child);
	}
}

tresolution_definition_ptr get_control(
		const std::string& control_type, const std::string& definition)
{
	const tgui_definition::tcontrol_definition_map::const_iterator
		control_definition = current_gui->second.control_definition.find(control_type);

	ASSERT_LOG(control_definition != current_gui->second.control_definition.end(),
			"Type '" << control_type << "' is unknown.");

	std::map<std::string, tcontrol_definition_ptr>::const_iterator
		control = control_definition->second.find(definition);

	if(control == control_definition->second.end()) {
		LOG_GUI_G << "Control: type '" << control_type << "' definition '"
			<< definition << "' not found, falling back to 'default'.\n";
		control = control_definition->second.find("default");
		assert(control != control_definition->second.end());
	}

	for(std::vector<tresolution_definition_ptr>::const_iterator
			itor = (*control->second).resolutions.begin(),
			end = (*control->second).resolutions.end();
			itor != end;
			++itor) {

		if(settings::screen_width <= (**itor).window_width &&
				settings::screen_height <= (**itor).window_height) {

			return *itor;
		} else if (itor == end - 1) {
			return *itor;
		}
	}

	ERROR_LOG(false);
}

std::vector<twindow_builder::tresolution>::const_iterator get_window_builder(const std::string& type)
{
	twindow::update_screen_size();

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

		if(settings::screen_width <= itor->window_width &&
				settings::screen_height <= itor->window_height) {

			return itor;
		} else if (itor == end - 1) {
			return itor;
		}
	}

	ERROR_LOG(false);
}

/*WIKI
 * @page = GUIWidgetDefinitionWML
 * @order = ZZZZZZ_footer
 *
 * [[Category: WML Reference]]
 * [[Category: GUI WML Reference]]
 * [[Category: Generated]]
 *
 */

} // namespace gui2
