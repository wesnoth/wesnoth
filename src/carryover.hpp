
#ifndef CARRYOVER_H_INCLUDED
#define CARRYOVER_H_INCLUDED

class team;
class config;
class game_data;
#include <vector>
#include <string>
#include <set>

#include "game_end_exceptions.hpp"
#include "simple_rng.hpp"
#include "game_events/wmi_container.hpp"

class carryover{
public:
	carryover()
		: add_ ()
		, color_()
		, current_player_()
		, gold_()
		, name_()
		, previous_recruits_()
		, recall_list_()
		, save_id_()
	{}
	// Turns config from a loaded savegame into carryover_info
	explicit carryover(const config& side);
	carryover(const team& t, const int gold, const bool add);
	~carryover(){}

	const std::string& get_save_id() const{ return save_id_; }
	void transfer_all_gold_to(config& side_cfg);
	void transfer_all_recruits_to(config& side_cfg);
	void transfer_all_recalls_to(config& side_cfg);
	void update_carryover(const team& t, const int gold, const bool add);
	void initialize_team(config& side_cfg);
	const std::string to_string();
	void to_config(config& cfg);
	/// @param gold sets the gold of the team, always overwrites current gold.
	/// @param add whether the gold should be added to the next scenario gold.
	void set_gold(int gold, bool add);
private:
	bool add_;
	std::string color_;
	std::string current_player_;
	int gold_;
	std::string name_;
	std::set<std::string> previous_recruits_;
	// NOTE: we store configs instead of units because units often assume or
	//       assert that various resources:: are available, which is not the
	//       case between scenarios.
	std::vector<config> recall_list_;
	std::string save_id_;

	std::string get_recruits(bool erase=false);
};

class carryover_info{
public:
	carryover_info()
		: carryover_sides_()
		, end_level_()
		, variables_()
		, rng_()
		, wml_menu_items_()
		, next_scenario_()
		, next_underlying_unit_id_()
	{}
	/// Turns config from a loaded savegame into carryover_info
	/// @param from_snapshot true if cfg is a [snapshot], false if cfg is [carryover_sides(_start)]
	explicit carryover_info(const config& cfg, bool from_snapshot = false);

	carryover* get_side(std::string save_id);
	std::vector<carryover>& get_all_sides();
	void add_side(const config& cfg);
	void add_side(const team& t, const int gold, const bool add);
	void remove_side(const std::string& id);
	void set_end_level(const end_level_data& end_level) { end_level_ = end_level; }

	void transfer_from(const team& t, int carryover_gold);
	void transfer_all_to(config& side_cfg);

	void transfer_from(game_data& gamedata);
	void transfer_to(config& level);

	void set_variables(const config& vars) { variables_ = vars; }
	const config& get_variables() const { return variables_; }

	game_events::wmi_container& get_wml_menu_items() { return wml_menu_items_; }

	const rand_rng::simple_rng& rng() const { return rng_; }
	rand_rng::simple_rng& rng() { return rng_; }

	const end_level_data& get_end_level() const;

	const std::string& next_scenario() const { return next_scenario_; }

	const config to_config();

	void merge_old_carryover(const carryover_info& old_carryover);
private:
	std::vector<carryover> carryover_sides_;
	end_level_data end_level_;
	config variables_;
	rand_rng::simple_rng rng_;
	game_events::wmi_container wml_menu_items_;
	std::string next_scenario_;    /**< the scenario coming next (for campaigns) */
	int next_underlying_unit_id_;
};


#endif

