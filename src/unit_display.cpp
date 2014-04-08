/*
   Copyright (C) 2003 - 2014 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/** @file */

#include "global.hpp"
#include "unit_display.hpp"

#include "game_preferences.hpp"
#include "log.hpp"
#include "mouse_events.hpp"
#include "resources.hpp"
#include "terrain_filter.hpp"
#include "unit_map.hpp"

#include <boost/foreach.hpp>

#define LOG_DP LOG_STREAM(info, display)


/**
 * Returns a string whose first line is @a number, centered over a second line,
 * which consists of @a text.
 * If the number is 0, the first line is suppressed.
 */
static std::string number_and_text(int number, const std::string & text)
{
	// Simple case.
	if ( number == 0 )
		return text;

 	std::ostringstream result;

	if ( text.empty() )
		result << number;
	else
		result << std::string((text.size()+1)/2, ' ') << number << '\n' << text;

	return result.str();
}


/**
 * Animates a teleportation between hexes.
 *
 * @param a          The starting hex.
 * @param b          The ending hex.
 * @param temp_unit  The unit to animate (historically, a temporary unit).
 * @param disp       The game display. Assumed neither locked nor faked.
 */
static void teleport_unit_between(const map_location& a, const map_location& b,
                                  unit& temp_unit, display& disp)
{
	if ( disp.fogged(a) && disp.fogged(b) ) {
		return;
	}

	temp_unit.set_location(a);
	if ( !disp.fogged(a) ) { // teleport
		disp.invalidate(a);
		temp_unit.set_facing(a.get_relative_dir(b));
		disp.scroll_to_tiles(a, b, game_display::ONSCREEN, true, 0.0, false);
		unit_animator animator;
		animator.add_animation(&temp_unit,"pre_teleport",a);
		animator.start_animations();
		animator.wait_for_end();
	}

	temp_unit.set_location(b);
	if ( !disp.fogged(b) ) { // teleport
		disp.invalidate(b);
		temp_unit.set_facing(a.get_relative_dir(b));
		disp.scroll_to_tiles(b, a, game_display::ONSCREEN, true, 0.0, false);
		unit_animator animator;
		animator.add_animation(&temp_unit,"post_teleport",b);
		animator.start_animations();
		animator.wait_for_end();
	}

	temp_unit.set_standing();
	disp.update_display();
	events::pump();
}

/**
 * Animates a single step between hexes.
 * This will return before the animation actually finishes, allowing other
 * processing to occur during the animation.
 *
 * @param a          The starting hex.
 * @param b          The ending hex.
 * @param temp_unit  The unit to animate (historically, a temporary unit).
 * @param step_num   The number of steps taken so far (used to pick an animation).
 * @param step_left  The number of steps remaining (used to pick an animation).
 * @param animator   The unit_animator to use. This is assumed clear when we start,
 *                   but will likely not be clear when we return.
 * @param disp       The game display. Assumed neither locked nor faked.
 * @returns  The animation potential until this animation will finish.
 *           INT_MIN indicates that no animation is pending.
 */
static int move_unit_between(const map_location& a, const map_location& b,
                             unit& temp_unit, unsigned int step_num,
                             unsigned int step_left, unit_animator & animator,
                             display& disp)
{
	if ( disp.fogged(a) && disp.fogged(b) ) {
		return INT_MIN;
	}

	temp_unit.set_location(a);
	disp.invalidate(a);
	temp_unit.set_facing(a.get_relative_dir(b));
	animator.replace_anim_if_invalid(&temp_unit,"movement",a,b,step_num,
			false,"",0,unit_animation::INVALID,NULL,NULL,step_left);
	animator.start_animations();
	animator.pause_animation();
	disp.scroll_to_tiles(a, b, game_display::ONSCREEN, true, 0.0, false);
	animator.restart_animation();

	// useless now, previous short draw() just did one
	// new_animation_frame();

	int target_time = animator.get_animation_time_potential();
		// target_time must be short to avoid jumpy move
		// std::cout << "target time: " << target_time << "\n";
	// we round it to the next multiple of 200
	target_time += 200;
	target_time -= target_time%200;

	// This code causes backwards teleport because the time > 200 causes offset > 1.0
	// which will not match with the following -1.0
	// if(  target_time - animator.get_animation_time_potential() < 100 ) target_time +=200;

	return target_time;
}

namespace unit_display
{

/**
 * The path must remain unchanged for the life of this object.
 */
unit_mover::unit_mover(const std::vector<map_location>& path, bool animate, bool force_scroll) :
	disp_(game_display::get_singleton()),
	can_draw_(disp_  &&  !disp_->video().update_locked()  &&
	          !disp_->video().faked()  &&  path.size() > 1),
	animate_(animate),
	force_scroll_(force_scroll),
	animator_(),
	wait_until_(INT_MIN),
	shown_unit_(NULL),
	path_(path),
	current_(0),
	temp_unit_ptr_(NULL),
	// Somewhat arbitrary default values.
	was_hidden_(false),
	is_enemy_(true)
{
	// Some error conditions that indicate something has gone very wrong.
	// (This class can handle these conditions, but someone wanted them
	// to be assertions.)
	assert(!path_.empty());
	assert(disp_);
}


unit_mover::~unit_mover()
{
	// Make sure a unit hidden for movement is unhidden.
	update_shown_unit();
	// For safety, clear the animator before deleting the temp unit.
	animator_.clear();
	// The temp unit's destructor will remove it from the display.
	delete temp_unit_ptr_;
}


/**
 * Makes the temporary unit used by this match the supplied unit.
 * This is called when setting the initial unit, as well as replacing it with
 * something new.
 * When this finishes, the supplied unit is hidden, while the temporary unit
 * is not hidden.
 */
/* Note: Hide the unit in its current location; do not actually remove it.
 * Otherwise the status displays will be wrong during the movement.
 */
void unit_mover::replace_temporary(unit & u)
{
	if ( disp_ == NULL )
		// No point in creating a temp unit with no way to display it.
		return;

	// Save the hidden state of the unit.
	was_hidden_ = u.get_hidden();

	// Make our temporary unit mostly match u...
	if ( temp_unit_ptr_ == NULL ) {
		temp_unit_ptr_ = new game_display::fake_unit(u);
		temp_unit_ptr_->place_on_game_display(disp_);
	}
	else
		*temp_unit_ptr_ = u;
	// ... but keep the temporary unhidden and hide the original.
	temp_unit_ptr_->set_hidden(false);
	u.set_hidden(true);

	// Update cached data.
	is_enemy_ =	(*resources::teams)[u.side()-1].is_enemy(disp_->viewing_side());
}


/**
 * Switches the display back to *shown_unit_ after animating.
 * This uses temp_unit_ptr_, so (in the destructor) call this before deleting
 * temp_unit_ptr_.
 */
void unit_mover::update_shown_unit()
{
	if ( shown_unit_ != NULL ) {
		// Switch the display back to the real unit.
		shown_unit_->set_hidden(was_hidden_);
		temp_unit_ptr_->set_hidden(true);
		shown_unit_ = NULL;
	}
}


/**
 * Initiates the display of movement for the supplied unit.
 * This should be called before attempting to display moving to a new hex.
 */
void unit_mover::start(unit& u)
{
	// Nothing to do here if there is nothing to animate.
	if ( !can_draw_  || !animate_ )
		return;

	// This normally does nothing, but just in case...
	wait_for_anims();

	// Visually replace the original unit with the temporary.
	// (Original unit is left on the map, so the unit count is correct.)
	replace_temporary(u);

	// Initialize our temporary unit for the move.
	temp_unit_ptr_->set_location(path_[0]);
	temp_unit_ptr_->set_facing(path_[0].get_relative_dir(path_[1]));
	temp_unit_ptr_->set_standing(false);
	disp_->invalidate(path_[0]);

	// If the unit can be seen here by the viewing side:
	if ( !is_enemy_ || !temp_unit_ptr_->invisible(path_[0]) ) {
		// Scroll to the path, but only if it fully fits on screen.
		// If it does not fit we might be able to do a better scroll later.
		disp_->scroll_to_tiles(path_, game_display::ONSCREEN, true, true, 0.0, false);
	}
	// We need to clear big invalidation before the move and have a smooth animation
	// (mainly black stripes and invalidation after canceling attack dialog).
	// Two draw calls are needed to also redraw the previously invalidated hexes.
	// We use update=false because we don't need delay here (no time wasted)
	// and no screen refresh (will be done by last 3rd draw() and it optimizes
	// the double blitting done by these invalidations).
	disp_->draw(false);
	disp_->draw(false);

	// The last draw() was still slow, and its initial new_animation_frame() call
	// is now old, so we do another draw() to get a fresh one
	// TODO: replace that by a new_animation_frame() before starting anims
	//       don't forget to change the previous draw(false) to true
	disp_->draw(true);

	// extra immobile movement animation for take-off
	animator_.add_animation(temp_unit_ptr_, "pre_movement", path_[0], path_[1]);
	animator_.start_animations();
	animator_.wait_for_end();
	animator_.clear();

	// Switch the display back to the real unit.
	u.set_facing(temp_unit_ptr_->facing());
	u.set_standing(false);	// Need to reset u's animation so the new facing takes effect.
	u.set_hidden(was_hidden_);
	temp_unit_ptr_->set_hidden(true);
}


/**
 * Visually moves a unit from the last hex we drew to the one specified by
 * @a path_index. If @a path_index points to an earlier hex, we do nothing.
 * The moving unit will only be updated if update is set to true; otherwise,
 * the provided unit is merely hidden during the movement and re-shown after.
 * (Not updating the unit can produce smoother animations in some cases.)
 * If @a wait is set to false, this returns without waiting for the final
 * animation to finish. Call wait_for_anims() to explicitly get this final
 * wait (another call to proceed_to() or finish() will implicitly wait). The
 * unit must remain valid until the wait is finished.
 */
void unit_mover::proceed_to(unit& u, size_t path_index, bool update, bool wait)
{
	// Nothing to do here if animations cannot be shown.
	if ( !can_draw_ )
		return;

	// If no animation then hide unit until end of movement
	if (!animate_)
	{
		u.set_hidden(true);
		return;
	}

	// Handle pending visibility issues before introducing new ones.
	wait_for_anims();

	if ( update  ||  temp_unit_ptr_ == NULL )
		// Replace the temp unit (which also hides u and shows our temporary).
		replace_temporary(u);
	else
	{
		// Just switch the display from the real unit to our fake one.
		temp_unit_ptr_->set_hidden(false);
		u.set_hidden(true);
	}

	// Safety check.
	path_index = std::min(path_index, path_.size()-1);

	for ( ; current_ < path_index; ++current_ )
		// If the unit can be seen by the viewing side while making this step:
		if ( !is_enemy_ || !temp_unit_ptr_->invisible(path_[current_]) ||
		     !temp_unit_ptr_->invisible(path_[current_+1]) )
		{
			// Wait for the previous step to complete before drawing the next one.
			wait_for_anims();

			if ( !disp_->tile_fully_on_screen(path_[current_]) ||
			     !disp_->tile_fully_on_screen(path_[current_+1]))
			{
				// prevent the unit from disappearing if we scroll here with i == 0
				temp_unit_ptr_->set_location(path_[current_]);
				disp_->invalidate(path_[current_]);
				// scroll in as much of the remaining path as possible
				if ( temp_unit_ptr_->get_animation() )
					temp_unit_ptr_->get_animation()->pause_animation();
				disp_->scroll_to_tiles(path_.begin() + current_,
				                       path_.end(), game_display::ONSCREEN,
				                       true, false, 0.0, force_scroll_);
				if ( temp_unit_ptr_->get_animation() )
					temp_unit_ptr_->get_animation()->restart_animation();
			}

			if ( tiles_adjacent(path_[current_], path_[current_+1]) )
				wait_until_ =
					move_unit_between(path_[current_], path_[current_+1],
					                  *temp_unit_ptr_, current_,
					                  path_.size() - (current_+2), animator_,
					                  *disp_);
			else if ( path_[current_] != path_[current_+1] )
				teleport_unit_between(path_[current_], path_[current_+1],
				                      *temp_unit_ptr_, *disp_);
		}

	// Update the unit's facing.
	u.set_facing(temp_unit_ptr_->facing());
	u.set_standing(false);	// Need to reset u's animation so the new facing takes effect.
	// Remember the unit to unhide when the animation finishes.
	shown_unit_ = &u;
	if ( wait )
		wait_for_anims();
}


/**
 * Waits for the final animation of the most recent proceed_to() to finish.
 * It is not necessary to call this unless you want to wait before the next
 * call to proceed_to() or finish().
 */
void unit_mover::wait_for_anims()
{
	if ( wait_until_ == INT_MAX )
		// Wait for end (not currently used, but still supported).
		animator_.wait_for_end();
	else if ( wait_until_ != INT_MIN ) {
		// Wait until the specified time (used for normal movement).
		animator_.wait_until(wait_until_);
		// debug code, see unit_frame::redraw()
		// std::cout << "   end\n";

		/// @todo Are these invalidations really (or still) necessary?
		/// A quick check suggested they are not needed, but that was
		/// not comprehensive.
		if ( disp_ ) { // Should always be true if we get here.
			// Invalidate the hexes around the move that prompted this wait.
			map_location arr[6];
			get_adjacent_tiles(path_[current_-1], arr);
			for ( unsigned i = 0; i < 6; ++i )
				disp_->invalidate(arr[i]);
			get_adjacent_tiles(path_[current_], arr);
			for ( unsigned i = 0; i < 6; ++i )
				disp_->invalidate(arr[i]);
		}
	}

	// Reset data.
	wait_until_ = INT_MIN;
	animator_.clear();

	update_shown_unit();
}


/**
 * Finishes the display of movement for the supplied unit.
 * If called before showing the unit reach the end of the path, it will be
 * assumed that the movement ended early.
 * If @a dir is not supplied, the final direction will be determined by (the
 * last two traversed hexes of) the path.
 */
void unit_mover::finish(unit &u, map_location::DIRECTION dir)
{
	// Nothing to do here if the display is not valid.
	if ( !can_draw_ )
		return;

	const map_location & end_loc = path_[current_];
	const map_location::DIRECTION final_dir = current_ == 0 ?
		path_[0].get_relative_dir(path_[1]) :
		path_[current_-1].get_relative_dir(end_loc);

	if ( animate_ )
	{
		wait_for_anims(); // In case proceed_to() did not wait for the last animation.

		// Make sure the displayed unit is correct.
		replace_temporary(u);
		temp_unit_ptr_->set_location(end_loc);
		temp_unit_ptr_->set_facing(final_dir);

		// Animation
		animator_.add_animation(temp_unit_ptr_, "post_movement", end_loc);
		animator_.start_animations();
		animator_.wait_for_end();
		animator_.clear();

		// Switch the display back to the real unit.
		u.set_hidden(was_hidden_);
		temp_unit_ptr_->set_hidden(true);

		events::mouse_handler* mousehandler = events::mouse_handler::get_singleton();
		if ( mousehandler ) {
			mousehandler->invalidate_reachmap();
		}
	}
	else
	{
		// Show the unit at end of skipped animation
		u.set_hidden(was_hidden_);
	}

	// Facing gets set even when not animating.
	u.set_facing(dir == map_location::NDIRECTIONS ? final_dir : dir);
	u.set_standing(true);	// Need to reset u's animation so the new facing takes effect.

	// Redraw path ends (even if not animating).
	disp_->invalidate(path_.front());
	disp_->invalidate(end_loc);
}


/**
 * Display a unit moving along a given path.
 *
 * @param path     The path to traverse.
 * @param u        The unit to show being moved. Its facing will be updated,
 *                 but not its position.
 * @param animate  If set to false, only side-effects of move are applied
 *                 (correct unit facing, path hexes redrawing).
 * @param dir      Unit will be set facing this direction after move.
 *                 If nothing passed, direction will be set based on path.
 */
/* Note: Hide the unit in its current location,
 * but don't actually remove it until the move is done,
 * so that while the unit is moving status etc.
 * will still display the correct number of units.
 */
void move_unit(const std::vector<map_location>& path, unit& u,
               bool animate, map_location::DIRECTION dir,
               bool force_scroll)
{
	unit_mover mover(path, animate, force_scroll);

	mover.start(u);
	mover.proceed_to(u, path.size());
	mover.finish(u, dir);
}


void reset_helpers(const unit *attacker,const unit *defender);

void unit_draw_weapon(const map_location& loc, unit& attacker,
		const attack_type* attack,const attack_type* secondary_attack, const map_location& defender_loc,unit* defender)
{
	display* disp = display::get_singleton();
	if(!disp ||disp->video().update_locked() || disp->video().faked() || disp->fogged(loc) || preferences::show_combat() == false) {
		return;
	}
	unit_animator animator;
	animator.add_animation(&attacker,"draw_weapon",loc,defender_loc,0,false,"",0,unit_animation::HIT,attack,secondary_attack,0);
	animator.add_animation(defender,"draw_weapon",defender_loc,loc,0,false,"",0,unit_animation::MISS,secondary_attack,attack,0);
	animator.start_animations();
	animator.wait_for_end();

}


void unit_sheath_weapon(const map_location& primary_loc, unit* primary_unit,
		const attack_type* primary_attack,const attack_type* secondary_attack, const map_location& secondary_loc,unit* secondary_unit)
{
	display* disp = display::get_singleton();
	if(!disp ||disp->video().update_locked() || disp->video().faked() || disp->fogged(primary_loc) || preferences::show_combat() == false) {
		return;
	}
	unit_animator animator;
	if(primary_unit) {
		animator.add_animation(primary_unit,"sheath_weapon",primary_loc,secondary_loc,0,false,"",0,unit_animation::INVALID,primary_attack,secondary_attack,0);
	}
	if(secondary_unit) {
		animator.add_animation(secondary_unit,"sheath_weapon",secondary_loc,primary_loc,0,false,"",0,unit_animation::INVALID,secondary_attack,primary_attack,0);
	}

	if(primary_unit || secondary_unit) {
		animator.start_animations();
		animator.wait_for_end();
	}
	if(primary_unit) {
		primary_unit->set_standing();
	}
	if(secondary_unit) {
		secondary_unit->set_standing();
	}
	reset_helpers(primary_unit,secondary_unit);

}


void unit_die(const map_location& loc, unit& loser,
		const attack_type* attack,const attack_type* secondary_attack, const map_location& winner_loc,unit* winner)
{
	display* disp = display::get_singleton();
	if(!disp ||disp->video().update_locked() || disp->video().faked() || disp->fogged(loc) || preferences::show_combat() == false) {
		return;
	}
	unit_animator animator;
	// hide the hp/xp bars of the loser (useless and prevent bars around an erased unit)
	animator.add_animation(&loser,"death",loc,winner_loc,0,false,"",0,unit_animation::KILL,attack,secondary_attack,0);
	// but show the bars of the winner (avoid blinking and show its xp gain)
	animator.add_animation(winner,"victory",winner_loc,loc,0,true,"",0,
			unit_animation::KILL,secondary_attack,attack,0);
	animator.start_animations();
	animator.wait_for_end();

	reset_helpers(winner,&loser);
	events::mouse_handler* mousehandler = events::mouse_handler::get_singleton();
	if (mousehandler) {
		mousehandler->invalidate_reachmap();
	}
}


void unit_attack(
                 const map_location& a, const map_location& b, int damage,
                 const attack_type& attack, const attack_type* secondary_attack,
		  int swing,std::string hit_text,int drain_amount,std::string att_text)
{
	display* disp = display::get_singleton();
	if(!disp ||disp->video().update_locked() || disp->video().faked() ||
			(disp->fogged(a) && disp->fogged(b)) || preferences::show_combat() == false) {
		return;
	}
	unit_map& units = disp->get_units();
	disp->select_hex(map_location::null_location);

	// scroll such that there is at least half a hex spacing around fighters
	disp->scroll_to_tiles(a,b,game_display::ONSCREEN,true,0.5,false);

	log_scope("unit_attack");

	const unit_map::iterator att = units.find(a);
	assert(att != units.end());
	unit& attacker = *att;

	const unit_map::iterator def = units.find(b);
	assert(def != units.end());
	unit &defender = *def;
	int def_hitpoints = defender.hitpoints();

	att->set_facing(a.get_relative_dir(b));
	def->set_facing(b.get_relative_dir(a));
	defender.set_facing(b.get_relative_dir(a));


	unit_animator animator;
	unit_ability_list leaders = attacker.get_abilities("leadership");
	unit_ability_list helpers = defender.get_abilities("resistance");

	std::string text   = number_and_text(damage, hit_text);
	std::string text_2 = number_and_text(abs(drain_amount), att_text);

	unit_animation::hit_type hit_type;
	if(damage >= defender.hitpoints()) {
		hit_type = unit_animation::KILL;
	} else if(damage > 0) {
		hit_type = unit_animation::HIT;
	}else {
		hit_type = unit_animation::MISS;
	}
	animator.add_animation(&attacker, "attack", att->get_location(),
		def->get_location(), damage, true,  text_2,
		(drain_amount >= 0) ? display::rgb(0, 255, 0) : display::rgb(255, 0, 0),
		hit_type, &attack, secondary_attack, swing);

	// note that we take an anim from the real unit, we'll use it later
	const unit_animation *defender_anim = def->choose_animation(*disp,
		def->get_location(), "defend", att->get_location(), damage,
		hit_type, &attack, secondary_attack, swing);
	animator.add_animation(&defender, defender_anim, def->get_location(),
		true,  text , display::rgb(255, 0, 0));

	BOOST_FOREACH (const unit_ability & ability, leaders) {
		if(ability.second == a) continue;
		if(ability.second == b) continue;
		unit_map::iterator leader = units.find(ability.second);
		assert(leader != units.end());
		leader->set_facing(ability.second.get_relative_dir(a));
		animator.add_animation(&*leader, "leading", ability.second,
			att->get_location(), damage, true,  "", 0,
			hit_type, &attack, secondary_attack, swing);
	}
	BOOST_FOREACH (const unit_ability & ability, helpers) {
		if(ability.second == a) continue;
		if(ability.second == b) continue;
		unit_map::iterator helper = units.find(ability.second);
		assert(helper != units.end());
		helper->set_facing(ability.second.get_relative_dir(b));
		animator.add_animation(&*helper, "resistance", ability.second,
			def->get_location(), damage, true,  "", 0,
			hit_type, &attack, secondary_attack, swing);
	}


	animator.start_animations();
	animator.wait_until(0);
	int damage_left = damage;
	while(damage_left > 0 && !animator.would_end()) {
		int step_left = (animator.get_end_time() - animator.get_animation_time() )/50;
		if(step_left < 1) step_left = 1;
		int removed_hp =  damage_left/step_left ;
		if(removed_hp < 1) removed_hp = 1;
		defender.take_hit(removed_hp);
		damage_left -= removed_hp;
		animator.wait_until(animator.get_animation_time_potential() +50);
	}
	animator.wait_for_end();
	// pass the animation back to the real unit
	def->start_animation(animator.get_end_time(), defender_anim, true);
	reset_helpers(&*att, &*def);
	def->set_hitpoints(def_hitpoints);
}

// private helper function, set all helpers to default position
void reset_helpers(const unit *attacker,const unit *defender)
{
	display* disp = display::get_singleton();
	unit_map& units = disp->get_units();
	if(attacker) {
		unit_ability_list leaders = attacker->get_abilities("leadership");
		BOOST_FOREACH (const unit_ability & ability, leaders) {
			unit_map::iterator leader = units.find(ability.second);
			assert(leader != units.end());
			leader->set_standing();
		}
	}

	if(defender) {
		unit_ability_list helpers = defender->get_abilities("resistance");
		BOOST_FOREACH (const unit_ability & ability, helpers) {
			unit_map::iterator helper = units.find(ability.second);
			assert(helper != units.end());
			helper->set_standing();
		}
	}
}

void unit_recruited(const map_location& loc,const map_location& leader_loc)
{
	game_display* disp = game_display::get_singleton();
	if(!disp || disp->video().update_locked() || disp->video().faked() ||disp->fogged(loc)) return;
	unit_map::iterator u = disp->get_units().find(loc);
	if(u == disp->get_units().end()) return;
	u->set_hidden(true);

	unit_animator animator;
	if(leader_loc != map_location::null_location) {
		unit_map::iterator leader = disp->get_units().find(leader_loc);
		if(leader == disp->get_units().end()) return;
		disp->scroll_to_tiles(loc,leader_loc,game_display::ONSCREEN,true,0.0,false);
		leader->set_facing(leader_loc.get_relative_dir(loc));
		animator.add_animation(&*leader, "recruiting", leader_loc, loc, 0, true);
	} else {
		disp->scroll_to_tile(loc,game_display::ONSCREEN,true,false);
	}

	disp->draw();
	u->set_hidden(false);
	animator.add_animation(&*u, "recruited", loc, leader_loc);
	animator.start_animations();
	animator.wait_for_end();
	animator.set_all_standing();
	if (loc==disp->mouseover_hex()) disp->invalidate_unit();
}

void unit_healing(unit &healed, const std::vector<unit *> &healers, int healing,
                  const std::string & extra_text)
{
	game_display* disp = game_display::get_singleton();
	const map_location &healed_loc = healed.get_location();
	if(!disp || disp->video().update_locked() || disp->video().faked() || disp->fogged(healed_loc)) return;

	// This is all the pretty stuff.
	disp->scroll_to_tile(healed_loc, game_display::ONSCREEN,true,false);
	disp->display_unit_hex(healed_loc);
	unit_animator animator;

	BOOST_FOREACH(unit *h, healers) {
		h->set_facing(h->get_location().get_relative_dir(healed_loc));
		animator.add_animation(h, "healing", h->get_location(),
			healed_loc, healing);
	}

	if (healing < 0) {
		animator.add_animation(&healed, "poisoned", healed_loc,
		                       map_location::null_location, -healing, false,
		                       number_and_text(-healing, extra_text),
		                       display::rgb(255,0,0));
	} else if ( healing > 0 ) {
		animator.add_animation(&healed, "healed", healed_loc,
		                       map_location::null_location, healing, false,
		                       number_and_text(healing, extra_text),
		                       display::rgb(0,255,0));
	} else {
		animator.add_animation(&healed, "healed", healed_loc,
		                       map_location::null_location, 0, false,
		                       extra_text, display::rgb(0,255,0));
	}
	animator.start_animations();
	animator.wait_for_end();
	animator.set_all_standing();
}

void wml_animation_internal(unit_animator &animator, const vconfig &cfg, const map_location &default_location = map_location::null_location);

void wml_animation(const vconfig &cfg, const map_location &default_location)
{
	game_display &disp = *resources::screen;
	if (disp.video().update_locked() || disp.video().faked()) return;
	unit_animator animator;
	wml_animation_internal(animator, cfg, default_location);
	animator.start_animations();
	animator.wait_for_end();
	animator.set_all_standing();
}

void wml_animation_internal(unit_animator &animator, const vconfig &cfg, const map_location &default_location)
{
	unit_map::iterator u = resources::units->find(default_location);

	// Search for a valid unit filter,
	// and if we have one, look for the matching unit
	vconfig filter = cfg.child("filter");
	if(!filter.null()) {
		for (u = resources::units->begin(); u != resources::units->end(); ++u) {
			if ( u->matches_filter(filter) )
				break;
		}
	}

	// We have found a unit that matches the filter
	if (u.valid() && !resources::screen->fogged(u->get_location()))
	{
		attack_type *primary = NULL;
		attack_type *secondary = NULL;
		Uint32 text_color;
		unit_animation::hit_type hits=  unit_animation::INVALID;
		std::vector<attack_type> attacks = u->attacks();
		std::vector<attack_type>::iterator itor;

		filter = cfg.child("primary_attack");
		if(!filter.null()) {
			for(itor = attacks.begin(); itor != attacks.end(); ++itor){
				if(itor->matches_filter(filter.get_parsed_config())) {
					primary = &*itor;
					break;
				}
			}
		}

		filter = cfg.child("secondary_attack");
		if(!filter.null()) {
			for(itor = attacks.begin(); itor != attacks.end(); ++itor){
				if(itor->matches_filter(filter.get_parsed_config())) {
					secondary = &*itor;
					break;
				}
			}
		}

		if(cfg["hits"] == "yes" || cfg["hits"] == "hit") {
			hits = unit_animation::HIT;
		}
		if(cfg["hits"] == "no" || cfg["hits"] == "miss") {
			hits = unit_animation::MISS;
		}
		if( cfg["hits"] == "kill" ) {
			hits = unit_animation::KILL;
		}
		if(cfg["red"].empty() && cfg["green"].empty() && cfg["blue"].empty()) {
			text_color = display::rgb(0xff,0xff,0xff);
		} else {
			text_color = display::rgb(cfg["red"], cfg["green"], cfg["blue"]);
		}
		resources::screen->scroll_to_tile(u->get_location(), game_display::ONSCREEN, true, false);
		vconfig t_filter = cfg.child("facing");
		map_location secondary_loc = map_location::null_location;
		if(!t_filter.empty()) {
			terrain_filter filter(t_filter, *resources::units);
			std::set<map_location> locs;
			filter.get_locations(locs);
			if (!locs.empty() && u->get_location() != *locs.begin()) {
				map_location::DIRECTION dir =u->get_location().get_relative_dir(*locs.begin());
				u->set_facing(dir);
				secondary_loc = u->get_location().get_direction(dir);
			}
		}
		animator.add_animation(&*u, cfg["flag"], u->get_location(),
			secondary_loc, cfg["value"], cfg["with_bars"].to_bool(),
			cfg["text"], text_color, hits, primary, secondary,
			cfg["value_second"]);
	}
	const vconfig::child_list sub_anims = cfg.get_children("animate");
	vconfig::child_list::const_iterator anim_itor;
	for(anim_itor = sub_anims.begin(); anim_itor != sub_anims.end();++anim_itor) {
		wml_animation_internal(animator, *anim_itor);
	}

}
} // end unit_display namespace
