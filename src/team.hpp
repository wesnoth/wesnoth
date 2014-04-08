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
#ifndef TEAM_H_INCLUDED
#define TEAM_H_INCLUDED

#include "color_range.hpp"
#include "game_config.hpp"
#include "savegame_config.hpp"
#include "unit.hpp"

class gamemap;

namespace wb {
	class side_actions;
}

/**
 * This class stores all the data for a single 'side' (in game nomenclature).
 * E.g., there is only one leader unit per team.
 */
class team : public savegame::savegame_config
{
public:
	enum CONTROLLER { HUMAN, AI, NETWORK, NETWORK_AI, IDLE, EMPTY };

private:
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

	struct team_info
	{
		team_info();
		void read(const config &cfg);
		void write(config& cfg) const;
		std::string name;
		int gold;
		int start_gold;
		bool gold_add;
		int income;
		int income_per_village;
		int support_per_village;
		mutable int minimum_recruit_price;
		int recall_cost;
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

		bool scroll_to_leader;

		t_string objectives; /** < Team's objectives for the current level. */

		/** Set to true when the objectives for this time changes.
		 * Reset to false when the objectives for this team have been
		 * displayed to the user. */
		bool objectives_changed;

		CONTROLLER controller;
		char const *controller_string() const;

		bool share_maps, share_view;
		bool disallow_observers;
		bool allow_player;
		bool chose_random;
		bool no_leader;
		bool hidden;
		bool no_turn_confirmation;  // Can suppress confirmations when ending a turn.

		std::string color;

		int side;
		bool persistent;
		bool lost;
	};

	static const int default_team_gold_;

public:
	team();
	virtual ~team();

	/// Stores the attributes recognized by [side]. These should be stripped
	/// from a side's config before using it to create the side's leader.
	static const char * const attributes[];

	void build(const config &cfg, const gamemap &map, int gold = default_team_gold_);

	void write(config& cfg) const;

	bool get_village(const map_location&, const int owner_side, const bool fire_event = true);
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
	int recall_cost() const { return info_.recall_cost; }
	void set_village_gold(int income) { info_.income_per_village = income; }
	void set_recall_cost(int cost) { info_.recall_cost = cost; }
	int total_income() const { return base_income() + villages_.size() * info_.income_per_village; }
	/** @return The number of unit levels each village can support,
	    i.e. how much upkeep each village can bear. */
	int village_support() const { return info_.support_per_village; }
	/** @param support The number of unit levels each village can support */
	void set_village_support(int support) { info_.support_per_village = support; }
	/** Calculate total support capacity, based on support_per_village. */
	int support() const { return villages_.size()*village_support(); }
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

	bool get_scroll_to_leader() const {return info_.scroll_to_leader;}
	void set_scroll_to_leader(bool value) { info_.scroll_to_leader = value; }

	const std::set<std::string>& recruits() const
		{ return info_.can_recruit; }
	void add_recruit(const std::string &);
	void set_recruits(const std::set<std::string>& recruits);
	int minimum_recruit_price() const;
	const std::string& last_recruit() const { return last_recruit_; }
	void last_recruit(const std::string & u_type) { last_recruit_ = u_type; }
	const std::string& name() const
		{ return info_.name; }

	void set_name(const std::string& name) { info_.name = name; }
	const std::string& save_id() const { return info_.save_id; }
	void set_save_id(const std::string& save_id) { info_.save_id = save_id; }
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

	CONTROLLER controller() const { return info_.controller; }
	char const *controller_string() const { return info_.controller_string(); }
	const std::string& color() const { return info_.color; }
	void set_color(const std::string& color) { info_.color = color; }
	bool is_human() const { return info_.controller == HUMAN; }
	bool is_network_human() const { return info_.controller == NETWORK; }
	bool is_network_ai() const { return info_.controller == NETWORK_AI; }
	bool is_ai() const { return info_.controller == AI; }
	bool is_idle() const { return info_.controller == IDLE; }
	bool is_empty() const { return info_.controller == EMPTY; }

	bool is_local() const { return is_human() || is_ai() || is_idle(); }
	bool is_network() const { return is_network_human() || is_network_ai(); }

	void make_human() { info_.controller = HUMAN; }
	void make_network() { info_.controller = NETWORK; }
	void make_network_ai() { info_.controller = NETWORK_AI; }
	void make_ai() { info_.controller = AI; }
	void make_idle() { info_.controller = IDLE; }
	void change_controller(const std::string& controller);
	void change_controller(CONTROLLER controller) { info_.controller = controller; }

	const std::string& team_name() const { return info_.team_name; }
	const t_string &user_team_name() const { return info_.user_team_name; }
	void change_team(const std::string &name, const t_string &user_name);

	const std::string& flag() const { return info_.flag; }
	const std::string& flag_icon() const { return info_.flag_icon; }

	void set_flag(const std::string& flag) { info_.flag = flag; }
	void set_flag_icon(const std::string& flag_icon) { info_.flag_icon = flag_icon; }

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
	void reshroud() { shroud_.reset(); }
	void refog() { fog_.reset(); }
	void set_shroud(bool shroud) { shroud_.set_enabled(shroud); }
	void set_fog(bool fog) { fog_.set_enabled(fog); }

	/** Merge a WML shroud map with the shroud data of this player. */
	void merge_shroud_map_data(const std::string& shroud_data) { shroud_.merge(shroud_data); }

	bool knows_about_team(size_t index, bool is_multiplayer) const;
	bool copy_ally_shroud();
	/// Records hexes that were cleared of fog via WML.
	void add_fog_override(const std::set<map_location> &hexes) { fog_clearer_.insert(hexes.begin(), hexes.end()); }
	/// Removes the record of hexes that were cleared of fog via WML.
	void remove_fog_override(const std::set<map_location> &hexes);

	bool auto_shroud_updates() const { return auto_shroud_updates_; }
	void set_auto_shroud_updates(bool value) { auto_shroud_updates_ = value; }
	bool get_disallow_observers() const {return info_.disallow_observers; }
	bool no_leader() const { return info_.no_leader; }
	void have_leader(bool value=true) { info_.no_leader = !value; }
	bool hidden() const { return info_.hidden; }
	void set_hidden(bool value) { info_.hidden=value; }
	bool persistent() const {return info_.persistent;}
	void set_lost(bool value=true) { info_.lost = value; }
	bool lost() const { return info_.lost; }
	bool no_turn_confirmation() const { return info_.no_turn_confirmation; }
	void set_no_turn_confirmation(bool value) { info_.no_turn_confirmation = value; }

	static int nteams();

	//function which, when given a 1-based side will return the color used by that side.
	static const color_range get_side_color_range(int side);
	static Uint32 get_side_rgb(int side) { return(get_side_color_range(side).mid()); }
	static Uint32 get_side_rgb_max(int side) { return(get_side_color_range(side).max()); }
	static Uint32 get_side_rgb_min(int side) { return(get_side_color_range(side).min()); }
	static SDL_Color get_side_color(int side);
	static SDL_Color get_minimap_color(int side);
	static std::string get_side_color_index(int side);
	static std::string get_side_highlight(int side);
	static std::string get_side_highlight_pango(int side);

	void log_recruitable();

	/**set the share maps attribute */
	void set_share_maps( bool share_maps );
	/**set the share view attribute */
	void set_share_view( bool share_view );

	/** clear the shroud, fog, and enemies cache for all teams*/
	static void clear_caches();

	/** get the whiteboard planned actions for this team */
	boost::shared_ptr<wb::side_actions> get_side_actions() { return planned_actions_; }

	config to_config() const;

	bool share_maps() const { return info_.share_maps; }
	bool share_view() const { return info_.share_view; }
private:

	const std::vector<const shroud_map*>& ally_shroud(const std::vector<team>& teams) const;
	const std::vector<const shroud_map*>& ally_fog(const std::vector<team>& teams) const;

	int gold_;
	std::set<map_location> villages_;

	shroud_map shroud_, fog_;
	/// Stores hexes that have been cleared of fog via WML.
	std::set<map_location> fog_clearer_;

	bool auto_shroud_updates_;

	team_info info_;

	int countdown_time_;
	int action_bonus_count_;

	std::vector<unit> recall_list_;
	std::string last_recruit_;

	bool calculate_enemies(size_t index) const;
	bool calculate_is_enemy(size_t index) const;
	mutable std::vector<bool> enemies_;

	mutable std::vector<const shroud_map*> ally_shroud_, ally_fog_;

	/**
	 * Whiteboard planned actions for this team.
	 */
	boost::shared_ptr<wb::side_actions> planned_actions_;
};

namespace teams_manager {
	const std::vector<team> &get_teams();
}

/**
 * Given the location of a village, will return the 0-based index
 * of the team that currently owns it, and -1 if it is unowned.
 */
int village_owner(const map_location& loc);

//FIXME: this global method really needs to be moved into play_controller,
//or somewhere else that makes sense.
bool is_observer();

//function which will validate a side. Throws game::game_error
//if the side is invalid
void validate_side(int side); //throw game::game_error

#endif

