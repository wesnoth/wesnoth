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
 * Various dialogs: advance_unit, show_objectives, save+load game, network::connection.
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
#include "gui/widgets/window.hpp"
#include "gettext.hpp"
#include "help.hpp"
#include "language.hpp"
#include "log.hpp"
#include "map.hpp"
#include "map_exception.hpp"
#include "marked-up_text.hpp"
#include "menu_events.hpp"
#include "mouse_handler_base.hpp"
#include "minimap.hpp"
#include "replay.hpp"
#include "replay_helper.hpp"
#include "resources.hpp"
#include "savegame.hpp"
#include "strftime.hpp"
#include "synced_context.hpp"
#include "thread.hpp"
#include "unit_helper.hpp"
#include "unit_types.hpp"
#include "wml_separators.hpp"
#include "widgets/progressbar.hpp"
#include "wml_exception.hpp"
#include "formula_string_utils.hpp"
#include "gui/dialogs/game_save.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "ai/lua/unit_advancements_aspect.hpp"

#include <boost/foreach.hpp>

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

namespace
{

class delete_recall_unit : public gui::dialog_button_action
{
public:
	delete_recall_unit(display& disp, gui::filter_textbox& filter, const std::vector<const unit*>& units) : disp_(disp), filter_(filter), units_(units) {}
private:
	gui::dialog_button_action::RESULT button_pressed(int menu_selection);

	display& disp_;
	gui::filter_textbox& filter_;
	const std::vector<const unit*>& units_;
};

gui::dialog_button_action::RESULT delete_recall_unit::button_pressed(int menu_selection)
{
	const size_t index = size_t(filter_.get_index(menu_selection));
	if(index < units_.size()) {
		const unit &u = *(units_[index]);

		//If the unit is of level > 1, or is close to advancing,
		//we warn the player about it
		std::stringstream message;
		if (u.loyal()) {
			// TRANSLATORS: this string ends with a space
			message << _("This unit is loyal and requires no upkeep. ") << (u.gender() == unit_race::MALE ? _("Do you really want to dismiss him?")
					: _("Do you really want to dismiss her?"));
		} else if(u.level() > 1) {
			// TRANSLATORS: this string ends with a space
			message << _("This unit is an experienced one, having advanced levels. ") << (u.gender() == unit_race::MALE ? _("Do you really want to dismiss him?")
					: _("Do you really want to dismiss her?"));

		} else if(u.experience() > u.max_experience()/2) {
			// TRANSLATORS: this string ends with a space
			message << _("This unit is close to advancing a level. ") << (u.gender() == unit_race::MALE ? _("Do you really want to dismiss him?")
					: _("Do you really want to dismiss her?"));
		}

		if(!message.str().empty()) {
			const int res = gui2::show_message(disp_.video(), _("Dismiss Unit"), message.str(), gui2::tmessage::yes_no_buttons);
			if(res == gui2::twindow::CANCEL) {
				return gui::CONTINUE_DIALOG;
			}
		}
		// Remove the item from filter_textbox memory
		filter_.delete_item(menu_selection);
		//add dismissal to the undo stack
		resources::undo_stack->add_dismissal(u);

		// Find the unit in the recall list.
		std::vector<unit>& recall_list = (*resources::teams)[u.side() -1].recall_list();
		assert(!recall_list.empty());
		std::vector<unit>::iterator dismissed_unit =
				find_if_matches_id(recall_list, u.id());
		assert(dismissed_unit != recall_list.end());

		// Record the dismissal, then delete the unit.
		synced_context::run_in_synced_context("disband", replay_helper::get_disband(dismissed_unit->id()));
		//recorder.add_disband(dismissed_unit->id());
		//recall_list.erase(dismissed_unit);

		return gui::DELETE_ITEM;
	} else {
		return gui::CONTINUE_DIALOG;
	}
}

} //anon namespace

int advance_unit_dialog(const map_location &loc)
{
	unit_map::iterator u = resources::units->find(loc);

	const std::vector<std::string>& options = u->advances_to();

	std::vector<std::string> lang_options;

	std::vector<unit> sample_units;
	for(std::vector<std::string>::const_iterator op = options.begin(); op != options.end(); ++op) {
		sample_units.push_back(::get_advanced_unit(*u, *op));
		const unit& type = sample_units.back();

#ifdef LOW_MEM
		lang_options.push_back(IMAGE_PREFIX
				+ static_cast<std::string const &>(type.absolute_image())
				+ COLUMN_SEPARATOR + type.type_name());
#else
		lang_options.push_back(IMAGE_PREFIX + type.absolute_image() + u->image_mods() + COLUMN_SEPARATOR + type.type_name());
#endif
		preferences::encountered_units().insert(*op);
	}

	bool always_display = false;
	BOOST_FOREACH(const config &mod, u->get_modification_advances())
	{
		if (mod["always_display"].to_bool()) always_display = true;
		sample_units.push_back(::get_amla_unit(*u, mod));
		const unit& type = sample_units.back();
		if (!mod["image"].empty()) {
			lang_options.push_back(IMAGE_PREFIX + mod["image"].str() + COLUMN_SEPARATOR + mod["description"].str());
		} else {
#ifdef LOW_MEM
			lang_options.push_back(IMAGE_PREFIX
					+ static_cast<std::string const &>(type.absolute_image())
					+ COLUMN_SEPARATOR + mod["description"].str());
#else
			lang_options.push_back(IMAGE_PREFIX + type.absolute_image() + u->image_mods() + COLUMN_SEPARATOR + mod["description"].str());
#endif
		}
	}

	assert(!lang_options.empty());

	if (lang_options.size() > 1 || always_display)
	{
		units_list_preview_pane unit_preview(sample_units);
		std::vector<gui::preview_pane*> preview_panes;
		preview_panes.push_back(&unit_preview);

		gui::dialog advances = gui::dialog(*resources::screen,
				      _("Advance Unit"),
		                      _("What should our victorious unit become?"),
		                      gui::OK_ONLY);
		advances.set_menu(lang_options);
		advances.set_panes(preview_panes);
		return advances.show();
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
	const std::string heading = std::string(1,HEADING_PREFIX) +
			_("Type")          + COLUMN_SEPARATOR + // 0
			_("Name")          + COLUMN_SEPARATOR + // 1
			_("Moves")         + COLUMN_SEPARATOR + // 2
			_("Status")        + COLUMN_SEPARATOR + // 3
			_("HP")            + COLUMN_SEPARATOR + // 4
			_("Level^Lvl.")    + COLUMN_SEPARATOR + // 5
			_("XP")            + COLUMN_SEPARATOR + // 6
			_("unit list^Traits");                  // 7

	gui::menu::basic_sorter sorter;
	sorter.set_alpha_sort(0).set_alpha_sort(1).set_numeric_sort(2);
	sorter.set_alpha_sort(3).set_numeric_sort(4).set_level_sort(5, 6);
	sorter.set_xp_sort(6).set_alpha_sort(7);

	std::vector<std::string> items;
	items.push_back(heading);

	std::vector<map_location> locations_list;
	std::vector<unit> units_list;

	int selected = 0;

	const unit_map& units = gui.get_units();

	for(unit_map::const_iterator i = units.begin(); i != units.end(); ++i) {
		if (i->side() != gui.viewing_side())
			continue;

		std::stringstream row;
		// If a unit is already selected on the map, we do the same in the unit list dialog
		if (gui.selected_hex() == i->get_location()) {
			row << DEFAULT_ITEM;
			selected = units_list.size();
		}
		// If unit is leader, show name in special color, e.g. gold/silver
		/** @todo TODO: hero just has overlay "misc/hero-icon.png" - needs an ability to query */

		if (i->can_recruit() ) {
			row << "<205,173,0>";   // gold3
		}
		row << i->type_name() << COLUMN_SEPARATOR;
		if (i->can_recruit() ) {
			row << "<205,173,0>";   // gold3
		}
		row << i->name()   << COLUMN_SEPARATOR;

		// display move left (0=red, moved=yellow, not moved=green)
		if (i->movement_left() == 0) {
			row << font::RED_TEXT;
		} else if (i->movement_left() < i->total_movement() ) {
			row << "<255,255,0>";
		} else {
			row << font::GREEN_TEXT;
		}
		row << i->movement_left() << '/' << i->total_movement() << COLUMN_SEPARATOR;

		// show icons if unit is slowed, poisoned, petrified, invisible:
		if(i->get_state(unit::STATE_PETRIFIED))
			row << IMAGE_PREFIX << "misc/petrified.png"    << IMG_TEXT_SEPARATOR;
		if(i->get_state(unit::STATE_POISONED))
			row << IMAGE_PREFIX << "misc/poisoned.png" << IMG_TEXT_SEPARATOR;
		if(i->get_state(unit::STATE_SLOWED))
			row << IMAGE_PREFIX << "misc/slowed.png"   << IMG_TEXT_SEPARATOR;
		if(i->invisible(i->get_location(),false))
			row << IMAGE_PREFIX << "misc/invisible.png";
		row << COLUMN_SEPARATOR;

		// Display HP
		// see also unit_preview_pane in dialogs.cpp
		row << font::color2markup(i->hp_color());
		row << i->hitpoints()  << '/' << i->max_hitpoints() << COLUMN_SEPARATOR;

		// Show units of level (0=gray, 1 normal, 2 bold, 2+ bold&wbright)
		int level = i->level();
		if(level < 1) {
			row << "<150,150,150>";
		} else if(level == 1) {
			row << font::NORMAL_TEXT;
		} else if(level == 2) {
			row << font::BOLD_TEXT;
		} else if(level > 2 ) {
			row << font::BOLD_TEXT << "<255,255,255>";
		}
		row << level << COLUMN_SEPARATOR;

		// Display XP
		row << font::color2markup(i->xp_color());
		row << i->experience() << "/";
		if (i->can_advance()) {
			row << i->max_experience();
		} else {
			row << "-";
		}
		row << COLUMN_SEPARATOR;

		// TODO: show 'loyal' in green / xxx in red  //  how to handle translations ??
		row << utils::join(i->trait_names(), ", ");
		items.push_back(row.str());

		locations_list.push_back(i->get_location());
		units_list.push_back(*i);
	}

	{
		dialogs::units_list_preview_pane unit_preview(units_list);
		unit_preview.set_selection(selected);

		gui::dialog umenu(gui, _("Unit List"), "", gui::NULL_DIALOG);
		umenu.set_menu(items, &sorter);
		umenu.add_pane(&unit_preview);
		//sort by type name
		umenu.get_menu().sort_by(0);

		umenu.add_button(new gui::standard_dialog_button(gui.video(), _("Scroll To"), 0, false),
				gui::dialog::BUTTON_STANDARD);
		umenu.add_button(new gui::standard_dialog_button(gui.video(), _("Close"), 1, true),
				gui::dialog::BUTTON_STANDARD);
		umenu.set_basic_behavior(gui::OK_CANCEL);
		selected = umenu.show();
	} // this will kill the dialog before scrolling

	if(selected >= 0 && selected < int(locations_list.size())) {
		const map_location& loc = locations_list[selected];
		gui.scroll_to_tile(loc,game_display::WARP);
		gui.select_hex(loc);
	}
}

void show_objectives(const config &level, const std::string &objectives)
{
	static const std::string no_objectives(_("No objectives available"));
	gui2::show_transient_message(resources::screen->video(), level["name"],
		(objectives.empty() ? no_objectives : objectives), "", true);
}

int recruit_dialog(display& disp, std::vector< const unit_type* >& units, const std::vector< std::string >& items, int side, const std::string& title_suffix)
{
	dialogs::unit_types_preview_pane unit_preview(
		units, NULL, side);
	std::vector<gui::preview_pane*> preview_panes;
	preview_panes.push_back(&unit_preview);

	gui::menu::basic_sorter sorter;
	sorter.set_alpha_sort(1);

	gui::dialog rmenu(disp, _("Recruit") + title_suffix,
			  _("Select unit:") + std::string("\n"),
			  gui::OK_CANCEL,
			  gui::dialog::default_style);
	rmenu.add_button(new help::help_button(disp, "recruit_and_recall"),
		gui::dialog::BUTTON_HELP);

	gui::menu::imgsel_style units_display_style(gui::menu::bluebg_style);
	units_display_style.scale_images(font::relative_size(72), font::relative_size(72));

	gui::menu* units_menu = new gui::menu(disp.video(), items, false, -1,
		gui::dialog::max_menu_width, &sorter, &units_display_style, false);

	units_menu->sort_by(1); // otherwise it's unsorted by default

	rmenu.set_menu(units_menu);
	rmenu.set_panes(preview_panes);
	return rmenu.show();
}


#ifdef LOW_MEM
int recall_dialog(display& disp, std::vector< const unit* >& units, int /*side*/, const std::string& title_suffix, const int team_recall_cost)
#else
int recall_dialog(display& disp, std::vector< const unit* >& units, int side, const std::string& title_suffix, const int team_recall_cost)
#endif
{
	std::vector<std::string> options, options_to_filter;

	std::ostringstream heading;
	heading << HEADING_PREFIX << COLUMN_SEPARATOR << _("Type")
		<< COLUMN_SEPARATOR << _("Name")
		<< COLUMN_SEPARATOR << _("Level^Lvl.")
		<< COLUMN_SEPARATOR << _("XP");
	heading << COLUMN_SEPARATOR << _("Traits");

	gui::menu::basic_sorter sorter;
	sorter.set_alpha_sort(1).set_alpha_sort(2);
	sorter.set_level_sort(3,4).set_xp_sort(4).set_alpha_sort(5);

	options.push_back(heading.str());
	options_to_filter.push_back(options.back());

	BOOST_FOREACH(const unit* u, units)
	{
		std::stringstream option, option_to_filter;
		std::string name = u->name();
		if (name.empty()) name = utils::unicode_em_dash;

		option << IMAGE_PREFIX << u->absolute_image();

	#ifndef LOW_MEM
		option << "~RC("  << u->team_color() << '>'
			<< team::get_side_color_index(side) << ')';

		if(u->can_recruit()) {
			option << "~BLIT(" << unit::leader_crown() << ")";
		}

		BOOST_FOREACH(const std::string& overlay, u->overlays())
		{
			option << "~BLIT(" << overlay << ")";
		}
	#endif

		option << COLUMN_SEPARATOR;
		int cost = u->recall_cost();
		if(cost < 0) {
			cost = team_recall_cost;
		}
		option << u->type_name() << "\n";
		if(cost > team_recall_cost) {
			option << font::NORMAL_TEXT << "<255,0,0>";
		}
		else if(cost == team_recall_cost) {
			option << font::NORMAL_TEXT;
		}
		else if(cost < team_recall_cost) {
			option << font::NORMAL_TEXT << "<0,255,0>";
		}
		option << cost << " Gold" << COLUMN_SEPARATOR
			<< name << COLUMN_SEPARATOR;

		// Show units of level (0=gray, 1 normal, 2 bold, 2+ bold&wbright)
		const int level = u->level();
		if(level < 1) {
			option << "<150,150,150>";
		} else if(level == 1) {
			option << font::NORMAL_TEXT;
		} else if(level == 2) {
			option << font::BOLD_TEXT;
		} else if(level > 2 ) {
			option << font::BOLD_TEXT << "<255,255,255>";
		}
		option << level << COLUMN_SEPARATOR;

		option << font::color2markup(u->xp_color()) << u->experience() << "/";
		if (u->can_advance())
			option << u->max_experience();
		else
			option << "-";

		option_to_filter << u->type_name() << " " << name << " " << u->level();

		option << COLUMN_SEPARATOR;
		BOOST_FOREACH(const t_string& trait, u->trait_names()) {
			option << trait << '\n';
			option_to_filter << " " << trait;
		}

		options.push_back(option.str());
		options_to_filter.push_back(option_to_filter.str());
	}
	
	gui::dialog rmenu(disp, _("Recall") + title_suffix,
		_("Select unit:") + std::string("\n"),
		gui::OK_CANCEL, gui::dialog::default_style);

	gui::menu::imgsel_style units_display_style(gui::menu::bluebg_style);
	units_display_style.scale_images(font::relative_size(72), font::relative_size(72));

	gui::menu* units_menu = new gui::menu(disp.video(), options, false, -1,
		gui::dialog::max_menu_width, &sorter, &units_display_style, false);

	rmenu.set_menu(units_menu);

	gui::filter_textbox* filter = new gui::filter_textbox(disp.video(),
		_("Filter: "), options, options_to_filter, 1, rmenu, 200);
	rmenu.set_textbox(filter);

	delete_recall_unit recall_deleter(disp, *filter, units);
	gui::dialog_button_info delete_button(&recall_deleter,_("Dismiss Unit"));
	rmenu.add_button(delete_button);

	rmenu.add_button(new help::help_button(disp,"recruit_and_recall"),
		gui::dialog::BUTTON_HELP);

	dialogs::units_list_preview_pane unit_preview(units, filter);
	rmenu.add_pane(&unit_preview);

	//sort by level
	static int sort_by = 3;
	static bool sort_reversed = false;

	if(sort_by >= 0) {
		rmenu.get_menu().sort_by(sort_by);
		// "reclick" on the sorter to reverse the order
		if(sort_reversed) {
			rmenu.get_menu().sort_by(sort_by);
		}
	}

	int res = rmenu.show();
	res = filter->get_index(res);

	sort_by = rmenu.get_menu().get_sort_by();
	sort_reversed = rmenu.get_menu().get_sort_reversed();

	return res;
}



namespace {

/** Class to handle deleting a saved game. */
class delete_save : public gui::dialog_button_action
{
public:
	delete_save(display& disp, gui::filter_textbox& filter, std::vector<savegame::save_info>& saves) :
		disp_(disp), saves_(saves), filter_(filter) {}
private:
	gui::dialog_button_action::RESULT button_pressed(int menu_selection);

	display& disp_;
	std::vector<savegame::save_info>& saves_;
	gui::filter_textbox& filter_;
};

gui::dialog_button_action::RESULT delete_save::button_pressed(int menu_selection)
{
	const size_t index = size_t(filter_.get_index(menu_selection));
	if(index < saves_.size()) {

		// See if we should ask the user for deletion confirmation
		if(preferences::ask_delete_saves()) {
			if(!gui2::tgame_delete::execute(disp_.video())) {
				return gui::CONTINUE_DIALOG;
			}
		}

		// Remove the item from filter_textbox memory
		filter_.delete_item(menu_selection);

		// Delete the file
		savegame::delete_game(saves_[index].name());

		// Remove it from the list of saves
		saves_.erase(saves_.begin() + index);

		return gui::DELETE_ITEM;
	} else {
		return gui::CONTINUE_DIALOG;
	}
}

static const int save_preview_border = 10;

class save_preview_pane : public gui::preview_pane
{
public:
	save_preview_pane(CVideo &video, const config& game_config, gamemap* map,
			const std::vector<savegame::save_info>& info,
			const gui::filter_textbox& textbox,
			gui::dialog_button* difficulty_option) :
		gui::preview_pane(video),
		game_config_(&game_config),
		map_(map), info_(&info),
		index_(0),
		map_cache_(),
		textbox_(textbox),
		difficulty_option_(difficulty_option)
	{
		set_measurements(std::min<int>(200,video.getx()/4),
				 std::min<int>(400,video.gety() * 4/5));

		if (difficulty_option_ != NULL) {
			difficulty_option_->enable(false);
		}
	}

	void draw_contents();

	/**
	 * Enables "Change difficulty" option box for start-of-scenario
	 * save-files only
	 */
	void decide_on_difficulty_option();

	void set_selection(int index) {
		int res = textbox_.get_index(index);
		index_ = (res >= 0) ? res : index_;
		decide_on_difficulty_option();
		set_dirty();
	}

	bool left_side() const { return true; }

private:
	const config* game_config_;
	gamemap* map_;
	const std::vector<savegame::save_info>* info_;
	int index_;
	std::map<std::string,surface> map_cache_;
	const gui::filter_textbox& textbox_;
	gui::dialog_button* difficulty_option_;
};

void save_preview_pane::decide_on_difficulty_option()
{
	if (difficulty_option_ == NULL || size_t(index_) >= info_->size()) {
		return;
	}

	const config& summary = ((*info_)[index_]).summary();
	const bool start_of_scenario = summary["turn"].empty() && !summary["replay"].to_bool();
	difficulty_option_->enable(start_of_scenario);
}

void save_preview_pane::draw_contents()
{
	if(size_t(index_) >= info_->size()) {
		return;
	}

	surface screen = video().getSurface();

	SDL_Rect const &loc = location();
	const SDL_Rect area = create_rect(loc.x + save_preview_border
			, loc.y + save_preview_border
			, loc.w - save_preview_border * 2
			, loc.h - save_preview_border * 2);

	const clip_rect_setter clipper(screen, &area);

	int ypos = area.y;

	bool have_leader_image = false;

	const config& summary = ((*info_)[index_]).summary();
	const std::string& leader_image = summary["leader_image"].str();

	if(!leader_image.empty() && image::exists(leader_image))
	{
#ifdef LOW_MEM
		const surface& image(image::get_image(leader_image));
#else
		// NOTE: assuming magenta for TC here. This is what's used in all of
		// mainline, so the compromise should be good enough until we add more
		// summary fields to help with this and deciding the side color range.
		const surface& image(image::get_image(leader_image + "~RC(magenta>1)"));
#endif

		have_leader_image = !image.null();

		if(have_leader_image) {
			SDL_Rect image_rect = create_rect(area.x, area.y, image->w, image->h);
			ypos += image_rect.h + save_preview_border;

			sdl_blit(image,NULL,screen,&image_rect);
		}
	}

	std::string map_data = summary["map_data"];
	if(map_data.empty()) {
		const config &scenario = game_config_->find_child(summary["campaign_type"], "id", summary["scenario"]);
		if (scenario && !scenario.find_child("side", "shroud", "yes")) {
			map_data = scenario["map_data"].str();
			if (map_data.empty() && scenario.has_attribute("map")) {
				try {
					map_data = read_map(scenario["map"]);
				} catch(io_exception& e) {
					ERR_G << "could not read map '" << scenario["map"] << "': " << e.what() << '\n';
				}
			}
		}
	}

	surface map_surf(NULL);

	if(map_data.empty() == false) {
		const std::map<std::string,surface>::const_iterator itor = map_cache_.find(map_data);
		if(itor != map_cache_.end()) {
			map_surf = itor->second;
		} else if(map_ != NULL) {
			try {
				const int minimap_size = 100;
				map_->read(map_data);

				map_surf = image::getMinimap(minimap_size, minimap_size, *map_);
				if(map_surf != NULL) {
					map_cache_.insert(std::pair<std::string,surface>(map_data, map_surf));
				}
			} catch(incorrect_map_format_error& e) {
				ERR_CF << "map could not be loaded: " << e.message << '\n';
			} catch(twml_exception& e) {
				ERR_CF << "map could not be loaded: " << e.dev_message << '\n';
			}
		}
	}

	if(map_surf != NULL) {
		// Align the map to the left when the leader image is missing.
		const int map_x = have_leader_image ? area.x + area.w - map_surf->w : area.x;
		SDL_Rect map_rect = create_rect(map_x
				, area.y
				, map_surf->w
				, map_surf->h);

		ypos = std::max<int>(ypos,map_rect.y + map_rect.h + save_preview_border);
		sdl_blit(map_surf,NULL,screen,&map_rect);
	}

	char time_buf[256] = {0};
	const savegame::save_info& save = (*info_)[index_];
	tm* tm_l = localtime(&save.modified());
	if (tm_l) {
		const size_t res = util::strftime(time_buf,sizeof(time_buf),
			(preferences::use_twelve_hour_clock_format() ? _("%a %b %d %I:%M %p %Y") : _("%a %b %d %H:%M %Y")),
			tm_l);
		if(res == 0) {
			time_buf[0] = 0;
		}
	} else {
		LOG_NG << "localtime() returned null for time " << save.modified() << ", save " << save.name();
	}

	std::stringstream str;

	str << font::BOLD_TEXT << font::NULL_MARKUP
		<< (*info_)[index_].name() << '\n' << time_buf;

	const std::string& campaign_type = summary["campaign_type"];
	if (summary["corrupt"].to_bool()) {
		str << "\n" << _("#(Invalid)");
	} else if (!campaign_type.empty()) {
		str << "\n";

		if(campaign_type == "scenario") {
			const std::string campaign_id = summary["campaign"];
			const config *campaign = NULL;
			if (!campaign_id.empty()) {
				if (const config &c = game_config_->find_child("campaign", "id", campaign_id))
					campaign = &c;
			}
			utils::string_map symbols;
			if (campaign != NULL) {
				symbols["campaign_name"] = (*campaign)["name"];
			} else {
				// Fallback to nontranslatable campaign id.
				symbols["campaign_name"] = "(" + campaign_id + ")";
			}
			str << vgettext("Campaign: $campaign_name", symbols);

			// Display internal id for debug purposes if we didn't above
			if (game_config::debug && (campaign != NULL)) {
				str << '\n' << "(" << campaign_id << ")";
			}
		} else if(campaign_type == "multiplayer") {
			str << _("Multiplayer");
		} else if(campaign_type == "tutorial") {
			str << _("Tutorial");
		} else if(campaign_type == "test") {
			str << _("Test scenario");
		} else {
			str << campaign_type;
		}

		str << "\n";

		if (summary["replay"].to_bool() && !summary["snapshot"].to_bool(true)) {
			str << _("Replay");
		} else if (!summary["turn"].empty()) {
			str << _("Turn") << " " << summary["turn"];
		} else {
			str << _("Scenario start");
		}

		str << "\n" << _("Difficulty: ") << string_table[summary["difficulty"]];
		if(!summary["version"].empty()) {
			str << "\n" << _("Version: ") << summary["version"];
		}
	}

	font::draw_text(&video(), area, font::SIZE_SMALL, font::NORMAL_COLOR, str.str(), area.x, ypos, true);
}

} // end anon namespace

std::string load_game_dialog(display& disp, const config& game_config, bool* select_difficulty, bool* show_replay, bool* cancel_orders)
{
	std::vector<savegame::save_info> games;
	{
		cursor::setter cur(cursor::WAIT);
		games = savegame::get_saves_list();
	}

	if(games.empty()) {
		gui2::show_transient_message(disp.video(),
		                 _("No Saved Games"),
				 _("There are no saved games to load.\n\n(Games are saved automatically when you complete a scenario)"));
		return "";
	}

	const events::event_context context;

	std::vector<std::string> items;
	std::ostringstream heading;
	heading << HEADING_PREFIX << _("Name") << COLUMN_SEPARATOR << _("Date");
	items.push_back(heading.str());
	std::vector<savegame::save_info>::const_iterator i;
	for(i = games.begin(); i != games.end(); ++i) {
		std::string name = i->name();
		utf8::truncate(name, 40);	// truncate only acts if the name is longer

		std::ostringstream str;
		str << name << COLUMN_SEPARATOR << util::format_time_summary(i->modified());

		items.push_back(str.str());
	}

	gamemap map_obj(game_config, "");


	gui::dialog lmenu(disp,
			  _("Load Game"),
			  "", gui::NULL_DIALOG);
	lmenu.set_basic_behavior(gui::OK_CANCEL);

	gui::menu::basic_sorter sorter;
	sorter.set_alpha_sort(0).set_id_sort(1);
	lmenu.set_menu(items, &sorter);

	gui::filter_textbox* filter = new gui::filter_textbox(disp.video(), _("Filter: "), items, items, 1, lmenu);
	lmenu.set_textbox(filter);

	gui::dialog_button* change_difficulty_option = NULL;

	if(select_difficulty != NULL) {
		// implementation of gui::dialog::add_option, needed for storing a pointer to the option-box
		change_difficulty_option = new gui::dialog_button(disp.video(), _("Change difficulty"), gui::button::TYPE_CHECK);
		change_difficulty_option->set_check(false);

		change_difficulty_option->set_help_string(_("Change campaign difficulty before loading"));
		lmenu.add_button(change_difficulty_option, gui::dialog::BUTTON_CHECKBOX);
	}

	save_preview_pane save_preview(disp.video(),game_config,&map_obj,games,*filter,change_difficulty_option);
	lmenu.add_pane(&save_preview);
	// create an option for whether the replay should be shown or not
	if(show_replay != NULL) {
		lmenu.add_option(_("Show replay"), false,
			gui::dialog::BUTTON_CHECKBOX_LEFT,
			_("Play the embedded replay from the saved game if applicable")
			);
	}
	if(cancel_orders != NULL) {
		lmenu.add_option(_("Cancel orders"), false,
			gui::dialog::BUTTON_CHECKBOX_LEFT,
			_("Cancel any pending unit movements in the saved game")
			);
	}
	lmenu.add_button(new gui::standard_dialog_button(disp.video(),_("OK"),0,false), gui::dialog::BUTTON_STANDARD);
	lmenu.add_button(new gui::standard_dialog_button(disp.video(),_("Cancel"),1,true), gui::dialog::BUTTON_STANDARD);

	delete_save save_deleter(disp,*filter,games);
	gui::dialog_button_info delete_button(&save_deleter,_("Delete Save"));

	lmenu.add_button(delete_button,
		// Beware load screen glitches at low res!
		gui::dialog::BUTTON_HELP);

	int res = lmenu.show();

	if(res == -1)
		return "";

	res = filter->get_index(res);
	int option_index = 0;
	if (select_difficulty != NULL) {
		*select_difficulty = lmenu.option_checked(option_index++) && change_difficulty_option->enabled();
	}
	if(show_replay != NULL) {
	  *show_replay = lmenu.option_checked(option_index++);

		const config& summary = games[res].summary();
		if (summary["replay"].to_bool() && !summary["snapshot"].to_bool(true)) {
			*show_replay = true;
		}
	}
	if (cancel_orders != NULL) {
		*cancel_orders = lmenu.option_checked(option_index++);
	}

	return games[res].name();
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


handler_vector unit_preview_pane::handler_members()
{
	handler_vector h;
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

	surface screen = video().getSurface();

	SDL_Rect const &loc = location();
	const SDL_Rect area = create_rect(loc.x + unit_preview_border
			, loc.y + unit_preview_border
			, loc.w - unit_preview_border * 2
			, loc.h - unit_preview_border * 2);

	const clip_rect_setter clipper(screen, &area);

	surface unit_image = det.image;
	if (!left_)
		unit_image = image::reverse_image(unit_image);

	SDL_Rect image_rect = create_rect(area.x, area.y, 0, 0);

	if(unit_image != NULL) {
		SDL_Rect rect = create_rect(
				  right_align
					? area.x
					: area.x + area.w - unit_image->w
				, area.y
				, unit_image->w
				, unit_image->h);

		sdl_blit(unit_image,NULL,screen,&rect);
		image_rect = rect;

		if(!det.overlays.empty()) {
			BOOST_FOREACH(const std::string& overlay, det.overlays) {
				surface os = image::get_image(overlay);

				if(!os) {
					continue;
				}

				if(os->w > rect.w || os->h > rect.h) {
					os = scale_surface(os, rect.w, rect.h, false);
				}

				sdl_blit(os, NULL, screen, &rect);
			}
		}
	}

	// Place the 'unit profile' button
	const SDL_Rect button_loc = create_rect(
			  right_align
				? area.x
				: area.x + area.w - details_button_.location().w
			, image_rect.y + image_rect.h
			, details_button_.location().w
			, details_button_.location().h);
	details_button_.set_location(button_loc);

	SDL_Rect description_rect = create_rect(image_rect.x
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

		SDL_Rect cur_area = font::draw_text(&video(),location(),font::SIZE_SMALL,font::NORMAL_COLOR,*line,xpos,ypos);
		ypos += cur_area.h;
	}
}

units_list_preview_pane::units_list_preview_pane(const unit *u, TYPE type, bool on_left_side) :
	unit_preview_pane(NULL, type, on_left_side),
	units_(1, u)
{
}

units_list_preview_pane::units_list_preview_pane(const std::vector<const unit *> &units,
		const gui::filter_textbox* filter, TYPE type, bool on_left_side) :
	unit_preview_pane(filter, type, on_left_side),
	units_(units)
{
}

units_list_preview_pane::units_list_preview_pane(const std::vector<unit> &units,
		const gui::filter_textbox* filter, TYPE type, bool on_left_side) :
	unit_preview_pane(filter, type, on_left_side),
	units_(units.size())
{
	for (unsigned i = 0; i < units.size(); ++i)
		units_[i] = &units[i];
}

size_t units_list_preview_pane::size() const
{
	return units_.size();
}

const unit_preview_pane::details units_list_preview_pane::get_details() const
{
	const unit &u = *units_[index_];
	details det;

	det.image = u.still_image();

	det.name = u.name();
	det.type_name = u.type_name();
	det.race = u.race()->name(u.gender());
	det.level = u.level();
	det.alignment = unit_type::alignment_description(u.alignment(), u.gender());
	det.traits = utils::join(u.trait_names(), ", ");

	// The triples are base name, male/female name, description.
	const std::vector<boost::tuple<t_string,t_string,t_string> > &abilities = u.ability_tooltips();
	for(std::vector<boost::tuple<t_string,t_string,t_string> >::const_iterator a = abilities.begin();
		 a != abilities.end(); ++a) {
		det.abilities.push_back(a->get<1>());
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

	BOOST_FOREACH(const std::string& overlay, u.overlays()) {
		det.overlays.push_back(overlay);
	}

	return det;
}

void units_list_preview_pane::process_event()
{
	if (details_button_.pressed() && index_ >= 0 && index_ < int(size())) {
		show_unit_description(*units_[index_]);
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
	return (unit_types_!=NULL) ? unit_types_->size() : 0;
}

const unit_types_preview_pane::details unit_types_preview_pane::get_details() const
{
	const unit_type* t = (*unit_types_)[index_];
	details det;

	if (t==NULL)
		return det;

	// Make sure the unit type is built with enough data for our needs.
	unit_types.build_unit_type(*t, unit_type::WITHOUT_ANIMATIONS);

	std::string mod = "~RC(" + t->flag_rgb() + ">" + team::get_side_color_index(side_) + ")";
	det.image = image::get_image(t->image()+mod);

	det.name = "";
	det.type_name = t->type_name();
	det.level = t->level();
	det.alignment = unit_type::alignment_description(t->alignment(), t->genders().front());
	det.race = t->race()->name(t->genders().front());

	//FIXME: This probably must be move into a unit_type function
	BOOST_FOREACH(const config &tr, t->possible_traits())
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
	BOOST_FOREACH(const config &adv, t->modification_advancements())
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
	if (details_button_.pressed() && index_ >= 0 && index_ < int(size())) {
		const unit_type* type = (*unit_types_)[index_];
		if (type != NULL)
			show_unit_description(*type);
	}
}


void show_unit_description(const unit &u)
{
	show_unit_description(u.type());
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
		help::show_unit_help(*display::get_singleton(), t.id(), hide_help);
}

static network::connection network_data_dialog(display& disp, const std::string& msg, config& cfg, network::connection connection_num, network::statistics (*get_stats)(network::connection handle))
{
	const size_t width = 300;
	const size_t height = 80;
	const size_t border = 20;
	const int left = disp.w()/2 - width/2;
	const int top  = disp.h()/2 - height/2;

	const events::event_context dialog_events_context;

	gui::button cancel_button(disp.video(),_("Cancel"));
	std::vector<gui::button*> buttons_ptr(1,&cancel_button);

	gui::dialog_frame frame(disp.video(), msg, gui::dialog_frame::default_style, true, &buttons_ptr);
	SDL_Rect centered_layout = frame.layout(left,top,width,height).interior;
	centered_layout.x = disp.w() / 2 - centered_layout.w / 2;
	centered_layout.y = disp.h() / 2 - centered_layout.h / 2;
	// HACK: otherwise we get an empty useless space in the dialog below the progressbar
	centered_layout.h = height;
	frame.layout(centered_layout);
	frame.draw();

	const SDL_Rect progress_rect = create_rect(centered_layout.x + border
			, centered_layout.y + border
			, centered_layout.w - border * 2
			, centered_layout.h - border * 2);

	gui::progress_bar progress(disp.video());
	progress.set_location(progress_rect);

	events::raise_draw_event();
	disp.flip();

	network::statistics old_stats = get_stats(connection_num);

	cfg.clear();
	for(;;) {
		const network::connection res = network::receive_data(cfg,connection_num,100);
		const network::statistics stats = get_stats(connection_num);
		if(stats.current_max != 0 && stats != old_stats) {
			old_stats = stats;
			progress.set_progress_percent((stats.current*100)/stats.current_max);
			std::ostringstream stream;
			stream << utils::si_string(stats.current, true, _("unit_byte^B")) << "/" << utils::si_string(stats.current_max, true, _("unit_byte^B"));
			progress.set_text(stream.str());
		}

		events::raise_draw_event();
		disp.flip();
		events::pump();

		if(res != 0) {
			return res;
		}


		if(cancel_button.pressed()) {
			return res;
		}
	}
}

network::connection network_send_dialog(display& disp, const std::string& msg, config& cfg, network::connection connection_num)
{
	return network_data_dialog(disp, msg, cfg, connection_num,
							   network::get_send_stats);
}

network::connection network_receive_dialog(display& disp, const std::string& msg, config& cfg, network::connection connection_num)
{
	return network_data_dialog(disp, msg, cfg, connection_num,
							   network::get_receive_stats);
}

} // end namespace dialogs

namespace {

class connect_waiter : public threading::waiter
{
public:
	connect_waiter(display& disp, gui::button& button) : disp_(disp), button_(button)
	{}
	ACTION process();

private:
	display& disp_;
	gui::button& button_;
};

connect_waiter::ACTION connect_waiter::process()
{
	events::raise_draw_event();
	disp_.flip();
	events::pump();
	if(button_.pressed()) {
		return ABORT;
	} else {
		return WAIT;
	}
}

}

namespace dialogs
{

network::connection network_connect_dialog(display& disp, const std::string& msg, const std::string& hostname, int port)
{
	const size_t width = 250;
	const size_t height = 20;
	const int left = disp.w()/2 - width/2;
	const int top  = disp.h()/2 - height/2;

	const events::event_context dialog_events_context;

	gui::button cancel_button(disp.video(),_("Cancel"));
	std::vector<gui::button*> buttons_ptr(1,&cancel_button);

	gui::dialog_frame frame(disp.video(), msg, gui::dialog_frame::default_style, true, &buttons_ptr);
	frame.layout(left,top,width,height);
	frame.draw();

	events::raise_draw_event();
	disp.flip();

	connect_waiter waiter(disp,cancel_button);
	return network::connect(hostname,port,waiter);
}

} // end namespace dialogs
