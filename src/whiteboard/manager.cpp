/*
 Copyright (C) 2010 - 2018 by Gabriel Morin <gabrielmorin (at) gmail (dot) com>
 Part of the Battle for Wesnoth Project http://www.wesnoth.org

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY.

 See the COPYING file for more details.
 */

/**
 * @file
 */

#include "whiteboard/manager.hpp"

#include "whiteboard/action.hpp"
#include "whiteboard/highlighter.hpp"
#include "whiteboard/mapbuilder.hpp"
#include "whiteboard/move.hpp"
#include "whiteboard/attack.hpp"
#include "whiteboard/recall.hpp"
#include "whiteboard/recruit.hpp"
#include "whiteboard/side_actions.hpp"
#include "whiteboard/utility.hpp"

#include "actions/create.hpp"
#include "actions/undo.hpp"
#include "arrow.hpp"
#include "chat_events.hpp"
#include "fake_unit_manager.hpp"
#include "fake_unit_ptr.hpp"
#include "formula/string_utils.hpp"
#include "game_board.hpp"
#include "preferences/game.hpp"
#include "game_state.hpp"
#include "gettext.hpp"
#include "gui/dialogs/simple_item_selector.hpp"
#include "key.hpp"
#include "pathfind/pathfind.hpp"
#include "play_controller.hpp"
#include "resources.hpp"
#include "team.hpp"
#include "units/unit.hpp"
#include "units/animation_component.hpp"
#include "units/udisplay.hpp"

#include "utils/functional.hpp"

#include <sstream>

namespace wb {

manager::manager():
		active_(false),
		inverted_behavior_(false),
		self_activate_once_(true),
#if 0
		print_help_once_(true),
#endif
		wait_for_side_init_(true),
		planned_unit_map_active_(false),
		executing_actions_(false),
		executing_all_actions_(false),
		preparing_to_end_turn_(false),
		gamestate_mutated_(false),
		activation_state_lock_(new bool),
		unit_map_lock_(new bool),
		mapbuilder_(),
		highlighter_(),
		route_(),
		move_arrows_(),
		fake_units_(),
		temp_move_unit_underlying_id_(0),
		key_poller_(new CKey),
		hidden_unit_hexes_(),
		net_buffer_(resources::gameboard->teams().size()),
		team_plans_hidden_(resources::gameboard->teams().size()),
		units_owning_moves_()
{
	if(preferences::hide_whiteboard()) {
		team_plans_hidden_.flip();
	}
	LOG_WB << "Manager initialized.\n";
}

manager::~manager()
{
	LOG_WB << "Manager destroyed.\n";
}

//Used for chat-spamming debug info
#if 0
static void print_to_chat(const std::string& title, const std::string& message)
{
	display::get_singleton()->add_chat_message(time(nullptr), title, 0, message,
			events::chat_handler::MESSAGE_PRIVATE, false);
}
#endif

void manager::print_help_once()
{
#if 0
	if (!print_help_once_)
		return;
	else
		print_help_once_ = false;

	print_to_chat("whiteboard", std::string("Type :wb to activate/deactivate planning mode.")
		+ "  Hold TAB to temporarily deactivate/activate it.");
	std::stringstream hotkeys;
	const hotkey::hotkey_item& hk_execute = hotkey::get_hotkey(hotkey::HOTKEY_WB_EXECUTE_ACTION);
	if(!hk_execute.null()) {
		//print_to_chat("[execute action]", "'" + hk_execute.get_name() + "'");
		hotkeys << "Execute: " << hk_execute.get_name() << ", ";
	}
	const hotkey::hotkey_item& hk_execute_all = hotkey::get_hotkey(hotkey::HOTKEY_WB_EXECUTE_ALL_ACTIONS);
	if(!hk_execute_all.null()) {
		//print_to_chat("[execute action]", "'" + hk_execute_all.get_name() + "'");
		hotkeys << "Execute all: " << hk_execute_all.get_name() << ", ";
	}
	const hotkey::hotkey_item& hk_delete = hotkey::get_hotkey(hotkey::HOTKEY_WB_DELETE_ACTION);
	if(!hk_delete.null()) {
		//print_to_chat("[delete action]", "'" + hk_delete.get_name() + "'");
		hotkeys << "Delete: " << hk_delete.get_name() << ", ";
	}
	const hotkey::hotkey_item& hk_bump_up = hotkey::get_hotkey(hotkey::HOTKEY_WB_BUMP_UP_ACTION);
	if(!hk_bump_up.null()) {
		//print_to_chat("[move action earlier in queue]", "'" + hk_bump_up.get_name() + "'");
		hotkeys << "Move earlier: " << hk_bump_up.get_name() << ", ";
	}
	const hotkey::hotkey_item& hk_bump_down = hotkey::get_hotkey(hotkey::HOTKEY_WB_BUMP_DOWN_ACTION);
	if(!hk_bump_down.null()) {
		//print_to_chat("[move action later in queue]", "'" + hk_bump_down.get_name() + "'");
		hotkeys << "Move later: " << hk_bump_down.get_name() << ", ";
	}
	print_to_chat("HOTKEYS:", hotkeys.str() + "\n");
#endif
}

bool manager::can_modify_game_state() const
{
	if(wait_for_side_init_
					|| resources::gameboard == nullptr
					|| executing_actions_
					|| resources::gameboard->is_observer()
					|| resources::controller->is_linger_mode())
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool manager::can_activate() const
{
	//any more than one reference means a lock on whiteboard state was requested
	if(!activation_state_lock_.unique())
		return false;

	return can_modify_game_state();
}

void manager::set_active(bool active)
{
	if(!can_activate())
	{
		active_ = false;
		LOG_WB << "Whiteboard can't be activated now.\n";
	}
	else if (active != active_)
	{
		active_ = active;
		erase_temp_move();

		if (active_)
		{
			if(should_clear_undo())
				resources::undo_stack->clear();
			validate_viewer_actions();
			LOG_WB << "Whiteboard activated! " << *viewer_actions() << "\n";
			create_temp_move();
		} else {
			LOG_WB << "Whiteboard deactivated!\n";
		}
	}
}

void manager::set_invert_behavior(bool invert)
{
	//any more than one reference means a lock on whiteboard state was requested
	if(!activation_state_lock_.unique())
		return;

	bool block_whiteboard_activation = false;
	if(!can_activate())
	{
		 block_whiteboard_activation = true;
	}

	if (invert)
	{
		if (!inverted_behavior_)
		{
			if (active_)
			{
				DBG_WB << "Whiteboard deactivated temporarily.\n";
				inverted_behavior_ = true;
				set_active(false);
			}
			else if (!block_whiteboard_activation)
			{
				DBG_WB << "Whiteboard activated temporarily.\n";
				inverted_behavior_ = true;
				set_active(true);
			}
		}
	}
	else
	{
		if (inverted_behavior_)
		{
			if (active_)
			{
				DBG_WB << "Whiteboard set back to deactivated status.\n";
				inverted_behavior_ = false;
				set_active(false);
			}
			else if (!block_whiteboard_activation)
			{
				DBG_WB << "Whiteboard set back to activated status.\n";
				inverted_behavior_ = false;
				set_active(true);
			}
		}
	}
}

bool manager::can_enable_execution_hotkeys() const
{
	return can_enable_modifier_hotkeys() && viewer_side() == resources::controller->current_side()
			&& viewer_actions()->turn_size(0) > 0;
}

bool manager::can_enable_modifier_hotkeys() const
{
	return can_modify_game_state() && !viewer_actions()->empty();
}

bool manager::can_enable_reorder_hotkeys() const
{
	return can_enable_modifier_hotkeys() && highlighter_ && highlighter_->get_bump_target();
}

bool manager::allow_leader_to_move(const unit& leader) const
{
	if(!has_actions())
		return true;

	//Look for another leader on another keep in the same castle
	{ wb::future_map future; // start planned unit map scope
		if(!has_planned_unit_map()) {
			WRN_WB << "Unable to build future map to determine whether leader's allowed to move." << std::endl;
		}
		if(find_backup_leader(leader))
			return true;
	} // end planned unit map scope

	if(viewer_actions()->empty()) {
		return true;
	}

	//Look for planned recruits that depend on this leader
	for(action_const_ptr action : *viewer_actions())
	{
		recruit_const_ptr recruit = std::dynamic_pointer_cast<class recruit const>(action);
		recall_const_ptr recall = std::dynamic_pointer_cast<class recall const>(action);
		if(recruit || recall)
		{
			map_location const target_hex = recruit?recruit->get_recruit_hex():recall->get_recall_hex();
			if (dynamic_cast<game_state&>(*resources::filter_con).can_recruit_on(leader, target_hex))
				return false;
		}
	}
	return true;
}

void manager::on_init_side()
{
	//Turn should never start with action auto-execution already enabled!
	assert(!executing_all_actions_ && !executing_actions_);

	update_plan_hiding(); /* validates actions */
	wait_for_side_init_ = false;
	LOG_WB << "on_init_side()\n";

	if (self_activate_once_ && preferences::enable_whiteboard_mode_on_start())
	{
		self_activate_once_ = false;
		set_active(true);
	}
}

void manager::on_finish_side_turn(int side)
{
	preparing_to_end_turn_ = false;
	wait_for_side_init_ = true;
	if(side == viewer_side() && !viewer_actions()->empty()) {
		viewer_actions()->synced_turn_shift();
	}
	highlighter_.reset();
	erase_temp_move();
	LOG_WB << "on_finish_side_turn()\n";
}

void manager::pre_delete_action(action_ptr)
{
}

void manager::post_delete_action(action_ptr action)
{
	// The fake unit representing the destination of a chain of planned moves should have the regular animation.
	// If the last remaining action of the unit that owned this move is a move as well,
	// adjust its appearance accordingly.

	side_actions_ptr side_actions = resources::gameboard->teams().at(action->team_index()).get_side_actions();

	unit_ptr actor = action->get_unit();
	if(actor) { // The unit might have died following the execution of an attack
		side_actions::iterator action_it = side_actions->find_last_action_of(*actor);
		if(action_it != side_actions->end()) {
			move_ptr move = std::dynamic_pointer_cast<class move>(*action_it);
			if(move && move->get_fake_unit()) {
				move->get_fake_unit()->anim_comp().set_standing(true);
			}
		}
	}
}

static void hide_all_plans()
{
	for(team& t : resources::gameboard->teams()){
		t.get_side_actions()->hide();
	}
}

/* private */
void manager::update_plan_hiding(size_t team_index)
{
	//We don't control the "viewing" side ... we're probably an observer
	if(!resources::gameboard->teams().at(team_index).is_local_human())
		hide_all_plans();
	else // normal circumstance
	{
		for(team& t : resources::gameboard->teams())
		{
			//make sure only appropriate teams are hidden
			if(!t.is_network_human())
				team_plans_hidden_[t.side()-1] = false;

			if(t.is_enemy(team_index+1) || team_plans_hidden_[t.side()-1])
				t.get_side_actions()->hide();
			else
				t.get_side_actions()->show();
		}
	}
	validate_viewer_actions();
}
void manager::update_plan_hiding()
	{update_plan_hiding(viewer_team());}

void manager::on_viewer_change(size_t team_index)
{
	if(!wait_for_side_init_)
		update_plan_hiding(team_index);
}

void manager::on_change_controller(int side, const team& t)
{
	wb::side_actions& sa = *t.get_side_actions();
	if(t.is_local_human()) // we own this side now
	{
		//tell everyone to clear this side's actions -- we're starting anew
		resources::whiteboard->queue_net_cmd(sa.team_index(),sa.make_net_cmd_clear());
		sa.clear();
		//refresh the hidden_ attribute of every team's side_actions
		update_plan_hiding();
	}
	else if(t.is_local_ai() || t.is_network_ai()) // no one owns this side anymore
		sa.clear(); // clear its plans away -- the ai doesn't plan ... yet
	else if(t.is_network()) // Another client is taking control of the side
	{
		if(side==viewer_side()) // They're taking OUR side away!
			hide_all_plans(); // give up knowledge of everyone's plans, in case we became an observer

		//tell them our plans -- they may not have received them up to this point
		size_t num_teams = resources::gameboard->teams().size();
		for(size_t i=0; i<num_teams; ++i)
		{
			team& local_team = resources::gameboard->teams().at(i);
			if(local_team.is_local_human() && !local_team.is_enemy(side))
				resources::whiteboard->queue_net_cmd(i,local_team.get_side_actions()->make_net_cmd_refresh());
		}
	}
}

void manager::on_kill_unit()
{
	if(highlighter_ != nullptr) {
		highlighter_->set_selection_candidate(unit_ptr());
	}
}

bool manager::current_side_has_actions()
{
	if(current_side_actions()->empty()) {
		return false;
	}

	side_actions::range_t range = current_side_actions()->iter_turn(0);
	return range.first != range.second; //non-empty range
}

void manager::validate_viewer_actions()
{
	LOG_WB << "'gamestate_mutated_' flag dirty, validating actions.\n";
	gamestate_mutated_ = false;
	if(has_planned_unit_map()) {
		real_map();
	} else {
		future_map();
	}
}

//helper fcn
static void draw_numbers(const map_location& hex, side_actions::numbers_t numbers)
{
	std::vector<int>& numbers_to_draw = numbers.numbers_to_draw;
	std::vector<size_t>& team_numbers = numbers.team_numbers;
	int& main_number = numbers.main_number;
	std::set<size_t>& secondary_numbers = numbers.secondary_numbers;

	const double x_offset_base = 0.0;
	const double y_offset_base = 0.2;
	//position 0,0 in the hex is the upper left corner
	//0.8 = horizontal coord., close to the right side of the hex
	const double x_origin = 0.8 - numbers_to_draw.size() * x_offset_base;
	//0.5 = halfway in the hex vertically
	const double y_origin = 0.5 - numbers_to_draw.size() * (y_offset_base / 2);
	double x_offset = 0, y_offset = 0;

	size_t size = numbers_to_draw.size();
	for(size_t i=0; i<size; ++i)
	{
		int number = numbers_to_draw[i];

		std::string number_text = std::to_string(number);
		size_t font_size;
		if (int(i) == main_number) font_size = 19;
		else if (secondary_numbers.find(i)!=secondary_numbers.end()) font_size = 17;
		else font_size = 15;

		color_t color = team::get_side_color(static_cast<int>(team_numbers[i]+1));
		const double x_in_hex = x_origin + x_offset;
		const double y_in_hex = y_origin + y_offset;
		display::get_singleton()->draw_text_in_hex(hex, drawing_queue::LAYER_ACTIONS_NUMBERING,
				number_text, font_size, color, x_in_hex, y_in_hex);
		x_offset += x_offset_base;
		y_offset += y_offset_base;
	}
}


namespace
{
	//Helper struct that finds all units teams whose planned actions are currently visible
	//Only used by manager::pre_draw().
	//Note that this structure is used as a functor.
	struct move_owners_finder: public visitor
	{

	public:
		move_owners_finder(): move_owners_() { }

		void operator()(action* action) {
			action->accept(*this);
		}

		const std::set<size_t>& get_units_owning_moves() {
			return move_owners_;
		}

		virtual void visit(move_ptr move) {
			if(size_t id = move->get_unit_id()) {
				move_owners_.insert(id);
			}
		}

		virtual void visit(attack_ptr attack) {
			//also add attacks if they have an associated move
			if(attack->get_route().steps.size() >= 2) {
				if(size_t id = attack->get_unit_id()) {
					move_owners_.insert(id);
				}
			}
		}
		virtual void visit(recruit_ptr){}
		virtual void visit(recall_ptr){}
		virtual void visit(suppose_dead_ptr){}

	private:
		std::set<size_t> move_owners_;
	};
}

void manager::pre_draw()
{
	if (can_modify_game_state() && has_actions() && unit_map_lock_.unique()) {
		move_owners_finder move_finder;
		for_each_action(std::ref(move_finder));
		units_owning_moves_ = move_finder.get_units_owning_moves();

		for (size_t unit_id : units_owning_moves_) {
			unit_map::iterator unit_iter = resources::gameboard->units().find(unit_id);
			assert(unit_iter.valid());
			ghost_owner_unit(&*unit_iter);
		}
	}
}

void manager::post_draw()
{
	for (size_t unit_id : units_owning_moves_)
	{
		unit_map::iterator unit_iter = resources::gameboard->units().find(unit_id);
		if (unit_iter.valid()) {
			unghost_owner_unit(&*unit_iter);
		}
	}
	units_owning_moves_.clear();
}

void manager::draw_hex(const map_location& hex)
{
	/**
	 * IMPORTANT: none of the code in this method can call anything which would
	 * cause a hex to be invalidated (i.e. by calling in turn any variant of display::invalidate()).
	 * Doing so messes up the iterator currently going over the list of invalidated hexes to draw.
	 */

	if (!wait_for_side_init_ && has_actions())
	{
		//call draw() for all actions
		for_each_action(std::bind(&action::draw_hex, std::placeholders::_1, hex));

		//Info about the action numbers to be displayed on screen.
		side_actions::numbers_t numbers;
		for (team& t : resources::gameboard->teams())
		{
			side_actions& sa = *t.get_side_actions();
			if(!sa.hidden())
				sa.get_numbers(hex,numbers);
		}
		draw_numbers(hex,numbers); // helper fcn
	}

}

void manager::on_mouseover_change(const map_location& hex)
{

	map_location selected_hex = resources::controller->get_mouse_handler_base().get_selected_hex();
	bool hex_has_unit;
	{ wb::future_map future; // start planned unit map scope
		hex_has_unit = resources::gameboard->units().find(selected_hex) != resources::gameboard->units().end();
	} // end planned unit map scope
	if (!((selected_hex.valid() && hex_has_unit)
			|| has_temp_move() || wait_for_side_init_ || executing_actions_))
	{
		if (!highlighter_)
		{
			highlighter_.reset(new highlighter(viewer_actions()));
		}
		highlighter_->set_mouseover_hex(hex);
		highlighter_->highlight();
	}
}

void manager::on_gamestate_change()
{
	DBG_WB << "Manager received gamestate change notification.\n";
	// if on_gamestate_change() is called while the future unit map is applied,
	// it means that the future unit map scope is used where it shouldn't be.
	assert(!planned_unit_map_active_);
	// Set mutated flag so action queue gets validated on next future map build
	gamestate_mutated_ = true;
	//Clear exclusive draws that might not get a chance to be cleared the normal way
	display::get_singleton()->clear_exclusive_draws();
}

void manager::send_network_data()
{
	size_t size = net_buffer_.size();
	for(size_t team_index=0; team_index<size; ++team_index)
	{
		config& buf_cfg = net_buffer_[team_index];

		if(buf_cfg.empty())
			continue;

		config packet;
		config& wb_cfg = packet.add_child("whiteboard",buf_cfg);
		wb_cfg["side"] = static_cast<int>(team_index+1);
		wb_cfg["to_sides"] = resources::gameboard->teams().at(team_index).allied_human_teams();

		buf_cfg.clear();

		resources::controller->send_to_wesnothd(packet, "whiteboard");

		size_t count = wb_cfg.child_count("net_cmd");
		LOG_WB << "Side " << (team_index+1) << " sent wb data (" << count << " cmds).\n";
	}
}

void manager::process_network_data(const config& cfg)
{
	if(const config& wb_cfg = cfg.child("whiteboard"))
	{
		size_t count = wb_cfg.child_count("net_cmd");
		LOG_WB << "Received wb data (" << count << ").\n";

		team& team_from = resources::gameboard->get_team(wb_cfg["side"]);
		for(const side_actions::net_cmd& cmd : wb_cfg.child_range("net_cmd"))
			team_from.get_side_actions()->execute_net_cmd(cmd);
	}
}

void manager::queue_net_cmd(size_t team_index, const side_actions::net_cmd& cmd)
{
	assert(team_index < net_buffer_.size());
	net_buffer_[team_index].add_child("net_cmd",cmd);
}

void manager::create_temp_move()
{
	route_.reset();

	/*
	 * CHECK PRE-CONDITIONS
	 * (This section has multiple return paths.)
	 */

	if ( !active_ || !can_modify_game_state() )
		return;

	const pathfind::marked_route& route =
			resources::controller->get_mouse_handler_base().get_current_route();

	if (route.steps.empty() || route.steps.size() < 2) return;

	unit* temp_moved_unit =
			future_visible_unit(resources::controller->get_mouse_handler_base().get_selected_hex(), viewer_side());
	if (!temp_moved_unit) temp_moved_unit =
			future_visible_unit(resources::controller->get_mouse_handler_base().get_last_hex(), viewer_side());
	if (!temp_moved_unit) return;
	if (temp_moved_unit->side() != display::get_singleton()->viewing_side()) return;

	/*
	 * DONE CHECKING PRE-CONDITIONS, CREATE THE TEMP MOVE
	 * (This section has only one return path.)
	 */

	temp_move_unit_underlying_id_ = temp_moved_unit->underlying_id();

	//@todo: May be appropriate to replace these separate components by a temporary
	//      wb::move object

	route_.reset(new pathfind::marked_route(route));
	//NOTE: route_->steps.back() = dst, and route_->steps.front() = src

	size_t turn = 0;
	std::vector<map_location>::iterator prev_itor = route.steps.begin();
	std::vector<map_location>::iterator curr_itor = prev_itor;
	std::vector<map_location>::iterator end_itor  = route.steps.end();
	for(; curr_itor!=end_itor; ++curr_itor)
	{
		const map_location& hex = *curr_itor;

		//search for end-of-turn marks
		pathfind::marked_route::mark_map::const_iterator w =
				route.marks.find(hex);
		if(w != route.marks.end() && w->second.turns > 0)
		{
			turn = w->second.turns-1;

			if(turn >= move_arrows_.size())
				move_arrows_.resize(turn+1);
			if(turn >= fake_units_.size())
				fake_units_.resize(turn+1);

			arrow_ptr& move_arrow = move_arrows_[turn];
			fake_unit_ptr& fake_unit = fake_units_[turn];

			if(!move_arrow)
			{
				// Create temp arrow
				move_arrow.reset(new arrow());
				move_arrow->set_color(team::get_side_color_id(
						viewer_side()));
				move_arrow->set_style(arrow::STYLE_HIGHLIGHTED);
			}

			arrow_path_t path(prev_itor,curr_itor+1);
			move_arrow->set_path(path);

			if(path.size() >= 2)
			{
				// Bug #20299 demonstrates a situation where an incorrect fake/ghosted unit can be used.
				// So before assuming that a pre-existing fake_unit can be re-used, check that its ID matches the unit being moved.
				if(!fake_unit || fake_unit.get_unit_ptr()->id() != temp_moved_unit->id())
				{
					// Create temp ghost unit
					fake_unit = fake_unit_ptr(unit_ptr (new unit(*temp_moved_unit)), resources::fake_units);
					fake_unit->anim_comp().set_ghosted(true);
				}

				unit_display::move_unit(path, fake_unit.get_unit_ptr(), false); //get facing right
				fake_unit->anim_comp().invalidate(*game_display::get_singleton());
				fake_unit->set_location(*curr_itor);
				fake_unit->anim_comp().set_ghosted(true);
			}
			else //zero-hex path -- don't bother drawing a fake unit
				fake_unit.reset();

			prev_itor = curr_itor;
		}
	}
	//in case path shortens on next step and one ghosted unit has to be removed
	int ind = fake_units_.size() - 1;
	fake_units_[ind]->anim_comp().invalidate(*game_display::get_singleton());
	//toss out old arrows and fake units
	move_arrows_.resize(turn+1);
	fake_units_.resize(turn+1);
}

void manager::erase_temp_move()
{
	move_arrows_.clear();
	for(const fake_unit_ptr& tmp : fake_units_) {
		if(tmp) {
			tmp->anim_comp().invalidate(*game_display::get_singleton());
		}
	}
	fake_units_.clear();
	route_.reset();
	temp_move_unit_underlying_id_ = 0;
}

void manager::save_temp_move()
{
	if (has_temp_move() && !executing_actions_ && !resources::controller->is_linger_mode())
	{
		side_actions& sa = *viewer_actions();
		unit* u = future_visible_unit(route_->steps.front());
		assert(u);
		size_t first_turn = sa.get_turn_num_of(*u);

		validate_viewer_actions();

		assert(move_arrows_.size() == fake_units_.size());
		size_t size = move_arrows_.size();
		for(size_t i=0; i<size; ++i)
		{
			arrow_ptr move_arrow = move_arrows_[i];
			if(!arrow::valid_path(move_arrow->get_path()))
				continue;

			size_t turn = first_turn + i;
			fake_unit_ptr fake_unit = fake_units_[i];

			//@todo Using a marked_route here is wrong, since right now it's not marked
			//either switch over to a plain route for planned moves, or mark it correctly
			pathfind::marked_route route;
			route.steps = move_arrow->get_path();
			route.move_cost = path_cost(route.steps,*u);

			sa.queue_move(turn,*u,route,move_arrow,fake_unit);
		}
		erase_temp_move();

		LOG_WB << *viewer_actions() << "\n";
		print_help_once();
	}
}

unit_map::iterator manager::get_temp_move_unit() const
{
	return resources::gameboard->units().find(temp_move_unit_underlying_id_);
}

void manager::save_temp_attack(const map_location& attacker_loc, const map_location& defender_loc, int weapon_choice)
{
	if (active_ && !executing_actions_ && !resources::controller->is_linger_mode())
	{
		assert(weapon_choice >= 0);

		arrow_ptr move_arrow;
		fake_unit_ptr fake_unit;
		map_location source_hex;

		if (route_ && !route_->steps.empty())
		{
			//attack-move
			assert(move_arrows_.size() == 1);
			assert(fake_units_.size() == 1);
			move_arrow = move_arrows_.front();
			fake_unit = fake_units_.front();

			assert(route_->steps.back() == attacker_loc);
			source_hex = route_->steps.front();

			fake_unit->anim_comp().set_disabled_ghosted(true);
		}
		else
		{
			//simple attack
			move_arrow.reset(new arrow);
			source_hex = attacker_loc;
			route_.reset(new pathfind::marked_route);
			// We'll pass as parameter a one-hex route with no marks.
			route_->steps.push_back(attacker_loc);
		}

		unit* attacking_unit = future_visible_unit(source_hex);
		assert(attacking_unit);

		validate_viewer_actions();

		side_actions& sa = *viewer_actions();
		sa.queue_attack(sa.get_turn_num_of(*attacking_unit),*attacking_unit,defender_loc,weapon_choice,*route_,move_arrow,fake_unit);

		print_help_once();

		erase_temp_move();
		LOG_WB << *viewer_actions() << "\n";
	}
}

bool manager::save_recruit(const std::string& name, int side_num, const map_location& recruit_hex)
{
	bool created_planned_recruit = false;

	if (active_ && !executing_actions_ && !resources::controller->is_linger_mode()) {
		if (side_num != display::get_singleton()->viewing_side())
		{
			LOG_WB <<"manager::save_recruit called for a different side than viewing side.\n";
			created_planned_recruit = false;
		}
		else
		{
			side_actions& sa = *viewer_actions();
			unit* recruiter;
			{ wb::future_map raii;
				recruiter = find_recruiter(side_num-1,recruit_hex);
			} // end planned unit map scope
			assert(recruiter);
			size_t turn = sa.get_turn_num_of(*recruiter);
			sa.queue_recruit(turn,name,recruit_hex);
			created_planned_recruit = true;

			print_help_once();
		}
	}
	return created_planned_recruit;
}

bool manager::save_recall(const unit& unit, int side_num, const map_location& recall_hex)
{
	bool created_planned_recall = false;

	if (active_ && !executing_actions_ && !resources::controller->is_linger_mode())
	{
		if (side_num != display::get_singleton()->viewing_side())
		{
			LOG_WB <<"manager::save_recall called for a different side than viewing side.\n";
			created_planned_recall = false;
		}
		else
		{
			side_actions& sa = *viewer_actions();
			size_t turn = sa.num_turns();
			if(turn > 0)
				--turn;
			sa.queue_recall(turn,unit,recall_hex);
			created_planned_recall = true;

			print_help_once();
		}
	}
	return created_planned_recall;
}

void manager::save_suppose_dead(unit& curr_unit, const map_location& loc)
{
	if(active_ && !executing_actions_ && !resources::controller->is_linger_mode())
	{
		validate_viewer_actions();
		side_actions& sa = *viewer_actions();
		sa.queue_suppose_dead(sa.get_turn_num_of(curr_unit),curr_unit,loc);
	}
}

void manager::contextual_execute()
{
	validate_viewer_actions();
	if (can_enable_execution_hotkeys())
	{
		erase_temp_move();

		//For exception-safety, this struct sets executing_actions_ to false on destruction.
		variable_finalizer<bool> finally(executing_actions_, false);

		action_ptr action;
		side_actions::iterator it = viewer_actions()->end();
		unit const* selected_unit = future_visible_unit(resources::controller->get_mouse_handler_base().get_selected_hex(), viewer_side());
		if (selected_unit &&
				(it = viewer_actions()->find_first_action_of(*selected_unit)) != viewer_actions()->end())
		{
			executing_actions_ = true;
			viewer_actions()->execute(it);
		}
		else if (highlighter_ && (action = highlighter_->get_execute_target()) &&
				 (it = viewer_actions()->get_position_of(action)) != viewer_actions()->end())
		{
			executing_actions_ = true;
			viewer_actions()->execute(it);
		}
		else //we already check above for viewer_actions()->empty()
		{
			executing_actions_ = true;
			viewer_actions()->execute_next();
		}
	} //Finalizer struct sets executing_actions_ to false
}

bool manager::allow_end_turn()
{
	preparing_to_end_turn_ = true;
	return execute_all_actions();
}

bool manager::execute_all_actions()
{
	//exception-safety: finalizers set variables to false on destruction
	//i.e. when method exits naturally or exception is thrown
	variable_finalizer<bool> finalize_executing_actions(executing_actions_, false);
	variable_finalizer<bool> finalize_executing_all_actions(executing_all_actions_, false);

	validate_viewer_actions();
	if(viewer_actions()->empty() || viewer_actions()->turn_size(0) == 0)
	{
		//No actions to execute, job done.
		return true;
	}

	assert(can_enable_execution_hotkeys());

	erase_temp_move();

	// Build unit map once to ensure spent gold and other calculations are refreshed
	set_planned_unit_map();
	assert(has_planned_unit_map());
	set_real_unit_map();

	executing_actions_ = true;
	executing_all_actions_ = true;

	side_actions_ptr sa = viewer_actions();

	if (resources::whiteboard->has_planned_unit_map())
	{
		ERR_WB << "Modifying action queue while temp modifiers are applied!!!" << std::endl;
	}

	//LOG_WB << "Before executing all actions, " << *sa << "\n";

	while (sa->turn_begin(0) != sa->turn_end(0))
	{
		bool action_successful = sa->execute(sa->begin());

		// Interrupt on incomplete action
		if (!action_successful)
		{
			return false;
		}
	}
	return true;
}

void manager::contextual_delete()
{
	validate_viewer_actions();
	if(can_enable_modifier_hotkeys()) {
		erase_temp_move();

		action_ptr action;
		side_actions::iterator it = viewer_actions()->end();
		unit const* selected_unit = future_visible_unit(resources::controller->get_mouse_handler_base().get_selected_hex(), viewer_side());
		if(selected_unit && (it = viewer_actions()->find_last_action_of(*selected_unit)) != viewer_actions()->end()) {
			viewer_actions()->remove_action(it);
			///@todo Shouldn't we probably deselect the unit at this point?
		} else if(highlighter_ && (action = highlighter_->get_delete_target()) && (it = viewer_actions()->get_position_of(action)) != viewer_actions()->end()) {
			viewer_actions()->remove_action(it);
			validate_viewer_actions();
			highlighter_->set_mouseover_hex(highlighter_->get_mouseover_hex());
			highlighter_->highlight();
		} else { //we already check above for viewer_actions()->empty()
			it = (viewer_actions()->end() - 1);
			action = *it;
			viewer_actions()->remove_action(it);
			validate_viewer_actions();
		}
	}
}

void manager::contextual_bump_up_action()
{
	validate_viewer_actions();
	if(can_enable_reorder_hotkeys()) {
		action_ptr action = highlighter_->get_bump_target();
		if(action) {
			viewer_actions()->bump_earlier(viewer_actions()->get_position_of(action));
			validate_viewer_actions(); // Redraw arrows
		}
	}
}

void manager::contextual_bump_down_action()
{
	validate_viewer_actions();
	if(can_enable_reorder_hotkeys()) {
		action_ptr action = highlighter_->get_bump_target();
		if(action) {
			viewer_actions()->bump_later(viewer_actions()->get_position_of(action));
			validate_viewer_actions(); // Redraw arrows
		}
	}
}

bool manager::has_actions() const
{
	assert(resources::gameboard);
	return wb::has_actions();
}

bool manager::unit_has_actions(unit const* unit) const
{
	assert(unit != nullptr);
	assert(resources::gameboard);
	return viewer_actions()->unit_has_actions(*unit);
}

int manager::get_spent_gold_for(int side)
{
	if(wait_for_side_init_)
		return 0;

	return resources::gameboard->get_team(side).get_side_actions()->get_gold_spent();
}

bool manager::should_clear_undo() const
{
	return resources::controller->is_networked_mp();
}

void manager::options_dlg()
{
	int v_side = viewer_side();

	int selection = 0;

	std::vector<team*> allies;
	std::vector<std::string> options;
	utils::string_map t_vars;

	options.emplace_back(_("SHOW ALL allies’ plans"));
	options.emplace_back(_("HIDE ALL allies’ plans"));

	//populate list of networked allies
	for(team &t : resources::gameboard->teams())
	{
		//Exclude enemies, AIs, and local players
		if(t.is_enemy(v_side) || !t.is_network())
			continue;

		allies.push_back(&t);

		t_vars["player"] = t.current_player();
		size_t t_index = t.side()-1;
		if(team_plans_hidden_[t_index])
			options.emplace_back(vgettext("Show plans for $player", t_vars));
		else
			options.emplace_back(vgettext("Hide plans for $player", t_vars));
	}

	gui2::dialogs::simple_item_selector dlg("", _("Whiteboard Options"), options);
	dlg.show();
	selection = dlg.selected_index();

	if(selection == -1)
		return;

	switch(selection)
	{
	case 0:
		for(team* t : allies) {
			team_plans_hidden_[t->side()-1]=false;
		}
		break;
	case 1:
		for(team* t : allies) {
			team_plans_hidden_[t->side()-1]=true;
		}
		break;
	default:
		if(selection > 1)
		{
			size_t t_index = allies[selection-2]->side()-1;
			//toggle ...
			bool hidden = team_plans_hidden_[t_index];
			team_plans_hidden_[t_index] = !hidden;
		}
		break;
	}
	update_plan_hiding();
}

void manager::set_planned_unit_map()
{
	if (!can_modify_game_state()) {
		LOG_WB << "Not building planned unit map: cannot modify game state now.\n";
		return;
	}
	//any more than one reference means a lock on unit map was requested
	if(!unit_map_lock_.unique()) {
		LOG_WB << "Not building planned unit map: unit map locked.\n";
		return;
	}
	if (planned_unit_map_active_) {
		WRN_WB << "Not building planned unit map: already set." << std::endl;
		return;
	}

	log_scope2("whiteboard", "Building planned unit map");
	mapbuilder_.reset(new mapbuilder(resources::gameboard->units()));
	mapbuilder_->build_map();

	planned_unit_map_active_ = true;
}

void manager::set_real_unit_map()
{
	if (planned_unit_map_active_)
	{
		assert(!executing_actions_);
		assert(!wait_for_side_init_);
		if(mapbuilder_)
		{
			log_scope2("whiteboard", "Restoring regular unit map.");
			mapbuilder_.reset();
		}
		planned_unit_map_active_ = false;
	}
	else
	{
		LOG_WB << "Not disabling planned unit map: already disabled.\n";
	}
}

void manager::validate_actions_if_needed()
{
	if (gamestate_mutated_)	{
		validate_viewer_actions();
	}
}

future_map::future_map():
		initial_planned_unit_map_(resources::whiteboard && resources::whiteboard->has_planned_unit_map())
{
	if (!resources::whiteboard)
		return;
	if (!initial_planned_unit_map_)
		resources::whiteboard->set_planned_unit_map();
	// check if if unit map was successfully applied
	if (!resources::whiteboard->has_planned_unit_map()) {
		DBG_WB << "Scoped future unit map failed to apply.\n";
	}
}

future_map::~future_map()
{
	try {
	if (!resources::whiteboard)
		return;
	if (!initial_planned_unit_map_ && resources::whiteboard->has_planned_unit_map())
		resources::whiteboard->set_real_unit_map();
	} catch (...) {}
}

future_map_if_active::future_map_if_active():
		initial_planned_unit_map_(resources::whiteboard && resources::whiteboard->has_planned_unit_map()),
		whiteboard_active_(resources::whiteboard && resources::whiteboard->is_active())
{
	if (!resources::whiteboard)
		return;
	if (!whiteboard_active_)
		return;
	if (!initial_planned_unit_map_)
		resources::whiteboard->set_planned_unit_map();
	// check if if unit map was successfully applied
	if (!resources::whiteboard->has_planned_unit_map()) {
		DBG_WB << "Scoped future unit map failed to apply.\n";
	}
}

future_map_if_active::~future_map_if_active()
{
	try {
	if (!resources::whiteboard)
		return;
	if (!initial_planned_unit_map_ && resources::whiteboard->has_planned_unit_map())
		resources::whiteboard->set_real_unit_map();
	} catch (...) {}
}


real_map::real_map():
		initial_planned_unit_map_(resources::whiteboard && resources::whiteboard->has_planned_unit_map()),
		unit_map_lock_(resources::whiteboard ? resources::whiteboard->unit_map_lock_ : std::make_shared<bool>(false))
{
	if (!resources::whiteboard)
		return;
	if (initial_planned_unit_map_)
		resources::whiteboard->set_real_unit_map();
}

real_map::~real_map()
{
	if (!resources::whiteboard)
		return;
	assert(!resources::whiteboard->has_planned_unit_map());
	if (initial_planned_unit_map_)
	{
		resources::whiteboard->set_planned_unit_map();
	}
}

} // end namespace wb
