/* $Id$ */
/*
   Copyright (C) 2009 by Yurii Chernyi <terraninfo@terraninfo.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * Managing the AI lifecycle and interface for the rest of Wesnoth
 * @file ai/manager.cpp
 */

#include "ai2/ai.hpp"
#include "composite/ai.hpp"
#include "configuration.hpp"
#include "contexts.hpp"
#include "default/ai.hpp"
#include "dfool/ai.hpp"
#include "manager.hpp"
#include "formula/ai.hpp"
#include "registry.hpp"
#include "../game_events.hpp"
#include "../game_preferences.hpp"
#include "../log.hpp"
#include "../replay.hpp"
#include "../serialization/string_utils.hpp"
#include "../statistics.hpp"

#include <map>
#include <stack>
#include <vector>

namespace ai {

const std::string manager::AI_TYPE_COMPOSITE_AI = "composite_ai";
const std::string manager::AI_TYPE_SAMPLE_AI = "sample_ai";
const std::string manager::AI_TYPE_IDLE_AI = "idle_ai";
const std::string manager::AI_TYPE_FORMULA_AI = "formula_ai";
const std::string manager::AI_TYPE_DFOOL_AI = "dfool_ai";
const std::string manager::AI_TYPE_AI2 = "ai2";
const std::string manager::AI_TYPE_DEFAULT = "default";


static lg::log_domain log_ai_manager("ai/manager");
#define DBG_AI_MANAGER LOG_STREAM(debug, log_ai_manager)
#define LOG_AI_MANAGER LOG_STREAM(info, log_ai_manager)
#define ERR_AI_MANAGER LOG_STREAM(err, log_ai_manager)

holder::holder( int side, const std::string& ai_algorithm_type )
	: ai_(), side_context_(NULL), readonly_context_(NULL), readwrite_context_(NULL), default_ai_context_(NULL), ai_algorithm_type_(ai_algorithm_type), ai_effective_parameters_(),  ai_global_parameters_(), ai_memory_(), ai_parameters_(), side_(side)
{
	DBG_AI_MANAGER << describe_ai() << "Preparing new AI holder" << std::endl;
}


void holder::init( int side )
{
	LOG_AI_MANAGER << describe_ai() << "Preparing to create new managed master AI" << std::endl;
	if (side_context_ == NULL) {
		side_context_ = new side_context_impl(side);
	} else {
		side_context_->set_side(side);
	}
	if (readonly_context_ == NULL){
		readonly_context_ = new readonly_context_impl(*side_context_);
	}
	if (readwrite_context_ == NULL){
		readwrite_context_ = new readwrite_context_impl(*readonly_context_);
	}
	if (default_ai_context_ == NULL){
		default_ai_context_ = new default_ai_context_impl(*readwrite_context_);
	}
	this->ai_ = create_ai(side);
	if (!this->ai_) {
		ERR_AI_MANAGER << describe_ai()<<"AI lazy initialization error!" << std::endl;
	}

}


holder::~holder()
{
	if (this->ai_) {
		LOG_AI_MANAGER << describe_ai() << "Managed AI will be deleted" << std::endl;
	}
	delete this->default_ai_context_;
	delete this->readwrite_context_;
	delete this->readonly_context_;
	delete this->side_context_;
}


interface& holder::get_ai_ref( int side )
{
	if (!this->ai_) {
		this->init(side);
	}
	assert(this->ai_);

	return *this->ai_;
}

interface& holder::get_ai_ref()
{
	return get_ai_ref(this->side_);
}


const std::string& holder::get_ai_algorithm_type() const
{
	return this->ai_algorithm_type_;
}


config& holder::get_ai_memory()
{
	return this->ai_memory_;
}


std::vector<config>& holder::get_ai_parameters()
{
	return this->ai_parameters_;
}


void holder::set_ai_parameters( const std::vector<config>& ai_parameters )
{
	this->ai_parameters_ = ai_parameters;
	DBG_AI_MANAGER << describe_ai() << "AI parameters are set." << std::endl;
}


config& holder::get_ai_effective_parameters()
{
	return this->ai_effective_parameters_;
}


void holder::set_ai_effective_parameters( const config& ai_effective_parameters )
{
	this->ai_effective_parameters_ = ai_effective_parameters;
	DBG_AI_MANAGER << describe_ai() << "AI effective parameters are set." << std::endl;
}


config& holder::get_ai_global_parameters()
{
	return this->ai_global_parameters_;
}


void holder::set_ai_global_parameters( const config& ai_global_parameters )
{
	this->ai_global_parameters_ = ai_global_parameters;
	DBG_AI_MANAGER << describe_ai() << "AI global parameters are set." << std::endl;
}


void holder::set_ai_memory( const config& ai_memory )
{
	this->ai_memory_ = ai_memory;
	DBG_AI_MANAGER << describe_ai() << "AI memory is set." << std::endl;
}


void holder::set_ai_algorithm_type( const std::string& ai_algorithm_type ){
	this->ai_algorithm_type_ = ai_algorithm_type;
	DBG_AI_MANAGER << describe_ai() << "AI algorithm type is set to '"<< ai_algorithm_type_<< "'" << std::endl;
}


const std::string holder::describe_ai()
{
	std::string sidestr;
	//@todo 1.7 extract side naming to separate static function
	if (this->side_ == manager::AI_TEAM_FALLBACK_AI){
		sidestr = "'fallback_side'";
	} else if (this->side_ == manager::AI_TEAM_COMMAND_AI){
		sidestr = "'command_side'";
	} else {
		sidestr = lexical_cast<std::string>(this->side_);
	}
	if (this->ai_!=NULL) {
		return this->ai_->describe_self()+std::string(" for side ")+sidestr+std::string(" : ");
	} else {
		return std::string("[")+this->ai_algorithm_type_+std::string("] (not initialized) for side ")+sidestr+std::string(" : ");
	}
}

bool holder::is_mandate_ok()
{
	DBG_AI_MANAGER << describe_ai() << "AI mandate is ok" << std::endl;
	return true;
}

ai_ptr holder::create_ai( int side )
{
	assert (side > 0);
	assert (default_ai_context_!=NULL);
	//@note: ai_params, contexts, and ai_algorithm_type are supposed to be set before calling init(  );
	return manager::create_transient_ai(ai_algorithm_type_,default_ai_context_);

}

// =======================================================================
// AI COMMAND HISTORY ITEM
// =======================================================================

command_history_item::command_history_item()
	: number_(0), command_()
{

}


command_history_item::command_history_item( int number, const std::string& command )
	: number_(number), command_(command)
{

}


command_history_item::~command_history_item()
{

}


int command_history_item::get_number() const
{
	return this->number_;
}


void command_history_item::set_number( int number )
{
	this->number_ = number;
}

const std::string& command_history_item::get_command() const
{
	return this->command_;
}


void command_history_item::set_command( const std::string& command )
{
	this->command_ = command;
}

// =======================================================================
// LIFECYCLE
// =======================================================================


manager::manager()
{

}


manager::~manager()
{

}


manager::AI_map_of_stacks manager::ai_map_;
game_info *manager::ai_info_;
events::generic_event manager::user_interact_("ai_user_interact");
events::generic_event manager::unit_recruited_("ai_unit_recruited");
events::generic_event manager::unit_moved_("ai_unit_moved");
events::generic_event manager::enemy_attacked_("ai_enemy_attacked");
int manager::last_interact_ = 0;
int manager::num_interact_ = 0;


void manager::set_ai_info(const game_info& i)
{
	if (ai_info_!=NULL){
		clear_ai_info();
	}
	ai_info_ = new game_info(i);
	registry::init();
}


void manager::clear_ai_info(){
	if (ai_info_ != NULL){
		delete ai_info_;
		ai_info_ = NULL;
	}
}

void manager::add_observer( events::observer* event_observer){
	user_interact_.attach_handler(event_observer);
	unit_recruited_.attach_handler(event_observer);
	unit_moved_.attach_handler(event_observer);
	enemy_attacked_.attach_handler(event_observer);
}

void manager::remove_observer(events::observer* event_observer){
	user_interact_.detach_handler(event_observer);
	unit_recruited_.detach_handler(event_observer);
	unit_moved_.detach_handler(event_observer);
	enemy_attacked_.detach_handler(event_observer);
}

void manager::raise_user_interact() {
        const int interact_time = 30;
        const int time_since_interact = SDL_GetTicks() - last_interact_;
        if(time_since_interact < interact_time) {
                return;
        }

	++num_interact_;
        user_interact_.notify_observers();

        last_interact_ = SDL_GetTicks();

}

void manager::raise_unit_recruited() {
	unit_recruited_.notify_observers();
}

void manager::raise_unit_moved() {
	unit_moved_.notify_observers();
}

void manager::raise_enemy_attacked() {
	enemy_attacked_.notify_observers();
}


// =======================================================================
// EVALUATION
// =======================================================================

const std::string manager::evaluate_command( int side, const std::string& str )
{
	//insert new command into history
	history_.push_back(command_history_item(history_item_counter_++,str));

	//prune history - erase 1/2 of it if it grows too large
	if (history_.size()>MAX_HISTORY_SIZE){
		history_.erase(history_.begin(),history_.begin()+MAX_HISTORY_SIZE/2);
		LOG_AI_MANAGER << "AI MANAGER: pruned history" << std::endl;
	}

	if (!should_intercept(str)){
		interface& ai = get_command_ai(side);
		return ai.evaluate(str);
	}

	return internal_evaluate_command(side,str);
}


bool manager::should_intercept( const std::string& str )
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

std::deque< command_history_item > manager::history_;
long manager::history_item_counter_ = 1;

//this is stub code to allow testing of basic 'history', 'repeat-last-command', 'add/remove/replace ai' capabilities.
//yes, it doesn't look nice. but it is usable.
//to be refactored at earliest opportunity
//@todo 1.7 extract to separate class which will use fai or lua parser
const std::string manager::internal_evaluate_command( int side, const std::string& str ){
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
			j++;//this is *reverse* iterator
		}

		return strstream.str();
	};


	std::vector< std::string > cmd = utils::paranthetical_split(str, ' ',"'","'");

	if (cmd.size()==3){
		//!add_ai side file
		if (cmd.at(0)=="!add_ai"){
			int side = lexical_cast<int>(cmd.at(1));
			std::string file = cmd.at(2);
			if (add_ai_for_side_from_file(side,file,false)){
				return std::string("AI MANAGER: added [")+manager::get_active_ai_algorithm_type_for_side(side)+std::string("] AI for side ")+lexical_cast<std::string>(side)+std::string(" from file ")+file;
			} else {
				return std::string("AI MANAGER: failed attempt to add AI for side ")+lexical_cast<std::string>(side)+std::string(" from file ")+file;
			}
		}
		//!replace_ai side file
		if (cmd.at(0)=="!replace_ai"){
			int side = lexical_cast<int>(cmd.at(1));
			std::string file = cmd.at(2);
			if (add_ai_for_side_from_file(side,file,true)){
					return std::string("AI MANAGER: added [")+manager::get_active_ai_algorithm_type_for_side(side)+std::string("] AI for side ")+lexical_cast<std::string>(side)+std::string(" from file ")+file;
			} else {
					return std::string("AI MANAGER: failed attempt to add AI for side ")+lexical_cast<std::string>(side)+std::string(" from file ")+file;
			}
		}

	} else if (cmd.size()==2){
		//!remove_ai side
		if (cmd.at(0)=="!remove_ai"){
			int side = lexical_cast<int>(cmd.at(1));
			remove_ai_for_side(side);
			return std::string("AI MANAGER: made an attempt to remove AI for side ")+lexical_cast<std::string>(side);
		}
		if (cmd.at(0)=="!"){
			//this command should not be recorded in history
			if (!history_.empty()){
				history_.pop_back();
				history_item_counter_--;
			}

			int command = lexical_cast<int>(cmd.at(1));
			std::deque< command_history_item >::reverse_iterator j = history_.rbegin();
			//yes, the iterator could be precisely positioned (since command numbers go 1,2,3,4,..). will do it later.
			while ( (j!=history_.rend()) && (j->get_number()!=command) ){
				j++;// this is *reverse* iterator
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

//@todo 1.7 add error reporting
bool manager::add_ai_for_side_from_file( int side, const std::string& file, bool replace )
{
	config cfg;
	if (!configuration::get_side_config_from_file(file,cfg)){
		ERR_AI_MANAGER << " unable to read [SIDE] config for side "<< side << "from file [" << file <<"]"<< std::endl;
		return false;
	}
	return add_ai_for_side_from_config(side,cfg,replace);
}


bool manager::add_ai_for_side_from_config( int side, const config& cfg, bool replace ){
	config ai_memory;//AI memory
	std::vector<config> ai_parameters;//AI parameters inside [ai] tags. May contain filters
	config global_ai_parameters ;//AI parameters which do not have a filter applied
	const config& default_ai_parameters = configuration::get_default_ai_parameters();//default AI parameters
	std::string ai_algorithm_type;//AI algorithm type
	config effective_ai_parameters;//legacy effective ai parameters

	configuration::parse_side_config(cfg, ai_algorithm_type, global_ai_parameters, ai_parameters, default_ai_parameters, ai_memory, effective_ai_parameters);
	if (replace) {
		remove_ai_for_side (side);
	}
	holder new_holder(side,ai_algorithm_type);
	new_holder.set_ai_effective_parameters(effective_ai_parameters);
	new_holder.set_ai_global_parameters(global_ai_parameters);
	new_holder.set_ai_memory(ai_memory);
	new_holder.set_ai_parameters(ai_parameters);
	std::stack<holder>& ai_stack_for_specific_side = get_or_create_ai_stack_for_side(side);
	ai_stack_for_specific_side.push(new_holder);
	return true;
}


//@todo 1.7 add error reporting
bool manager::add_ai_for_side( int side, const std::string& ai_algorithm_type, bool replace )
{
	if (replace) {
		remove_ai_for_side (side);
	}
	holder new_holder(side,ai_algorithm_type);
	std::stack<holder>& ai_stack_for_specific_side = get_or_create_ai_stack_for_side(side);
	ai_stack_for_specific_side.push(new_holder);
	return true;
}


ai_ptr manager::create_transient_ai( const std::string &ai_algorithm_type, default_ai_context *ai_context )
{
	assert(ai_context!=NULL);

	//to add your own ai, register it in registry,cpp
	ai_factory::factory_map::iterator aii = ai_factory::get_list().find(ai_algorithm_type);
	if (aii == ai_factory::get_list().end()){
		aii = ai_factory::get_list().find("");
		if (aii == ai_factory::get_list().end()){
			throw game::game_error("no default ai set!");
		}
	}
	LOG_AI_MANAGER << "Creating new AI of type [" << ai_algorithm_type << "]"<< std::endl;
	ai_ptr new_ai = aii->second->get_new_instance(*ai_context);
	return new_ai;
}


// =======================================================================
// REMOVE
// =======================================================================

void manager::remove_ai_for_side( int side )
{
	std::stack<holder>& ai_stack_for_specific_side = get_or_create_ai_stack_for_side(side);
	if (!ai_stack_for_specific_side.empty()){
		ai_stack_for_specific_side.pop();
	}
}


void manager::remove_all_ais_for_side( int side )
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

// =======================================================================
// GET active AI parameters
// =======================================================================

const std::vector<config>& manager::get_active_ai_parameters_for_side( int side )
{
	return get_active_ai_holder_for_side(side).get_ai_parameters();
}


const config& manager::get_active_ai_effective_parameters_for_side( int side )
{
	return get_active_ai_holder_for_side(side).get_ai_effective_parameters();
}


const config& manager::get_active_ai_global_parameters_for_side( int side )
{
	return get_active_ai_holder_for_side(side).get_ai_global_parameters();
}


const config& manager::get_active_ai_memory_for_side( int side )
{
	return get_active_ai_holder_for_side(side).get_ai_memory();
}


const std::string& manager::get_active_ai_algorithm_type_for_side( int side )
{
	return get_active_ai_holder_for_side(side).get_ai_algorithm_type();
}


game_info& manager::get_active_ai_info_for_side( int /*side*/ )
{
	return *ai_info_;
}


game_info& manager::get_ai_info()
{
	return *ai_info_;
}

// =======================================================================
// SET active AI parameters
// =======================================================================

void manager::set_active_ai_parameters_for_side( int side, const std::vector<config>& ai_parameters )
{
	get_active_ai_holder_for_side(side).set_ai_parameters(ai_parameters);
}


void manager::set_active_ai_effective_parameters_for_side( int side, const config& ai_parameters )
{
	get_active_ai_holder_for_side(side).set_ai_effective_parameters(ai_parameters);
}


void manager::set_active_ai_global_parameters_for_side( int side, const config& ai_global_parameters )
{
	get_active_ai_holder_for_side(side).set_ai_global_parameters(ai_global_parameters);
}


void manager::set_active_ai_memory_for_side( int side, const config& ai_memory )
{
	get_active_ai_holder_for_side(side).set_ai_memory(ai_memory);
}


void manager::set_active_ai_algorithm_type_for_side( int side, const std::string& ai_algorithm_type )
{
	get_active_ai_holder_for_side(side).set_ai_algorithm_type(ai_algorithm_type);
}


// =======================================================================
// PROXY
// =======================================================================

void manager::play_turn( int side, events::observer* /*event_observer*/ ){
	last_interact_ = 0;
	num_interact_ = 0;
	const int turn_start_time = SDL_GetTicks();
	interface& ai_obj = get_active_ai_for_side(side);
	ai_obj.new_turn();
	ai_obj.play_turn();
	const int turn_end_time= SDL_GetTicks();
	DBG_AI_MANAGER << "side " << side << ": number of user interactions: "<<num_interact_<<std::endl;
	DBG_AI_MANAGER << "side " << side << ": total turn time: "<<turn_end_time - turn_start_time << " ms "<< std::endl;
}


// =======================================================================
// PRIVATE
// =======================================================================
// =======================================================================
// AI STACKS
// =======================================================================
std::stack<holder>& manager::get_or_create_ai_stack_for_side( int side )
{
	AI_map_of_stacks::iterator iter = ai_map_.find(side);
	if (iter!=ai_map_.end()){
		return iter->second;
	}
	return ai_map_.insert(std::pair<int, std::stack<holder> >(side, std::stack<holder>())).first->second;
}

// =======================================================================
// AI HOLDERS
// =======================================================================
holder& manager::get_active_ai_holder_for_side( int side )
{
	std::stack<holder>& ai_stack_for_specific_side = get_or_create_ai_stack_for_side(side);

	if (!ai_stack_for_specific_side.empty()){
		return ai_stack_for_specific_side.top();
	} else if (side==manager::AI_TEAM_COMMAND_AI){
		return get_command_ai_holder( side );
	} else {
		return get_fallback_ai_holder( side );
	}

}

holder& manager::get_command_ai_holder( int /*side*/ )
{
	holder& ai_holder = get_or_create_active_ai_holder_for_side_without_fallback(manager::AI_TEAM_COMMAND_AI,AI_TYPE_FORMULA_AI);
	return ai_holder;
}

holder& manager::get_fallback_ai_holder( int /*side*/ )
{
	holder& ai_holder = get_or_create_active_ai_holder_for_side_without_fallback(manager::AI_TEAM_FALLBACK_AI,AI_TYPE_IDLE_AI);
	return ai_holder;
}

holder& manager::get_or_create_active_ai_holder_for_side_without_fallback(int side, const std::string& ai_algorithm_type)
{
	std::stack<holder>& ai_stack_for_specific_side = get_or_create_ai_stack_for_side(side);

	if (!ai_stack_for_specific_side.empty()){
		return ai_stack_for_specific_side.top();
	} else {
		holder new_holder(side, ai_algorithm_type);
		ai_stack_for_specific_side.push(new_holder);
		return ai_stack_for_specific_side.top();
	}

}


// =======================================================================
// AI POINTERS
// =======================================================================

interface& manager::get_active_ai_for_side( int side )
{
	return get_active_ai_holder_for_side(side).get_ai_ref();
}


interface& manager::get_or_create_active_ai_for_side_without_fallback( int side, const std::string& ai_algorithm_type )
{
	holder& ai_holder = get_or_create_active_ai_holder_for_side_without_fallback(side,ai_algorithm_type);
	return ai_holder.get_ai_ref();
}

interface& manager::get_command_ai( int side )
{
	holder& ai_holder = get_command_ai_holder(side);
	interface& ai = ai_holder.get_ai_ref(side);
	return ai;
}

interface& manager::get_fallback_ai( int side )
{
	holder& ai_holder = get_fallback_ai_holder(side);
	interface& ai = ai_holder.get_ai_ref(side);
	return ai;
}

// =======================================================================
// MISC
// =======================================================================

} //end of namespace ai
