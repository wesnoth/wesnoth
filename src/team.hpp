/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
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
#include "utils/make_enum.hpp"
#include "map/location.hpp"
#include "recall_list_manager.hpp"
#include "units/ptr.hpp"
#include "config.hpp"

#include <set>

#include <cstdint>
#include <boost/dynamic_bitset.hpp>

class game_data;
class gamemap;
struct color_t;


namespace wb {
	class side_actions;
}

/**
 * This class stores all the data for a single 'side' (in game nomenclature).
 * E.g., there is only one leader unit per team.
 */
class team
{
public:

	MAKE_ENUM(CONTROLLER,
		(HUMAN,       "human")
		(AI,          "ai")
		(EMPTY,	      "null")
	)

	MAKE_ENUM(PROXY_CONTROLLER,
		(PROXY_HUMAN, "human")
		(PROXY_AI,    "ai")
		(PROXY_IDLE,  "idle")
	)

	MAKE_ENUM(DEFEAT_CONDITION,
		(NO_LEADER, "no_leader_left")
		(NO_UNITS, "no_units_left")
		(NEVER, "never")
		(ALWAYS, "always")
	)

	MAKE_ENUM(SHARE_VISION,
		(ALL, "all")
		(SHROUD, "shroud")
		(NONE, "none")
	)

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
		int gold;
		int start_gold;
		int income;
		int income_per_village;
		int support_per_village;
		mutable int minimum_recruit_price;
		int recall_cost;
		std::set<std::string> can_recruit;
		std::string team_name;
		t_string user_team_name;
		t_string side_name;
		std::string faction;
		t_string faction_name;
		std::string save_id;
		// 'id' of the current player (not necessarily unique)
		std::string current_player;
		std::string countdown_time;
		int action_bonus_count;

		std::string flag;
		std::string flag_icon;

		std::string id;

		bool scroll_to_leader;

		t_string objectives; /** < Team's objectives for the current level. */

		/** Set to true when the objectives for this time changes.
		 * Reset to false when the objectives for this team have been
		 * displayed to the user. */
		mutable bool objectives_changed;

		CONTROLLER controller;
		bool is_local;
		DEFEAT_CONDITION defeat_condition;

		PROXY_CONTROLLER proxy_controller;	// when controller == HUMAN, the proxy controller determines what input method is actually used.
							// proxy controller is an interface property, not gamestate. it is not synced, not known to server.
							// also not saved in save game file
		SHARE_VISION share_vision;
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

		int carryover_percentage;
		bool carryover_add;
		// TODO: maybe make this integer percentage? I like the float version more but this might casue OOS error because of floating point rounding differences on different hardware.
		double carryover_bonus;
		int carryover_gold;
		config variables;
		void handle_legacy_share_vision(const config& cfg);
	};

	static const int default_team_gold_;

public:
	team();
	virtual ~team();

	/// Stores the attributes recognized by [side]. These should be stripped
	/// from a side's config before using it to create the side's leader.
	static const std::set<std::string> attributes;

	void build(const config &cfg, const gamemap &map, int gold = default_team_gold_);

	void write(config& cfg) const;

	bool get_village(const map_location&, const int owner_side, game_data * fire_event); //!< Acquires a village from owner_side. Pointer fire_event should be the game_data for the game if it is desired to fire an event -- a "capture" event with owner_side variable scoped in will be fired. For no event, pass it nullptr. Default is the resources::gamedata pointer
	void lose_village(const map_location&);
	void clear_villages() { villages_.clear(); }
	const std::set<map_location>& villages() const { return villages_; }
	bool owns_village(const map_location& loc) const
		{ return villages_.count(loc) > 0; }

	int side() const { return info_.side; }
	int gold() const { return gold_; }
	int start_gold() const { return info_.start_gold; }
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
	void set_start_gold(const int amount) { info_.start_gold = amount; }
	void spend_gold(const int amount) { gold_ -= amount; }
	void set_base_income(int amount) { info_.income = amount - game_config::base_income; }
	int countdown_time() const {  return countdown_time_; }
	void set_countdown_time (const int amount) const
		{ countdown_time_ = amount; }
	int action_bonus_count() const { return action_bonus_count_; }
	void set_action_bonus_count(const int count) { action_bonus_count_ = count; }
	recall_list_manager& recall_list() {return recall_list_;}
	const recall_list_manager & recall_list() const {return recall_list_;}
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

	const std::string& save_id() const { return info_.save_id; }
	void set_save_id(const std::string& save_id) { info_.save_id = save_id; }
	const std::string& current_player() const { return info_.current_player; }

	void set_objectives(const t_string& new_objectives, bool silently=false);
	void set_objectives_changed(bool c = true) const { info_.objectives_changed = c; }
	void reset_objectives_changed() const { info_.objectives_changed = false; }

	const t_string& objectives() const { return info_.objectives; }
	bool objectives_changed() const { return info_.objectives_changed; }

	bool is_enemy(int n) const {
		const size_t index = size_t(n-1);
		if(index >= enemies_.size()) {
			calculate_enemies(index);
		}
		if(index < enemies_.size()) {
			return enemies_[index];
		} else {
			return false;
		}
	}

	CONTROLLER controller() const { return info_.controller; }
	const std::string& color() const { return info_.color; }
	void set_color(const std::string& color) { info_.color = color; }
	bool is_empty() const { return info_.controller == CONTROLLER::EMPTY; }

	bool is_local() const { return !is_empty() && info_.is_local; }
	bool is_network() const { return !is_empty() && !info_.is_local; }

	bool is_human() const { return info_.controller == CONTROLLER::HUMAN; }
	bool is_ai() const { return info_.controller == CONTROLLER::AI; }

	bool is_local_human() const {  return is_human() && is_local(); }
	bool is_local_ai() const { return is_ai() && is_local(); }
	bool is_network_human() const { return is_human() && is_network(); }
	bool is_network_ai() const { return is_ai() && is_network(); }

	void set_local(bool local) { info_.is_local = local; }
	void make_human() { info_.controller = CONTROLLER::HUMAN; }
	void make_ai() { info_.controller = CONTROLLER::AI; }
	void change_controller(const std::string& new_controller) {
		info_.controller = CONTROLLER::AI;
		info_.controller.parse(new_controller);
	}
	void change_controller(CONTROLLER controller) { info_.controller = controller; }
	void change_controller_by_wml(const std::string& new_controller);

	PROXY_CONTROLLER proxy_controller() const { return info_.proxy_controller; }
	bool is_proxy_human() const { return info_.proxy_controller == PROXY_CONTROLLER::PROXY_HUMAN; }
	bool is_droid() const { return info_.proxy_controller == PROXY_CONTROLLER::PROXY_AI; }
	bool is_idle() const { return info_.proxy_controller == PROXY_CONTROLLER::PROXY_IDLE; }

	void make_droid() { info_.proxy_controller = PROXY_CONTROLLER::PROXY_AI; }
	void make_idle() { info_.proxy_controller = PROXY_CONTROLLER::PROXY_IDLE; }
	void make_proxy_human() { info_.proxy_controller = PROXY_CONTROLLER::PROXY_HUMAN; }
	void clear_proxy() { make_proxy_human(); }

	void change_proxy(PROXY_CONTROLLER proxy) { info_.proxy_controller = proxy; }

	void toggle_droid() { info_.proxy_controller = (info_.proxy_controller == PROXY_CONTROLLER::PROXY_AI  ) ? PROXY_CONTROLLER::PROXY_HUMAN : PROXY_CONTROLLER::PROXY_AI;   }
	void toggle_idle()  { info_.proxy_controller = (info_.proxy_controller == PROXY_CONTROLLER::PROXY_IDLE) ? PROXY_CONTROLLER::PROXY_HUMAN : PROXY_CONTROLLER::PROXY_IDLE; }

	const std::string& team_name() const { return info_.team_name; }
	const t_string &user_team_name() const { return info_.user_team_name; }
	void change_team(const std::string &name, const t_string &user_name);

	const std::string& flag() const { return info_.flag; }
	const std::string& flag_icon() const { return info_.flag_icon; }

	void set_flag(const std::string& flag) { info_.flag = flag; }
	void set_flag_icon(const std::string& flag_icon) { info_.flag_icon = flag_icon; }

	const std::string& side_name() const { return info_.side_name.empty() ? info_.current_player : info_.side_name.str(); }
	const std::string& faction() const { return info_.faction; }
	const t_string& faction_name() const { return info_.faction_name; }
	//Returns true if the hex is shrouded/fogged for this side, or
	//any other ally with shared vision.
	bool shrouded(const map_location& loc) const;
	bool fogged(const map_location& loc) const;

	bool uses_shroud() const { return shroud_.enabled(); }
	bool uses_fog() const { return fog_.enabled(); }
	bool fog_or_shroud() const { return uses_shroud() || uses_fog(); }
	bool clear_shroud(const map_location& loc) { return shroud_.clear(loc.wml_x(),loc.wml_y()); }
	void place_shroud(const map_location& loc) { shroud_.place(loc.wml_x(),loc.wml_y()); }
	bool clear_fog(const map_location& loc) { return fog_.clear(loc.wml_x(),loc.wml_y()); }
	void reshroud() { shroud_.reset(); }
	void refog() { fog_.reset(); }
	void set_shroud(bool shroud) { shroud_.set_enabled(shroud); }
	void set_fog(bool fog) { fog_.set_enabled(fog); }

	/** Merge a WML shroud map with the shroud data of this player. */
	void merge_shroud_map_data(const std::string& shroud_data) { shroud_.merge(shroud_data); }

	bool knows_about_team(size_t index) const;
	/// Records hexes that were cleared of fog via WML.
	void add_fog_override(const std::set<map_location> &hexes) { fog_clearer_.insert(hexes.begin(), hexes.end()); }
	/// Removes the record of hexes that were cleared of fog via WML.
	void remove_fog_override(const std::set<map_location> &hexes);

	bool auto_shroud_updates() const { return auto_shroud_updates_; }
	void set_auto_shroud_updates(bool value) { auto_shroud_updates_ = value; }
	bool get_disallow_observers() const {return info_.disallow_observers; }
	bool no_leader() const { return info_.no_leader; }
	DEFEAT_CONDITION defeat_condition() const { return info_.defeat_condition; }
	void set_defeat_condition(DEFEAT_CONDITION value) { info_.defeat_condition = value; }
	///sets the defeat condition if @param value is a valid defeat condition, otherwise nothing happes.
	void set_defeat_condition_string(const std::string& value) { info_.defeat_condition.parse(value); }
	void have_leader(bool value=true) { info_.no_leader = !value; }
	bool hidden() const { return info_.hidden; }
	void set_hidden(bool value) { info_.hidden=value; }
	bool persistent() const { return info_.persistent; }
	void set_persistent(bool value) { info_.persistent = value; }
	void set_lost(bool value=true) { info_.lost = value; }
	bool lost() const { return info_.lost; }

	void set_carryover_percentage(int value) { info_.carryover_percentage = value; }
	int carryover_percentage() const { return info_.carryover_percentage; }
	void set_carryover_add(bool value) { info_.carryover_add = value; }
	bool carryover_add() const { return info_.carryover_add; }
	void set_carryover_bonus(double value) { info_.carryover_bonus = value; }
	double carryover_bonus() const { return info_.carryover_bonus; }
	void set_carryover_gold(int value) { info_.carryover_gold = value; }
	int carryover_gold() const { return info_.carryover_gold; }
	config& variables() { return info_.variables; }
	const config& variables() const { return info_.variables; }

	bool no_turn_confirmation() const { return info_.no_turn_confirmation; }
	void set_no_turn_confirmation(bool value) { info_.no_turn_confirmation = value; }

	//function which, when given a 1-based side will return the color used by that side.
	static const color_range get_side_color_range(int side);
	static color_t get_side_rgb(int side) { return(get_side_color_range(side).mid()); }
	static color_t get_side_rgb_max(int side) { return(get_side_color_range(side).max()); }
	static color_t get_side_rgb_min(int side) { return(get_side_color_range(side).min()); }
	static color_t get_side_color(int side);
	static color_t get_minimap_color(int side);
	static std::string get_side_color_index(int side);
	static std::string get_side_highlight_pango(int side);

	void log_recruitable() const;

	/** clear the shroud, fog, and enemies cache for all teams*/
	static void clear_caches();

	/** get the whiteboard planned actions for this team */
	std::shared_ptr<wb::side_actions> get_side_actions() const { return planned_actions_; }

	config to_config() const;

	bool share_maps() const { return info_.share_vision != SHARE_VISION::NONE ; }
	bool share_view() const { return info_.share_vision == SHARE_VISION::ALL; }
	SHARE_VISION share_vision() const { return info_.share_vision; }

	void set_share_vision(const std::string& vision_status) {
		info_.share_vision = SHARE_VISION::ALL;
		info_.share_vision.parse(vision_status);
	}

	void set_share_vision(SHARE_VISION vision_status) { info_.share_vision = vision_status; }

	void handle_legacy_share_vision(const config& cfg)
	{
		info_.handle_legacy_share_vision(cfg);
	}
	std::string allied_human_teams() const;
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

	mutable int countdown_time_;
	int action_bonus_count_;

	recall_list_manager recall_list_;
	std::string last_recruit_;

private:
	void calculate_enemies(size_t index) const;
	bool calculate_is_enemy(size_t index) const;
	mutable boost::dynamic_bitset<> enemies_;

	mutable std::vector<const shroud_map*> ally_shroud_, ally_fog_;

	/**
	 * Whiteboard planned actions for this team.
	 */
	std::shared_ptr<wb::side_actions> planned_actions_;
};

//function which will validate a side. Throws game::game_error
//if the side is invalid
void validate_side(int side); //throw game::game_error

#endif

