/*
   Copyright (C) 2013 - 2017 by Andrius Silinskas <silinskas.andrius@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "config.hpp"
#include "game_initialization/depcheck.hpp"
#include "generators/map_generator.hpp"
#include "mp_game_settings.hpp"
#include "utils/make_enum.hpp"
#include "utils/irdya_datetime.hpp"

#include <numeric>
#include <string>
#include <utility>
#include <cctype>

class CVideo;
class saved_game;
class gamemap;

namespace ng {

static bool contains_ignore_case(const std::string& str1, const std::string& str2)
{
	if(str2.size() > str1.size()) {
		return false;
	}

	for(size_t i = 0; i < str1.size() - str2.size() + 1; i++) {
		bool ok = true;
		for(size_t j = 0; j < str2.size(); j++) {
			if(std::tolower(str1[i + j]) != std::tolower(str2[j])) {
				ok = false;
				break;
			}
		}

		if(ok) {
			return true;
		}
	}

	return false;
}

/** Base class for all level type classes. */
class level
{
public:
	level(const config& data);
	virtual ~level() = default;

	MAKE_ENUM(TYPE,
		(SCENARIO,      "scenario")
		(USER_MAP,      "user_map")
		(USER_SCENARIO, "user_scenario")
		(RANDOM_MAP,    "random_map")
		(CAMPAIGN,      "campaign")
		(SP_CAMPAIGN,   "sp_campaign")
	)

	virtual void set_metadata() = 0;

	virtual bool can_launch_game() const = 0;
	virtual bool player_count_filter(int player_count) const = 0;

	virtual std::string id() const
	{
		return data_["id"];
	}

	virtual std::string name() const
	{
		return data_["name"];
	}

	virtual std::string icon() const
	{
		return data_["icon"];
	}

	virtual std::string description() const
	{
		return data_["description"];
	}

	virtual bool allow_era_choice() const
	{
		return data_["allow_era_choice"].to_bool(true);
	}

	void set_data(const config& data)
	{
		data_ = data;
	}

	const config& data() const
	{
		return data_;
	}

	config& data()
	{
		return data_;
	}

protected:
	config data_;

private:
	level(const level&) = delete;
	void operator=(const level&) = delete;
};

class scenario : public level
{
public:
	scenario(const config& data);

	bool can_launch_game() const;

	void set_metadata();

	int num_players() const
	{
		return num_players_;
	}

	std::string map_size() const;

	bool player_count_filter(int player_count) const
	{
		return num_players_ == player_count;
	}

protected:
	void set_sides();

	std::unique_ptr<gamemap> map_;

	std::string map_hash_;

private:
	scenario(const scenario&);
	void operator=(const scenario&);

	int num_players_;
};

class user_map : public scenario
{
public:
	user_map(const config& data, const std::string& name, gamemap* map);

	void set_metadata();

	/** For user maps, the id and name are the same. */
	std::string id() const
	{
		return name_;
	}

	std::string name() const
	{
		return name_;
	}

	std::string description() const;

private:
	user_map(const user_map&) = delete;
	void operator=(const user_map&) = delete;

	std::string name_;
};

class random_map : public scenario
{
public:
	random_map(const config& data);

	const config& generator_data() const
	{
		return generator_data_;
	}

	std::string generator_name() const
		{
		return generator_name_;
	}

	map_generator* create_map_generator() const;

	bool generate_whole_scenario() const
	{
		return generate_whole_scenario_;
	}

private:
	random_map(const random_map&) = delete;
	void operator=(const random_map&) = delete;

	config generator_data_;

	bool generate_whole_scenario_;
	std::string generator_name_;
};

class campaign : public level
{
public:
	campaign(const config& data);

	bool can_launch_game() const;

	void set_metadata();

	void mark_if_completed();

	std::string id() const
	{
		return id_;
	}

	bool allow_era_choice() const
	{
		return allow_era_choice_;
	}

	int min_players() const
	{
		return min_players_;
	}

	int max_players() const
	{
		return max_players_;
	}

	bool player_count_filter(int player_count) const
	{
		return min_players_ <= player_count && max_players_ >= player_count;
	}

	std::pair<irdya_date, irdya_date> dates() const
	{
		return dates_;
	}

private:
	campaign(const campaign&) = delete;
	void operator=(const campaign&) = delete;

	std::string id_;
	bool allow_era_choice_;
	std::string image_label_;
	int min_players_;
	int max_players_;
	std::pair<irdya_date, irdya_date> dates_;
};

class create_engine
{
public:
	create_engine(CVideo& v, saved_game& state);

	enum MP_EXTRA { ERA, MOD };

	struct extras_metadata
	{
		std::string id;
		std::string name;
		std::string description;
		const config* cfg;
	};

	typedef std::shared_ptr<extras_metadata> extras_metadata_ptr;

	typedef std::shared_ptr<level> level_ptr;
	typedef std::shared_ptr<scenario> scenario_ptr;
	typedef std::shared_ptr<user_map> user_map_ptr;
	typedef std::shared_ptr<random_map> random_map_ptr;
	typedef std::shared_ptr<campaign> campaign_ptr;

	void init_generated_level_data();

	void prepare_for_new_level();
	void prepare_for_era_and_mods();
	void prepare_for_scenario();
	void prepare_for_campaign(const std::string& difficulty = "");
	void prepare_for_saved_game();
	// random maps, user maps
	void prepare_for_other();

	/**
	 * select_campaign_difficulty
	 *
	 * Launches difficulty selection gui and returns selected difficulty name.
	 *
	 * The gui can be bypassed by supplying a number from 1 to the number of
	 * difficulties available, corresponding to a choice of difficulty.
	 * This is useful for specifying difficulty via command line.
	 *
	 * @param	set_value Preselected difficulty number. The default -1 launches the gui.
	 * @return	Selected difficulty. Returns "FAIL" if set_value is invalid,
	 *	        and "CANCEL" if the gui is canceled.
	 */
	std::string select_campaign_difficulty(int set_value = -1);
	std::string get_selected_campaign_difficulty() const
	{
		return selected_campaign_difficulty_;
	}

	void apply_level_filter(const std::string& name);
	void apply_level_filter(int players);
	void reset_level_filters();

	const std::string& level_name_filter() const
	{
		return level_name_filter_;
	}

	int player_num_filter() const
	{
		return player_count_filter_;
	}

	std::vector<level_ptr> get_levels_by_type_unfiltered(level::TYPE type) const;
	std::vector<level_ptr> get_levels_by_type(level::TYPE type) const;

	std::vector<size_t> get_filtered_level_indices(level::TYPE type) const;

	level& current_level() const;
	const extras_metadata& current_era() const;

	void set_current_level_type(const level::TYPE type)
	{
		current_level_type_ = type;
	}

	level::TYPE current_level_type() const
	{
		return current_level_type_;
	}

	void set_current_level(const size_t index);

	void set_current_era_index(const size_t index, bool force = false);

	size_t current_era_index() const
	{
		return current_era_index_;
	}

	const config& curent_era_cfg() const;

	const std::vector<extras_metadata_ptr>& get_const_extras_by_type(const MP_EXTRA extra_type) const;
	std::vector<extras_metadata_ptr>& get_extras_by_type(const MP_EXTRA extra_type);

	bool toggle_mod(int index, bool force = false);

	bool generator_assigned() const;
	bool generator_has_settings() const;
	void generator_user_config();

	std::pair<level::TYPE, int> find_level_by_id(const std::string& id) const;
	int find_extra_by_id(const MP_EXTRA extra_type, const std::string& id) const;

	const depcheck::manager& dependency_manager() const
	{
		return *dependency_manager_;
	}

	void init_active_mods();

	std::vector<std::string>& active_mods();
	std::vector<extras_metadata_ptr> active_mods_data();

	const mp_game_settings& get_parameters();

	saved_game& get_state();

	/** Returns true if the current level has one or more [side] tags. */
	bool current_level_has_side_data();

private:
	create_engine(const create_engine&) = delete;
	void operator=(const create_engine&) = delete;

	void init_all_levels();
	void init_extras(const MP_EXTRA extra_type);
	void apply_level_filters();

	size_t map_level_index(size_t index) const;

	level::TYPE current_level_type_;
	size_t current_level_index_;

	size_t current_era_index_;

	std::string level_name_filter_;
	int player_count_filter_;

	struct type_list
	{
		explicit type_list() : games(), games_filtered() {}

		std::vector<level_ptr> games;
		std::vector<size_t> games_filtered;

		void apply_filter(const int player_count, const std::string& name_filter)
		{
			games_filtered.clear();

			for(size_t i = 0; i < games.size(); i++) {
				const bool num_players_match = player_count == 1 || games[i]->player_count_filter(player_count);

				if(contains_ignore_case(games[i]->name(), name_filter) && num_players_match) {
					games_filtered.push_back(i);
				}
			}
		}

		void reset_filter()
		{
			games_filtered.resize(games.size());
			std::iota(games_filtered.begin(), games_filtered.end(), 0);
		}
	};

	std::map<level::TYPE, type_list> type_map_;

	std::vector<std::string> user_map_names_;
	std::vector<std::string> user_scenario_names_;

	std::vector<extras_metadata_ptr> eras_;
	std::vector<extras_metadata_ptr> mods_;

	saved_game& state_;

	CVideo& video_;

	// Never nullptr
	std::unique_ptr<depcheck::manager> dependency_manager_;

	std::unique_ptr<map_generator> generator_;

	std::string selected_campaign_difficulty_;
};

} // end namespace ng
