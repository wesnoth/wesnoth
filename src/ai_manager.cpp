/* $Id: ai_manager.cpp   $ */
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
 * Managing the AI lifecycle
 * @file ai_manager.cpp
 */

//@todo: shorten this list of includes, for this list is copypasted from ai.cpp
#include "ai.hpp"
#include "ai2.hpp"
#include "ai_configuration.hpp"
#include "ai_manager.hpp"
#include "ai_dfool.hpp"
#include "formula_ai.hpp"
#include "game_events.hpp"
#include "game_preferences.hpp"
#include "log.hpp"
#include "replay.hpp"
#include "serialization/string_utils.hpp"
#include "statistics.hpp"

#include <map>
#include <stack>
#include <vector>

const std::string ai_manager::AI_TYPE_SAMPLE_AI = "sample_ai";
const std::string ai_manager::AI_TYPE_IDLE_AI = "idle_ai";
const std::string ai_manager::AI_TYPE_FORMULA_AI = "formula_ai";
const std::string ai_manager::AI_TYPE_DFOOL_AI = "dfool_ai";
const std::string ai_manager::AI_TYPE_AI2 = "ai2";
const std::string ai_manager::AI_TYPE_DEFAULT = "default";


#define DBG_AI_MANAGER LOG_STREAM(debug, ai_manager)
#define LOG_AI_MANAGER LOG_STREAM(info, ai_manager)
#define WRN_AI_MANAGER LOG_STREAM(warn, ai_manager)
#define ERR_AI_MANAGER LOG_STREAM(err, ai_manager)

ai_holder::ai_holder( int team, const std::string& ai_algorithm_type )
	: ai_(NULL), ai_algorithm_type_(ai_algorithm_type), ai_effective_parameters_(),  ai_global_parameters_(), ai_memory_(), ai_parameters_(), team_(team)
{
	DBG_AI_MANAGER << describe_ai() << "Preparing new AI holder" << std::endl;
}


void ai_holder::init( ai_interface::info& i )
{
	LOG_AI_MANAGER << describe_ai() << "Preparing to create new managed master AI" << std::endl;
	this->ai_ = create_ai(i);
	if (this->ai_ == NULL) {
		ERR_AI_MANAGER << describe_ai()<<"AI lazy initialization error!" << std::endl;
	}

}


ai_holder::~ai_holder()
{
	if (this->ai_ != NULL) {
		LOG_AI_MANAGER << describe_ai() << "Managed AI will be deleted" << std::endl;
		delete this->ai_;
	}
}


ai_interface& ai_holder::get_ai_ref( ai_interface::info& i )
{
	if (this->ai_ == NULL) {
		this->init(i);
	}
	assert(this->ai_ != NULL);

	return *this->ai_;
}


const std::string& ai_holder::get_ai_algorithm_type() const
{
	return this->ai_algorithm_type_;
}


config& ai_holder::get_ai_memory()
{
	return this->ai_memory_;
}


std::vector<config>& ai_holder::get_ai_parameters()
{
	return this->ai_parameters_;
}


void ai_holder::set_ai_parameters( const std::vector<config>& ai_parameters )
{
	this->ai_parameters_ = ai_parameters;
	DBG_AI_MANAGER << describe_ai() << "AI parameters are set." << std::endl;
}


config& ai_holder::get_ai_effective_parameters()
{
	return this->ai_effective_parameters_;
}


void ai_holder::set_ai_effective_parameters( const config& ai_effective_parameters )
{
	this->ai_effective_parameters_ = ai_effective_parameters;
	DBG_AI_MANAGER << describe_ai() << "AI effective parameters are set." << std::endl;
}


config& ai_holder::get_ai_global_parameters()
{
	return this->ai_global_parameters_;
}


void ai_holder::set_ai_global_parameters( const config& ai_global_parameters )
{
	this->ai_global_parameters_ = ai_global_parameters;
	DBG_AI_MANAGER << describe_ai() << "AI global parameters are set." << std::endl;
}


void ai_holder::set_ai_memory( const config& ai_memory )
{
	this->ai_memory_ = ai_memory;
	DBG_AI_MANAGER << describe_ai() << "AI memory is set." << std::endl;
}


void ai_holder::set_ai_algorithm_type( const std::string& ai_algorithm_type ){
	this->ai_algorithm_type_ = ai_algorithm_type;
	DBG_AI_MANAGER << describe_ai() << "AI algorithm type is set to '"<< ai_algorithm_type_<< "'" << std::endl;
}


const std::string ai_holder::describe_ai()
{
	std::string teamstr;
	//@todo: extract team naming to separate static function
	if (this->team_ == ai_manager::AI_TEAM_FALLBACK_AI){
		teamstr = "'fallback_team'";
	} else if (this->team_ == ai_manager::AI_TEAM_COMMAND_AI){
		teamstr = "'command_team'";
	} else {
		teamstr = lexical_cast<std::string>(this->team_);
	}
	if (this->ai_!=NULL) {
		return this->ai_->describe_self()+std::string(" for team ")+teamstr+std::string(" : ");
	} else {
		return std::string("[")+this->ai_algorithm_type_+std::string("] (not initialized) for team ")+teamstr+std::string(" : ");
	}
}

bool ai_holder::is_mandate_ok( ai_interface::info &/*i*/ )
{
	DBG_AI_MANAGER << describe_ai() << "AI mandate is ok" << std::endl;
	return true;
}

ai_interface* ai_holder::create_ai( ai_interface::info& i )
{
	//@note: ai_params and ai_algorithm_type are supposed to be set before calling init(  );
	return ai_manager::create_transient_ai(ai_algorithm_type_,i,team_,true);

}

// =======================================================================
// AI COMMAND HISTORY ITEM
// =======================================================================

ai_command_history_item::ai_command_history_item()
	: number_(0), command_()
{

}


ai_command_history_item::ai_command_history_item( int number, const std::string& command )
	: number_(number), command_(command)
{

}


ai_command_history_item::~ai_command_history_item()
{

}


int ai_command_history_item::get_number() const
{
	return this->number_;
}


void ai_command_history_item::set_number( int number )
{
	this->number_ = number;
}

const std::string& ai_command_history_item::get_command() const
{
	return this->command_;
}


void ai_command_history_item::set_command( const std::string& command )
{
	this->command_ = command;
}

// =======================================================================
// LIFECYCLE
// =======================================================================


ai_manager::ai_manager()
{

}


ai_manager::~ai_manager()
{

}


ai_manager::AI_map_of_stacks ai_manager::ai_map_;


// =======================================================================
// EVALUATION
// =======================================================================

const std::string ai_manager::evaluate_command( ai_interface::info& i, int side, const std::string& str )
{
	//insert new command into history
	history_.push_back(ai_command_history_item(history_item_counter++,str));

	//prune history - erase 1/2 of it if it grows too large
	if (history_.size()>MAX_HISTORY_SIZE){
		history_.erase(history_.begin(),history_.begin()+MAX_HISTORY_SIZE/2);
		LOG_AI_MANAGER << "AI MANAGER: pruned history" << std::endl;
	}

	if (!should_intercept(str)){
		ai_interface& ai = get_command_ai(side,i);
		return ai.evaluate(str);
	}

	return internal_evaluate_command(i,side,str);
}


bool ai_manager::should_intercept( const std::string& str )
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

std::deque< ai_command_history_item > ai_manager::history_;
long ai_manager::history_item_counter = 1;

//this is stub code to allow testing of basic 'history', 'repeat-last-command', 'add/remove/replace ai' capabilities.
//yes, it doesn't look nice. but it is usable.
//to be refactored at earliest opportunity
//@todo: extract to separate class which will use fai or lua parser
const std::string ai_manager::internal_evaluate_command( ai_interface::info& i, int side, const std::string& str ){
	const int MAX_HISTORY_VISIBLE = 30;

	//repeat last command
	if (str=="!") {
			//this command should not be recorded in history
			if (!history_.empty()){
				history_.pop_back();
				history_item_counter--;
			}

			if (history_.empty()){
				return "AI MANAGER: empty history";
			}
			return evaluate_command(i,side, history_.back().get_command());//no infinite loop since '!' commands are not present in history
	};
	//show last command
	if (str=="?") {
		//this command should not be recorded in history
		if (!history_.empty()){
			history_.pop_back();
			history_item_counter--;
		}

		if (history_.empty()){
			return "AI MANAGER: History is empty";
		}

		int n = std::min<int>( MAX_HISTORY_VISIBLE, history_.size() );
		std::stringstream strstream;
		strstream << "AI MANAGER: History - last "<< n <<" commands:\n";
		std::deque< ai_command_history_item >::reverse_iterator j = history_.rbegin();

		for (int cmd_id=n; cmd_id>0; --cmd_id){
			strstream << j->get_number() << "    :" << j->get_command() << '\n';
			j++;//this is *reverse* iterator
		}

		return strstream.str();
	};


	std::vector< std::string > cmd = utils::paranthetical_split(str, ' ',"'","'");

	if (cmd.size()==3){
		//!add_ai team file
		if (cmd.at(0)=="!add_ai"){
			int team = lexical_cast<int>(cmd.at(1));
			std::string file = cmd.at(2);
			if (add_ai_for_team_from_file(team,file)){
				return std::string("AI MANAGER: added [")+ai_manager::get_active_ai_algorithm_type_for_team(team)+std::string("] AI for team ")+lexical_cast<std::string>(team)+std::string(" from file ")+file;
			} else {
				return std::string("AI MANAGER: failed attempt to add AI for team ")+lexical_cast<std::string>(team)+std::string(" from file ")+file;
			}
		}
		//!replace_ai team file
		if (cmd.at(0)=="!replace_ai"){
			int team = lexical_cast<int>(cmd.at(1));
			std::string file = cmd.at(2);
			remove_ai_for_team(team);
			if (add_ai_for_team_from_file(team,file)){
					return std::string("AI MANAGER: added [")+ai_manager::get_active_ai_algorithm_type_for_team(team)+std::string("] AI for team ")+lexical_cast<std::string>(team)+std::string(" from file ")+file;
			} else {
					return std::string("AI MANAGER: failed attempt to add AI for team ")+lexical_cast<std::string>(team)+std::string(" from file ")+file;
			}
		}

	} else if (cmd.size()==2){
		//!remove_ai team
		if (cmd.at(0)=="!remove_ai"){
			int team = lexical_cast<int>(cmd.at(1));
			remove_ai_for_team(team);
			return std::string("AI MANAGER: made an attempt to remove AI for team ")+lexical_cast<std::string>(team);
		}
		if (cmd.at(0)=="!"){
			//this command should not be recorded in history
			if (!history_.empty()){
				history_.pop_back();
				history_item_counter--;
			}

			int command = lexical_cast<int>(cmd.at(1));
			std::deque< ai_command_history_item >::reverse_iterator j = history_.rbegin();
			//yes, the iterator could be precisely positioned (since command numbers go 1,2,3,4,..). will do it later.
			while ( (j!=history_.rend()) && (j->get_number()!=command) ){
				j++;// this is *reverse* iterator
			}
			if (j!=history_.rend()){
				return evaluate_command(i,side,j->get_command());//no infinite loop since '!' commands are not present in history
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
				"!add_ai TEAM FILE    - add a AI to team (0 - command AI, N - AI for team #N) from file\n"
				"!remove_ai TEAM    - remove AI from team (0 - command AI, N - AI for team #N)\n"
				"!replace_ai TEAM FILE    - replace AI of team (0 - command AI, N - AI for team #N) from file\n"
				"!help    - show this help message";
		}
	}


	return "AI MANAGER: nothing to do";
}

// =======================================================================
// ADD, CREATE AIs, OR LIST AI TYPES
// =======================================================================

//@todo: add error reporting
bool ai_manager::add_ai_for_team_from_file( int team, const std::string& file )
{
	config cfg;
	if (!ai_configuration::get_side_config_from_file(file,cfg)){
		ERR_AI_MANAGER << " unable to read [SIDE] config for team "<< team << "from file [" << file <<"]"<< std::endl;
		return false;
	}
	config ai_memory;//AI memory
	std::vector<config> ai_parameters;//AI parameters inside [ai] tags. May contain filters
	config global_ai_parameters ;//AI parameters which do not have a filter applied
	const config& default_ai_parameters = ai_configuration::get_default_ai_parameters();//default AI parameters
	std::string ai_algorithm_type;//AI algorithm type
	config effective_ai_parameters;//legacy effective ai parameters

	ai_configuration::parse_side_config(cfg, ai_algorithm_type, global_ai_parameters, ai_parameters, default_ai_parameters, ai_memory, effective_ai_parameters);

	add_ai_for_team(team,ai_algorithm_type);
	set_active_ai_effective_parameters_for_team(team,effective_ai_parameters);
	set_active_ai_global_parameters_for_team(team,global_ai_parameters);
	set_active_ai_memory_for_team(team,ai_memory);
	set_active_ai_parameters_for_team(team,ai_parameters);
	return true;
}


//@todo: add error reporting
bool ai_manager::add_ai_for_team( int team, const std::string& ai_algorithm_type)
{

	ai_holder new_ai_holder(team,ai_algorithm_type);
	std::stack<ai_holder>& ai_stack_for_specific_team = get_or_create_ai_stack_for_team(team);
	ai_stack_for_specific_team.push(new_ai_holder);
	return true;
}


ai_interface* ai_manager::create_transient_ai( const std::string& ai_algorithm_type, ai_interface::info& i, int side, bool master )
{
	//@todo: modify this code to use a 'factory lookup' pattern -
	//a singleton which holds a map<string,ai_factory> of all functors which can create AIs.
	//this will allow individual AI implementations to 'register' themselves.

	//: To add an AI of your own, put
	//	if(ai_algorithm_type == "my_ai") {
	//		LOG_AI_MANAGER << "Creating new AI of type [" << "my_ai" << "]"<< std::endl;
	//		return new my_ai(i,side,master);
	//	}
	// at the top of this function

	//if(ai_algorithm_type == ai_manager::AI_TYPE_SAMPLE_AI) {
	//  LOG_AI_MANAGER << "Creating new AI of type [" << ai_manager::AI_TYPE_IDLE_AI << "]"<< std::endl;
	//	return new sample_ai(i,side,master);
	//}

	if(ai_algorithm_type == ai_manager::AI_TYPE_IDLE_AI) {
		LOG_AI_MANAGER << "Creating new AI of type [" << ai_manager::AI_TYPE_IDLE_AI << "]"<< std::endl;
		return new idle_ai(i,side,master);
	}

	if(ai_algorithm_type == ai_manager::AI_TYPE_FORMULA_AI) {
		LOG_AI_MANAGER << "Creating new AI of type [" << ai_manager::AI_TYPE_FORMULA_AI << "]"<< std::endl;
		return new formula_ai(i,side,master);
	}

	if(ai_algorithm_type == ai_manager::AI_TYPE_DFOOL_AI) {
		LOG_AI_MANAGER << "Creating new AI of type [" << ai_manager::AI_TYPE_DFOOL_AI << "]"<< std::endl;
		return new dfool::dfool_ai(i,side,master);
	}

	if(ai_algorithm_type == ai_manager::AI_TYPE_AI2) {
		LOG_AI_MANAGER << "Creating new AI of type [" << ai_manager::AI_TYPE_AI2 << "]"<< std::endl;
		return new ai2(i,side,master);
	}

	if (!ai_algorithm_type.empty() && ai_algorithm_type != ai_manager::AI_TYPE_DEFAULT) {
		ERR_AI_MANAGER << "AI not found: [" << ai_algorithm_type << "]. Using default instead.\n";
	}

	LOG_AI_MANAGER  << "Creating new AI of type [" << ai_manager::AI_TYPE_DEFAULT << "]"<< std::endl;
	return new ai(i,side,master);
}

std::vector<std::string> ai_manager::get_available_ais()
{
	std::vector<std::string> ais;
	ais.push_back("default");
	return ais;
}




// =======================================================================
// REMOVE
// =======================================================================

void ai_manager::remove_ai_for_team( int team )
{
	std::stack<ai_holder>& ai_stack_for_specific_team = get_or_create_ai_stack_for_team(team);
	if (!ai_stack_for_specific_team.empty()){
		ai_stack_for_specific_team.pop();
	}
}


void ai_manager::remove_all_ais_for_team( int team )
{
	std::stack<ai_holder>& ai_stack_for_specific_team = get_or_create_ai_stack_for_team(team);

	//clear the stack. std::stack doesn't have a '.clear()' method to do it
	while (!ai_stack_for_specific_team.empty()){
			ai_stack_for_specific_team.pop();
	}
}


void ai_manager::clear_ais()
{
	ai_map_.clear();
}

// =======================================================================
// GET active AI parameters
// =======================================================================

const std::vector<config>& ai_manager::get_active_ai_parameters_for_team( int team )
{
	return get_active_ai_holder_for_team(team).get_ai_parameters();
}


const config& ai_manager::get_active_ai_effective_parameters_for_team( int team )
{
	return get_active_ai_holder_for_team(team).get_ai_effective_parameters();
}


const config& ai_manager::get_active_ai_global_parameters_for_team( int team )
{
	return get_active_ai_holder_for_team(team).get_ai_global_parameters();
}


const config& ai_manager::get_active_ai_memory_for_team( int team )
{
	return get_active_ai_holder_for_team(team).get_ai_memory();
}


const std::string& ai_manager::get_active_ai_algorithm_type_for_team( int team )
{
	return get_active_ai_holder_for_team(team).get_ai_algorithm_type();
}


// =======================================================================
// SET active AI parameters
// =======================================================================

void ai_manager::set_active_ai_parameters_for_team( int team, const std::vector<config>& ai_parameters )
{
	get_active_ai_holder_for_team(team).set_ai_parameters(ai_parameters);
}


void ai_manager::set_active_ai_effective_parameters_for_team( int team, const config& ai_parameters )
{
	get_active_ai_holder_for_team(team).set_ai_effective_parameters(ai_parameters);
}


void ai_manager::set_active_ai_global_parameters_for_team( int team, const config& ai_global_parameters )
{
	get_active_ai_holder_for_team(team).set_ai_global_parameters(ai_global_parameters);
}


void ai_manager::set_active_ai_memory_for_team( int team, const config& ai_memory )
{
	get_active_ai_holder_for_team(team).set_ai_memory(ai_memory);
}


void ai_manager::set_active_ai_algorithm_type_for_team( int team, const std::string& ai_algorithm_type )
{
	get_active_ai_holder_for_team(team).set_ai_algorithm_type(ai_algorithm_type);
}


// =======================================================================
// PROXY
// =======================================================================

void ai_manager::play_turn( int team, ai_interface::info& i, events::observer* event_observer ){
	ai_interface& ai_obj = get_active_ai_for_team(team, i);
	ai_obj.user_interact().attach_handler(event_observer);
	ai_obj.unit_recruited().attach_handler(event_observer);
	ai_obj.unit_moved().attach_handler(event_observer);
	ai_obj.enemy_attacked().attach_handler(event_observer);
	ai_obj.play_turn();
}


// =======================================================================
// PRIVATE
// =======================================================================
// =======================================================================
// AI STACKS
// =======================================================================
std::stack<ai_holder>& ai_manager::get_or_create_ai_stack_for_team( int team )
{
	AI_map_of_stacks::iterator iter = ai_map_.find(team);
	if (iter!=ai_map_.end()){
		return iter->second;
	}
	return ai_map_.insert(std::pair<int, std::stack<ai_holder> >(team, std::stack<ai_holder>())).first->second;
}

// =======================================================================
// AI HOLDERS
// =======================================================================
ai_holder& ai_manager::get_active_ai_holder_for_team( int team )
{
	std::stack<ai_holder>& ai_stack_for_specific_team = get_or_create_ai_stack_for_team(team);

	if (!ai_stack_for_specific_team.empty()){
		return ai_stack_for_specific_team.top();
	} else if (team==ai_manager::AI_TEAM_COMMAND_AI){
		return get_command_ai_holder( team );
	} else {
		return get_fallback_ai_holder( team );
	}

}

ai_holder& ai_manager::get_command_ai_holder( int /*team*/ )
{
	ai_holder& ai_holder = get_or_create_active_ai_holder_for_team_without_fallback(ai_manager::AI_TEAM_COMMAND_AI,AI_TYPE_FORMULA_AI);
	return ai_holder;
}

ai_holder& ai_manager::get_fallback_ai_holder( int /*team*/ )
{
	ai_holder& ai_holder = get_or_create_active_ai_holder_for_team_without_fallback(ai_manager::AI_TEAM_FALLBACK_AI,AI_TYPE_IDLE_AI);
	return ai_holder;
}

ai_holder& ai_manager::get_or_create_active_ai_holder_for_team_without_fallback(int team, const std::string& ai_algorithm_type)
{
	std::stack<ai_holder>& ai_stack_for_specific_team = get_or_create_ai_stack_for_team(team);

	if (!ai_stack_for_specific_team.empty()){
		return ai_stack_for_specific_team.top();
	} else {
		ai_holder new_ai_holder(team, ai_algorithm_type);
		ai_stack_for_specific_team.push(new_ai_holder);
		return ai_stack_for_specific_team.top();
	}

}


// =======================================================================
// AI POINTERS
// =======================================================================

ai_interface& ai_manager::get_active_ai_for_team( int team, ai_interface::info& i )
{
	return get_active_ai_holder_for_team(team).get_ai_ref(i);
}


ai_interface& ai_manager::get_or_create_active_ai_for_team_without_fallback( int team, ai_interface::info& i, const std::string& ai_algorithm_type )
{
	ai_holder& ai_holder = get_or_create_active_ai_holder_for_team_without_fallback(team,ai_algorithm_type);
	return ai_holder.get_ai_ref(i);
}

ai_interface& ai_manager::get_command_ai( int team, ai_interface::info& i )
{
	ai_holder& ai_holder = get_command_ai_holder(team);
	ai_interface& ai = ai_holder.get_ai_ref(i);
	ai.set_team(team);
	return ai;
}

ai_interface& ai_manager::get_fallback_ai( int team, ai_interface::info& i )
{
	ai_holder& ai_holder = get_fallback_ai_holder(team);
	ai_interface& ai = ai_holder.get_ai_ref(i);
	ai.set_team(team);
	return ai;
}

// =======================================================================
// MISC
// =======================================================================

