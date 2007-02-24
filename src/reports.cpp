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
	const std::string report_names[] = { "unit_description", "unit_type",
		"unit_level","unit_amla","unit_traits","unit_status",
		"unit_alignment","unit_abilities","unit_hp","unit_xp",
		"unit_advancement_options","unit_moves","unit_weapons",
		"unit_image","unit_profile","time_of_day",
		"turn","gold","villages","num_units","upkeep", "expenses",
		"income", "terrain", "position", "side_playing", "observers",
		"report_countdown", "report_clock",
		"selected_terrain","edit_left_button_function"
	};
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

report generate_report(TYPE type, const gamemap& map, unit_map& units,
                       const std::vector<team>& teams, const team& current_team,
                       int current_side, int playing_side,
                       const gamemap::location& loc, const gamemap::location& mouseover,
                       const gamestatus& status, const std::set<std::string>& observers)
{
	unit_map::iterator u = units.end();
	SDL_Color HPC;
	SDL_Color XPC;


	if(int(type) >= int(UNIT_REPORTS_BEGIN) && int(type) < int(UNIT_REPORTS_END) || type == POSITION) {

		u = find_visible_unit(units,mouseover,
							  map,
							  teams,current_team);
		if(u == units.end()) {
			u = find_visible_unit(units,loc,
								  map,
								  teams,current_team);
			if(u == units.end() && type != POSITION) {
				return report();
			}
		}
	}

	std::stringstream str;

	switch(type) {
	case UNIT_DESCRIPTION:
		return report(u->second.description(),"",u->second.description());
	case UNIT_TYPE:
	        return report(u->second.language_name(),"",u->second.unit_description());
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

		if(map.on_board(mouseover) && u->second.invisible(mouseover,units,teams)) {
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
			tooltip << i->c_str();//string_table[*i + "_description"];
			res.add_text(str,tooltip);
		}

		return res;
	}
	case UNIT_HP: {
	  HPC=u->second.hp_color();
	  str << "<" << (int) HPC.r << "," << (int) HPC.g << "," << (int) HPC.b << ">"
	      << u->second.hitpoints() << "/" << u->second.max_hitpoints();
	  break;
	}
	case UNIT_XP: {
	  XPC=u->second.xp_color();
	  str << "<" << (int) XPC.r << "," << (int) XPC.g << "," << (int) XPC.b << ">"
	      << u->second.experience() << "/" << u->second.max_experience();
	  break;
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
	  unsigned int movement_left = u->second.movement_left();
	  if (u->second.side() != playing_side){
		  movement_left = u->second.total_movement();
	  }

	  int x = 180;
	  if(utils::string_bool(u->second.get_state("stoned"))){
	    x = 128;
	  }else{
	    x = (int)(128 + (255-128)*((float)movement_left/u->second.total_movement()));
	  }
	  str << "<" << x << "," << x << "," << x <<">";
	  str << movement_left << "/" << u->second.total_movement();
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
			at_it->set_specials_context(u->first,u->second);
			const std::string& lang_type = gettext(at_it->type().c_str());
			str.str("");
			str << "<245,230,193>";
			if(utils::string_bool(u->second.get_state("slowed"))) {
				str << round_damage(at_it->damage(),1,2) << "-" ;
			} else {
				str << at_it->damage() << "-" ;
			}
			int nattacks = at_it->num_attacks();
			// compute swarm attacks;
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
			tooltip << effdmg << " " << _n("tooltip^damage", "damage", effdmg) << ", ";
			tooltip << nattacks << " " << _n("tooltip^attack", "attacks", nattacks);

			str<<"\n";
			res.add_text(str,tooltip);

			str << "<166,146,117>  ";
			std::string range = _(at_it->range().c_str());
			str << range << "--" << lang_type << "\n";
			str<<"\n";

			tooltip << _("weapon range: ") << range <<"\n";
			tooltip << _("damage type: ") << lang_type << "\n";
			//find all the unit types on the map, and show this weapon's bonus against all the different units
			std::set<std::string> seen_units;
			std::map<int,std::vector<std::string> > resistances;
			for(unit_map::const_iterator u_it = units.begin(); u_it != units.end(); ++u_it) {
				if(teams[team_index].is_enemy(u_it->second.side()) && !current_team.fogged(u_it->first.x,u_it->first.y) &&
				   seen_units.count(u_it->second.id()) == 0) {
					seen_units.insert(u_it->second.id());
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
		const std::vector<Uint32>& old_rgb = u->second.team_rgb_range();
		color_range new_rgb = team::get_side_color_range(u->second.side());
	    return report("",image::locator(u->second.absolute_image(), new_rgb, old_rgb),"");
	}
	case UNIT_PROFILE:
		return report("",u->second.profile(),"");
	case TIME_OF_DAY: {
		time_of_day tod = timeofday_at(status,units,mouseover,map);
		// don't show illuminated time on fogged/shrouded tiles
		if (current_team.fogged(mouseover.x,mouseover.y) || current_team.shrouded(mouseover.x,mouseover.y)) {
			tod = status.get_time_of_day(false,mouseover);
		}
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
		const std::string& underlying = map.underlying_union_terrain(terrain);

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

		const int move_cost = u->second.movement_cost(terrain);
		const int defense = 100 - u->second.defense_modifier(terrain);

		if(move_cost > 10) {
			str << " (-)";
		} else {
			str << " (" << defense << "%," << move_cost << ")";
		}

		break;
	}

	case SIDE_PLAYING: {

		std::string flag;
		color_range new_rgb;
		std::vector<Uint32> old_rgb;

		if(current_team.flag().empty()) {
			flag = game_config::flag_image;
			old_rgb = game_config::flag_rgb;
			new_rgb = team::get_side_color_range(playing_side);
		} else {
			flag = current_team.flag();
		}

		//must recolor flag image
		animated<image::locator> temp_anim;

		// remove animation stuff we don't care about
		const std::vector<std::string> items = utils::split(flag);
		const std::vector<std::string> sub_items = utils::split(items[0], ':');
		image::locator flag_image(sub_items[0], new_rgb, old_rgb);
		return report("",flag_image,teams[playing_side-1].current_player());
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
		}
		break;
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
