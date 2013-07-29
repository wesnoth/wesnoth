/*
   Copyright (C) 2013 by Andrius Silinskas <silinskas.andrius@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#include "multiplayer_connect_engine.hpp"

#include "ai/configuration.hpp"
#include "formula_string_utils.hpp"
#include "game_preferences.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "multiplayer_ui.hpp"
#include "mp_game_utils.hpp"
#include "tod_manager.hpp"

#include <boost/foreach.hpp>

static lg::log_domain log_config("config");
#define LOG_CF LOG_STREAM(info, log_config)
#define WRN_CF LOG_STREAM(warn, log_config)
#define ERR_CF LOG_STREAM(err, log_config)

static lg::log_domain log_mp_connect_engine("mp/connect/engine");
#define DBG_MP LOG_STREAM(debug, log_mp_connect_engine)
#define LOG_MP LOG_STREAM(info, log_mp_connect_engine)

namespace {

const char* controller_names[] = {
	"network",
	"human",
	"ai",
	"null",
	"reserved"
};

}

namespace mp {

connect_engine::connect_engine(game_display& disp, controller mp_controller,
	const mp_game_settings& params) :
	level_(),
	state_(),
	disp_(disp),
	params_(params),
	side_engines_(),
	era_factions_(),
	users_(),
	mp_controller_(mp_controller),
	team_names_(),
	user_team_names_()
{
	level_ = initial_level_config(disp, params, state_);
	if (level_.empty()) {
		return;
	}

	BOOST_FOREACH(const config &era,
		level_.child("era").child_range("multiplayer_side")) {

		era_factions_.push_back(&era);
	}

	// Adds the current user as default user.
	users_.push_back(connected_user(preferences::login(), CNTR_LOCAL, 0));
}

connect_engine::~connect_engine()
{
}

config* connect_engine::current_config() {
	config* cfg_level = NULL;

	// It might make sense to invent a mechanism of some sort to check
	// whether a config node contains information
	// that you can load from(side information, specifically).
	config &snapshot = level_.child("snapshot");
	if (snapshot && snapshot.child("side")) {
		// Savegame.
		cfg_level = &snapshot;
	} else if (!level_.child("side")) {
		// Start-of-scenario save,
		// the info has to be taken from the starting_pos.
		cfg_level = &state_.replay_start();
	} else {
		// Fresh game, no snapshot available.
		cfg_level = &level_;
	}

	return cfg_level;
}

void connect_engine::add_side_engine(side_engine_ptr engine)
{
	side_engines_.push_back(engine);
}

void connect_engine::update_level()
{
	DBG_MP << "updating level" << std::endl;

	level_.clear_children("side");

	BOOST_FOREACH(side_engine_ptr side, side_engines_) {
		level_.add_child("side", side->new_config());
	}
}

void connect_engine::update_and_send_diff(bool update_time_of_day)
{
	config old_level = level_;
	update_level();

	if (update_time_of_day) {
		// Set random start ToD.
		tod_manager tod_mng(level_, level_["turns"]);
	}

	config diff = level_.get_diff(old_level);
	if (!diff.empty()) {
		config scenario_diff;
		scenario_diff.add_child("scenario_diff", diff);
		network::send_data(scenario_diff, 0);
	}
}

bool connect_engine::sides_available() const
{
	BOOST_FOREACH(side_engine_ptr side, side_engines_) {
		if (side->available()) {
			return true;
		}
	}

	return false;
}

bool connect_engine::can_start_game() const
{
	// First check if all sides are ready to start the game.
	BOOST_FOREACH(side_engine_ptr side, side_engines_) {
		if (!side->ready_for_start()) {
			DBG_MP << "not all sides are ready, side " <<
				side->new_config().get("side")->str() << " not ready" <<
				std::endl;

			return false;
		}
	}

	DBG_MP << "all sides are ready" << std::endl;

	/*
	 * If at least one human player is slotted with a player/ai we're allowed
	 * to start. Before used a more advanced test but it seems people are
	 * creative in what is used in multiplayer [1] so use a simpler test now.
	 * [1] http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=568029
	 */
	BOOST_FOREACH(side_engine_ptr side, side_engines_) {
		if (side->mp_controller() != CNTR_EMPTY) {
			if (side->allow_player()) {
				return true;
			}
		}
	}

	return false;
}

void connect_engine::start_game()
{
	DBG_MP << "starting a new game" << std::endl;

    // Resolves the "random faction", "random gender" and "random message"
    // Must be done before shuffle sides, or some cases will cause errors
	BOOST_FOREACH(side_engine_ptr side, side_engines_) {
		side->resolve_random();
	}

	// Shuffle sides (check preferences and if it is a re-loaded game).
	// Must be done after resolve_random() or shuffle sides, or they won't work.
	if (preferences::shuffle_sides() && !(level_.child("snapshot") &&
		level_.child("snapshot").child("side"))) {

		// Only playable sides should be shuffled.
		std::vector<int> playable_sides;
		BOOST_FOREACH(side_engine_ptr side, side_engines_) {
			if (side->allow_player()) {
				playable_sides.push_back(side->index());
			}
		}

		// Fisher-Yates shuffle.
		for (int i = playable_sides.size(); i > 1; i--)
		{
			int j_side = playable_sides[get_random() % i];
			int i_side = playable_sides[i - 1];

			int tmp_index = side_engines_[j_side]->index();
			side_engines_[j_side]->set_index(side_engines_[i_side]->index());
			side_engines_[i_side]->set_index(tmp_index);

			int tmp_team = side_engines_[j_side]->team();
			side_engines_[j_side]->set_team(side_engines_[i_side]->team());
			side_engines_[i_side]->set_team(tmp_team);

			// This is needed otherwise fog bugs will appear.
			side_engine_ptr tmp_side = side_engines_[j_side];
			side_engines_[j_side] = side_engines_[i_side];
			side_engines_[i_side] = tmp_side;
		}
	}

	// Make other clients not show the results of resolve_random().
	config lock("stop_updates");
	network::send_data(lock, 0);
	update_and_send_diff(true);

	// Build the gamestate object after updating the level.
	level_to_gamestate(level_, state_);

	network::send_data(config("start_game"), 0);
}

void connect_engine::start_game_commandline(
	const commandline_options& cmdline_opts)
{
	DBG_MP << "starting a new game in commandline mode" << std::endl;

	typedef boost::tuple<unsigned int, std::string> mp_option;

	unsigned num = 0;
	BOOST_FOREACH(side_engine_ptr side, side_engines_) {
		num++;

		// Set the faction, if commandline option is given.
		if (cmdline_opts.multiplayer_side) {
			BOOST_FOREACH(const mp_option& option,
				*cmdline_opts.multiplayer_side) {

				if (option.get<0>() == num) {
					DBG_MP << "\tsetting side " << option.get<0>() <<
						"\tfaction: " << option.get<1>() << std::endl;

					side->set_faction_commandline(option.get<1>());
				}
			}
		}

		// Set the controller, if commandline option is given.
		if (cmdline_opts.multiplayer_controller) {
			BOOST_FOREACH(const mp_option& option,
				*cmdline_opts.multiplayer_controller) {

				if (option.get<0>() == num) {
					DBG_MP << "\tsetting side " << option.get<0>() <<
						"\tfaction: " << option.get<1>() << std::endl;

					side->set_controller_commandline(option.get<1>());
				}
			}
		}

		// Set AI algorithm to RCA AI for all sides,
		// then override if commandline option was given.
		side->set_ai_algorithm_commandline("ai_default_rca");
		if (cmdline_opts.multiplayer_algorithm) {
			BOOST_FOREACH(const mp_option& option,
				*cmdline_opts.multiplayer_algorithm) {

				if (option.get<0>() == num) {
					DBG_MP << "\tsetting side " << option.get<0>() <<
						"\tfaction: " << option.get<1>() << std::endl;

					side->set_ai_algorithm_commandline(option.get<1>());
				}
			}
		}

		// Finally, resolve "random faction",
		// "random gender" and "random message", if any remains unresolved.
		side->resolve_random();
	} // end top-level loop

	update_and_send_diff(true);

	// Update sides with commandline parameters.
	if (cmdline_opts.multiplayer_turns) {
		DBG_MP << "\tsetting turns: " << cmdline_opts.multiplayer_turns <<
			std::endl;
		level_["turns"] = *cmdline_opts.multiplayer_turns;
	}

	BOOST_FOREACH(config &side, level_.child_range("side"))
	{
		if (cmdline_opts.multiplayer_ai_config) {
			BOOST_FOREACH(const mp_option& option,
				*cmdline_opts.multiplayer_ai_config) {

				if (option.get<0>() == side["side"].to_unsigned()) {
					DBG_MP << "\tsetting side " << side["side"] <<
						"\tai_config: " << option.get<1>() << std::endl;

					side["ai_config"] = option.get<1>();
				}
			}
		}

		// Having hard-coded values here is undesirable,
		// but that's how it is done in the MP lobby
		// part of the code also.  Should be replaced by settings/constants in both places
		if (cmdline_opts.multiplayer_ignore_map_settings) {
			side["gold"] = 100;
			side["income"] = 1;
		}

		typedef boost::tuple<unsigned int, std::string, std::string>
			mp_parameter;

		if (cmdline_opts.multiplayer_parm) {
			BOOST_FOREACH(const mp_parameter& parameter,
				*cmdline_opts.multiplayer_parm) {

				if (parameter.get<0>() == side["side"].to_unsigned()) {
					DBG_MP << "\tsetting side " << side["side"] << " " <<
						parameter.get<1>() << ": " << parameter.get<2>() << std::endl;

					side[parameter.get<1>()] = parameter.get<2>();
				}
			}
		}
    }

	// Build the gamestate object after updating the level
	level_to_gamestate(level_, state_);
	network::send_data(config("start_game"), 0);
}

void connect_engine::process_network_connection(const network::connection sock)
{
	network::send_data(config("join_game"), 0);
	network::send_data(level_, sock);
}

connected_user_list::iterator connect_engine::find_player(const std::string& id)
{
	connected_user_list::iterator itor;
	for (itor = users_.begin(); itor != users_.end(); ++itor) {
		if (itor->name == id) {
			break;
		}
	}

	return itor;
}

int connect_engine::find_player_side(const std::string& id) const
{
	size_t i = 0;
	BOOST_FOREACH(side_engine_ptr side, side_engines_) {
		if (side->player_id() == id) {
			break;
		}

		i++;
	}

	if (i >= side_engines_.size()) {
		return -1;
	}

	return i;
}

side_engine::side_engine(const config& cfg, connect_engine& parent_engine,
	const int index) :
	cfg_(cfg),
	parent_(parent_engine),
	available_factions_(),
	choosable_factions_(),
	current_faction_(),
	mp_controller_(CNTR_NETWORK),
	index_(index),
	team_(0),
	color_(index),
	gold_(cfg["gold"].to_int(100)),
	income_(cfg["income"]),
	id_(cfg["id"]),
	player_id_(cfg["player_id"]),
	save_id_(cfg["save_id"]),
	current_player_(cfg["current_player"]),
	leader_(),
	gender_(),
	ai_algorithm_(),
	ready_for_start_(false),
	allow_player_(cfg["controller"] == "ai" && cfg["allow_player"].empty() ?
		false : cfg["allow_player"].to_bool(true)),
	allow_changes_(cfg["allow_changes"].to_bool(true)),
	leaders_(),
	genders_(),
	gender_ids_()
{
	// Tweak the controllers.
	if (cfg["controller"] == "human_ai" ||
		cfg["controller"] == "network_ai") {

		cfg_["controller"] = "ai";
	}
	if (allow_player_ && !parent_.params_.saved_game) {
		mp_controller_ = parent_.mp_controller_;
	} else {
		size_t i = CNTR_NETWORK;
		if (!allow_player_) {
			if (cfg["controller"] == "null") {
				mp_controller_ = CNTR_EMPTY;
			} else {
				cfg_["controller"] = controller_names[CNTR_COMPUTER];
				mp_controller_ = CNTR_COMPUTER;
			}
		} else {
			if (cfg["controller"] == "network" ||
				cfg["controller"] == "human") {

				cfg_["controller"] = "reserved";
			}

			for (; i != CNTR_LAST; ++i) {
				if (cfg["controller"] == controller_names[i]) {
					mp_controller_ = static_cast<mp::controller>(i);
					break;
				}
			}
		}
	}

	// Set faction lock for custom recruit list.
	if (!cfg["recruit"].empty() && parent_.params_.use_map_settings) {
		cfg_["faction"] = "Custom";
	}

	// Initialize faction lists.
	available_factions_ = init_available_factions(parent_.era_factions(), cfg_);
	choosable_factions_ = init_choosable_factions(available_factions_, cfg_,
		parent_.params_.use_map_settings);
	current_faction_ = choosable_factions_[0];

	// Initialize team and color.
	std::vector<std::string>::const_iterator itor =
		std::find(parent_.team_names_.begin(), parent_.team_names_.end(),
			cfg["team_name"].str());
	if (itor == parent_.team_names_.end()) {
		assert(!parent_.team_names_.empty());
		team_ = 0;
	} else {
		team_ = itor - parent_.team_names_.begin();
	}
	if (!cfg["color"].empty()) {
		color_ = game_config::color_info(cfg["color"]).index() - 1;
	}

	// Initialize ai algorithm.
	if (const config &ai = cfg.child("ai")) {
		ai_algorithm_ = ai["ai_algorithm"].str();
	}
}

side_engine::~side_engine()
{
}

config side_engine::new_config() const
{
	config res = cfg_;

	// If the user is allowed to change type, faction, leader etc,
	// then import their new values in the config.
	if (!parent_.params_.saved_game && !choosable_factions_.empty()) {
		// Merge the faction data to res.
		res.append(*current_faction_);
		res["faction_name"] = res["name"];
	}

	if (!cfg_.has_attribute("side") || cfg_["side"].to_int() != index_ + 1) {
		res["side"] = index_ + 1;
	}
	res["controller"] = controller_names[mp_controller_];
	res["current_player"] = player_id_.empty() ? current_player_ : player_id_;
	res["id"] = id_;

	if (player_id_.empty()) {
		std::string description;
		switch(mp_controller_) {
		case CNTR_NETWORK:
			description = N_("(Vacant slot)");

			break;
		case CNTR_LOCAL:
			if (!parent_.params_.saved_game && !cfg_.has_attribute("save_id")) {
				res["save_id"] = preferences::login() + res["side"].str();
			}
			res["player_id"] = preferences::login() + res["side"].str();
			res["current_player"] = preferences::login();
			description = N_("Anonymous local player");

			break;
		case CNTR_COMPUTER: {
			if (!parent_.params_.saved_game && !cfg_.has_attribute("saved_id")) {
				res["save_id"] = "ai" + res["side"].str();
			}

			utils::string_map symbols;
			if (allow_player_) {
				const config &ai_cfg =
					ai::configuration::get_ai_config_for(ai_algorithm_);
				res.add_child("ai", ai_cfg);
				symbols["playername"] = ai_cfg["description"];
			} else {
				// Do not import default ai cfg here -
				// all is set by scenario config.
				symbols["playername"] = _("Computer Player");
			}

			symbols["side"] = res["side"].str();
			description = vgettext("$playername $side", symbols);

			break;
		}
		case CNTR_EMPTY:
			description = N_("(Empty slot)");
			res["no_leader"] = true;

			break;
		case CNTR_RESERVED: {
			utils::string_map symbols;
			symbols["playername"] = current_player_;
			description = vgettext("(Reserved for $playername)",symbols);

			break;
		}
		case CNTR_LAST:
		default:
			description = N_("(empty)");
			assert(false);

			break;
		} // end switch

		res["user_description"] = t_string(description, "wesnoth");
	} else {
		res["player_id"] = player_id_ + res["side"];
		if (!parent_.params_.saved_game && !cfg_.has_attribute("save_id")) {
			res["save_id"] = player_id_ + res["side"];
		}

		res["user_description"] = player_id_;
	}

	res["name"] = res["user_description"];
	res["allow_changes"] = !parent_.params_.saved_game && allow_changes_;

	if (!parent_.params_.saved_game) {
		res["type"] = leader();

		if (!gender_.empty() || gender_ != "null" || gender_ != "?") {
			res["gender"] = gender_;
		}

		res["team_name"] = parent_.team_names_[team_];
		res["user_team_name"] = parent_.user_team_names_[team_];
		res["allow_player"] = allow_player_;
		res["color"] = color_ + 1;
		res["gold"] = gold_;
		res["income"] = income_;

		if (!parent_.params_.use_map_settings || res["fog"].empty() ||
			(res["fog"] != "yes" && res["fog"] != "no")) {
			res["fog"] = parent_.params_.fog_game;
		}

		if (!parent_.params_.use_map_settings || res["shroud"].empty() ||
			(res["shroud"] != "yes" && res["shroud"] != "no")) {
			res["shroud"] = parent_.params_.shroud_game;
		}

		res["share_maps"] = parent_.params_.share_maps;
		res["share_view"] =  parent_.params_.share_view;

		if (!parent_.params_.use_map_settings || res["village_gold"].empty()) {
			res["village_gold"] = parent_.params_.village_gold;
		}
		if (!parent_.params_.use_map_settings ||
			res["village_support"].empty()) {
			res["village_support"] =
				lexical_cast<std::string>(parent_.params_.village_support);
		}

	}

	if (parent_.params_.use_map_settings && !parent_.params_.saved_game) {
		config trimmed = cfg_;

		static char const *attrs[] = {"side", "controller", "id",
			"team_name", "user_team_name", "color", "gold",
			"income", "allow_changes"};

		BOOST_FOREACH(const char *attr, attrs) {
			trimmed.remove_attribute(attr);
		}

		if (mp_controller_ != CNTR_COMPUTER) {
			// Only override names for computer controlled players.
			trimmed.remove_attribute("user_description");
		}

		res.merge_with(trimmed);
	}

	return res;
}

bool side_engine::ready_for_start() const
{
	// Sides without players are always ready.
	if (!allow_player_) {
		return true;
	}

	// The host and the AI are always ready.
	if ((mp_controller_ == mp::CNTR_COMPUTER) ||
		(mp_controller_ == mp::CNTR_EMPTY) ||
		(mp_controller_== mp::CNTR_LOCAL)) {

		return true;
	}

	return ready_for_start_;
}

bool side_engine::available(const std::string& name) const
{
	if (name.empty()) {
		return allow_player_ && ((mp_controller_ == CNTR_NETWORK &&
			player_id_.empty()) || mp_controller_ == CNTR_RESERVED);
	}

	return allow_player_ &&
		((mp_controller_ == CNTR_NETWORK && player_id_.empty()) ||
		(mp_controller_ == CNTR_RESERVED && current_player_ == name));
}

void side_engine::resolve_random()
{
	if (parent_.params_.saved_game || choosable_factions_.empty()) {
		return;
	}

	if ((*current_faction_)["random_faction"].to_bool()) {
		std::vector<std::string> faction_choices, faction_excepts;

		faction_choices = utils::split((*current_faction_)["choices"]);
		if (faction_choices.size() == 1 && faction_choices.front() == "") {
			faction_choices.clear();
		}

		faction_excepts = utils::split((*current_faction_)["except"]);
		if (faction_excepts.size() == 1 && faction_excepts.front() == "") {
			faction_excepts.clear();
		}

		// Builds the list of sides eligible for choice (nonrandom factions).
		std::vector<int> nonrandom_sides;
		int num = -1;
		BOOST_FOREACH(const config* i, available_factions_) {
			++num;
			if (!(*i)["random_faction"].to_bool()) {
				const std::string& faction_id = (*i)["id"];

				if (!faction_choices.empty() &&
					std::find(faction_choices.begin(), faction_choices.end(),
						faction_id) == faction_choices.end()) {
					continue;
				}

				if (!faction_excepts.empty() &&
					std::find(faction_excepts.begin(), faction_excepts.end(),
						faction_id) != faction_excepts.end()) {
					continue;
				}

				nonrandom_sides.push_back(num);
			}
		}

		if (nonrandom_sides.empty()) {
			throw config::error(_("Only random sides in the current era."));
		}

		const int faction_index =
			nonrandom_sides[rand() % nonrandom_sides.size()];
		current_faction_ = available_factions_[faction_index];
	}

	LOG_MP << "FACTION" << (index_ + 1) << ": " << (*current_faction_)["name"]
		<< std::endl;

	bool solved_random_leader = false;

	if (leader() == "random") {
		// Choose a random leader type, and force gender to be random.
		gender_ = "random";
		std::vector<std::string> types =
			utils::split((*current_faction_)["random_leader"]);
		if (!types.empty()) {
			const int lchoice = rand() % types.size();
			leader_ = types[lchoice];
		} else {
			// If 'random_leader' doesn't exist, we use 'leader'.
			types = utils::split((*current_faction_)["leader"]);
			if (!types.empty()) {
				const int lchoice = rand() % types.size();
				leader_ = types[lchoice];
			} else {
				utils::string_map i18n_symbols;
				i18n_symbols["faction"] = (*current_faction_)["name"];
				throw config::error(vgettext(
					"Unable to find a leader type for faction $faction",
					i18n_symbols));
			}
		}
		solved_random_leader = true;
	}

	// Resolve random genders "very much" like standard unit code.
	if (gender() == "random" || solved_random_leader) {
		const unit_type *ut =
			unit_types.find(leader());

		if (ut) {
			const std::vector<unit_race::GENDER> glist = ut->genders();
			const int gchoice = rand() % glist.size();

			// Pick up a gender, using the random 'gchoice' index.
			unit_race::GENDER sgender = glist[gchoice];
			switch (sgender) {
				case unit_race::FEMALE:
					gender_ = unit_race::s_female;
					break;
				case unit_race::MALE:
					gender_ = unit_race::s_male;
					break;
				default:
					gender_ = "null";
			}
		} else {
			ERR_CF << "cannot obtain genders for invalid leader '" << leader_
				<< "'.\n";
			gender_ = "null";
		}
	}
}

int side_engine::selected_faction_index() const
{
	int index = 0;
	BOOST_FOREACH(const config* faction, choosable_factions_) {
		if ((*faction)["id"] == (*current_faction_)["id"]) {
			return index;
		}

		index++;
	}

	return 0;
}

void side_engine::set_player_from_users_list(const std::string& player_id)
{
	connected_user_list::iterator i = parent_.find_player(player_id);
	if (i != parent_.users_.end()) {
		player_id_ = player_id;
		mp_controller_ = i->controller;
	}
}

void side_engine::set_faction_commandline(const std::string& faction_name)
{
	BOOST_FOREACH(const config* faction, choosable_factions_) {
		if ((*faction)["name"] == faction_name) {
			current_faction_ = faction;
			break;
		}
	}
}

void side_engine::set_controller_commandline(const std::string& controller_name)
{
	mp_controller_ = CNTR_LOCAL;

	if (controller_name == "ai") {
		mp_controller_ = CNTR_COMPUTER;
	}
	if (controller_name == "null") {
		mp_controller_ = CNTR_EMPTY;
	}

	player_id_ = "";
}

void side_engine::set_ai_algorithm_commandline(
	const std::string& algorithm_name)
{
	ai_algorithm_ = algorithm_name;
}

void side_engine::assign_sides_on_drop_target(const int drop_target) {
	const std::string target_id = parent_.side_engines_[drop_target]->player_id_;
	const mp::controller target_controller =
		parent_.side_engines_[drop_target]->mp_controller_;
	const std::string target_ai =
		parent_.side_engines_[drop_target]->ai_algorithm_;

	parent_.side_engines_[drop_target]->ai_algorithm_ = ai_algorithm_;
	if (player_id_.empty()) {
		parent_.side_engines_[drop_target]->mp_controller_ = mp_controller_;
	} else {
		parent_.side_engines_[drop_target]->
			set_player_from_users_list(player_id_);
	}

	ai_algorithm_ = target_ai;
	if (target_id.empty())
	{
		mp_controller_ = target_controller;
		player_id_ = "";
	} else {
		set_player_from_users_list(target_id);
	}
}

std::string side_engine::leader() const
{
	if (parent_.params_.saved_game) {
		return _("?");
	}

	if (leader_.empty()) {
		return "random";
	}

	return leader_;
}


std::string side_engine::gender() const
{
	if (parent_.params_.saved_game || genders_.empty()) {
		return "null";
	}

	return gender_;
}

} // end namespace mp
