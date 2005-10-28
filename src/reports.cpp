/* $Id$ */
/*
   Copyright (C) 2003-5 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include "actions.hpp"
#include "game_config.hpp"
#include "gamestatus.hpp"
#include "gettext.hpp"
#include "language.hpp"
#include "marked-up_text.hpp"
#include "reports.hpp"
#include "wassert.hpp"
#include "preferences.hpp"

#include <ctime>
#include <map>
#include <set>
#include <sstream>

namespace {
	const std::string report_names[] = { "unit_description", "unit_type", "unit_level",
		"unit_traits","unit_status","unit_alignment","unit_abilities","unit_hp","unit_xp",
		"unit_moves","unit_weapons","unit_image","unit_profile","time_of_day",
		"turn","gold","villages","num_units","upkeep", "expenses",
		 "income", "terrain", "position", "side_playing", "observers",
		 "report_clock",
		 "selected_terrain","edit_left_button_function"};
	std::map<reports::TYPE, std::string> report_contents;
}

namespace reports {

const std::string& report_name(TYPE type)
{
	wassert(sizeof(report_names)/sizeof(*report_names) == NUM_REPORTS);
	wassert(type < NUM_REPORTS);

	return report_names[type];
}

void report::add_text(std::stringstream& text, std::stringstream& tooltip) {
	add_text(text.str(), tooltip.str());
	// Clear the streams
	text.str("");
	tooltip.str("");
}

void report::add_text(const std::string& text, const std::string& tooltip) {
	this->push_back(element(text,"",tooltip));
}

void report::add_image(std::stringstream& image, std::stringstream& tooltip) {
	add_image(image.str(), tooltip.str());
	// Clear the streams
	image.str("");
	tooltip.str("");
}

void report::add_image(const std::string& image, const std::string& tooltip) {
	this->push_back(element("",image,tooltip));
}

report generate_report(TYPE type, const gamemap& map, const unit_map& units,
                       const std::vector<team>& teams, const team& current_team,
                       int current_side, int playing_side,
                       const gamemap::location& loc, const gamemap::location& mouseover,
                       const gamestatus& status, const std::set<std::string>& observers)
{
	unit_map::const_iterator u = units.end();

	if(int(type) >= int(UNIT_REPORTS_BEGIN) && int(type) < int(UNIT_REPORTS_END) || type == POSITION) {

		if(!current_team.fogged(mouseover.x,mouseover.y)) {
			u = find_visible_unit(units,mouseover,
					map,
					status.get_time_of_day().lawful_bonus,
					teams,current_team);
		}

		if(u == units.end()) {
			if(!current_team.fogged(loc.x,loc.y)) {
				u = find_visible_unit(units,loc,
						map,
						status.get_time_of_day().lawful_bonus,
						teams,current_team);
			}

			if(u == units.end() && type != POSITION) {
				return report();
			}
		}
	}

	std::stringstream str;

	switch(type) {
	case UNIT_DESCRIPTION:
		return report(u->second.description());
	case UNIT_TYPE:
		return report(u->second.type().language_name(),"",u->second.unit_description());
	case UNIT_LEVEL:
		str << u->second.type().level();
		break;
	case UNIT_TRAITS:
		return report(u->second.traits_description(),"",u->second.modification_description("trait"));
	case UNIT_STATUS: {
		std::stringstream unit_status;
		std::stringstream tooltip;
		report res;

		if(map.on_board(loc) && u->second.invisible(map.underlying_terrain(map[loc.x][loc.y]),status.get_time_of_day().lawful_bonus,loc,units,teams)) {
			unit_status << "misc/invisible.png";
			tooltip << _("invisible: ") << _("This unit is invisible. It cannot be seen or attacked by enemy units.");
			res.add_image(unit_status,tooltip);
		}
		if(u->second.has_flag("slowed")) {
			unit_status << "misc/slowed.png";
			tooltip << _("slowed: ") << _("This unit has been slowed. It moves at half normal speed and receives one less attack than normal in combat.");
			res.add_image(unit_status,tooltip);
		}
		if(u->second.has_flag("poisoned")) {
			unit_status << "misc/poisoned.png";
			tooltip << _("poisoned: ") << _("This unit is poisoned. It will lose 8 HP every turn until it can seek a cure to the poison in a village or from a friendly unit with the 'cures' ability.\n\
\n\
Units cannot be killed by poison alone. The poison will not reduce it below 1 HP.");
			res.add_image(unit_status,tooltip);
		}
		if(u->second.has_flag("stone")) {
			unit_status << "misc/stone.png";
			tooltip << _("stone: ") << _("This unit has been turned to stone. It may not move or attack.");
			res.add_image(unit_status,tooltip);
		}

		return res;
	}
	case UNIT_ALIGNMENT: {
		const std::string& align = unit_type::alignment_description(u->second.type().alignment());
		const std::string& align_id = unit_type::alignment_id(u->second.type().alignment());
		return report(align, "", string_table[align_id + "_description"]);
	}
	case UNIT_ABILITIES: {
		report res;
		std::stringstream tooltip;
		const std::vector<std::string>& abilities = u->second.type().abilities();
		for(std::vector<std::string>::const_iterator i = abilities.begin(); i != abilities.end(); ++i) {
			str << gettext(i->c_str());
			if(i+1 != abilities.end())
				str << ",";

			tooltip << string_table[*i + "_description"];
			res.add_text(str,tooltip);
		}

		return res;
	}
	case UNIT_HP:
		if(u->second.hitpoints() <= u->second.max_hitpoints()/3)
			str << font::BAD_TEXT;
		else if(u->second.hitpoints() > 2*(u->second.max_hitpoints()/3))
			str << font::GOOD_TEXT;

		str << u->second.hitpoints()
		    << "/" << u->second.max_hitpoints() << "\n";

		break;
	case UNIT_XP:
		if(u->second.can_advance() == false) {
			str << u->second.experience() << "/-";
		} else {
			//if killing a unit of the same level as us lets us advance, display in 'good' colour
			if(u->second.max_experience() - u->second.experience() <= game_config::kill_experience*u->second.type().level()) {
				str << font::GOOD_TEXT;
			}

			str << u->second.experience() << "/" << u->second.max_experience();
		}

		break;
	case UNIT_MOVES:
		str << u->second.movement_left() << "/" << u->second.total_movement();
		break;
	case UNIT_WEAPONS: {
		report res;
		std::stringstream tooltip;

		const size_t team_index = u->second.side()-1;
		if(team_index >= teams.size()) {
			std::cerr << "illegal team index in reporting: " << team_index << "\n";
			return res;
		}

		const std::vector<attack_type>& attacks = u->second.attacks();
		for(std::vector<attack_type>::const_iterator at_it = attacks.begin();
		    at_it != attacks.end(); ++at_it) {
			const std::string& lang_type = gettext(at_it->type().c_str());

			str << at_it->name() << " (" << lang_type << ")\n";

			tooltip << at_it->name() << " (" << lang_type << ")\n";

			//find all the unit types on the map, and show this weapon's bonus against all the different units
			std::set<std::string> seen_units;
			std::map<int,std::vector<std::string> > resistances;
			for(unit_map::const_iterator u_it = units.begin(); u_it != units.end(); ++u_it) {
				if(teams[team_index].is_enemy(u_it->second.side()) && !current_team.fogged(u_it->first.x,u_it->first.y) &&
				   seen_units.count(u_it->second.type().id()) == 0) {
					seen_units.insert(u_it->second.type().id());
					const int resistance = u_it->second.type().movement_type().resistance_against(*at_it) - 100;
					resistances[resistance].push_back(u_it->second.type().language_name());
				}
			}

			for(std::map<int,std::vector<std::string> >::reverse_iterator resist = resistances.rbegin(); resist != resistances.rend(); ++resist) {
				std::sort(resist->second.begin(),resist->second.end());
				tooltip << (resist->first >= 0 ? "+" : "") << resist->first << "% " << _("vs") << " ";
				for(std::vector<std::string>::const_iterator i = resist->second.begin(); i != resist->second.end(); ++i) {
					if(i != resist->second.begin()) {
						tooltip << ",";
					}

					tooltip << *i;
				}

				tooltip << "\n";
			}

			res.add_text(str,tooltip);

			if (!at_it->special().empty()) {
				str << gettext(at_it->special().c_str()) << "\n";
				tooltip << string_table["weapon_special_" + at_it->special() + "_description"];
				res.add_text(str,tooltip);
			}

			str << at_it->damage() << "-" << at_it->num_attacks() << " -- "
		        << (at_it->range() == attack_type::SHORT_RANGE ?
		            _("melee") :
					_("ranged"));
			tooltip << at_it->damage() << " " << _("damage") << ", "
					<< at_it->num_attacks() << " " << _("attacks");

			str << "\n";
			res.add_text(str,tooltip);
		}

		return res;
	}
	case UNIT_IMAGE:
		return report("",u->second.type().image(),"");
	case UNIT_PROFILE:
		return report("",u->second.type().image_profile(),"");
	case TIME_OF_DAY: {
		const time_of_day& tod = timeofday_at(status,units,mouseover);
		std::stringstream tooltip;

		tooltip << tod.name << "\n"
		        << _("Lawful units: ")
				<< (tod.lawful_bonus > 0 ? "+" : "") << tod.lawful_bonus << "%\n"
				<< _("Neutral units: ") << "0%\n"
				<< _("Chaotic units: ")
				<< (tod.lawful_bonus < 0 ? "+" : "") << (tod.lawful_bonus*-1) << "%";

		return report("",tod.image,tooltip.str());
	}
	case TURN:
		str << status.turn();

		if(status.number_of_turns() != -1) {
			str << "/" << status.number_of_turns();
		}

		str << "\n";
		break;
	case GOLD:
		str << (current_team.gold() < 0 ? font::BAD_TEXT : font::NULL_MARKUP) << current_team.gold();
		break;
	case VILLAGES: {
		const team_data data = calculate_team_data(current_team,current_side,units);
		str << data.villages;

		break;
	}
	case NUM_UNITS: {
		str << team_units(units,current_side);
		break;
	}
	case UPKEEP: {
		const team_data data = calculate_team_data(current_team,current_side,units);
		str << data.expenses << " (" << data.upkeep << ")";
		break;
	}
	case EXPENSES: {
		const team_data data = calculate_team_data(current_team,current_side,units);
		str << data.expenses;
		break;
	}
	case INCOME: {
		const team_data data = calculate_team_data(current_team,current_side,units);
		str << (data.net_income < 0 ? font::BAD_TEXT : font::NULL_MARKUP) << data.net_income;
		break;
	}
	case TERRAIN: {
		if(!map.on_board(mouseover) || current_team.shrouded(mouseover.x,mouseover.y))
			break;

		const gamemap::TERRAIN terrain = map.get_terrain(mouseover);
		const std::string& underlying = map.underlying_terrain(terrain);

		if(map.is_village(mouseover)) {
			const int owner = village_owner(mouseover,teams)+1;
			if(owner == 0 || current_team.fogged(mouseover.x,mouseover.y)) {
				str << _("Village");
			} else if(owner == current_side) {
				str << _("Owned village");
			} else if(current_team.is_enemy(owner)) {
				str << _("Enemy village");
			} else {
				str << _("Allied village");
			}

			str << " ";
		} else {
			str << map.get_terrain_info(terrain).name();
		}

		if(underlying.size() != 1 || underlying[0] != terrain) {
			str << " (";

			for(std::string::const_iterator i = underlying.begin(); i != underlying.end(); ++i) {
				str << map.get_terrain_info(*i).name();
				if(i+1 != underlying.end()) {
					str << ",";
				}
			}

			str << ")";
		}

		break;
	}
	case POSITION: {
		if(!map.on_board(mouseover)) {
			break;
		}

		str << mouseover;

		if(u == units.end() || current_team.shrouded(mouseover.x,mouseover.y))
			break;

		const gamemap::TERRAIN terrain = map[mouseover.x][mouseover.y];

		const int move_cost = u->second.movement_cost(map,terrain);
		const int defense = 100 - u->second.defense_modifier(map,terrain);

		if(move_cost > 10) {
			str << " (-)";
		} else {
			str << " (" << defense << "%," << move_cost << ")";
		}

		break;
	}

	case SIDE_PLAYING: {
		char buf[50];
		sprintf(buf,"terrain/flag-team%d-1.png",team::get_side_colour_index(playing_side));

		u = find_leader(units,playing_side);
		return report("",buf,u != units.end() ? u->second.description() : "");
	}

	case OBSERVERS: {
		if(observers.empty()) {
			return report();
		}

		str << _("Observers:") << "\n";

		for(std::set<std::string>::const_iterator i = observers.begin(); i != observers.end(); ++i) {
			str << *i << "\n";
		}

		return report("",game_config::observer_image,str.str());
	}
	case SELECTED_TERRAIN: {
		std::map<TYPE, std::string>::const_iterator it =
			report_contents.find(SELECTED_TERRAIN);
		if (it != report_contents.end()) {
			return report(it->second);
		}
		else {
			return report();
		}
	}
	case EDIT_LEFT_BUTTON_FUNCTION: {
		std::map<TYPE, std::string>::const_iterator it =
			report_contents.find(EDIT_LEFT_BUTTON_FUNCTION);
		if (it != report_contents.end()) {
			return report(it->second);
		}
		else {
			return report();
		}
	}
	case REPORT_CLOCK: {
		time_t t = time(NULL);
		struct tm *lt=localtime(&t);
		char temp[10];
		size_t s = strftime(temp,10,preferences::clock_format().c_str(),lt);
		if(s>0) {
			return report(temp);
		} else {
			return report();
		}
	}

	default:
		wassert(false);
		break;
	}
	return report(str.str());
}


void set_report_content(const TYPE which_report, const std::string &content) {
	report_contents[which_report] = content;
}


}
