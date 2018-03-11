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

#include "whiteboard/move.hpp"

#include "whiteboard/visitor.hpp"
#include "whiteboard/manager.hpp"
#include "whiteboard/side_actions.hpp"
#include "whiteboard/utility.hpp"

#include "arrow.hpp"
#include "config.hpp"
#include "fake_unit_manager.hpp"
#include "fake_unit_ptr.hpp"
#include "font/standard_colors.hpp"
#include "game_board.hpp"
#include "game_end_exceptions.hpp"
#include "map/map.hpp"
#include "mouse_events.hpp"
#include "play_controller.hpp"
#include "replay.hpp"
#include "resources.hpp"
#include "team.hpp"
#include "units/unit.hpp"
#include "units/animation_component.hpp"
#include "units/udisplay.hpp"
#include "units/map.hpp"

namespace wb {

std::ostream& operator<<(std::ostream &s, move_ptr move)
{
	assert(move);
	return move->print(s);
}

std::ostream& operator<<(std::ostream &s, move_const_ptr move)
{
	assert(move);
	return move->print(s);
}

std::ostream& move::print(std::ostream &s) const
{
	s << "Move for unit " << get_unit()->name() << " [" << get_unit()->id() << "] "
			<< "from (" << get_source_hex() << ") to (" << get_dest_hex() << ")";
	return s;
}

move::move(size_t team_index, bool hidden, unit& u, const pathfind::marked_route& route,
		arrow_ptr arrow, fake_unit_ptr fake_unit)
: action(team_index,hidden),
  unit_underlying_id_(u.underlying_id()),
  unit_id_(),
  route_(new pathfind::marked_route(route)),
  movement_cost_(0),
  turn_number_(0),
  arrow_(arrow),
  fake_unit_(fake_unit),
  arrow_brightness_(),
  arrow_texture_(),
  mover_(),
  fake_unit_hidden_(false)
{
	assert(!route_->steps.empty());

	if(hidden)
		fake_unit_->set_hidden(true);

	this->init();
}

move::move(const config& cfg, bool hidden)
	: action(cfg,hidden)
	, unit_underlying_id_(0)
	, unit_id_()
	, route_(new pathfind::marked_route())
	, movement_cost_(0)
	, turn_number_(0)
	, arrow_(new arrow(hidden))
	, fake_unit_()
	, arrow_brightness_()
	, arrow_texture_()
	, mover_()
	, fake_unit_hidden_(false)
{
	// Construct and validate unit_
	unit_map::iterator unit_itor = resources::gameboard->units().find(cfg["unit_"]);
	if(unit_itor == resources::gameboard->units().end())
		throw action::ctor_err("move: Invalid underlying_id");
	unit_underlying_id_ = unit_itor->underlying_id();

	// Construct and validate route_
	const config& route_cfg = cfg.child("route_");
	if(!route_cfg)
		throw action::ctor_err("move: Invalid route_");
	route_->move_cost = route_cfg["move_cost"];
	for(const config& loc_cfg : route_cfg.child_range("step")) {
		route_->steps.emplace_back(loc_cfg["x"],loc_cfg["y"], wml_loc());
	}
	for(const config& mark_cfg : route_cfg.child_range("mark")) {
		route_->marks[map_location(mark_cfg["x"],mark_cfg["y"], wml_loc())]
			= pathfind::marked_route::mark(mark_cfg["turns"],
				mark_cfg["zoc"].to_bool(),
				mark_cfg["capture"].to_bool(),
				mark_cfg["invisible"].to_bool());
	}

	// Validate route_ some more
	const std::vector<map_location>& steps = route_->steps;
	if(steps.empty())
		throw action::ctor_err("move: Invalid route_");

	// Construct arrow_
	arrow_->set_color(team::get_side_color_id(side_number()));
	arrow_->set_style(arrow::STYLE_STANDARD);
	arrow_->set_path(route_->steps);

	// Construct fake_unit_
	fake_unit_ = fake_unit_ptr(unit::create(*get_unit()) , resources::fake_units );
	if(hidden)
		fake_unit_->set_hidden(true);
	fake_unit_->anim_comp().set_ghosted(true);
	unit_display::move_unit(route_->steps, fake_unit_.get_unit_ptr(), false); //get facing right
	fake_unit_->set_location(route_->steps.back());

	this->init();
}

void move::init()
{
	// If a unit is invalid, return immediately to avoid crashes such as trying to plan a move for a planned recruit.
	// As per Bug #18637, this should be fixed so that planning moves on planned recruits work properly.
	// The alternative is to disable movement on planned recruits altogether,
	// possibly in select_or_action() where the fake unit is selected in the first place.
	if (get_unit() == nullptr)
		return;

	assert(get_unit());
	unit_id_ = get_unit()->id();

	//This action defines the future position of the unit, make its fake unit more visible
	//than previous actions' fake units
	if (fake_unit_)
	{
		fake_unit_->anim_comp().set_ghosted(true);
	}
	side_actions_ptr side_actions = resources::gameboard->teams().at(team_index()).get_side_actions();
	side_actions::iterator action = side_actions->find_last_action_of(*(get_unit()));
	if (action != side_actions->end())
	{
		if (move_ptr move = std::dynamic_pointer_cast<class move>(*action))
		{
			if (move->fake_unit_)
				move->fake_unit_->anim_comp().set_disabled_ghosted(true);
		}
	}

	this->calculate_move_cost();

	// Initialize arrow_brightness_ and arrow_texture_ using arrow_->style_
	std::string arrow_style = arrow_->get_style();
	if(arrow_style == arrow::STYLE_STANDARD)
	{
		arrow_brightness_ = ARROW_BRIGHTNESS_STANDARD;
		arrow_texture_ = ARROW_TEXTURE_VALID;
	}
	else if(arrow_style == arrow::STYLE_HIGHLIGHTED)
	{
		arrow_brightness_ = ARROW_BRIGHTNESS_HIGHLIGHTED;
		arrow_texture_ = ARROW_TEXTURE_VALID;
	}
	else if(arrow_style == arrow::STYLE_FOCUS)
	{
		arrow_brightness_ = ARROW_BRIGHTNESS_FOCUS;
		arrow_texture_ = ARROW_TEXTURE_VALID;
	}
	else if(arrow_style == arrow::STYLE_FOCUS_INVALID)
	{
		arrow_brightness_ = ARROW_BRIGHTNESS_STANDARD;
		arrow_texture_ = ARROW_TEXTURE_INVALID;
	}
}

move::~move(){}

void move::accept(visitor& v)
{
	v.visit(shared_from_this());
}

void move::execute(bool& success, bool& complete)
{
	if(!valid()) {
		success = false;
		//Setting complete to true signifies to side_actions to delete the planned action.
		complete = true;
		return;
	}

	if(get_source_hex() == get_dest_hex()) {
		//zero-hex move, used by attack subclass
		success = complete = true;
		return;
	}

	LOG_WB << "Executing: " << shared_from_this() << "\n";

	// Copy the current route to ensure it remains valid throughout the animation.
	const std::vector<map_location> steps = route_->steps;

	set_arrow_brightness(ARROW_BRIGHTNESS_HIGHLIGHTED);
	hide_fake_unit();

	size_t num_steps;
	bool interrupted;
	try {
		events::mouse_handler& mouse_handler = resources::controller->get_mouse_handler_base();
		num_steps = mouse_handler.move_unit_along_route(steps, interrupted);
	} catch (return_to_play_side_exception&) {
		set_arrow_brightness(ARROW_BRIGHTNESS_STANDARD);
		throw; // we rely on the caller to delete this action
	}
	const map_location & final_location = steps[num_steps];
	unit_map::const_iterator unit_it = resources::gameboard->units().find(final_location);

	if ( num_steps == 0 )
	{
		LOG_WB << "Move execution resulted in zero movement.\n";
		success = false;
		complete = true;
	}
	else if ( unit_it == resources::gameboard->units().end()  ||  unit_it->id() != unit_id_ )
	{
		WRN_WB << "Unit disappeared from map during move execution." << std::endl;
		success = false;
		complete = true;
	}
	else
	{
		complete = num_steps + 1 == steps.size();
		success = complete && !interrupted;

		if ( !success )
		{
			if ( complete )
			{
				LOG_WB << "Move completed, but interrupted on final hex. Halting.\n";
				//reset to a single-hex path, just in case *this is a wb::attack
				route_->steps = std::vector<map_location>(1, final_location);
				arrow_.reset();
			}
			else
			{
				LOG_WB << "Move finished at (" << final_location << ") instead of at (" << get_dest_hex() << "). Setting new path.\n";
				route_->steps = std::vector<map_location>(steps.begin() + num_steps, steps.end());
				//FIXME: probably better to use the new calculate_new_route() instead of the above:
				//calculate_new_route(final_location, steps.back());
				// Of course, "better" would need to be verified.
				arrow_->set_path(route_->steps);
			}
		}
	}

	if(!complete)
	{
		set_arrow_brightness(ARROW_BRIGHTNESS_STANDARD);
		show_fake_unit();
	}
}

unit_ptr move::get_unit() const
{
	unit_map::iterator itor = resources::gameboard->units().find(unit_underlying_id_);
	if (itor.valid())
		return itor.get_shared_ptr();
	else
		return unit_ptr();
}

map_location move::get_source_hex() const
{
	assert(route_ && !route_->steps.empty());
	return route_->steps.front();
}

map_location move::get_dest_hex() const
{
	assert(route_ && !route_->steps.empty());
	return route_->steps.back();
}

void move::set_route(const pathfind::marked_route& route)
{
	route_.reset(new pathfind::marked_route(route));
	this->calculate_move_cost();
	arrow_->set_path(route_->steps);
}

bool move::calculate_new_route(const map_location& source_hex, const map_location& dest_hex)
{
	pathfind::plain_route new_plain_route;
	pathfind::shortest_path_calculator path_calc(*get_unit(),
						resources::gameboard->teams().at(team_index()),
						resources::gameboard->teams(), resources::gameboard->map());
	new_plain_route = pathfind::a_star_search(source_hex,
						dest_hex, 10000, path_calc, resources::gameboard->map().w(), resources::gameboard->map().h());
	if (new_plain_route.move_cost >= path_calc.getNoPathValue()) return false;
	route_.reset(new pathfind::marked_route(pathfind::mark_route(new_plain_route)));
	calculate_move_cost();
	return true;
}

void move::apply_temp_modifier(unit_map& unit_map)
{
	if (get_source_hex() == get_dest_hex())
		return; //zero-hex move, used by attack subclass

	// Safety: Make sure the old temporary_unit_mover (if any) is destroyed
	// before creating a new one.
	mover_.reset();

	//@todo: deal with multi-turn moves, which may for instance end their first turn
	// by capturing a village

	//@todo: we may need to change unit status here and change it back in remove_temp_modifier
	unit* unit;
	{
		unit_map::iterator unit_it = unit_map.find(get_source_hex());
		assert(unit_it != unit_map.end());
		unit = &*unit_it;
	}

	//Modify movement points
	DBG_WB <<"Move: Changing movement points for unit " << unit->name() << " [" << unit->id()
			<< "] from " << unit->movement_left() << " to "
			<< unit->movement_left() - movement_cost_ << ".\n";
	// Move the unit
	DBG_WB << "Move: Temporarily moving unit " << unit->name() << " [" << unit->id()
			<< "] from (" << get_source_hex() << ") to (" << get_dest_hex() <<")\n";
	mover_.reset(new temporary_unit_mover(unit_map, get_source_hex(), get_dest_hex(),
	                                      unit->movement_left() - movement_cost_));

	//Update status of fake unit (not undone by remove_temp_modifiers)
	//@todo this contradicts the name "temp_modifiers"
	fake_unit_->set_movement(unit->movement_left(), true);
}

void move::remove_temp_modifier(unit_map&)
{
	if (get_source_hex() == get_dest_hex())
		return; //zero-hex move, probably used by attack subclass

	// Debug movement points
	if ( !lg::debug().dont_log(log_whiteboard) )
	{
		unit* unit;
		{
			unit_map::iterator unit_it = resources::gameboard->units().find(get_dest_hex());
			assert(unit_it != resources::gameboard->units().end());
			unit = &*unit_it;
		}
		DBG_WB << "Move: Movement points for unit " << unit->name() << " [" << unit->id()
					<< "] should get changed from " << unit->movement_left() << " to "
					<< unit->movement_left() + movement_cost_ << ".\n";
	}

	// Restore the unit to its original position and movement.
	mover_.reset();
}

void move::draw_hex(const map_location& hex)
{
	//display turn info for turns 2 and above
	if (hex == get_dest_hex() && turn_number_ >= 2)
	{
		std::stringstream turn_text;
		turn_text << turn_number_;
		display::get_singleton()->draw_text_in_hex(hex, display::LAYER_MOVE_INFO, turn_text.str(), 17, font::NORMAL_COLOR, 0.5,0.8);
	}
}

void move::do_hide()
{
	arrow_->hide();
	if(!fake_unit_hidden_)
		fake_unit_->set_hidden(true);
}

void move::do_show()
{
	arrow_->show();
	if(!fake_unit_hidden_)
		fake_unit_->set_hidden(false);
}

void move::hide_fake_unit()
{
	fake_unit_hidden_ = true;
	if(!hidden())
		fake_unit_->set_hidden(true);
}

void move::show_fake_unit()
{
	fake_unit_hidden_ = false;
	if(!hidden())
		fake_unit_->set_hidden(false);
}

map_location move::get_numbering_hex() const
{
	return get_dest_hex();
}

action::error move::check_validity() const
{
	// Used to deal with multiple return paths.
	class arrow_texture_setter {
	public:
		arrow_texture_setter(const move *target, move::ARROW_TEXTURE current_texture, move::ARROW_TEXTURE setting_texture):
			target(target),
			current_texture(current_texture),
			setting_texture(setting_texture) {}

		~arrow_texture_setter() {
			if(current_texture!=setting_texture) {
				target->set_arrow_texture(setting_texture);
			}
		}

		void set_texture(move::ARROW_TEXTURE texture) { setting_texture=texture; }

	private:
		const move *target;
		move::ARROW_TEXTURE current_texture, setting_texture;
	};

	arrow_texture_setter setter(this, arrow_texture_, ARROW_TEXTURE_INVALID);

	if(!(get_source_hex().valid() && get_dest_hex().valid())) {
		return INVALID_LOCATION;
	}

	//Check that the unit still exists in the source hex
	unit_map::iterator unit_it;
	unit_it = resources::gameboard->units().find(get_source_hex());
	if(unit_it == resources::gameboard->units().end()) {
		return NO_UNIT;
	}

	//check if the unit in the source hex has the same unit id as before,
	//i.e. that it's the same unit
	if(unit_id_ != unit_it->id() || unit_underlying_id_ != unit_it->underlying_id()) {
		return UNIT_CHANGED;
	}

	//If the path has at least two hexes (it can have less with the attack subclass), ensure destination hex is free
	if(get_route().steps.size() >= 2 && resources::gameboard->get_visible_unit(get_dest_hex(),resources::gameboard->teams().at(viewer_team())) != nullptr) {
		return LOCATION_OCCUPIED;
	}

	//check that the path is good
	if(get_source_hex() != get_dest_hex()) { //skip zero-hex move used by attack subclass
		// Mark the plain route to see if the move can still be done in one turn,
		// which is always the case for planned moves
		pathfind::marked_route checked_route = pathfind::mark_route(get_route().route);

		if(checked_route.marks[checked_route.steps.back()].turns != 1) {
			return TOO_FAR;
		}
	}

	// The move is valid, so correct the setter.
	setter.set_texture(ARROW_TEXTURE_VALID);

	return OK;
}

config move::to_config() const
{
	config final_cfg = action::to_config();

	final_cfg["type"]="move";
	final_cfg["unit_"]=static_cast<int>(unit_underlying_id_);
//	final_cfg["movement_cost_"]=movement_cost_; //Unnecessary
//	final_cfg["unit_id_"]=unit_id_; //Unnecessary

	//Serialize route_
	config route_cfg;
	route_cfg["move_cost"]=route_->move_cost;
	for(const map_location& loc : route_->steps)
	{
		config loc_cfg;
		loc_cfg["x"]=loc.wml_x();
		loc_cfg["y"]=loc.wml_y();
		route_cfg.add_child("step", std::move(loc_cfg));
	}
	typedef std::pair<map_location,pathfind::marked_route::mark> pair_loc_mark;
	for(const pair_loc_mark& item : route_->marks)
	{
		config mark_cfg;
		mark_cfg["x"]=item.first.wml_x();
		mark_cfg["y"]=item.first.wml_y();
		mark_cfg["turns"]=item.second.turns;
		mark_cfg["zoc"]=item.second.zoc;
		mark_cfg["capture"]=item.second.capture;
		mark_cfg["invisible"]=item.second.invisible;
		route_cfg.add_child("mark", std::move(mark_cfg));
	}
	final_cfg.add_child("route_", std::move(route_cfg));

	return final_cfg;
}

void move::calculate_move_cost()
{
	assert(get_unit());
	assert(route_);
	if (get_source_hex().valid() && get_dest_hex().valid() && get_source_hex() != get_dest_hex())
	{

		// @todo: find a better treatment of movement points when defining moves out-of-turn
		if(get_unit()->movement_left() - route_->move_cost < 0
				&& resources::controller->current_side() == display::get_singleton()->viewing_side()) {
			WRN_WB << "Move defined with insufficient movement left." << std::endl;
		}

		// If unit finishes move in a village it captures, set the move cost to unit's movement_left()
		 if (route_->marks[get_dest_hex()].capture)
		 {
			 movement_cost_ = get_unit()->movement_left();
		 }
		 else
		 {
			 movement_cost_ = route_->move_cost;
		 }
	}
}

void move::redraw()
{
	display::get_singleton()->invalidate(get_source_hex());
	display::get_singleton()->invalidate(get_dest_hex());
	update_arrow_style();
}

//If you add more arrow styles, this will need to change
/* private */
void move::update_arrow_style()
{
	if(arrow_texture_ == ARROW_TEXTURE_INVALID)
	{
		arrow_->set_style(arrow::STYLE_FOCUS_INVALID);
		return;
	}

	switch(arrow_brightness_)
	{
	case ARROW_BRIGHTNESS_STANDARD:
		arrow_->set_style(arrow::STYLE_STANDARD);
		break;
	case ARROW_BRIGHTNESS_HIGHLIGHTED:
		arrow_->set_style(arrow::STYLE_HIGHLIGHTED);
		break;
	case ARROW_BRIGHTNESS_FOCUS:
		arrow_->set_style(arrow::STYLE_FOCUS);
		break;
	}
}

} // end namespace wb
