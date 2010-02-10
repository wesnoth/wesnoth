/* $Id$ */
/*
   Copyright (C) 2003 - 2010 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef TEAM_H_INCLUDED
#define TEAM_H_INCLUDED

#include "config.hpp"
#include "color_range.hpp"
#include "game_config.hpp"
#include "map_location.hpp"
#include "viewpoint.hpp"
#include "savegame_config.hpp"
#include "unit.hpp"

class gamemap;

#include <set>
#include <string>
#include <vector>

#include "SDL.h"

/**
 * This class stores all the data for a single 'side' (in game nomenclature).
 * E.g., there is only one leader unit per team.
 */
class team : public viewpoint, public savegame::savegame_config
{
	class shroud_map {
	public:
		shroud_map() : enabled_(false), data_() {}

		void place(int x, int y);
		bool clear(int x, int y);
		void reset();

		bool value(int x, int y) const;
		bool shared_value(const std::vector<const shroud_map*>& maps, int x, int y) const;

		bool copy_from(const std::vector<const shroud_map*>& maps);

		std::string write() const;
		void read(const std::string& shroud_data);
		void merge(const std::string& shroud_data);

		bool enabled() const { return enabled_; }
		void set_enabled(bool enabled) { enabled_ = enabled; }
	private:
		bool enabled_;
		std::vector<std::vector<bool> > data_;
	};
public:

	struct team_info
	{
		team_info(const config& cfg);
		void write(config& cfg) const;
		std::string name;
		int gold;
		int start_gold;
		bool gold_add;
		int income;
		int income_per_village;
		mutable int average_price;
		std::set<std::string> can_recruit;
		std::string team_name;
		t_string user_team_name;
		std::string save_id;
		// 'id' of the current player (not necessarily unique)
		std::string current_player;
		std::string countdown_time;
		int action_bonus_count;

		std::string flag;
		std::string flag_icon;

		std::string description;

		t_string objectives; /** < Team's objectives for the current level. */

		/** Set to true when the objectives for this time changes.
		 * Reset to false when the objectives for this team have been
		 * displayed to the user. */
		bool objectives_changed;

		enum CONTROLLER { HUMAN, HUMAN_AI, AI, NETWORK, NETWORK_AI, EMPTY };
		CONTROLLER controller;

		bool share_maps, share_view;
		bool disallow_observers;
		bool allow_player;
		bool no_leader;
		bool hidden;

		std::string music;

		std::string colour;

		int side;
		bool persistent;
	};

	static std::map<int, color_range> team_color_range_;
	static const int default_team_gold;
	team(const config& cfg, const gamemap& map, int gold=default_team_gold);

	~team() {};

	void write(config& cfg) const;

	bool get_village(const map_location&);
	void lose_village(const map_location&);
	void clear_villages() { villages_.clear(); }
	const std::set<map_location>& villages() const { return villages_; }
	bool owns_village(const map_location& loc) const
		{ return villages_.count(loc) > 0; }

	int side() const { return info_.side; }
	int gold() const { return gold_; }
	int start_gold() const { return info_.start_gold; }
	bool gold_add() const { return info_.gold_add; }
	int base_income() const { return info_.income + game_config::base_income; }
	int village_gold() const { return info_.income_per_village; }
	void set_village_gold(int income) { info_.income_per_village = income; }
	int total_income() const { return base_income() + villages_.size() * info_.income_per_village; }
	void new_turn() { gold_ += total_income(); }
	void get_shared_maps();
	void set_gold(int amount) { gold_ = amount; }
	void spend_gold(const int amount) { gold_ -= amount; }
	void set_gold_add(bool b) {info_.gold_add = b; }
	void set_base_income(int amount) { info_.income = amount - game_config::base_income; }
	int countdown_time() const {  return countdown_time_; }
	void set_countdown_time(const int amount)
		{ countdown_time_ = amount; }
	int action_bonus_count() const { return action_bonus_count_; }
	void set_action_bonus_count(const int count) { action_bonus_count_ = count; }
	std::vector<unit>& recall_list() {return recall_list_;}
	const std::vector<unit>& recall_list() const {return recall_list_;}
	void set_current_player(const std::string& player)
		{ info_.current_player = player; }

	int average_recruit_price() const;

	const std::set<std::string>& recruits() const
		{ return info_.can_recruit; }
	void add_recruit(const std::string &);
	void remove_recruit(const std::string& recruits);
	void set_recruits(const std::set<std::string>& recruits);
	const std::string& name() const
		{ return info_.name; }
	const std::string& save_id() const { return info_.save_id; }
	const std::string& current_player() const { return info_.current_player; }

	void set_objectives(const t_string& new_objectives, bool silently=false);
	void set_objectives_changed(bool c = true) { info_.objectives_changed = c; }
	void reset_objectives_changed() { info_.objectives_changed = false; }

	const t_string& objectives() const { return info_.objectives; }
	bool objectives_changed() const { return info_.objectives_changed; }

	bool is_enemy(int n) const {
		const size_t index = size_t(n-1);
		if(index < enemies_.size()) {
			return enemies_[index];
		} else {
			return calculate_enemies(index);
		}
	}

	bool has_seen(unsigned int index) const {
		if(!uses_shroud() && !uses_fog()) return true;
		if(index < seen_.size()) {
			return seen_[index];
		} else {
			return false;
		}
	}
	void see(unsigned int index) {
		if(index >= seen_.size()) {
			seen_.resize(index+1);
		}
		seen_[index] = true;
	}

	team_info::CONTROLLER controller() const { return info_.controller; }
	const std::string& colour() const { return info_.colour; }
	bool is_human() const { return info_.controller == team_info::HUMAN; }
	bool is_human_ai() const { return info_.controller == team_info::HUMAN_AI; }
	bool is_network_human() const { return info_.controller == team_info::NETWORK; }
	bool is_network_ai() const { return info_.controller == team_info::NETWORK_AI; }
	bool is_ai() const { return info_.controller == team_info::AI || is_human_ai(); }
	bool is_empty() const { return info_.controller == team_info::EMPTY; }

	bool is_local() const { return is_human() || is_ai(); }
	bool is_network() const { return is_network_human() || is_network_ai(); }

	void make_human() { info_.controller = team_info::HUMAN; }
	void make_human_ai() { info_.controller = team_info::HUMAN_AI; }
	void make_network() { info_.controller = team_info::NETWORK; }
	void make_network_ai() { info_.controller = team_info::NETWORK_AI; }
	void make_ai() { info_.controller = team_info::AI; }
	// Should make the above make_*() functions obsolete, as it accepts controller
	// by lexical or numerical id
	void change_controller(team_info::CONTROLLER controller) { info_.controller = controller; }
	void change_controller(const std::string& controller);

	const std::string& team_name() const { return info_.team_name; }
	const t_string &user_team_name() const { return info_.user_team_name; }
	void change_team(const std::string &name, const t_string &user_name);

	const std::string& flag() const { return info_.flag; }
	const std::string& flag_icon() const { return info_.flag_icon; }

	//Returns true if the hex is shrouded/fogged for this side, or
	//any other ally with shared vision.
	bool shrouded(const map_location& loc) const;
	bool fogged(const map_location& loc) const;

	bool uses_shroud() const { return shroud_.enabled(); }
	bool uses_fog() const { return fog_.enabled(); }
	bool fog_or_shroud() const { return uses_shroud() || uses_fog(); }
	bool clear_shroud(const map_location& loc) { return shroud_.clear(loc.x+1,loc.y+1); }
	void place_shroud(const map_location& loc) { shroud_.place(loc.x+1,loc.y+1); }
	bool clear_fog(const map_location& loc) { return fog_.clear(loc.x+1,loc.y+1); }
	void refog() { fog_.reset(); }
	void set_shroud(bool shroud) { shroud_.set_enabled(shroud); }
	void set_fog(bool fog) { fog_.set_enabled(fog); }

	/** Merge a WML shroud map with the shroud data of this player. */
	void merge_shroud_map_data(const std::string& shroud_data);

	bool knows_about_team(size_t index, bool is_multiplayer) const;
	bool copy_ally_shroud();

	bool auto_shroud_updates() const { return auto_shroud_updates_; }
	void set_auto_shroud_updates(bool value) { auto_shroud_updates_ = value; }
	bool get_disallow_observers() const {return info_.disallow_observers; };
	std::string map_colour_to() const { return info_.colour; };
	bool no_leader() const { return info_.no_leader; }
	void have_leader(bool value=true) { info_.no_leader = !value; }
	bool hidden() const { return info_.hidden; }
	void set_hidden(bool value) { info_.hidden=value; }
	bool persistent() const {return info_.persistent;}

	static int nteams();

	//function which, when given a 1-based side will return the colour used by that side.
	static const color_range get_side_color_range(int side);
	static Uint32 get_side_rgb(int side) { return(get_side_color_range(side).mid()); }
	static Uint32 get_side_rgb_max(int side) { return(get_side_color_range(side).max()); }
	static Uint32 get_side_rgb_min(int side) { return(get_side_color_range(side).min()); }
	static const SDL_Color get_minimap_colour(int side);
	static std::string get_side_colour_index(int side);
	static std::string get_side_highlight(int side);

	void log_recruitable();

	/**set the share maps attribute */
	void set_share_maps( bool share_maps );
	/**set the share view attribute */
	void set_share_view( bool share_view );

	/** clear the shroud, fog, and enemies cache for all teams*/
	static void clear_caches();

	config to_config() const;

private:
	//Make these public if you need them, but look at knows_about_team(...) first.
	bool share_maps() const { return info_.share_maps; }
	bool share_view() const { return info_.share_view; }

	const std::vector<const shroud_map*>& ally_shroud(const std::vector<team>& teams) const;
	const std::vector<const shroud_map*>& ally_fog(const std::vector<team>& teams) const;

	int gold_;
	std::set<map_location> villages_;

	shroud_map shroud_, fog_;

	bool auto_shroud_updates_;

	team_info info_;

	int countdown_time_;
	int action_bonus_count_;

	std::vector<unit> recall_list_;

	bool calculate_enemies(size_t index) const;
	bool calculate_is_enemy(size_t index) const;
	mutable std::vector<bool> enemies_;

	mutable std::vector<bool> seen_;

	mutable std::vector<const shroud_map*> ally_shroud_, ally_fog_;
};

namespace teams_manager {
	const std::vector<team> &get_teams();
}

namespace player_teams {
	int village_owner(const map_location& loc);
}

bool is_observer();

//function which will validate a side. Trows game::game_error
//if the side is invalid
void validate_side(int side); //throw game::game_error

#endif

