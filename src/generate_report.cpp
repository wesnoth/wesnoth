/* $Id$ */
/*
   Copyright (C) 2003 - 2008 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file generate_report.cpp
//! Formatted output of various stats about units and the game.
//! Used for the right sidebar and the top line of the main game-display.

#include "global.hpp"

#include "actions.hpp"
#include "game_config.hpp"
#include "gamestatus.hpp"
#include "gettext.hpp"
#include "language.hpp"
#include "marked-up_text.hpp"
#include "reports.hpp"
#include "game_preferences.hpp"

#include <cassert>
#include <ctime>
#include <map>
#include <set>
#include <sstream>

namespace reports {

report generate_report(TYPE type,
                       std::map<reports::TYPE, std::string> report_contents,
                       const gamemap& map, unit_map& units,
                       const std::vector<team>& teams, const team& current_team,
                       unsigned int current_side, unsigned int playing_side,
                       const gamemap::location& loc, const gamemap::location& mouseover, const gamemap::location& displayed_unit_hex,
                       const gamestatus& status, const std::set<std::string>& observers,
                       const config& level)
{
	unit_map::iterator u = units.end();

	if(int(type) >= int(UNIT_REPORTS_BEGIN) && int(type) < int(UNIT_REPORTS_END) || type == POSITION) {

		u = find_visible_unit(units,displayed_unit_hex,
				      map,
				      teams,current_team);
		if(u == units.end() && type != POSITION) {
			return report();
		}
	}

	std::stringstream str;

	switch(type) {
	// this one is for backward compatibility (should be removed in v1.5.3)
	case UNIT_NAME_OLD:
		std::cerr << "warning: ThemeWML [unit_description] tag will be removed in 1.5.3, run wmllint on WML defining " << std::endl;
	case UNIT_NAME:
		return report(u->second.name(),"",u->second.name());
	case UNIT_TYPE:
	        return report(u->second.language_name(),"",u->second.unit_description());
	case UNIT_RACE:
	        return report(u->second.race()->name(u->second.gender()));
	case UNIT_SIDE: {
		std::string flag_icon = teams[u->second.side()-1].flag_icon();
		std::string old_rgb = game_config::flag_rgb;
		std::string new_rgb = team::get_side_colour_index(u->second.side());
		std::string mods = "~RC(" + old_rgb + ">" + new_rgb + ")";

		if(flag_icon.empty()) {
			flag_icon = game_config::flag_icon_image;
		}

		image::locator flag_icon_img(flag_icon, mods);
		return report("",flag_icon_img,teams[u->second.side()-1].current_player());
	}
	case UNIT_LEVEL:
		str << u->second.level();
		break;
	case UNIT_AMLA: {
	  report res;
	  const std::vector<std::pair<std::string,std::string> > & amla_icons=u->second.amla_icons();
	  for(std::vector<std::pair<std::string,std::string> >::const_iterator i=amla_icons.begin();i!=amla_icons.end();i++){
	    res.add_image(i->first,i->second);
	  }
	  return(res);
	}
	case UNIT_TRAITS:
		return report(u->second.traits_description(),"",u->second.modification_description("trait"));
	case UNIT_STATUS: {
		std::stringstream unit_status;
		std::stringstream tooltip;
		report res;

		if(map.on_board(u->first) && u->second.invisible(u->first,units,teams))
		{
			unit_status << "misc/invisible.png";
			tooltip << _("invisible: ") << _("This unit is invisible. It cannot be seen or attacked by enemy units.");
			res.add_image(unit_status,tooltip);
		}
		if(utils::string_bool(u->second.get_state("slowed"))) {
			unit_status << "misc/slowed.png";
			tooltip << _("slowed: ") << _("This unit has been slowed. It will only deal half its normal damage when attacking.");
			res.add_image(unit_status,tooltip);
		}
		if(utils::string_bool(u->second.get_state("poisoned"))) {
			unit_status << "misc/poisoned.png";
			tooltip << _("poisoned: ") << _("This unit is poisoned. It will lose 8 HP every turn until it can seek a cure to the poison in a village or from a friendly unit with the 'cures' ability.\n\
\n\
Units cannot be killed by poison alone. The poison will not reduce it below 1 HP.");
			res.add_image(unit_status,tooltip);
		}
		if(utils::string_bool(u->second.get_state("stoned"))) {
			unit_status << "misc/stone.png";
			tooltip << _("stone: ") << _("This unit has been turned to stone. It may not move or attack.");
			res.add_image(unit_status,tooltip);
		}

		return res;
	}
	case UNIT_ALIGNMENT: {
		const std::string& align = unit_type::alignment_description(u->second.alignment());
		const std::string& align_id = unit_type::alignment_id(u->second.alignment());
		return report(align, "", string_table[align_id + "_description"]);
	}
	case UNIT_ABILITIES: {
		report res;
		std::stringstream tooltip;
		const std::vector<std::string>& abilities = u->second.ability_tooltips(u->first);
		for(std::vector<std::string>::const_iterator i = abilities.begin(); i != abilities.end(); ++i) {
			str << gettext(i->c_str());
			if(i+2 != abilities.end())
				str << ",";
			++i;
			tooltip << i->c_str(); //string_table[*i + "_description"];
			res.add_text(str,tooltip);
		}

		return res;
	}
	case UNIT_HP: {
	    report res;
	    std::stringstream tooltip;
		str << font::color2markup( u->second.hp_color() );
		str << u->second.hitpoints() << "/" << u->second.max_hitpoints();

		std::vector<std::string> resistances_table;

		string_map resistances = u->second.get_base_resistances();

		for(string_map::iterator resist = resistances.begin();
				resist != resistances.end(); ++resist) {
			std::stringstream line;
			line << gettext(resist->first.c_str()) << ": ";

			// Some units have different resistances when
			// attacking or defending.
			int res_att = 100 - u->second.resistance_against(resist->first, true, u->first);
			int res_def = 100 - u->second.resistance_against(resist->first, false, u->first);
			if (res_att == res_def) {
				line << res_def << "%\n";
			} else {
				line << res_att << "% / " << res_def << "%\n";
			}
			resistances_table.push_back(line.str());
		}

		tooltip << _("Resistances: ") << "\n";
		std::sort(resistances_table.begin(),resistances_table.end());
		for(std::vector<std::string>::iterator line = resistances_table.begin();
				line != resistances_table.end(); ++line) {
			tooltip << (*line);
		}

		res.add_text(str,tooltip);
		return res ;
	}
	case UNIT_XP: {
	  report res;
	  std::stringstream tooltip;

	  str << font::color2markup( u->second.xp_color() );
	  str << u->second.experience() << "/" << u->second.max_experience();

	  tooltip << _("Experience Modifier: ") << ((level["experience_modifier"] != "") ? level["experience_modifier"] : "100") << "%";
	  res.add_text(str,tooltip);

	  return res;
	}
	case UNIT_ADVANCEMENT_OPTIONS: {
	  report res;
	  const std::map<std::string,std::string>& adv_icons=u->second.advancement_icons();
	  for(std::map<std::string,std::string>::const_iterator i=adv_icons.begin();i!=adv_icons.end();i++){
	    res.add_image(i->first,i->second);
	  }
	  return(res);

	}
	case UNIT_MOVES: {
	  float movement_frac = 1.0;
	  if (u->second.side() == playing_side){
		  movement_frac = u->second.movement_left() / maximum<float>(1.0, u->second.total_movement());
		  if (movement_frac > 1.0) movement_frac = 1.0;
	  }

	  int grey = 128 + static_cast<int>((255-128) * movement_frac);
	  str << "<" << grey << "," << grey << "," << grey <<">";
	  str << u->second.movement_left() << "/" << u->second.total_movement();
		break;
	}
	case UNIT_WEAPONS: {
		report res;
		std::stringstream tooltip;

		const size_t team_index = u->second.side()-1;
		if(team_index >= teams.size()) {
			std::cerr << "illegal team index in reporting: " << team_index << "\n";
			return res;
		}

		std::vector<attack_type>& attacks = u->second.attacks();
		for(std::vector<attack_type>::iterator at_it = attacks.begin();
		    at_it != attacks.end(); ++at_it) {
			at_it->set_specials_context(u->first,gamemap::location(),u->second);
			const std::string& lang_type = gettext(at_it->type().c_str());
			str.str("");
			str << "<245,230,193>";
			if(utils::string_bool(u->second.get_state("slowed"))) {
				str << round_damage(at_it->damage(),1,2) << "-" ;
			} else {
				str << at_it->damage() << "-" ;
			}
			int nattacks = at_it->num_attacks();
			// Compute swarm attacks:
			unit_ability_list swarm = at_it->get_specials("attacks");
			if(!swarm.empty()) {
				int swarm_max_attacks = swarm.highest("attacks_max",nattacks).first;
				int swarm_min_attacks = swarm.highest("attacks_min").first;
				int hitp = u->second.hitpoints();
				int mhitp = u->second.max_hitpoints();

				nattacks = swarm_min_attacks + swarm_max_attacks * hitp / mhitp;

			} else {
				nattacks = at_it->num_attacks();
			}
			str << nattacks;
			str << " " << at_it->name();
			tooltip << at_it->name() << "\n";
			int effdmg;
			if(utils::string_bool(u->second.get_state("slowed"))) {
				effdmg = round_damage(at_it->damage(),1,2);
			} else {
				effdmg = at_it->damage();
			}
			tooltip << effdmg   << " " << _n("tooltip^damage", "damage",  effdmg) << ", ";
			tooltip << nattacks << " " << _n("tooltip^attack", "attacks", nattacks);

			str<<"\n";
			res.add_text(str,tooltip);

			str << "<166,146,117>  ";
			std::string range = _(at_it->range().c_str());
			str << range << "--" << lang_type << "\n";
			str<<"\n";

			tooltip << _("weapon range: ") << range <<"\n";
			tooltip << _("damage type: ")  << lang_type << "\n";
			// Find all the unit types on the map, and
			// show this weapon's bonus against all the different units.
			// Don't show invisible units, except if they are in our team or allied.
			std::set<std::string> seen_units;
			std::map<int,std::vector<std::string> > resistances;
			for(unit_map::const_iterator u_it = units.begin(); u_it != units.end(); ++u_it) {
				if(teams[team_index].is_enemy(u_it->second.side()) &&
				   !current_team.fogged(u_it->first) &&
				   seen_units.count(u_it->second.type_id()) == 0 &&
				   ( !current_team.is_enemy(u_it->second.side()) ||
				     !u_it->second.invisible(u_it->first,units,teams)))
				{
					seen_units.insert(u_it->second.type_id());
					const int resistance = u_it->second.resistance_against(*at_it,false,u_it->first) - 100;
					resistances[resistance].push_back(u_it->second.language_name());
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


			const std::vector<std::string>& specials = at_it->special_tooltips();

			if(! specials.empty()) {
				for(std::vector<std::string>::const_iterator sp_it = specials.begin(); sp_it != specials.end(); ++sp_it) {
					str << "<166,146,117>  ";
					str << gettext(sp_it->c_str());
					str<<"\n";
					++sp_it;
					tooltip << gettext(sp_it->c_str());
				}
				res.add_text(str,tooltip);
			}
		}

		return res;
	}
	case UNIT_IMAGE:
	{
//		const std::vector<Uint32>& old_rgb = u->second.team_rgb_range();
//		color_range new_rgb = team::get_side_color_range(u->second.side());
	    return report("",image::locator(u->second.absolute_image(),u->second.image_mods()),"");
	}
	case UNIT_PROFILE:
		return report("",u->second.profile(),"");
	case TIME_OF_DAY: {
		time_of_day tod = timeofday_at(status,units,mouseover,map);
		const std::string tod_image = tod.image + (preferences::flip_time() ? "~FL(horiz)" : "");

		// Don't show illuminated time on fogged/shrouded tiles
		if (current_team.fogged(mouseover) || current_team.shrouded(mouseover)) {
			tod = status.get_time_of_day(false,mouseover);
		}
		std::stringstream tooltip;

		tooltip << tod.name << "\n"
		        << _("Lawful units: ")
				<< (tod.lawful_bonus > 0 ? "+" : "") << tod.lawful_bonus << "%\n"
				<< _("Neutral units: ") << "0%\n"
				<< _("Chaotic units: ")
				<< (tod.lawful_bonus < 0 ? "+" : "") << (tod.lawful_bonus*-1) << "%";

		return report("",tod_image,tooltip.str());
	}
	case TURN:
		str << status.turn();

		if(status.number_of_turns() != -1) {
			str << "/" << status.number_of_turns();
		}

		str << "\n";
		break;
	// For the following status reports, show them in gray text
	// when it is not the active player's turn.
	case GOLD:
		str << (current_side != playing_side ? font::GRAY_TEXT : (current_team.gold() < 0 ? font::BAD_TEXT : font::NULL_MARKUP)) << current_team.gold();
		break;
	case VILLAGES: {
		const team_data data = calculate_team_data(current_team,current_side,units);
		str << (current_side != playing_side ? font::GRAY_TEXT : font::NULL_MARKUP) << data.villages << "/";
		if (current_team.uses_shroud()) {
			int unshrouded_villages = 0;
			std::vector<gamemap::location>::const_iterator i = map.villages().begin();
			for (; i != map.villages().end(); i++) {
				if (!current_team.shrouded(*i))
					 unshrouded_villages++;
			}
			str << unshrouded_villages;
		} else {
			str << map.villages().size();
		}
		break;
	}
	case NUM_UNITS: {
		str << (current_side != playing_side ? font::GRAY_TEXT : font::NULL_MARKUP) << team_units(units,current_side);
		break;
	}
	case UPKEEP: {
		const team_data data = calculate_team_data(current_team,current_side,units);
		str << (current_side != playing_side ? font::GRAY_TEXT : font::NULL_MARKUP) << data.expenses << " (" << data.upkeep << ")";
		break;
	}
	case EXPENSES: {
		const team_data data = calculate_team_data(current_team,current_side,units);
		str << (current_side != playing_side ? font::GRAY_TEXT : font::NULL_MARKUP) << data.expenses;
		break;
	}
	case INCOME: {
		const team_data data = calculate_team_data(current_team,current_side,units);
		str << (current_side != playing_side ? font::GRAY_TEXT : (data.net_income < 0 ? font::BAD_TEXT : font::NULL_MARKUP)) << data.net_income;
		break;
	}
	case TERRAIN: {
		if(!map.on_board(mouseover) || current_team.shrouded(mouseover))
			break;

		const t_translation::t_terrain terrain = map.get_terrain(mouseover);
		if (terrain == t_translation::OFF_MAP_USER)
			break;

		const t_translation::t_list& underlying = map.underlying_union_terrain(terrain);

		if(map.is_village(mouseover)) {
			const unsigned int owner = village_owner(mouseover,teams)+1;
			if(owner == 0 || current_team.fogged(mouseover)) {
				str << map.get_terrain_info(terrain).income_description();
			} else if(owner == current_side) {
				str << map.get_terrain_info(terrain).income_description_own();
			} else if(current_team.is_enemy(owner)) {
				str << map.get_terrain_info(terrain).income_description_enemy();
			} else {
				str << map.get_terrain_info(terrain).income_description_ally();
			}
			str << " ";
		} else {
		        str << map.get_terrain_info(terrain).name();
		}

		if(underlying.size() != 1 || underlying.front() != terrain) {
			str << " (";

			for(t_translation::t_list::const_iterator i =
					underlying.begin(); i != underlying.end(); ++i) {

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

		const t_translation::t_terrain terrain = map[mouseover];

		if (terrain == t_translation::OFF_MAP_USER)
			break;

		str << mouseover;

		if(u == units.end())
			break;
		if(displayed_unit_hex != mouseover && displayed_unit_hex != loc)
			break;
		if(current_team.shrouded(mouseover))
			break;

		const int move_cost = u->second.movement_cost(terrain);
		const int defense = 100 - u->second.defense_modifier(terrain);

		if(move_cost < 99) {
			str << " (" << defense << "%," << move_cost << ")";
		} else if (mouseover == displayed_unit_hex) {
			str << " (" << defense << "%,-)";
		} else {
			str << " (-)";
		}

		break;
	}

	case SIDE_PLAYING: {
		std::string flag_icon = teams[playing_side-1].flag_icon();
		std::string old_rgb = game_config::flag_rgb;
		std::string new_rgb = team::get_side_colour_index(playing_side);
		std::string mods = "~RC(" + old_rgb + ">" + new_rgb + ")";

		if(flag_icon.empty()) {
			flag_icon = game_config::flag_icon_image;
		}

		image::locator flag_icon_img(flag_icon, mods);
		return report("",flag_icon_img,teams[playing_side-1].current_player());
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
	case REPORT_COUNTDOWN: {
		int min;
		int sec;
		if (current_team.countdown_time() > 0){
			sec = current_team.countdown_time() / 1000;

			str << (current_side != playing_side ? font::GRAY_TEXT : font::NORMAL_TEXT);

			if(sec < 60)
				str << "<200,0,0>";
			else if(sec < 120)
				str << "<200,200,0>";

			min = sec / 60;
			str << min << ":";
			sec = sec % 60;
			if (sec < 10) {
				str << "0";
			}
			str << sec;
			break;
		} // Intentional fall-through to REPORT_CLOCK
		  // if the time countdown isn't valid.
		  // If there is no turn time limit,
		  // then we display the clock instead.
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
		assert(false);
		break;
	}
	return report(str.str());
}

} // end namespace reports

