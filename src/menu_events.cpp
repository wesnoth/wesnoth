#include "global.hpp"

#include "dialogs.hpp"
#include "display.hpp"
#include "game_config.hpp"
#include "game_errors.hpp"
#include "game_events.hpp"
#include "gettext.hpp"
#include "help.hpp"
#include "marked-up_text.hpp"
#include "menu_events.hpp"
#include "preferences_display.hpp"
#include "replay.hpp"
#include "unit_types.hpp"
#include "wml_separators.hpp"

#include <sstream>

namespace events{

	bool is_illegal_file_char(char c)
	{
		return c == '/' || c == '\\' || c == ':';
	}

	void menu_handler::objectives(display& gui, const config& level, team& current_team)
	{
		dialogs::show_objectives(gui, level, current_team.objectives());
		current_team.reset_objectives_changed();
	}

	void menu_handler::show_statistics(display& gui, const game_data& gameinfo)
	{
		const statistics::stats& stats = statistics::calculate_stats(0,gui.viewing_team()+1);
		std::vector<std::string> items;

		{
			std::stringstream str;
			str << _("Recruits") << COLUMN_SEPARATOR << statistics::sum_str_int_map(stats.recruits);
			items.push_back(str.str());
		}

		{
			std::stringstream str;
			str << _("Recalls") << COLUMN_SEPARATOR << statistics::sum_str_int_map(stats.recalls);
			items.push_back(str.str());
		}

		{
			std::stringstream str;
			str << _("Advancements") << COLUMN_SEPARATOR << statistics::sum_str_int_map(stats.advanced_to);
			items.push_back(str.str());
		}

		{
			std::stringstream str;
			str << _("Losses") << COLUMN_SEPARATOR << statistics::sum_str_int_map(stats.deaths);
			items.push_back(str.str());
		}

		{
			std::stringstream str;
			str << _("Kills") << COLUMN_SEPARATOR << statistics::sum_str_int_map(stats.killed);
			items.push_back(str.str());
		}

		{
			std::stringstream str;
			str << _("Damage Inflicted") << COLUMN_SEPARATOR << stats.damage_inflicted;
			items.push_back(str.str());
		}

		{
			std::stringstream str;
			str << _("Damage Taken") << COLUMN_SEPARATOR << stats.damage_taken;
			items.push_back(str.str());
		}

		{
			std::stringstream str;
			str << _("Damage Inflicted (EV)") << COLUMN_SEPARATOR
				<< (stats.expected_damage_inflicted / 100.0);
			items.push_back(str.str());
		}

		{
			std::stringstream str;
			str << _("Damage Taken (EV)") <<  COLUMN_SEPARATOR
				<< (stats.expected_damage_taken / 100.0);
			items.push_back(str.str());
		}

		for(;;) {
			const int res = gui::show_dialog(gui, NULL, _("Statistics"), "", gui::OK_CANCEL, &items);
			std::string title;
			std::vector<std::string> items_sub;

			switch(res) {
			case 0:
				items_sub = create_unit_table(stats.recruits, gameinfo);
				title = _("Recruits");
				break;
			case 1:
				items_sub = create_unit_table(stats.recalls, gameinfo);
				title = _("Recalls");
				break;
			case 2:
				items_sub = create_unit_table(stats.advanced_to, gameinfo);
				title = _("Advancements");
				break;
			case 3:
				items_sub = create_unit_table(stats.deaths, gameinfo);
				title = _("Losses");
				break;
			case 4:
				items_sub = create_unit_table(stats.killed, gameinfo);
				title = _("Kills");
				break;
			default:
				return;
			}

			if (items_sub.empty() == false)
				gui::show_dialog(gui, NULL, "", title, gui::OK_ONLY, &items_sub);
		}
	}

	std::vector<std::string> menu_handler::create_unit_table(const statistics::stats::str_int_map& m, const game_data& gameinfo)
	{
		std::vector<std::string> table;
		for(statistics::stats::str_int_map::const_iterator i = m.begin(); i != m.end(); ++i) {
			const game_data::unit_type_map::const_iterator type = gameinfo.unit_types.find(i->first);
			if(type == gameinfo.unit_types.end()) {
				continue;
			}

			std::stringstream str;
			str << IMAGE_PREFIX << type->second.image() << COLUMN_SEPARATOR
				<< type->second.language_name() << COLUMN_SEPARATOR << i->second << "\n";
			table.push_back(str.str());
		}

		return table;
	}

	void menu_handler::unit_list(const unit_map& units, display& gui, const gamemap& map)
	{
		const std::string heading = std::string(1,HEADING_PREFIX) +
									_("Type") + COLUMN_SEPARATOR +
									_("Name") + COLUMN_SEPARATOR +
									_("HP") + COLUMN_SEPARATOR +
									_("XP") + COLUMN_SEPARATOR +
									_("Traits") + COLUMN_SEPARATOR +
									_("Moves") + COLUMN_SEPARATOR +
									_("Location");

		gui::menu::basic_sorter sorter;
		sorter.set_alpha_sort(0).set_alpha_sort(1).set_numeric_sort(2).set_numeric_sort(3)
			  .set_alpha_sort(4).set_numeric_sort(5).set_numeric_sort(6);

		std::vector<std::string> items;
		items.push_back(heading);

		std::vector<gamemap::location> locations_list;
		std::vector<unit> units_list;
		for(unit_map::const_iterator i = units.begin(); i != units.end(); ++i) {
			if(i->second.side() != (gui.viewing_team()+1))
				continue;

			std::stringstream row;
			row << i->second.type().language_name() << COLUMN_SEPARATOR
				<< i->second.description() << COLUMN_SEPARATOR
				<< i->second.hitpoints() << "/" << i->second.max_hitpoints() << COLUMN_SEPARATOR
				<< i->second.experience() << "/";

			if(i->second.can_advance() == false)
				row << "-";
			else
				row << i->second.max_experience();

			row << COLUMN_SEPARATOR
				<< i->second.traits_description() << COLUMN_SEPARATOR
				<< i->second.movement_left() << "/"
				<< i->second.total_movement() << COLUMN_SEPARATOR
				<< i->first;

			items.push_back(row.str());

			locations_list.push_back(i->first);
			units_list.push_back(i->second);
		}

		int selected = 0;

		{
			const events::event_context dialog_events_context;
			dialogs::unit_preview_pane unit_preview(gui,&map,units_list);
			std::vector<gui::preview_pane*> preview_panes;
			preview_panes.push_back(&unit_preview);

			selected = gui::show_dialog(gui,NULL,_("Unit List"),"",
										gui::OK_ONLY,&items,&preview_panes,
										"",NULL,0,NULL,NULL,-1,-1,NULL,NULL,"",&sorter);
		}

		if(selected > 0 && selected < int(locations_list.size())) {
			const gamemap::location& loc = locations_list[selected];
			gui.scroll_to_tile(loc.x,loc.y,display::WARP);
		}
	}

	void menu_handler::status_table(std::vector<team>& teams, display& gui, const unit_map& units)
	{
		std::stringstream heading;
		heading << HEADING_PREFIX << _("Leader") << COLUMN_SEPARATOR << ' ' << COLUMN_SEPARATOR
				<< _("Gold") << COLUMN_SEPARATOR
				<< _("Villages") << COLUMN_SEPARATOR
				<< _("Units") << COLUMN_SEPARATOR
				<< _("Upkeep") << COLUMN_SEPARATOR
				<< _("Income");

		gui::menu::basic_sorter sorter;
		sorter.set_redirect_sort(0,1).set_alpha_sort(1).set_numeric_sort(2).set_numeric_sort(3)
			  .set_numeric_sort(4).set_numeric_sort(5).set_numeric_sort(6).set_numeric_sort(7);

		if(game_config::debug)
			heading << COLUMN_SEPARATOR << _("Gold");

		std::vector<std::string> items;
		items.push_back(heading.str());

		const team& viewing_team = teams[gui.viewing_team()];

		//if the player is under shroud or fog, they don't get to see
		//details about the other sides, only their own side, allied sides and a ??? is
		//shown to demonstrate lack of information about the other sides
		bool fog = false;
		for(size_t n = 0; n != teams.size(); ++n) {
			if(teams[n].is_empty()) {
				continue;
			}

			const bool known = viewing_team.knows_about_team(n);
			const bool enemy = viewing_team.is_enemy(n+1);
			if(!known) {
				fog = true;
				continue;
			}

			const team_data data = calculate_team_data(teams[n],n+1,units);

			std::stringstream str;

			const unit_map::const_iterator leader = team_leader(n+1,units);
			//output the number of the side first, and this will
			//cause it to be displayed in the correct colour
			if(leader != units.end()) {
				str << IMAGE_PREFIX << leader->second.type().image() << COLUMN_SEPARATOR
					<< "\033[3" << lexical_cast<char, size_t>(n+1) << 'm' << leader->second.description() << COLUMN_SEPARATOR;
			} else {
				str << ' ' << COLUMN_SEPARATOR << "\033[3" << lexical_cast<char, size_t>(n+1) << "m-" << COLUMN_SEPARATOR;
			}

			if(enemy) {
				str << ' ' << COLUMN_SEPARATOR;
			} else {
				str << data.gold << COLUMN_SEPARATOR;
			}
			str << data.villages << COLUMN_SEPARATOR
				<< data.units << COLUMN_SEPARATOR << data.upkeep << COLUMN_SEPARATOR
				<< (data.net_income < 0 ? font::BAD_TEXT : font::NULL_MARKUP) << data.net_income;

			if(game_config::debug)
				str << COLUMN_SEPARATOR << teams[n].gold();

			items.push_back(str.str());
		}

		if(fog)
			items.push_back(IMAGE_PREFIX + std::string("random-enemy.png") + COLUMN_SEPARATOR +
							IMAGE_PREFIX + "random-enemy.png");

		gui::show_dialog(gui,NULL,"","",gui::CLOSE_ONLY,&items,
						 NULL,"",NULL,0,NULL,NULL,-1,-1,NULL,NULL,"",&sorter);
	}

	void menu_handler::save_game(const std::string& message, gui::DIALOG_TYPE dialog_type,
		const game_state& gamestate, const gamestatus& status, display& gui, const config& level,
		std::vector<team>& teams, const unit_map& units, const gamemap& map)
	{
		std::stringstream stream;

		const std::string ellipsed_name = font::make_text_ellipsis(gamestate.label,
				font::SIZE_NORMAL, 200);
		stream << ellipsed_name << " " << _("Turn")
			   << " " << status.turn();
		std::string label = stream.str();
		if(dialog_type == gui::NULL_DIALOG && message != "") {
			label = message;
		}

		label.erase(std::remove_if(label.begin(),label.end(),is_illegal_file_char),label.end());

		const int res = dialog_type == gui::NULL_DIALOG ? 0 : dialogs::get_save_name(gui,message,_("Name:"),&label,dialog_type);

		if(res == 0) {

			if(std::count_if(label.begin(),label.end(),is_illegal_file_char)) {
				gui::show_dialog(gui,NULL,_("Error"),_("Save names may not contain colons, slashes, or backslashes. Please choose a different name."),gui::OK_ONLY);
				save_game(message,dialog_type, gamestate, status, gui, level, teams, units, map);
				return;
			}

			config snapshot;
			write_game_snapshot(snapshot, level, gui, teams, units, status, gamestate, map);
			try {
				recorder.save_game(label, snapshot, gamestate.starting_pos);
				if(dialog_type != gui::NULL_DIALOG) {
					gui::show_dialog(gui,NULL,_("Saved"),_("The game has been saved"), gui::OK_ONLY);
				}
			} catch(game::save_game_failed&) {
				gui::show_dialog(gui,NULL,_("Error"),_("The game could not be saved"),gui::MESSAGE);
				//do not bother retrying, since the user can just try to save the game again
			};
		}
	}

	void menu_handler::write_game_snapshot(config& start, const config& level, display& gui,
		std::vector<team>& teams, const unit_map& units, const gamestatus& status,
		const game_state& gamestate, const gamemap& map) const
	{
		start.values = level.values;

		start["snapshot"] = "yes";

		std::stringstream buf;
		buf << gui.playing_team();
		start["playing_team"] = buf.str();

		for(std::vector<team>::const_iterator t = teams.begin(); t != teams.end(); ++t) {
			const int side_num = t - teams.begin() + 1;

			config& side = start.add_child("side");
			t->write(side);
			side["no_leader"] = "yes";
			buf.str(std::string());
			buf << side_num;
			side["side"] = buf.str();

			for(std::map<gamemap::location,unit>::const_iterator i = units.begin(); i != units.end(); ++i) {
				if(i->second.side() == side_num) {
					config& u = side.add_child("unit");
					i->first.write(u);
					i->second.write(u);
				}
			}
		}

		status.write(start);
		game_events::write_events(start);

		// Write terrain_graphics data in snapshot, too
		const config::child_list& terrains = level.get_children("terrain_graphics");
		for(config::child_list::const_iterator tg = terrains.begin();
				tg != terrains.end(); ++tg) {

			start.add_child("terrain_graphics", **tg);
		}

		write_game(gamestate,start /*,WRITE_SNAPSHOT_ONLY*/);

		// Clobber gold values to make sure the snapshot uses the values
		// in [side] instead.
		const config::child_list& players=start.get_children("player");
		for(config::child_list::const_iterator pi=players.begin();
			pi!=players.end(); ++pi) {
			(**pi)["gold"] = "-1000000";
		}

		//write out the current state of the map
		start["map_data"] = map.write();

		gui.labels().write(start);
	}

	void menu_handler::load_game(display& gui, const config& terrain_config, const game_data& gameinfo){
		bool show_replay = false;
		const std::string game = dialogs::load_game_dialog(gui,terrain_config,gameinfo,&show_replay);
		if(game != "") {
			throw game::load_game_exception(game,show_replay);
		}
	}

	void menu_handler::preferences(display& gui, const config& terrain_config)
	{
		preferences::show_preferences_dialog(gui,terrain_config);
		gui.redraw_everything();
	}

	void menu_handler::show_chat_log(std::vector<team>& teams, display& gui)
	{
		std::string text = recorder.build_chat_log(teams[gui.viewing_team()].team_name());
		gui::show_dialog(gui,NULL,_("Chat Log"),"",gui::CLOSE_ONLY,NULL,NULL,"",&text);
	}

	void menu_handler::show_help(display& gui)
	{
		help::show_help(gui);
	}
}
