#include "multiplayer_configure_engine.hpp"
#include "formula_string_utils.hpp"
#include "settings.hpp"

namespace mp {

configure_engine::configure_engine(saved_game& state) :
	state_(state),
	parameters_(state_.mp_settings()),
	sides_(state_.get_starting_pos().child_range("side")),
	cfg_(*sides_.first)
{
	set_use_map_settings(use_map_settings_default());
}

void configure_engine::set_default_values() {
	set_use_map_settings(use_map_settings_default());
	set_game_name(game_name_default());
	set_num_turns(num_turns_default());
	set_village_gold(village_gold_default());
	set_village_support(village_support_default());
	set_xp_modifier(xp_modifier_default());
	set_mp_countdown_init_time(mp_countdown_init_time_default());
	set_mp_countdown_reservoir_time(mp_countdown_reservoir_time_default());
	set_mp_countdown_action_bonus(mp_countdown_action_bonus_default());
	set_mp_countdown(mp_countdown_default());
	set_random_start_time(random_start_time_default());
	set_fog_game(fog_game_default());
	set_shroud_game(shroud_game_default());
}

bool configure_engine::force_lock_settings() const {
	const config::attribute_value& fls = state_.get_starting_pos()["force_lock_settings"];
	return (fls.empty() && state_.classification().campaign_type == game_classification::SCENARIO)
		|| fls.to_bool();
}

std::string configure_engine::game_name() const { return parameters_.name; }
int configure_engine::num_turns() const { return parameters_.num_turns; }
int configure_engine::village_gold() const { return parameters_.village_gold; }
int configure_engine::village_support() const { return parameters_.village_support; }
int configure_engine::xp_modifier() const { return parameters_.xp_modifier; }
int configure_engine::mp_countdown_init_time() const { return parameters_.mp_countdown_init_time; }
int configure_engine::mp_countdown_reservoir_time() const { return parameters_.mp_countdown_reservoir_time; }
int configure_engine::mp_countdown_turn_bonus() const { return parameters_.mp_countdown_turn_bonus; }
int configure_engine::mp_countdown_action_bonus() const { return parameters_.mp_countdown_action_bonus; }
bool configure_engine::mp_countdown() const { return parameters_.mp_countdown; }
bool configure_engine::use_map_settings() const { return parameters_.use_map_settings; }
bool configure_engine::random_start_time() const { return parameters_.random_start_time; }
bool configure_engine::fog_game() const { return parameters_.fog_game; }
bool configure_engine::shroud_game() const { return parameters_.shroud_game; }
bool configure_engine::allow_observers() const { return parameters_.allow_observers; }
bool configure_engine::shuffle_sides() const { return parameters_.shuffle_sides; }
const config& configure_engine::options() const { return parameters_.options; }

void configure_engine::set_game_name(std::string val) { parameters_.name = val; }
void configure_engine::set_num_turns(int val) { parameters_.num_turns = val; }
void configure_engine::set_village_gold(int val) { parameters_.village_gold = val; }
void configure_engine::set_village_support(int val) { parameters_.village_support = val; }
void configure_engine::set_xp_modifier(int val) { parameters_.xp_modifier = val; }
void configure_engine::set_mp_countdown_init_time(int val) { parameters_.mp_countdown_init_time = val; }
void configure_engine::set_mp_countdown_reservoir_time(int val) { parameters_.mp_countdown_reservoir_time = val; }
void configure_engine::set_mp_countdown_turn_bonus(int val) { parameters_.mp_countdown_turn_bonus = val; }
void configure_engine::set_mp_countdown_action_bonus(int val) { parameters_.mp_countdown_action_bonus = val; }
void configure_engine::set_mp_countdown(bool val) { parameters_.mp_countdown = val; }
void configure_engine::set_use_map_settings(bool val) { parameters_.use_map_settings = val; }
void configure_engine::set_random_start_time(bool val) { parameters_.random_start_time = val; }
void configure_engine::set_fog_game(bool val) { parameters_.fog_game = val; }
void configure_engine::set_shroud_game(bool val) { parameters_.shroud_game = val; }
void configure_engine::set_allow_observers(bool val) { parameters_.shroud_game = val; }
void configure_engine::set_shuffle_sides(bool val) { parameters_.shuffle_sides = val; }
void configure_engine::set_options(const config& cfg) { parameters_.options = cfg; }

std::string configure_engine::game_name_default() const {
	utils::string_map i18n_symbols;
	i18n_symbols["login"] = preferences::login();
	return vgettext("$login|’s game", i18n_symbols);
}
int configure_engine::num_turns_default() const {
	return use_map_settings() ?
		settings::get_turns(state_.get_starting_pos()["turns"]) :
		preferences::turns();
}
int configure_engine::village_gold_default() const {
	return use_map_settings() && sides_.first != sides_.second ?
		settings::get_village_gold(cfg_["village_gold"], state_.classification().campaign_type) :
		preferences::village_gold();
}
int configure_engine::village_support_default() const {
	return use_map_settings() && sides_.first != sides_.second ?
		settings::get_village_support(cfg_["village_support"]) :
		preferences::village_support();
}
int configure_engine::xp_modifier_default() const {
	return use_map_settings() ?
		settings::get_xp_modifier(state_.get_starting_pos()["experience_modifier"]) :
		preferences::xp_modifier();
}
int configure_engine::mp_countdown_init_time_default() const {
	return preferences::countdown_init_time();
}
int configure_engine::mp_countdown_reservoir_time_default() const {
	return preferences::countdown_reservoir_time();
}
int configure_engine::mp_countdown_turn_bonus_default() const {
	return preferences::countdown_turn_bonus();
}
int configure_engine::mp_countdown_action_bonus_default() const {
	return preferences::countdown_action_bonus();
}
bool configure_engine::mp_countdown_default() const {
	return preferences::countdown();
}
bool configure_engine::use_map_settings_default() const {
	return force_lock_settings() || preferences::use_map_settings();
}
bool configure_engine::random_start_time_default() const {
	return use_map_settings() ?
		state_.get_starting_pos()["random_start_time"].to_bool(true) :
		preferences::random_start_time();
}
bool configure_engine::fog_game_default() const {
	return use_map_settings() && sides_.first != sides_.second ?
		cfg_["fog"].to_bool(state_.classification().campaign_type != game_classification::SCENARIO) :
		preferences::fog();
}
bool configure_engine::shroud_game_default() const {
	return use_map_settings() && sides_.first != sides_.second ?
		cfg_["shroud"].to_bool(false) :
		preferences::shroud();
}
bool configure_engine::allow_observers_default() const {
	return preferences::allow_observers();
}
bool configure_engine::shuffle_sides_default() const {
	return preferences::shuffle_sides();
}
const config& configure_engine::options_default() const {
	return preferences::options();
}

const mp_game_settings& configure_engine::get_parameters() const {
	return parameters_;
}

} //end namespace mp
