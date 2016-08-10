/*
   Copyright (C) 2003 - 2016 by David White <dave@whitevine.net>
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
 * Various dialogs: advance_unit, show_objectives, save+load game
 */

#include "global.hpp"

#include "actions/attack.hpp"
#include "actions/undo.hpp"
#include "dialogs.hpp"
#include "format_time_summary.hpp"
#include "game_display.hpp"
#include "game_preferences.hpp"
#include "gui/dialogs/game_delete.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/unit_list.hpp"
#include "gui/dialogs/unit_advance.hpp"
#include "gui/widgets/window.hpp"
#include "gettext.hpp"
#include "help/help.hpp"
#include "help/help_button.hpp"
#include "language.hpp"
#include "log.hpp"
#include "map/map.hpp"
#include "map/exception.hpp"
#include "marked-up_text.hpp"
#include "menu_events.hpp"
#include "mouse_handler_base.hpp"
#include "minimap.hpp"
#include "replay.hpp"
#include "replay_helper.hpp"
#include "resources.hpp"
#include "savegame.hpp"
#include "save_index.hpp"
#include "strftime.hpp"
#include "synced_context.hpp"
#include "terrain/type_data.hpp"
#include "units/unit.hpp"
#include "units/animation.hpp"
#include "units/helper.hpp"
#include "units/types.hpp"
#include "wml_separators.hpp"
#include "widgets/progressbar.hpp"
#include "wml_exception.hpp"
#include "formula/string_utils.hpp"
#include "gui/dialogs/game_save.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "gui/dialogs/network_transmission.hpp"
#include "ai/lua/aspect_advancements.hpp"
#include "wesnothd_connection.hpp"

//#ifdef _WIN32
//#include "locale.h"
//#endif

#include <clocale>

static lg::log_domain log_engine("engine");
#define LOG_NG LOG_STREAM(info, log_engine)
#define ERR_NG LOG_STREAM(err, log_engine)

static lg::log_domain log_display("display");
#define LOG_DP LOG_STREAM(info, log_display)

#define ERR_G  LOG_STREAM(err, lg::general)

static lg::log_domain log_config("config");
#define ERR_CF LOG_STREAM(err, log_config)

namespace dialogs
{

int advance_unit_dialog(const map_location &loc)
{
	const unit& u = *resources::units->find(loc);
	std::vector<unit_const_ptr> previews;

	for(const std::string& advance : u.advances_to()) {
		preferences::encountered_units().insert(advance);
		previews.push_back(get_advanced_unit(u, advance));
	}

	size_t num_real_advances = previews.size();
	bool always_display = false;

	for(const config& advance : u.get_modification_advances()) {
		if(advance["always_display"]) {
			always_display = true;
		}
		previews.push_back(get_amla_unit(u, advance));
	}

	if(previews.size() > 1 || always_display) {
		gui2::tunit_advance dlg(previews, num_real_advances);

		dlg.show(CVideo::get_singleton());

		if(dlg.get_retval() == gui2::twindow::OK) {
			return dlg.get_selected_index();
		}

		// This should be unreachable, since canceling is disabled for the dialog
		assert(false && "Unit advance dialog was cancelled, which should be impossible.");
	}

	return 0;
}

bool animate_unit_advancement(const map_location &loc, size_t choice, const bool &fire_event, const bool animate)
{
	const events::command_disabler cmd_disabler;

	unit_map::iterator u = resources::units->find(loc);
	if (u == resources::units->end()) {
		LOG_DP << "animate_unit_advancement suppressed: invalid unit\n";
		return false;
	} else if (!u->advances()) {
		LOG_DP << "animate_unit_advancement suppressed: unit does not advance\n";
		return false;
	}

	const std::vector<std::string>& options = u->advances_to();
	std::vector<config> mod_options = u->get_modification_advances();

	if(choice >= options.size() + mod_options.size()) {
		LOG_DP << "animate_unit_advancement suppressed: invalid option\n";
		return false;
	}

	// When the unit advances, it fades to white, and then switches
	// to the new unit, then fades back to the normal color

	if (animate && !resources::screen->video().update_locked()) {
		unit_animator animator;
		bool with_bars = true;
		animator.add_animation(&*u,"levelout", u->get_location(), map_location(), 0, with_bars);
		animator.start_animations();
		animator.wait_for_end();
	}

	if(choice < options.size()) {
		// chosen_unit is not a reference, since the unit may disappear at any moment.
		std::string chosen_unit = options[choice];
		::advance_unit(loc, chosen_unit, fire_event);
	} else {
		const config &mod_option = mod_options[choice - options.size()];
		::advance_unit(loc, "", fire_event, &mod_option);
	}

	u = resources::units->find(loc);
	resources::screen->invalidate_unit();

	if (animate && u != resources::units->end() && !resources::screen->video().update_locked()) {
		unit_animator animator;
		animator.add_animation(&*u, "levelin", u->get_location(), map_location(), 0, true);
		animator.start_animations();
		animator.wait_for_end();
		animator.set_all_standing();
		resources::screen->invalidate(loc);
		resources::screen->draw();
		events::pump();
	}

	resources::screen->invalidate_all();
	resources::screen->draw();

	return true;
}

void show_unit_list(display& gui)
{
	gui2::tunit_list dlg(gui);

	dlg.show(gui.video());

	if(dlg.get_retval() == gui2::twindow::OK) {
		const map_location& loc = dlg.get_location_to_scroll_to();

		gui.scroll_to_tile(loc, game_display::WARP);
		gui.select_hex(loc);
	}
}

void show_objectives(const std::string &scenarioname, const std::string &objectives)
{
	static const std::string no_objectives(_("No objectives available"));
	gui2::show_transient_message(resources::screen->video(), scenarioname,
		(objectives.empty() ? no_objectives : objectives), "", true);
}

static void network_transmission_dialog(CVideo& video, gui2::tnetwork_transmission::connection_data& conn, const std::string& msg1, const std::string& msg2)
{
	if (video.faked()) {
		while (!conn.finished()) {
			conn.poll();
			SDL_Delay(1);
		}
	}
	else {
		gui2::tnetwork_transmission(conn, msg1, msg2).show(video);
	}
}

struct read_wesnothd_connection_data : public gui2::tnetwork_transmission::connection_data
{
	read_wesnothd_connection_data(twesnothd_connection& conn) : conn_(conn) {}
	size_t total() override { return conn_.bytes_to_read(); }
	virtual size_t current()  override { return conn_.bytes_read(); }
	virtual bool finished() override { return conn_.has_data_received(); }
	virtual void cancel() override { }
	virtual void poll() override { conn_.poll(); }
	twesnothd_connection& conn_;
};

bool network_receive_dialog(CVideo& video, const std::string& msg, config& cfg, twesnothd_connection& wesnothd_connection)
{
	read_wesnothd_connection_data gui_data(wesnothd_connection);
	network_transmission_dialog(video, gui_data, msg, _("Waiting"));
	return wesnothd_connection.receive_data(cfg);
}

struct connect_wesnothd_connection_data : public gui2::tnetwork_transmission::connection_data
{
	connect_wesnothd_connection_data(twesnothd_connection& conn) : conn_(conn) {}
	virtual bool finished() override { return conn_.handshake_finished(); }
	virtual void cancel() override { }
	virtual void poll() override { conn_.poll(); }
	twesnothd_connection& conn_;
};

std::unique_ptr<twesnothd_connection> network_connect_dialog(CVideo& video, const std::string& msg, const std::string& hostname, int port)
{
	std::unique_ptr<twesnothd_connection> res(new twesnothd_connection(hostname, std::to_string(port)));
	connect_wesnothd_connection_data gui_data(*res);
	network_transmission_dialog(video, gui_data, msg, _("Connecting"));
	return std::move(res);

}

} // end namespace dialogs
