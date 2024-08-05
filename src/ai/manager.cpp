/*
	Copyright (C) 2009 - 2024
	by Yurii Chernyi <terraninfo@terraninfo.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

/**
 * Managing the AI lifecycle and interface for the rest of Wesnoth
 * @file
 */

#include "ai/manager.hpp"

#include "config.hpp"             // for config, etc
#include "game_events/pump.hpp"
#include "log.hpp"
#include "map/location.hpp"       // for map_location
#include "resources.hpp"
#include "serialization/string_utils.hpp"
#include "tod_manager.hpp"

#include "ai/composite/ai.hpp"             // for ai_composite
#include "ai/composite/component.hpp"      // for component_manager
#include "ai/composite/engine.hpp"         // for engine
#include "ai/composite/value_translator.hpp"
#include "ai/configuration.hpp"            // for configuration
#include "ai/contexts.hpp"                 // for readonly_context, etc
#include "ai/default/contexts.hpp"  // for default_ai_context, etc
#include "ai/game_info.hpp"             // for side_number, engine_ptr, etc
#include "game_config.hpp"              // for debug
#include "ai/lua/aspect_advancements.hpp"
#include "ai/registry.hpp"              // for init
#include "ai/lua/engine_lua.hpp"

#include <algorithm>                    // for min
#include <cassert>                     // for assert
#include <iterator>                     // for reverse_iterator, etc
#include <map>                          // for _Rb_tree_iterator, etc
#include <set>                          // for set
#include <stack>                        // for stack
#include <utility>                      // for pair, make_pair
#include <vector>                       // for vector, allocator, etc

#include <SDL2/SDL_timer.h>

namespace ai {

const std::string manager::AI_TYPE_COMPOSITE_AI = "composite_ai";
const std::string manager::AI_TYPE_SAMPLE_AI = "sample_ai";
const std::string manager::AI_TYPE_IDLE_AI = "idle_ai";
const std::string manager::AI_TYPE_FORMULA_AI = "formula_ai";
const std::string manager::AI_TYPE_DEFAULT = "default";

static lg::log_domain log_ai_manager("ai/manager");
#define DBG_AI_MANAGER LOG_STREAM(debug, log_ai_manager)
#define LOG_AI_MANAGER LOG_STREAM(info, log_ai_manager)
#define ERR_AI_MANAGER LOG_STREAM(err, log_ai_manager)

static lg::log_domain log_ai_mod("ai/mod");
#define DBG_AI_MOD LOG_STREAM(debug, log_ai_mod)
#define LOG_AI_MOD LOG_STREAM(info, log_ai_mod)
#define WRN_AI_MOD LOG_STREAM(warn, log_ai_mod)
#define ERR_AI_MOD LOG_STREAM(err, log_ai_mod)

holder::holder( side_number side, const config &cfg )
	: ai_(), side_context_(nullptr), readonly_context_(nullptr), readwrite_context_(nullptr), default_ai_context_(nullptr), side_(side), cfg_(cfg)
{
	DBG_AI_MANAGER << describe_ai() << "Preparing new AI holder";
}

void holder::init( side_number side )
{
	if (side_context_ == nullptr) {
		side_context_.reset(new side_context_impl(side,cfg_));
	} else {
		side_context_->set_side(side);
	}
	if (readonly_context_ == nullptr){
		readonly_context_.reset(new readonly_context_impl(*side_context_,cfg_));
		readonly_context_->on_readonly_context_create();
	}
	if (readwrite_context_ == nullptr){
		readwrite_context_.reset(new readwrite_context_impl(*readonly_context_,cfg_));
	}
	if (default_ai_context_ == nullptr){
		default_ai_context_.reset(new default_ai_context_impl(*readwrite_context_,cfg_));
	}
	if (!ai_){
		ai_.reset(new ai_composite(*default_ai_context_,cfg_));
	}

	if (ai_) {
		ai_->on_create();
		for (config &mod_ai : cfg_.child_range("modify_ai")) {
			if (!mod_ai.has_attribute("side")) {
				mod_ai["side"] = side;
			}
			modify_ai(mod_ai);
		}
		for(config& micro : cfg_.child_range("micro_ai")) {
			micro["side"] = side;
			micro["action"] = "add";
			micro_ai(micro);
		}
		cfg_.clear_children("modify_ai", "micro_ai");

		std::vector<engine_ptr> engines = ai_->get_engines();
		for (std::vector<engine_ptr>::iterator it = engines.begin(); it != engines.end(); ++it)
		{
			(*it)->set_ai_context(&(ai_->get_ai_context()));
		}

	} else {
		ERR_AI_MANAGER << describe_ai()<<"AI lazy initialization error!";
	}

}

holder::~holder()
{
	try {
	if (ai_) {
		LOG_AI_MANAGER << describe_ai() << "Managed AI will be deleted";
	}
	} catch (...) {}
}

ai_composite& holder::get_ai_ref()
{
	if (!ai_) {
		init(side_);
	}
	assert(ai_);

	return *ai_;
}

void holder::micro_ai(const config& cfg)
{
	if (!ai_) {
		init(side_);
	}
	assert(ai_);

	auto engine = ai_->get_engine_by_cfg(config{"engine", "lua"});
	if(auto lua = std::dynamic_pointer_cast<engine_lua>(engine)) {
		lua->apply_micro_ai(cfg);
	}
}

void holder::modify_ai(const config &cfg)
{
	if (!ai_) {
		// if not initialized, initialize now.
		get_ai_ref();
	}
	const std::string &act = cfg["action"];
	LOG_AI_MOD << "side "<< side_ << "        "<<act<<"_ai_component \""<<cfg["path"]<<"\"";
	DBG_AI_MOD << std::endl << cfg;
	DBG_AI_MOD << "side "<< side_ << " before "<<act<<"_ai_component"<<std::endl << to_config();
	bool res = false;
	if (act == "add") {
		res = component_manager::add_component(&*ai_,cfg["path"],cfg);
	} else if (act == "change") {
		res = component_manager::change_component(&*ai_,cfg["path"],cfg);
	} else if (act == "delete") {
		res = component_manager::delete_component(&*ai_,cfg["path"]);
	} else {
		ERR_AI_MOD << "modify_ai tag has invalid 'action' attribute " << act;
	}
	DBG_AI_MOD << "side "<< side_ << "  after [modify_ai]"<<act<<std::endl << to_config();
	if (!res) {
		LOG_AI_MOD << act << "_ai_component failed";
	} else {
		LOG_AI_MOD << act << "_ai_component success";
	}

}

void holder::append_ai(const config& cfg)
{
	if(!ai_) {
		get_ai_ref();
	}
	for(const config& aspect : cfg.child_range("aspect")) {
		const std::string& id = aspect["id"];
		for(const config& facet : aspect.child_range("facet")) {
			ai_->add_facet(id, facet);
		}
	}
	for(const config& goal : cfg.child_range("goal")) {
		ai_->add_goal(goal);
	}
	for(const config& stage : cfg.child_range("stage")) {
		if(stage["name"] != "empty") {
			ai_->add_stage(stage);
		}
	}
	for(config mod : cfg.child_range("modify_ai")) {
		if (!mod.has_attribute("side")) {
			mod["side"] = side_context_->get_side();
		}
		modify_ai(mod);
	}
	for(config micro : cfg.child_range("micro_ai")) {
		micro["side"] = side_context_->get_side();
		micro["action"] = "add";
		micro_ai(micro);
	}
}

config holder::to_config() const
{
	if (!ai_) {
		return cfg_;
	} else {
		config cfg = ai_->to_config();
		if (side_context_!=nullptr) {
			cfg.merge_with(side_context_->to_side_context_config());
		}
		if (readonly_context_!=nullptr) {
			cfg.merge_with(readonly_context_->to_readonly_context_config());
		}
		if (readwrite_context_!=nullptr) {
			cfg.merge_with(readwrite_context_->to_readwrite_context_config());
		}
		if (default_ai_context_!=nullptr) {
			cfg.merge_with(default_ai_context_->to_default_ai_context_config());
		}

		return cfg;
	}
}

std::string holder::describe_ai() const
{
	if(ai_) {
		return formatter() << ai_->describe_self() << " for side " << side_ << " : ";
	} else {
		return formatter() << "not initialized ai with id=[" << cfg_["id"] << "] for side " << side_ << " : ";
	}
}

std::string holder::get_ai_overview()
{
	if (!ai_) {
		get_ai_ref();
	}
	// These assignments are necessary because the code will otherwise not compile on some platforms with an lvalue/rvalue mismatch error
	auto lik = ai_->get_leader_ignores_keep();
	auto pl = ai_->get_passive_leader();
	auto plsk = ai_->get_passive_leader_shares_keep();
	// In order to display booleans as yes/no rather than 1/0 or true/false
	config cfg;
	cfg["allow_ally_villages"] = ai_->get_allow_ally_villages();
	cfg["simple_targeting"] = ai_->get_simple_targeting();
	cfg["support_villages"] = ai_->get_support_villages();
	std::stringstream s;
	s << "advancements:  " << ai_->get_advancements().get_value() << std::endl;
	s << "aggression:  " << ai_->get_aggression() << std::endl;
	s << "allow_ally_villages:  " << cfg["allow_ally_villages"] << std::endl;
	s << "caution:  " << ai_->get_caution() << std::endl;
	s << "grouping:  " << ai_->get_grouping() << std::endl;
	s << "leader_aggression:  " << ai_->get_leader_aggression() << std::endl;
	s << "leader_ignores_keep:  " << utils::visit(leader_aspects_visitor(), lik) << std::endl;
	s << "leader_value:  " << ai_->get_leader_value() << std::endl;
	s << "passive_leader:  " << utils::visit(leader_aspects_visitor(), pl) << std::endl;
	s << "passive_leader_shares_keep:  " << utils::visit(leader_aspects_visitor(), plsk) << std::endl;
	s << "recruitment_diversity:  " << ai_->get_recruitment_diversity() << std::endl;
	s << "recruitment_instructions:  " << std::endl << "----config begin----" << std::endl;
	s << ai_->get_recruitment_instructions() << "-----config end-----" << std::endl;
	s << "recruitment_more:  " << utils::join(ai_->get_recruitment_more()) << std::endl;
	s << "recruitment_pattern:  " << utils::join(ai_->get_recruitment_pattern()) << std::endl;
	s << "recruitment_randomness:  " << ai_->get_recruitment_randomness() << std::endl;
	s << "recruitment_save_gold:  " << std::endl << "----config begin----" << std::endl;
	s << ai_->get_recruitment_save_gold() << "-----config end-----" << std::endl;
	s << "retreat_enemy_weight:  " << ai_->get_retreat_enemy_weight() << std::endl;
	s << "retreat_factor:  " << ai_->get_retreat_factor() << std::endl;
	s << "scout_village_targeting:  " << ai_->get_scout_village_targeting() << std::endl;
	s << "simple_targeting:  " << cfg["simple_targeting"] << std::endl;
	s << "support_villages:  " << cfg["support_villages"] << std::endl;
	s << "village_value:  " << ai_->get_village_value() << std::endl;
	s << "villages_per_scout:  " << ai_->get_villages_per_scout() << std::endl;

	return s.str();
}

std::string holder::get_ai_structure()
{
	if (!ai_) {
		get_ai_ref();
	}
	return component_manager::print_component_tree(&*ai_,"");
}

std::string holder::get_ai_identifier() const
{
	return cfg_["id"];
}

component* holder::get_component(component *root, const std::string &path) {
	if (!game_config::debug) // Debug guard
	{
		return nullptr;
	}

	if (root == nullptr) // Return root component(ai_)
	{
		if (!ai_) {
			init(side_);
		}
		assert(ai_);

		return &*ai_;
	}

	return component_manager::get_component(root, path);
}

// =======================================================================
// LIFECYCLE
// =======================================================================

manager::manager()
	: history_()
	, history_item_counter_(0)
	, ai_info_()
	, map_changed_("ai_map_changed")
	, recruit_list_changed_("ai_recruit_list_changed")
	, user_interact_("ai_user_interact")
	, sync_network_("ai_sync_network")
	, tod_changed_("ai_tod_changed")
	, gamestate_changed_("ai_gamestate_changed")
	, turn_started_("ai_turn_started")
	, last_interact_(0)
	, num_interact_(0)
{
	registry::init();
	singleton_ = this;
}

manager* manager::singleton_ = nullptr;

void manager::add_observer( events::observer* event_observer){
	user_interact_.attach_handler(event_observer);
	sync_network_.attach_handler(event_observer);
	turn_started_.attach_handler(event_observer);
	gamestate_changed_.attach_handler(event_observer);
}

void manager::remove_observer(events::observer* event_observer){
	user_interact_.detach_handler(event_observer);
	sync_network_.detach_handler(event_observer);
	turn_started_.detach_handler(event_observer);
	gamestate_changed_.detach_handler(event_observer);
}

void manager::add_gamestate_observer( events::observer* event_observer){
	gamestate_changed_.attach_handler(event_observer);
	turn_started_.attach_handler(event_observer);
	map_changed_.attach_handler(event_observer);
}

void manager::remove_gamestate_observer(events::observer* event_observer){
	gamestate_changed_.detach_handler(event_observer);
	turn_started_.detach_handler(event_observer);
	map_changed_.detach_handler(event_observer);
}

void manager::add_tod_changed_observer( events::observer* event_observer){
	tod_changed_.attach_handler(event_observer);
}

void manager::remove_tod_changed_observer(events::observer* event_observer){
	tod_changed_.detach_handler(event_observer);
}

void manager::add_map_changed_observer( events::observer* event_observer )
{
	map_changed_.attach_handler(event_observer);
}

void manager::add_recruit_list_changed_observer( events::observer* event_observer )
{
	recruit_list_changed_.attach_handler(event_observer);
}

void manager::add_turn_started_observer( events::observer* event_observer )
{
	turn_started_.attach_handler(event_observer);
}

void manager::remove_recruit_list_changed_observer( events::observer* event_observer )
{
	recruit_list_changed_.detach_handler(event_observer);
}

void manager::remove_map_changed_observer( events::observer* event_observer )
{
	map_changed_.detach_handler(event_observer);
}

void manager::remove_turn_started_observer( events::observer* event_observer )
{
	turn_started_.detach_handler(event_observer);
}

void manager::raise_user_interact() {
	if(resources::simulation_){
		return;
	}

	const int interact_time = 30;
	const int time_since_interact = SDL_GetTicks() - last_interact_;
	if(time_since_interact < interact_time) {
		return;
	}

	++num_interact_;
	user_interact_.notify_observers();

	last_interact_ = SDL_GetTicks();

}

void manager::raise_sync_network() {
	sync_network_.notify_observers();
}

void manager::raise_gamestate_changed() {
	gamestate_changed_.notify_observers();
}

void manager::raise_tod_changed() {
	tod_changed_.notify_observers();
}

void manager::raise_turn_started() {
	turn_started_.notify_observers();
}

void manager::raise_recruit_list_changed() {
	recruit_list_changed_.notify_observers();
}

void manager::raise_map_changed() {
	map_changed_.notify_observers();
}

// =======================================================================
// EVALUATION
// =======================================================================

const std::string manager::evaluate_command( side_number side, const std::string& str )
{
	//insert new command into history
	history_.emplace_back(history_item_counter_++,str);

	//prune history - erase 1/2 of it if it grows too large
	if (history_.size()>MAX_HISTORY_SIZE){
		history_.erase(history_.begin(),history_.begin()+MAX_HISTORY_SIZE/2);
		LOG_AI_MANAGER << "AI MANAGER: pruned history";
	}

	if (!should_intercept(str)){
		ai_composite& ai = get_active_ai_for_side(side);
		raise_gamestate_changed();
		return ai.evaluate(str);
	}

	return internal_evaluate_command(side,str);
}

bool manager::should_intercept( const std::string& str ) const
{
	if (str.length()<1) {
		return false;
	}
	if (str.at(0)=='!'){
		return true;
	}
	if (str.at(0)=='?'){
		return true;
	}
	return false;

}

// this is stub code to allow testing of basic 'history', 'repeat-last-command', 'add/remove/replace ai' capabilities.
// yes, it doesn't look nice. but it is usable.
// to be refactored at earliest opportunity
// TODO: extract to separate class which will use fai or lua parser
const std::string manager::internal_evaluate_command( side_number side, const std::string& str ){
	const int MAX_HISTORY_VISIBLE = 30;

	//repeat last command
	if (str=="!") {
			//this command should not be recorded in history
			if (!history_.empty()){
				history_.pop_back();
				history_item_counter_--;
			}

			if (history_.empty()){
				return "AI MANAGER: empty history";
			}
			return evaluate_command(side, history_.back().get_command());//no infinite loop since '!' commands are not present in history
	};
	//show last command
	if (str=="?") {
		//this command should not be recorded in history
		if (!history_.empty()){
			history_.pop_back();
			history_item_counter_--;
		}

		if (history_.empty()){
			return "AI MANAGER: History is empty";
		}

		int n = std::min<int>( MAX_HISTORY_VISIBLE, history_.size() );
		std::stringstream strstream;
		strstream << "AI MANAGER: History - last "<< n <<" commands:\n";
		std::deque< command_history_item >::reverse_iterator j = history_.rbegin();

		for (int cmd_id=n; cmd_id>0; --cmd_id){
			strstream << j->get_number() << "    :" << j->get_command() << '\n';
			++j;//this is *reverse* iterator
		}

		return strstream.str();
	};

	std::vector< std::string > cmd = utils::parenthetical_split(str, ' ',"'","'");

	if (cmd.size()==3){
		// add_ai side file
		if (cmd.at(0)=="!add_ai"){
			side = std::stoi(cmd.at(1));
			std::string file = cmd.at(2);
			if (add_ai_for_side_from_file(side,file,false)){
				return std::string("AI MANAGER: added [")+manager::get_active_ai_identifier_for_side(side)+std::string("] AI for side ")+std::to_string(side)+std::string(" from file ")+file;
			} else {
				return std::string("AI MANAGER: failed attempt to add AI for side ")+std::to_string(side)+std::string(" from file ")+file;
			}
		}
		// replace_ai side file
		if (cmd.at(0)=="!replace_ai"){
			side = std::stoi(cmd.at(1));
			std::string file = cmd.at(2);
			if (add_ai_for_side_from_file(side,file,true)){
					return std::string("AI MANAGER: added [")+manager::get_active_ai_identifier_for_side(side)+std::string("] AI for side ")+std::to_string(side)+std::string(" from file ")+file;
			} else {
					return std::string("AI MANAGER: failed attempt to add AI for side ")+std::to_string(side)+std::string(" from file ")+file;
			}
		}

	} else if (cmd.size()==2){
		// remove_ai side
		if (cmd.at(0)=="!remove_ai"){
			side = std::stoi(cmd.at(1));
			remove_ai_for_side(side);
			return std::string("AI MANAGER: made an attempt to remove AI for side ")+std::to_string(side);
		}
		if (cmd.at(0)=="!"){
			//this command should not be recorded in history
			if (!history_.empty()){
				history_.pop_back();
				history_item_counter_--;
			}

			int command = std::stoi(cmd.at(1));
			std::deque< command_history_item >::reverse_iterator j = history_.rbegin();
			//yes, the iterator could be precisely positioned (since command numbers go 1,2,3,4,..). will do it later.
			while ( (j!=history_.rend()) && (j->get_number()!=command) ){
				++j;// this is *reverse* iterator
			}
			if (j!=history_.rend()){
				return evaluate_command(side,j->get_command());//no infinite loop since '!' commands are not present in history
			}
			return "AI MANAGER: no command with requested number found";
		}
	} else if (cmd.size()==1){
		if (cmd.at(0)=="!help") {
			return
				"known commands:\n"
				"!    - repeat last command (? and ! do not count)\n"
				"! NUMBER    - repeat numbered command\n"
				"?    - show a history list\n"
				"!add_ai TEAM FILE    - add a AI to side (0 - command AI, N - AI for side #N) from file\n"
				"!remove_ai TEAM    - remove AI from side (0 - command AI, N - AI for side #N)\n"
				"!replace_ai TEAM FILE    - replace AI of side (0 - command AI, N - AI for side #N) from file\n"
				"!help    - show this help message";
		}
	}

	return "AI MANAGER: nothing to do";
}

// =======================================================================
// ADD, CREATE AIs, OR LIST AI TYPES
// =======================================================================

bool manager::add_ai_for_side_from_file( side_number side, const std::string& file, bool replace )
{
	config cfg;
	if (!configuration::get_side_config_from_file(file,cfg)){
		ERR_AI_MANAGER << " unable to read [SIDE] config for side "<< side << "from file [" << file <<"]";
		return false;
	}
	return add_ai_for_side_from_config(side,cfg,replace);
}

bool manager::add_ai_for_side_from_config( side_number side, const config& cfg, bool replace ){
	config parsed_cfg;
	configuration::parse_side_config(side, cfg, parsed_cfg);

	if (replace) {
		remove_ai_for_side(side);
	}

	std::stack<holder>& ai_stack_for_specific_side = get_or_create_ai_stack_for_side(side);
	ai_stack_for_specific_side.emplace(side, parsed_cfg);
	return true;
}

// =======================================================================
// REMOVE
// =======================================================================

void manager::remove_ai_for_side( side_number side )
{
	std::stack<holder>& ai_stack_for_specific_side = get_or_create_ai_stack_for_side(side);
	if (!ai_stack_for_specific_side.empty()){
		ai_stack_for_specific_side.pop();
	}
}

void manager::remove_all_ais_for_side( side_number side )
{
	std::stack<holder>& ai_stack_for_specific_side = get_or_create_ai_stack_for_side(side);

	//clear the stack. std::stack doesn't have a '.clear()' method to do it
	while (!ai_stack_for_specific_side.empty()){
			ai_stack_for_specific_side.pop();
	}
}

void manager::clear_ais()
{
	ai_map_.clear();
}

void manager::modify_active_ai_for_side ( side_number side, const config &cfg )
{
	get_active_ai_holder_for_side(side).modify_ai(cfg);
}

void manager::append_active_ai_for_side(side_number side, const config& cfg)
{
	get_active_ai_holder_for_side(side).append_ai(cfg);
}

std::string manager::get_active_ai_overview_for_side( side_number side )
{
	return get_active_ai_holder_for_side(side).get_ai_overview();
}

std::string manager::get_active_ai_structure_for_side( side_number side )
{
	return get_active_ai_holder_for_side(side).get_ai_structure();
}

std::string manager::get_active_ai_identifier_for_side( side_number side )
{
	return get_active_ai_holder_for_side(side).get_ai_identifier();
}

ai::holder& manager::get_active_ai_holder_for_side_dbg( side_number side )
{
	if (!game_config::debug)
	{
		static ai::holder empty_holder(side, config());
		return empty_holder;
	}
	return get_active_ai_holder_for_side(side);
}

config manager::to_config( side_number side )
{
	return get_active_ai_holder_for_side(side).to_config();
}

game_info& manager::get_active_ai_info_for_side( side_number /*side*/ )
{
	return ai_info_;
}

game_info& manager::get_ai_info()
{
	return ai_info_;
}

const ai::unit_advancements_aspect& manager::get_advancement_aspect_for_side(side_number side)
{
	return get_active_ai_holder_for_side(side).get_ai_ref().get_advancements();
}

// =======================================================================
// PROXY
// =======================================================================

void manager::play_turn( side_number side ){
	last_interact_ = 0;
	num_interact_ = 0;
	const int turn_start_time = SDL_GetTicks();
	get_ai_info().recent_attacks.clear();
	ai_composite& ai_obj = get_active_ai_for_side(side);
	resources::game_events->pump().fire("ai_turn");
	raise_turn_started();
	if (resources::tod_manager->has_tod_bonus_changed()) {
		raise_tod_changed();
	}
	ai_obj.new_turn();
	ai_obj.play_turn();
	const int turn_end_time= SDL_GetTicks();
	DBG_AI_MANAGER << "side " << side << ": number of user interactions: "<<num_interact_;
	DBG_AI_MANAGER << "side " << side << ": total turn time: "<<turn_end_time - turn_start_time << " ms ";
}

// =======================================================================
// PRIVATE
// =======================================================================
// =======================================================================
// AI STACKS
// =======================================================================
std::stack<holder>& manager::get_or_create_ai_stack_for_side( side_number side )
{
	AI_map_of_stacks::iterator iter = ai_map_.find(side);
	if (iter!=ai_map_.end()){
		return iter->second;
	}
	return ai_map_.emplace(side, std::stack<holder>()).first->second;
}

// =======================================================================
// AI HOLDERS
// =======================================================================
holder& manager::get_active_ai_holder_for_side( side_number side )
{
	std::stack<holder>& ai_stack_for_specific_side = get_or_create_ai_stack_for_side(side);

	if (!ai_stack_for_specific_side.empty()){
		return ai_stack_for_specific_side.top();
	} else {
		config cfg = configuration::get_default_ai_parameters();
		ai_stack_for_specific_side.emplace(side, cfg);
		return ai_stack_for_specific_side.top();
	}
}

// =======================================================================
// AI POINTERS
// =======================================================================

ai_composite& manager::get_active_ai_for_side( side_number side )
{
	return get_active_ai_holder_for_side(side).get_ai_ref();
}

// =======================================================================
// MISC
// =======================================================================

} //end of namespace ai
