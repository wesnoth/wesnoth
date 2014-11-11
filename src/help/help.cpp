/*
   Copyright (C) 2003 - 2014 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file
 * Routines for showing the help-dialog.
 */

#define GETTEXT_DOMAIN "wesnoth-help"

#include "global.hpp"
#include "asserts.hpp"
#include "help.hpp"

#include "about.hpp"
#include "construct_dialog.hpp"
#include "display.hpp"
#include "display_context.hpp"
#include "exceptions.hpp"
#include "game_preferences.hpp"
#include "gettext.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "help_impl.hpp"
#include "help_browser.hpp"
#include "hotkey/hotkey_command.hpp"
#include "language.hpp"
#include "log.hpp"
#include "map.hpp"
#include "marked-up_text.hpp"
#include "resources.hpp"
#include "sound.hpp"
#include "unit.hpp"
#include "unit_helper.hpp"
#include "wml_separators.hpp"
#include "serialization/parser.hpp"
#include "time_of_day.hpp"
#include "tod_manager.hpp"

#include <boost/foreach.hpp>

#include <queue>

static lg::log_domain log_display("display");
#define WRN_DP LOG_STREAM(warn, log_display)

static lg::log_domain log_help("help");
#define WRN_HP LOG_STREAM(warn, log_help)
#define DBG_HP LOG_STREAM(debug, log_help)

namespace help {

void show_unit_description(const unit &u)
{
	help::show_unit_description(u.type());
}

void show_terrain_description(const terrain_type &t)
{
	help::show_terrain_help(*display::get_singleton(), t.id(), t.hide_in_editor() || t.is_combined());
}

void show_unit_description(const unit_type &t)
{
	std::string var_id = t.get_cfg()["variation_id"].str();
	if (var_id.empty())
		var_id = t.get_cfg()["variation_name"].str();
	bool hide_help = t.hide_help();
	bool use_variation = false;
	if (!var_id.empty()) {
		const unit_type *parent = unit_types.find(t.id());
		assert(parent);
		if (hide_help) {
			hide_help = parent->hide_help();
		} else {
			use_variation = true;
		}
	}

	if (use_variation)
		help::show_variation_help(*display::get_singleton(), t.id(), var_id, hide_help);
	else
		help::show_unit_help(*display::get_singleton(), t.id(), !t.variations().empty(), hide_help);
}

help_manager::help_manager(const config *cfg) //, gamemap *_map)
{
	game_cfg = cfg == NULL ? &dummy_cfg : cfg;
//	map = _map;
}

help_manager::~help_manager()
{
	game_cfg = NULL;
//	map = NULL;
	toplevel.clear();
	hidden_sections.clear();
    // These last numbers must be reset so that the content is regenerated.
    // Upon next start.
	last_num_encountered_units = -1;
	last_num_encountered_terrains = -1;
}

/**
 * Open the help browser, show topic with id show_topic.
 *
 * If show_topic is the empty string, the default topic will be shown.
 */
void show_help(display &disp, const std::string& show_topic, int xloc, int yloc)
{
	show_help(disp, toplevel, show_topic, xloc, yloc);
}

/**
 * Open the help browser, show unit with id unit_id.
 *
 * If show_topic is the empty string, the default topic will be shown.
 */
void show_unit_help(display &disp, const std::string& show_topic, bool has_variations, bool hidden, int xloc, int yloc)
{
	show_help(disp, toplevel,
			  hidden_symbol(hidden) + (has_variations ? ".." : "") + unit_prefix + show_topic, xloc, yloc);
}

/**
 * Open the help browser, show terrain with id terrain_id.
 *
 * If show_topic is the empty string, the default topic will be shown.
 */
void show_terrain_help(display &disp, const std::string& show_topic, bool hidden, int xloc, int yloc)
{
	show_help(disp, toplevel, hidden_symbol(hidden) + terrain_prefix + show_topic, xloc, yloc);
}



/**
 * Open the help browser, show the variation of the unit matching.
 */
void show_variation_help(display &disp, const std::string& unit, const std::string &variation, bool hidden, int xloc, int yloc)
{
	show_help(disp, toplevel, hidden_symbol(hidden) + variation_prefix + unit + "_" + variation, xloc, yloc);
}

/**
 * Open a help dialog using a toplevel other than the default.
 *
 * This allows for complete customization of the contents, although not in a
 * very easy way.
 */
void show_help(display &disp, const section &toplevel_sec,
			   const std::string& show_topic,
			   int xloc, int yloc)
{
	const events::event_context dialog_events_context;
	const gui::dialog_manager manager;
	const resize_lock prevent_resizing;

	CVideo& screen = disp.video();
	const surface& scr = screen.getSurface();

	const int width  = std::min<int>(font::relative_size(900), scr->w - font::relative_size(20));
	const int height = std::min<int>(font::relative_size(800), scr->h - font::relative_size(150));
	const int left_padding = font::relative_size(10);
	const int right_padding = font::relative_size(10);
	const int top_padding = font::relative_size(10);
	const int bot_padding = font::relative_size(10);

	// If not both locations were supplied, put the dialog in the middle
	// of the screen.
	if (yloc <= -1 || xloc <= -1) {
		xloc = scr->w / 2 - width / 2;
		yloc = scr->h / 2 - height / 2;
	}
	std::vector<gui::button*> buttons_ptr;
	gui::button close_button_(disp.video(), _("Close"));
	buttons_ptr.push_back(&close_button_);

	gui::dialog_frame f(disp.video(), _("The Battle for Wesnoth Help"), gui::dialog_frame::default_style,
					 true, &buttons_ptr);
	f.layout(xloc, yloc, width, height);
	f.draw();

    // Find all unit_types that have not been constructed yet and fill in the information
    // needed to create the help topics
	unit_types.build_all(unit_type::HELP_INDEXED);

	if (preferences::encountered_units().size() != size_t(last_num_encountered_units) ||
	    preferences::encountered_terrains().size() != size_t(last_num_encountered_terrains) ||
	    last_debug_state != game_config::debug ||
		last_num_encountered_units < 0) {
		// More units or terrains encountered, update the contents.
		last_num_encountered_units = preferences::encountered_units().size();
		last_num_encountered_terrains = preferences::encountered_terrains().size();
		last_debug_state = game_config::debug;
		generate_contents();
	}
	try {
		help_browser hb(disp, toplevel_sec);
		hb.set_location(xloc + left_padding, yloc + top_padding);
		hb.set_width(width - left_padding - right_padding);
		hb.set_height(height - top_padding - bot_padding);
		if (show_topic != "") {
			hb.show_topic(show_topic);
		}
		else {
			hb.show_topic(default_show_topic);
		}
		hb.set_dirty(true);
		events::raise_draw_event();
		disp.flip();
		disp.invalidate_all();
		CKey key;
		for (;;) {
			events::pump();
			events::raise_process_event();
			events::raise_draw_event();
			if (key[SDLK_ESCAPE]) {
				// Escape quits from the dialog.
				return;
			}
			for (std::vector<gui::button*>::iterator button_it = buttons_ptr.begin();
				 button_it != buttons_ptr.end(); ++button_it) {
				if ((*button_it)->pressed()) {
					// There is only one button, close.
					return;
				}
			}
			disp.flip();
			disp.delay(10);
		}
	}
	catch (parse_error& e) {
		std::stringstream msg;
		msg << _("Parse error when parsing help text: ") << "'" << e.message << "'";
		gui2::show_transient_message(disp.video(), "", msg.str());
	}
}

} // End namespace help.
