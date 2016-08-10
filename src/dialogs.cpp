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
		assert(false);
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

namespace {
	static const int unit_preview_border = 10;
}

unit_preview_pane::details::details() :
	image(),
	name(),
	type_name(),
	race(),
	level(0),
	alignment(),
	traits(),
	abilities(),
	hitpoints(0),
	max_hitpoints(0),
	experience(0),
	max_experience(0),
	hp_color(),
	xp_color(),
	movement_left(0),
	total_movement(0),
	attacks(),
	overlays()
{
}

unit_preview_pane::unit_preview_pane(const gui::filter_textbox *filter, TYPE type, bool on_left_side) :
	gui::preview_pane(display::get_singleton()->video()), index_(0),
	details_button_(display::get_singleton()->video(), _("Profile"),
				      gui::button::TYPE_PRESS, "button_normal/button_small_H22", gui::button::MINIMUM_SPACE),
				      filter_(filter), weapons_(type == SHOW_ALL), left_(on_left_side)
{
	unsigned w = font::relative_size(weapons_ ? 200 : 190);
// advance test
	unsigned h = font::relative_size(weapons_ ? 440 : 140);
	set_measurements(w, h);
}


sdl_handler_vector unit_preview_pane::handler_members()
{
	sdl_handler_vector h;
	h.push_back(&details_button_);
	return h;
}

bool unit_preview_pane::show_above() const
{
	return !weapons_;
}

bool unit_preview_pane::left_side() const
{
	return left_;
}

void unit_preview_pane::set_selection(int index)
{
	index = std::min<int>(static_cast<int>(size())-1,index);
	if (filter_) {
		index = filter_->get_index(index);
	}
	if(index != index_) {
		index_ = index;
		set_dirty();
		if (index >= 0) {
			details_button_.set_dirty();
		}
	}
}

void unit_preview_pane::draw_contents()
{
	if(index_ < 0 || index_ >= int(size())) {
		return;
	}

	const details det = get_details();

	const bool right_align = left_side();

	SDL_Rect const &loc = location();
	const SDL_Rect area = sdl::create_rect(loc.x + unit_preview_border
			, loc.y + unit_preview_border
			, loc.w - unit_preview_border * 2
			, loc.h - unit_preview_border * 2);

#ifdef SDL_GPU
	GPU_SetClip(get_render_target(), area.x, area.y, area.w, area.h);

	sdl::timage unit_image = det.image;
	// TODO: port this to SDL_gpu
//	if (!left_)
//		unit_image = image::reverse_image(unit_image);

	SDL_Rect image_rect = sdl::create_rect(area.x, area.y, 0, 0);

	if(!unit_image.null()) {
		SDL_Rect rect = sdl::create_rect(
				  right_align
					? area.x
					: area.x + area.w - unit_image.width()
				, area.y
				, unit_image.width()
				, unit_image.height());

		video().draw_texture(unit_image, rect.x, rect.y);
		image_rect = rect;

		if(!det.overlays.empty()) {
			for(const std::string& overlay : det.overlays) {
				sdl::timage oi = image::get_texture(overlay);

				if(!oi.null()) {
					continue;
				}

				if(oi.width() > rect.w || oi.height() > rect.h) {
					oi.set_scale(float(rect.w) / oi.width(), float(rect.h) / oi.height());
				}

				video().draw_texture(oi, rect.x, rect.y);
			}
		}
	}

	// Place the 'unit profile' button
	const SDL_Rect button_loc = sdl::create_rect(
			  right_align
				? area.x
				: area.x + area.w - details_button_.location().w
			, image_rect.y + image_rect.h
			, details_button_.location().w
			, details_button_.location().h);
	details_button_.set_location(button_loc);

	SDL_Rect description_rect = sdl::create_rect(image_rect.x
			, image_rect.y + image_rect.h + details_button_.location().h
			, 0
			, 0);

	if(det.name.empty() == false) {
		std::stringstream desc;
		desc << font::BOLD_TEXT << det.name;
		const std::string description = desc.str();
		description_rect = font::text_area(description, font::SIZE_NORMAL);
		sdl::timage img = font::draw_text_to_texture(area,
							font::SIZE_NORMAL, font::NORMAL_COLOR,
							desc.str());
		video().draw_texture(img, right_align ?  image_rect.x :
							 image_rect.x + image_rect.w - description_rect.w,
							 image_rect.y + image_rect.h + details_button_.location().h);
	}

	std::stringstream text;
	text << font::unit_type << det.type_name << "\n"
		<< font::race
		<< (right_align && !weapons_ ? det.race+"  " : "  "+det.race) << "\n"
		<< _("level") << " " << det.level << "\n"
		<< det.alignment << "\n"
		<< det.traits << "\n";

	for(std::vector<t_string>::const_iterator a = det.abilities.begin(); a != det.abilities.end(); ++a) {
		if(a != det.abilities.begin()) {
			text << ", ";
		}
		text << (*a);
	}
	text << "\n";

	// Use same coloring as in generate_report.cpp:
	text << det.hp_color << _("HP: ")
		<< det.hitpoints << "/" << det.max_hitpoints << "\n";

	text << det.xp_color << _("XP: ")
		<< det.experience << "/" << det.max_experience << "\n";

	if(weapons_) {
		text << _("Moves: ")
			<< det.movement_left << "/" << det.total_movement << "\n";

		for(std::vector<attack_type>::const_iterator at_it = det.attacks.begin();
		    at_it != det.attacks.end(); ++at_it) {
			// see generate_report() in generate_report.cpp
			text << font::weapon
				<< at_it->damage()
				<< font::weapon_numbers_sep
				<< at_it->num_attacks()
				<< " " << at_it->name() << "\n";
			text << font::weapon_details
				<< "  " << string_table["range_" + at_it->range()]
				<< font::weapon_details_sep
				<< string_table["type_" + at_it->type()] << "\n";

			std::string accuracy_parry = at_it->accuracy_parry_description();
			if(accuracy_parry.empty() == false) {
				text << font::weapon_details << "  " << accuracy_parry << "\n";
			}

			std::string special = at_it->weapon_specials();
			if (!special.empty()) {
				text << font::weapon_details << "  " << special << "\n";
			}
		}
	}

	// we don't remove empty lines, so all fields stay at the same place
	const std::vector<std::string> lines = utils::split(text.str(), '\n',
		utils::STRIP_SPACES & !utils::REMOVE_EMPTY);


	int ypos = area.y;

	if(weapons_) {
		ypos += image_rect.h + description_rect.h + details_button_.location().h;
	}

	for(std::vector<std::string>::const_iterator line = lines.begin(); line != lines.end(); ++line) {
		int xpos = area.x;
		if(right_align && !weapons_) {
			const SDL_Rect& line_area = font::text_area(*line,font::SIZE_SMALL);
			// right align, but if too long, don't hide line's beginning
			if (line_area.w < area.w)
				xpos = area.x + area.w - line_area.w;
		}

		sdl::timage img = font::draw_text_to_texture(location(),font::SIZE_SMALL,font::NORMAL_COLOR,*line);
		video().draw_texture(img, xpos, ypos);
		ypos += img.height();
	}

	GPU_UnsetClip(get_render_target());
#else
	surface& screen(video().getSurface());
	const clip_rect_setter clipper(screen, &area);

	surface unit_image = det.image;
	if (!left_)
		unit_image = image::reverse_image(unit_image);

	SDL_Rect image_rect = sdl::create_rect(area.x, area.y, 0, 0);

	if(unit_image != nullptr) {
		SDL_Rect rect = sdl::create_rect(
				  right_align
					? area.x
					: area.x + area.w - unit_image->w
				, area.y
				, unit_image->w
				, unit_image->h);

		sdl_blit(unit_image,nullptr,screen,&rect);
		image_rect = rect;

		if(!det.overlays.empty()) {
			for(const std::string& overlay : det.overlays) {
				surface os = image::get_image(overlay);

				if(!os) {
					continue;
				}

				if(os->w > rect.w || os->h > rect.h) {
					os = scale_surface(os, rect.w, rect.h, false);
				}

				sdl_blit(os, nullptr, screen, &rect);
			}
		}
	}

	// Place the 'unit profile' button
	const SDL_Rect button_loc = sdl::create_rect(
			  right_align
				? area.x
				: area.x + area.w - details_button_.location().w
			, image_rect.y + image_rect.h
			, details_button_.location().w
			, details_button_.location().h);
	details_button_.set_location(button_loc);
	details_button_.set_dirty(true);

	SDL_Rect description_rect = sdl::create_rect(image_rect.x
			, image_rect.y + image_rect.h + details_button_.location().h
			, 0
			, 0);

	if(det.name.empty() == false) {
		std::stringstream desc;
		desc << font::BOLD_TEXT << det.name;
		const std::string description = desc.str();
		description_rect = font::text_area(description, font::SIZE_NORMAL);
		description_rect = font::draw_text(&video(), area,
							font::SIZE_NORMAL, font::NORMAL_COLOR,
							desc.str(), right_align ?  image_rect.x :
							image_rect.x + image_rect.w - description_rect.w,
							image_rect.y + image_rect.h + details_button_.location().h);
	}

	std::stringstream text;
	text << font::unit_type << det.type_name << "\n"
		<< font::race
		<< (right_align && !weapons_ ? det.race+"  " : "  "+det.race) << "\n"
		<< _("level") << " " << det.level << "\n"
		<< det.alignment << "\n"
		<< det.traits << "\n";

	for(std::vector<t_string>::const_iterator a = det.abilities.begin(); a != det.abilities.end(); ++a) {
		if(a != det.abilities.begin()) {
			text << ", ";
		}
		text << (*a);
	}
	text << "\n";

	// Use same coloring as in generate_report.cpp:
	text << det.hp_color << _("HP: ")
		<< det.hitpoints << "/" << det.max_hitpoints << "\n";

	text << det.xp_color << _("XP: ")
		<< det.experience << "/" << det.max_experience << "\n";

	if(weapons_) {
		text << _("Moves: ")
			<< det.movement_left << "/" << det.total_movement << "\n";

		for(std::vector<attack_type>::const_iterator at_it = det.attacks.begin();
			at_it != det.attacks.end(); ++at_it) {
			// see generate_report() in generate_report.cpp
			text << font::weapon
				<< at_it->damage()
				<< font::weapon_numbers_sep
				<< at_it->num_attacks()
				<< " " << at_it->name() << "\n";
			text << font::weapon_details
				<< "  " << string_table["range_" + at_it->range()]
				<< font::weapon_details_sep
				<< string_table["type_" + at_it->type()] << "\n";

			std::string accuracy_parry = at_it->accuracy_parry_description();
			if(accuracy_parry.empty() == false) {
				text << font::weapon_details << "  " << accuracy_parry << "\n";
			}

			std::string special = at_it->weapon_specials();
			if (!special.empty()) {
				text << font::weapon_details << "  " << special << "\n";
			}
		}
	}

	// we don't remove empty lines, so all fields stay at the same place
	const std::vector<std::string> lines
			= utils::split(text.str(), '\n', utils::STRIP_SPACES);


	int ypos = area.y;

	if(weapons_) {
		ypos += image_rect.h + description_rect.h + details_button_.location().h;
	}

	for(std::vector<std::string>::const_iterator line = lines.begin(); line != lines.end(); ++line) {
		int xpos = area.x;
		if(right_align && !weapons_) {
			const SDL_Rect& line_area = font::text_area(*line,font::SIZE_SMALL);
			// right align, but if too long, don't hide line's beginning
			if (line_area.w < area.w)
				xpos = area.x + area.w - line_area.w;
		}

		SDL_Rect cur_area = font::draw_text(&video(),location(),font::SIZE_SMALL,font::NORMAL_COLOR,*line,xpos,ypos);
		ypos += cur_area.h;
	}
#endif
}


units_list_preview_pane::units_list_preview_pane(unit_const_ptr u, TYPE type, bool on_left_side) :
	unit_preview_pane(nullptr, type, on_left_side),
	units_(std::make_shared<const std::vector<unit_const_ptr> >(1, u))
{
}

units_list_preview_pane::units_list_preview_pane(const std::shared_ptr<const std::vector<unit_const_ptr > > &units,
		const gui::filter_textbox* filter, TYPE type, bool on_left_side) :
	unit_preview_pane(filter, type, on_left_side),
	units_(units)
{
}

size_t units_list_preview_pane::size() const
{
	return units_->size();
}

const unit_preview_pane::details units_list_preview_pane::get_details() const
{
	const unit &u = *units_->at(index_);
	details det;

	/** Get an SDL surface, ready for display for place where we need a still-image of the unit. */
	image::locator image_loc;

#ifdef LOW_MEM
	image_loc = image::locator(u.absolute_image());
#else
	std::string mods=u.image_mods();
	if(!mods.empty()){
		image_loc = image::locator(u.absolute_image(),mods);
	} else {
		image_loc = image::locator(u.absolute_image());
	}
#endif
	det.image = image::get_image(image_loc, image::UNSCALED);
	/***/

	det.name = u.name();
	det.type_name = u.type_name();
	det.race = u.race()->name(u.gender());
	det.level = u.level();
	det.alignment = unit_type::alignment_description(u.alignment(), u.gender());
	det.traits = utils::join(u.trait_names(), ", ");

	// The triples are base name, male/female name, description.
	const std::vector<std::tuple<t_string,t_string,t_string> > &abilities = u.ability_tooltips();
	for(auto a : abilities) {
		det.abilities.push_back(std::get<1>(a));
	}

	det.hitpoints = u.hitpoints();
	det.max_hitpoints = u.max_hitpoints();
	det.hp_color = font::color2markup(u.hp_color());

	det.experience = u.experience();
	det.max_experience = u.max_experience();
	det.xp_color = font::color2markup(u.xp_color());

	det.movement_left = u.movement_left();
	det.total_movement= u.total_movement();

	det.attacks = u.attacks();

	if(u.can_recruit()) {
		det.overlays.push_back(unit::leader_crown());
	};

	for(const std::string& overlay : u.overlays()) {
		det.overlays.push_back(overlay);
	}

	return det;
}

void units_list_preview_pane::process_event()
{
	assert(resources::screen);
	if (details_button_.pressed() && index_ >= 0 && index_ < int(size())) {
		help::show_unit_description(resources::screen->video(), *units_->at(index_));
	}
}

unit_types_preview_pane::unit_types_preview_pane(
					std::vector<const unit_type*>& unit_types, const gui::filter_textbox* filter,
					int side, TYPE type, bool on_left_side)
	: unit_preview_pane(filter, type, on_left_side),
					  unit_types_(&unit_types), side_(side)
{}

size_t unit_types_preview_pane::size() const
{
	return (unit_types_!=nullptr) ? unit_types_->size() : 0;
}

const unit_types_preview_pane::details unit_types_preview_pane::get_details() const
{
	const unit_type* t = (*unit_types_)[index_];
	details det;

	if (t==nullptr)
		return det;

	// Make sure the unit type is built with enough data for our needs.
	unit_types.build_unit_type(*t, unit_type::FULL);

	std::string mod = "~RC(" + t->flag_rgb() + ">" + team::get_side_color_index(side_) + ")";
	det.image = image::get_image(t->image()+mod);

	det.name = "";
	det.type_name = t->type_name();
	det.level = t->level();
	det.alignment = unit_type::alignment_description(t->alignment(), t->genders().front());
	det.race = t->race()->name(t->genders().front());

	//FIXME: This probably must be move into a unit_type function
	for (const config &tr : t->possible_traits())
	{
		if (tr["availability"] != "musthave") continue;

		const std::string gender_string = t->genders().front()== unit_race::FEMALE ?
		                                  "female_name" : "male_name";
		t_string name = tr[gender_string];
		if (name.empty()) {
			name = tr["name"];
		}
		if (!name.empty()) {
			if (!det.traits.empty()) {
				det.traits += ", ";
			}
			det.traits += name;
		}
	}

	det.abilities = t->abilities();

	det.hitpoints = t->hitpoints();
	det.max_hitpoints = t->hitpoints();
	det.hp_color = "<33,225,0>"; // from unit::hp_color()

	det.experience = 0;
	det.max_experience = t->experience_needed();
	det.xp_color = "<0,160,225>"; // from unit::xp_color()

	// Check if AMLA color is needed
	// FIXME: not sure if it's fully accurate (but not very important for unit_type)
	// xp_color also need a simpler function for doing this
	for (const config &adv : t->modification_advancements())
	{
		if (!adv["strict_amla"].to_bool() || !t->can_advance()) {
			det.xp_color = "<170,0,255>"; // from unit::xp_color()
			break;
		}
	}

	det.movement_left = 0;
	det.total_movement= t->movement();

	det.attacks = t->attacks();
	return det;
}

void unit_types_preview_pane::process_event()
{
	assert(resources::screen);
	if (details_button_.pressed() && index_ >= 0 && index_ < int(size())) {
		const unit_type* type = (*unit_types_)[index_];
		if (type != nullptr)
			help::show_unit_description(resources::screen->video(), *type);
	}
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
