/* $Id$ */
/*
 Copyright (C) 2010 - 2011 by Gabriel Morin <gabrielmorin (at) gmail (dot) com>
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

#include "manager.hpp"

#include "action.hpp"
#include "highlight_visitor.hpp"
#include "mapbuilder.hpp"
#include "move.hpp"
#include "recruit.hpp"
#include "side_actions.hpp"
#include "utility.hpp"

#include "actions.hpp"
#include "arrow.hpp"
#include "chat_events.hpp"
#include "foreach.hpp"
#include "formula_string_utils.hpp"
#include "game_preferences.hpp"
#include "gettext.hpp"
#include "gui/dialogs/simple_item_selector.hpp"
#include "key.hpp"
#include "network.hpp"
#include "pathfind/pathfind.hpp"
#include "play_controller.hpp"
#include "resources.hpp"
#include "team.hpp"
#include "unit_display.hpp"

#include <boost/lexical_cast.hpp>
#include <sstream>

namespace wb {

manager::manager():
		active_(false),
		inverted_behavior_(false),
		self_activate_once_(true),
		print_help_once_(true),
		wait_for_side_init_(true),
		planned_unit_map_active_(false),
		executing_actions_(false),
		gamestate_mutated_(false),
		activation_state_lock_(new bool),
		mapbuilder_(),
		highlighter_(),
		route_(),
		move_arrows_(),
		fake_units_(),
		key_poller_(new CKey),
		hidden_unit_hexes_(),
		net_buffer_(resources::teams->size()),
		team_plans_hidden_(resources::teams->size(),preferences::hide_whiteboard())
{
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
	resources::screen->add_chat_message(time(NULL), title, 0, message,
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

bool manager::can_activate() const
{
	//any more than one reference means a lock on whiteboard state was requested
	if(!activation_state_lock_.unique())
		return false;

	if(wait_for_side_init_
				|| executing_actions_
				|| is_observer()
				|| resources::controller->is_linger_mode())
	{
		return false;
	}

	return true;
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
				clear_undo();
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
	return can_enable_modifier_hotkeys() && viewer_side() == resources::controller->current_side();
}

bool manager::can_enable_modifier_hotkeys() const
{
	return can_activate() && !viewer_actions()->empty();
}

bool manager::can_enable_reorder_hotkeys() const
{
	return can_enable_modifier_hotkeys() && highlighter_ && highlighter_->get_bump_target();
}

bool manager::allow_leader_to_move(unit const& leader) const
{
	//Look for another leader on another keep in the same castle
	{ wb::scoped_planned_unit_map future; //< start planned unit map scope
		if(find_backup_leader(leader))
			return true;
	} // end planned unit map scope

	//Look for planned recruits that depend on this leader
	foreach(action_const_ptr action, *viewer_actions())
	{
		if(recruit_const_ptr recruit = boost::dynamic_pointer_cast<class recruit const>(action))
		{
			if (can_recruit_on(*resources::game_map, leader.get_location(), recruit->get_recruit_hex()))
				return false;
		}
	}
	return true;
}

void manager::on_init_side()
{
	update_plan_hiding(); //< validates actions
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
	wait_for_side_init_ = true;
	if(side == viewer_side())
		viewer_actions()->synced_turn_shift();
	highlighter_.reset();
	erase_temp_move();
	LOG_WB << "on_finish_side_turn()\n";
}

static void hide_all_plans()
{
	foreach(team& t, *resources::teams)
		t.get_side_actions()->hide();
}

/* private */
void manager::update_plan_hiding(size_t team_index)
{
	//We don't control the "viewing" side ... we're probably an observer
	if(!resources::teams->at(team_index).is_human())
		hide_all_plans();
	else //< normal circumstance
	{
		foreach(team& t, *resources::teams)
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
	resources::teams->at(team_index).get_side_actions()->validate_actions();
}
void manager::update_plan_hiding()
	{update_plan_hiding(viewer_team());}

void manager::on_viewer_change(size_t team_index)
{
	if(!wait_for_side_init_)
		update_plan_hiding(team_index);
}

void manager::on_change_controller(int side, team& t)
{
	wb::side_actions& sa = *t.get_side_actions();
	if(t.is_human()) //< we own this side now
	{
		//tell everyone to clear this side's actions -- we're starting anew
		resources::whiteboard->queue_net_cmd(sa.team_index(),sa.make_net_cmd_clear());
		sa.clear();
		//refresh the hidden_ attribute of every team's side_actions
		update_plan_hiding();
	}
	else if(t.is_ai() || t.is_network_ai()) //< no one owns this side anymore
		sa.clear(); //< clear its plans away -- the ai doesn't plan ... yet
	else if(t.is_network()) //< Another client is taking control of the side
	{
		if(side==viewer_side()) //< They're taking OUR side away!
			hide_all_plans(); //< give up knowledge of everyone's plans, in case we became an observer

		//tell them our plans -- they may not have received them up to this point
		size_t num_teams = resources::teams->size();
		for(size_t i=0; i<num_teams; ++i)
		{
			team& local_team = resources::teams->at(i);
			if(local_team.is_human() && !local_team.is_enemy(side))
				resources::whiteboard->queue_net_cmd(i,local_team.get_side_actions()->make_net_cmd_refresh());
		}
	}
}

bool manager::current_side_has_actions()
{
	side_actions::range_t range = current_side_actions()->iter_turn(0);
	return range.first != range.second; //non-empty range
}

void manager::validate_viewer_actions()
{
	assert(!executing_actions_);
	gamestate_mutated_ = false;
	if (viewer_actions()->empty()) return;
	viewer_actions()->validate_actions();
}

//helper fcn
static void draw_numbers(map_location const& hex, side_actions::numbers_t numbers)
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

		std::string number_text = boost::lexical_cast<std::string>(number);
		size_t font_size;
		if (int(i) == main_number) font_size = 19;
		else if (secondary_numbers.find(i)!=secondary_numbers.end()) font_size = 17;
		else font_size = 15;

		SDL_Color color = team::get_side_color(static_cast<int>(team_numbers[i]+1));
		const double x_in_hex = x_origin + x_offset;
		const double y_in_hex = y_origin + y_offset;
		resources::screen->draw_text_in_hex(hex, display::LAYER_ACTIONS_NUMBERING,
				number_text, font_size, color, x_in_hex, y_in_hex);
		x_offset += x_offset_base;
		y_offset += y_offset_base;
	}
}

namespace
{
	//Only used by manager::draw_hex()
	struct draw_visitor
		: private enable_visit_all<draw_visitor>
	{
		friend class enable_visit_all<draw_visitor>;

	public:
		draw_visitor(map_location const& hex): hex_(hex) {}

		using enable_visit_all<draw_visitor>::visit_all;

	private:
		//"Inherited" from enable_visit_all
		bool visit(size_t /*team_index*/, team&, side_actions&, side_actions::iterator itor)
			{ (*itor)->draw_hex(hex_);   return true; }
		//using default pre_visit_team()
		//using default post_visit_team()

		map_location const& hex_;
	};
}

void manager::draw_hex(const map_location& hex)
{
	/**
	 * IMPORTANT: none of the code in this method can call anything which would
	 * cause a hex to be invalidated (i.e. by calling in turn any variant of display::invalidate()).
	 * Doing so messes up the iterator currently going over the list of invalidated hexes to draw.
	 */

	if (!wait_for_side_init_)
	{
		//call draw() for all actions
		draw_visitor(hex).visit_all();

		//Info about the action numbers to be displayed on screen.
		side_actions::numbers_t numbers;
		foreach(team& t, *resources::teams)
		{
			side_actions& sa = *t.get_side_actions();
			if(!sa.hidden())
				sa.get_numbers(hex,numbers);
		}
		draw_numbers(hex,numbers); //< helper fcn
	}

}

void manager::on_mouseover_change(const map_location& hex)
{
	foreach(map_location const& hex, hidden_unit_hexes_)
		resources::screen->remove_exclusive_draw(hex);
	hidden_unit_hexes_.clear();

	map_location selected_hex = resources::screen->selected_hex();
	unit_map::iterator it;
	{ wb::scoped_planned_unit_map future; //< start planned unit map scope
		it = resources::units->find(selected_hex);
	} // end planned unit map scope
	if (!((selected_hex.valid() && it != resources::units->end())
			|| has_temp_move() || wait_for_side_init_ || executing_actions_))
	{
		if (!highlighter_)
		{
			highlighter_.reset(new highlight_visitor(*resources::units, viewer_actions()));
		}
		highlighter_->set_mouseover_hex(hex);
		highlighter_->highlight();
	}
}

void manager::on_gamestate_change()
{
	DBG_WB << "Manager received gamestate change notification.\n";
	gamestate_mutated_ = true;
	//Clear exclusive draws that might not get a chance to be cleared the normal way
	resources::screen->clear_exclusive_draws();
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
		wb_cfg["team_name"] = resources::teams->at(team_index).team_name();

		buf_cfg = config();

		network::send_data(packet,0,"whiteboard");

		size_t count = wb_cfg.child_count("net_cmd");
		LOG_WB << "Side " << (team_index+1) << " sent wb data (" << count << " cmds).\n";
	}
}

void manager::process_network_data(config const& cfg)
{
	if(config const& wb_cfg = cfg.child("whiteboard"))
	{
		size_t count = wb_cfg.child_count("net_cmd");
		LOG_WB << "Received wb data (" << count << ").\n";

		team& team_from = resources::teams->at(wb_cfg["side"]-1);
		foreach(side_actions::net_cmd const& cmd, wb_cfg.child_range("net_cmd"))
			team_from.get_side_actions()->execute_net_cmd(cmd);
	}
}

void manager::queue_net_cmd(size_t team_index, side_actions::net_cmd const& cmd)
{
	net_buffer_[team_index].add_child("net_cmd",cmd);
}

void manager::create_temp_move()
{
	route_.reset();

	/*
	 * CHECK PRE-CONDITIONS
	 * (This section has multiple return paths.)
	 */

	if(!active_
			|| wait_for_side_init_
			|| executing_actions_
			|| is_observer()
			|| resources::controller->is_linger_mode())
		return;

	assert(!has_planned_unit_map());

	pathfind::marked_route const& route =
			resources::controller->get_mouse_handler_base().get_current_route();

	if (route.steps.empty() || route.steps.size() < 2) return;

	unit const* selected_unit = future_visible_unit(resources::screen->selected_hex(), viewer_side());
	if (!selected_unit) return;
	if (selected_unit->side() != resources::screen->viewing_side()) return;

	/*
	 * DONE CHECKING PRE-CONDITIONS, CREATE THE TEMP MOVE
	 * (This section has only one return path.)
	 */

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
				move_arrow->set_color(team::get_side_color_index(
						viewer_side()));
				move_arrow->set_style(arrow::STYLE_HIGHLIGHTED);
			}

			arrow_path_t path(prev_itor,curr_itor+1);
			move_arrow->set_path(path);

			if(path.size() >= 2)
			{
				if(!fake_unit)
				{
					// Create temp ghost unit
					fake_unit.reset(new game_display::fake_unit(*selected_unit));
					fake_unit->place_on_game_display( resources::screen);
					fake_unit->set_ghosted(false);
				}

				unit_display::move_unit(path, *fake_unit, *resources::teams,
						false); //get facing right
				fake_unit->set_location(*curr_itor);
				fake_unit->set_ghosted(false);

				//if destination is over another unit, temporarily hide it
				resources::screen->add_exclusive_draw(fake_unit->get_location(), *fake_unit);
				hidden_unit_hexes_.push_back(fake_unit->get_location());
			}
			else //zero-hex path -- don't bother drawing a fake unit
				fake_unit.reset();

			prev_itor = curr_itor;
		}
	}
	//toss out old arrows and fake units
	move_arrows_.resize(turn+1);
	fake_units_.resize(turn+1);
}

void manager::erase_temp_move()
{
	move_arrows_.clear();
	fake_units_.clear();
	route_.reset();
}

void manager::save_temp_move()
{
	if (has_temp_move() && !executing_actions_ && !resources::controller->is_linger_mode())
	{
		side_actions& sa = *viewer_actions();
		unit* u = future_visible_unit(route_->steps.front());
		assert(u);
		size_t first_turn = sa.get_turn_num_of(*u);

		on_save_action(u);

		assert(move_arrows_.size() == fake_units_.size());
		size_t size = move_arrows_.size();
		for(size_t i=0; i<size; ++i)
		{
			arrow_ptr move_arrow = move_arrows_[i];
			if(!arrow::valid_path(move_arrow->get_path()))
				continue;

			size_t turn = first_turn + i;
			fake_unit_ptr fake_unit = fake_units_[i];
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

			fake_unit->set_disabled_ghosted(false);
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

		on_save_action(attacking_unit);

		side_actions& sa = *viewer_actions();
		sa.queue_attack(sa.get_turn_num_of(*attacking_unit),*attacking_unit,defender_loc,weapon_choice,*route_,move_arrow,fake_unit);

		print_help_once();

		resources::screen->invalidate(defender_loc);
		resources::screen->invalidate(attacker_loc);
		erase_temp_move();
		LOG_WB << *viewer_actions() << "\n";
	}
}

bool manager::save_recruit(const std::string& name, int side_num, const map_location& recruit_hex)
{
	bool created_planned_recruit = false;

	if (active_ && !executing_actions_ && !resources::controller->is_linger_mode()) {
		if (side_num != resources::screen->viewing_side())
		{
			LOG_WB <<"manager::save_recruit called for a different side than viewing side.\n";
			created_planned_recruit = false;
		}
		else
		{
			on_save_action(NULL);

			side_actions& sa = *viewer_actions();
			unit* recruiter;
			{ wb::scoped_planned_unit_map raii;
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
		if (side_num != resources::screen->viewing_side())
		{
			LOG_WB <<"manager::save_recall called for a different side than viewing side.\n";
			created_planned_recall = false;
		}
		else
		{
			on_save_action(NULL);

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

void manager::save_suppose_dead(unit& curr_unit, map_location const& loc)
{
	if(active_ && !executing_actions_ && !resources::controller->is_linger_mode())
	{
		on_save_action(&curr_unit);
		side_actions& sa = *viewer_actions();
		sa.queue_suppose_dead(sa.get_turn_num_of(curr_unit),curr_unit,loc);
	}
}

void manager::on_save_action(unit const* u) const
{
	if(u)
		viewer_actions()->remove_invalid_of(u);
}

void manager::contextual_execute()
{
//	if (!(executing_actions_ || viewer_actions()->empty() || resources::controller->is_linger_mode())
//			&& resources::controller->current_side() == resources::screen->viewing_side())
	if (can_enable_execution_hotkeys())
	{
		erase_temp_move();
		validate_viewer_actions();

		action_ptr action;
		side_actions::iterator it;
		unit const* selected_unit = future_visible_unit(resources::screen->selected_hex(), viewer_side());
		if (selected_unit &&
				(it = viewer_actions()->find_first_action_of(selected_unit)) != viewer_actions()->end())
		{
			executing_actions_ = true;
			viewer_actions()->execute(it);
			executing_actions_ = false;
		}
		else if (highlighter_ && (action = highlighter_->get_execute_target()) &&
				 (it = viewer_actions()->get_position_of(action)) != viewer_actions()->end())
		{
			executing_actions_ = true;
			viewer_actions()->execute(it);
			executing_actions_ = false;
		}
		else //we already check above for viewer_actions()->empty()
		{
			executing_actions_ = true;
			viewer_actions()->execute_next();
			executing_actions_ = false;
		}
	}
}

void manager::execute_all_actions()
{
//	if (!(executing_actions_ || viewer_actions()->empty() || resources::controller->is_linger_mode())
//			&& resources::controller->current_side() == resources::screen->viewing_side())
	if(can_enable_execution_hotkeys())
	{
		erase_temp_move();
		validate_viewer_actions();
		executing_actions_ = true;
		viewer_actions()->execute_all();
		executing_actions_ = false;
	}
}

void manager::contextual_delete()
{
//	if (!(executing_actions_ || viewer_actions()->empty() || resources::controller->is_linger_mode()))
	if (can_enable_modifier_hotkeys())
	{
		erase_temp_move();
		validate_viewer_actions();

		action_ptr action;
		side_actions::iterator it;
		unit const* selected_unit = future_visible_unit(resources::screen->selected_hex(), viewer_side());
		if (selected_unit &&
				(it = viewer_actions()->find_first_action_of(selected_unit)) != viewer_actions()->end())
		{
			///@todo Shouldn't it be "find_last_action_of" instead of "find_first_action_of" above?
			viewer_actions()->remove_action(it);
			///@todo Shouldn't we probably deselect the unit at this point?
		}
		else if (highlighter_ && (action = highlighter_->get_delete_target()) &&
				(it = viewer_actions()->get_position_of(action)) != viewer_actions()->end())
		{
			viewer_actions()->remove_action(it);
			viewer_actions()->remove_invalid_of(action->get_unit());
			highlighter_->set_mouseover_hex(highlighter_->get_mouseover_hex());
			highlighter_->highlight();
		}
		else //we already check above for viewer_actions()->empty()
		{
			it = (viewer_actions()->end() - 1);
			action = *it;
			viewer_actions()->remove_action(it);
			viewer_actions()->remove_invalid_of(action->get_unit());
		}
	}
}

void manager::contextual_bump_up_action()
{
//	if (!(executing_actions_ || viewer_actions()->empty() || resources::controller->is_linger_mode())
//			&& highlighter_)
	if(can_enable_reorder_hotkeys())
	{
		validate_viewer_actions();
		action_ptr action = highlighter_->get_bump_target();
		if (action)
		{
			viewer_actions()->bump_earlier(viewer_actions()->get_position_of(action));
		}
	}
}

void manager::contextual_bump_down_action()
{
//	if (!(executing_actions_ || viewer_actions()->empty() || resources::controller->is_linger_mode())
//			&& highlighter_)
	if(can_enable_reorder_hotkeys())
	{
		validate_viewer_actions();
		action_ptr action = highlighter_->get_bump_target();
		if (action)
		{
			viewer_actions()->bump_later(viewer_actions()->get_position_of(action));
		}
	}
}

void manager::erase_all_actions()
{
	on_deselect_hex();
	set_real_unit_map();
	foreach(team& team, *resources::teams)
	{
		team.get_side_actions()->clear();
	}
}

bool manager::unit_has_actions(unit const* unit) const
{
	return viewer_actions()->unit_has_actions(unit);
}

int manager::get_spent_gold_for(int side)
{
	return resources::teams->at(side - 1).get_side_actions()->get_gold_spent();
}

void manager::clear_undo()
{
	apply_shroud_changes(*resources::undo_stack, viewer_side());
	resources::undo_stack->clear();
	resources::redo_stack->clear();
}

void manager::options_dlg()
{
	int v_side = viewer_side();

	int selection = 0;

	std::vector<team*> allies;
	std::vector<std::string> options;
	utils::string_map t_vars;

	options.push_back(_("SHOW ALL allies’ plans"));
	options.push_back(_("HIDE ALL allies’ plans"));

	//populate list of networked allies
	foreach(team &t, *resources::teams)
	{
		//Exclude enemies, AIs, and local players
		if(t.is_enemy(v_side) || !t.is_network())
			continue;

		allies.push_back(&t);

		t_vars["player"] = t.current_player();
		size_t t_index = t.side()-1;
		if(team_plans_hidden_[t_index])
			options.push_back(vgettext("Show plans for $player", t_vars));
		else
			options.push_back(vgettext("Hide plans for $player", t_vars));
	}

	gui2::tsimple_item_selector dlg("", _("Whiteboard Options"), options);
	dlg.show(resources::screen->video());
	selection = dlg.selected_index();

	if(selection == -1)
		return;

	switch(selection)
	{
	case 0:
		foreach(team* t, allies)
			team_plans_hidden_[t->side()-1]=false;
		break;
	case 1:
		foreach(team* t, allies)
			team_plans_hidden_[t->side()-1]=true;
		break;
	default:
		if(selection > 1)
		{
			size_t t_index = allies[selection-2]->side()-1;
			//toggle ...
			bool hidden = team_plans_hidden_[t_index];
			team_plans_hidden_[t_index] = !hidden;
		}
	}
	update_plan_hiding();
}

void manager::set_planned_unit_map()
{
	if (!executing_actions_ && !wait_for_side_init_)
	{
		if (!planned_unit_map_active_)
		{
			validate_actions_if_needed();
			log_scope2("whiteboard", "Building planned unit map");
			mapbuilder_.reset(new mapbuilder(*resources::units));
			mapbuilder_->build_map();
			planned_unit_map_active_ = true;
		}
		else
		{
			WRN_WB << "Attempt to set planned unit map when it was already set.\n";
		}
	}
	else if (executing_actions_)
	{
		DBG_WB << "Attempt to set planned_unit_map during action execution.\n";
	}
	else if (wait_for_side_init_)
	{
		DBG_WB << "Attempt to set planned_unit_map while waiting for side init.\n";
	}
}

void manager::set_real_unit_map()
{
	if (!executing_actions_)
	{
		if (planned_unit_map_active_)
		{
			if (mapbuilder_)
			{
				log_scope2("whiteboard", "Restoring regular unit map.");
				mapbuilder_.reset();
			}
			planned_unit_map_active_ = false;
		}
		else if (!wait_for_side_init_)
		{
			LOG_WB << "Attempt to disable the planned unit map, when it was already disabled.\n";
		}
	}
	else //executing_actions_
	{
		DBG_WB << "Attempt to set planned_unit_map during action execution.\n";
	}
}

void manager::validate_actions_if_needed()
{
	if (gamestate_mutated_)
	{
		LOG_WB << "'gamestate_mutated_' flag dirty, validating actions.\n";
		validate_viewer_actions(); //sets gamestate_mutated_ to false
	}
}

scoped_planned_unit_map::scoped_planned_unit_map():
		has_planned_unit_map_(resources::whiteboard->has_planned_unit_map())
{
	if (!has_planned_unit_map_)
		resources::whiteboard->set_planned_unit_map();
	else
		WRN_WB << "Using scoped planned unit map with future map already active, unit map will be reset when exiting.\n";
}

scoped_planned_unit_map::~scoped_planned_unit_map()
{
	if (!has_planned_unit_map_)
		resources::whiteboard->set_real_unit_map();
}

scoped_real_unit_map::scoped_real_unit_map():
		has_planned_unit_map_(resources::whiteboard->has_planned_unit_map())
{
	if (has_planned_unit_map_)
		resources::whiteboard->set_real_unit_map();
}

scoped_real_unit_map::~scoped_real_unit_map()
{
	if (has_planned_unit_map_ &&
			!resources::whiteboard->has_planned_unit_map())
		resources::whiteboard->set_planned_unit_map();
}

bool unit_comparator_predicate::operator()(unit const& unit)
{
	return unit_.id() == unit.id();
}

} // end namespace wb
