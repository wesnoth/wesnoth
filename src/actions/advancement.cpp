/*
   Copyright (C) 2016 - 2018 by the Battle for Wesnoth Project http://www.wesnoth.org/

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
 * Fighting.
 */

#include "actions/advancement.hpp"

#include "actions/vision.hpp"

#include "ai/lua/aspect_advancements.hpp"
#include "game_events/pump.hpp"
#include "preferences/game.hpp"
#include "game_data.hpp" //resources::gamedata->phase()
#include "gui/dialogs/unit_advance.hpp"
#include "gui/widgets/window.hpp" //gui2::window::OK
#include "log.hpp"
#include "play_controller.hpp" //resources::controller
#include "random.hpp"
#include "resources.hpp"
#include "statistics.hpp"
#include "synced_user_choice.hpp"
#include "units/unit.hpp"
#include "units/abilities.hpp"
#include "units/animation_component.hpp"
#include "units/udisplay.hpp"
#include "units/helper.hpp" //number_of_possible_advances
#include "whiteboard/manager.hpp"

static lg::log_domain log_engine("engine");
#define DBG_NG LOG_STREAM(debug, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)
#define WRN_NG LOG_STREAM(err, log_engine)
#define ERR_NG LOG_STREAM(err, log_engine)

static lg::log_domain log_config("config");
#define LOG_CF LOG_STREAM(info, log_config)

static lg::log_domain log_display("display");
#define LOG_DP LOG_STREAM(info, log_display)


namespace
{
	int advance_unit_dialog(const map_location &loc)
	{
		const unit& u = *resources::gameboard->units().find(loc);
		std::vector<unit_const_ptr> previews;

		for (const std::string& advance : u.advances_to()) {
			preferences::encountered_units().insert(advance);
			previews.push_back(get_advanced_unit(u, advance));
		}

		size_t num_real_advances = previews.size();
		bool always_display = false;

		for (const config& advance : u.get_modification_advances()) {
			if (advance["always_display"]) {
				always_display = true;
			}
			previews.push_back(get_amla_unit(u, advance));
		}

		if (previews.size() > 1 || always_display) {
			gui2::dialogs::unit_advance dlg(previews, num_real_advances);

			dlg.show();

			if (dlg.get_retval() == gui2::window::OK) {
				return dlg.get_selected_index();
			}

			// This should be unreachable, since canceling is disabled for the dialog
			assert(false && "Unit advance dialog was cancelled, which should be impossible.");
		}

		return 0;
	}

	bool animate_unit_advancement(const map_location &loc, size_t choice, const bool &fire_event, const bool animate)
	{
		const events::command_disabler cmd_disabler;

		unit_map::iterator u = resources::gameboard->units().find(loc);
		if (u == resources::gameboard->units().end()) {
			LOG_DP << "animate_unit_advancement suppressed: invalid unit\n";
			return false;
		}
		else if (!u->advances()) {
			LOG_DP << "animate_unit_advancement suppressed: unit does not advance\n";
			return false;
		}

		const std::vector<std::string>& options = u->advances_to();
		std::vector<config> mod_options = u->get_modification_advances();

		if (choice >= options.size() + mod_options.size()) {
			LOG_DP << "animate_unit_advancement suppressed: invalid option\n";
			return false;
		}

		// When the unit advances, it fades to white, and then switches
		// to the new unit, then fades back to the normal color

		if (animate && !CVideo::get_singleton().update_locked()) {
			unit_animator animator;
			bool with_bars = true;
			animator.add_animation(&*u, "levelout", u->get_location(), map_location(), 0, with_bars);
			animator.start_animations();
			animator.wait_for_end();
		}

		if (choice < options.size()) {
			// chosen_unit is not a reference, since the unit may disappear at any moment.
			std::string chosen_unit = options[choice];
			::advance_unit(loc, chosen_unit, fire_event);
		}
		else {
			const config &mod_option = mod_options[choice - options.size()];
			::advance_unit(loc, &mod_option, fire_event);
		}

		u = resources::gameboard->units().find(loc);
		resources::screen->invalidate_unit();

		if (animate && u != resources::gameboard->units().end() && !CVideo::get_singleton().update_locked()) {
			unit_animator animator;
			animator.add_animation(&*u, "levelin", u->get_location(), map_location(), 0, true);
			animator.start_animations();
			animator.wait_for_end();
			animator.set_all_standing();
			resources::screen->invalidate(loc);
			events::pump();
		}

		resources::screen->invalidate_all();

		return true;
	}

	class unit_advancement_choice : public mp_sync::user_choice
	{
	public:
		unit_advancement_choice(const map_location& loc, int total_opt, int side_num, const ai::unit_advancements_aspect* ai_advancement, bool force_dialog)
			: loc_ (loc), nb_options_(total_opt), side_num_(side_num), ai_advancement_(ai_advancement), force_dialog_(force_dialog)
		{
		}

		virtual ~unit_advancement_choice()
		{
		}

		virtual config query_user(int /*side*/) const
		{
			//the 'side' parameter might differ from side_num_-
			int res = 0;
			team t = resources::gameboard->get_team(side_num_);
			//i wonder how this got included here ?
			bool is_mp = resources::controller->is_networked_mp();
			bool is_current_side = resources::controller->current_side() == side_num_;
			//note, that the advancements for networked sides are also determined on the current playing side.

			//to make mp games equal we only allow selecting advancements to the current side.
			//otherwise we'd give an unfair advantage to the side that hosts ai sides if units advance during ai turns.
			if(!CVideo::get_singleton().non_interactive() && (force_dialog_ || (t.is_local_human() && !t.is_idle() && (is_current_side || !is_mp))))
			{
				res = advance_unit_dialog(loc_);
			}
			else if(t.is_local_ai() || t.is_network_ai() || t.is_empty())
			{
				res = randomness::generator->get_random_int(0, nb_options_-1);

				//if ai_advancement_ is the default advancement the following code will
				//have no effect because get_advancements returns an empty list.
				if(ai_advancement_ != nullptr)
				{
					unit_map::iterator u = resources::gameboard->units().find(loc_);
					const std::vector<std::string>& options = u->advances_to();
					const std::vector<std::string>& allowed = ai_advancement_->get_advancements(u);

					for(std::vector<std::string>::const_iterator a = options.begin(); a != options.end(); ++a) {
						if (std::find(allowed.begin(), allowed.end(), *a) != allowed.end()){
							res = a - options.begin();
							break;
						}
					}
				}

			}
			else
			{
				//we are in the situation, that the unit is owned by a human, but he's not allowed to do this decision.
				//because it's a mp game and it's not his turn.
				//note that it doesn't matter whether we call randomness::generator->next_random() or rand().
				res = randomness::generator->get_random_int(0, nb_options_-1);
			}
			LOG_NG << "unit at position " << loc_ << "choose advancement number " << res << "\n";
			config retv;
			retv["value"] = res;
			return retv;

		}
		virtual config random_choice(int /*side*/) const
		{
			config retv;
			retv["value"] = 0;
			return retv;
		}
		virtual std::string description() const
		{
			return "an advancement choice";
		}
	private:
		const map_location loc_;
		int nb_options_;
		int side_num_;
		const ai::unit_advancements_aspect* ai_advancement_;
		bool force_dialog_;
	};
}

/*
advances the unit and stores data in the replay (or reads data from replay).
*/
void advance_unit_at(const advance_unit_params& params)
{
	//i just don't want infinite loops...
	// the 20 is picked rather randomly.
	for(int advacment_number = 0; advacment_number < 20; advacment_number++)
	{
		unit_map::iterator u = resources::gameboard->units().find(params.loc_);
		//this implies u.valid()
		if(!unit_helper::will_certainly_advance(u)) {
			return;
		}

		if(params.fire_events_)
		{
			LOG_NG << "Firing pre advance event at " << params.loc_ <<".\n";
			resources::game_events->pump().fire("pre_advance", params.loc_);
			//TODO: maybe use id instead of location here ?.
			u = resources::gameboard->units().find(params.loc_);
			if(!unit_helper::will_certainly_advance(u))
			{
				LOG_NG << "pre advance event aborted advancing.\n";
				return;
			}
		}
		//we don't want to let side 1 decide it during start/prestart.
		int side_for = resources::gamedata->phase() == game_data::PLAY ? 0: u->side();
		config selected = mp_sync::get_user_choice("choose",
			unit_advancement_choice(params.loc_, unit_helper::number_of_possible_advances(*u), u->side(), params.ai_advancements_, params.force_dialog_), side_for);
		//calls actions::advance_unit.
		bool result = animate_unit_advancement(params.loc_, selected["value"], params.fire_events_, params.animate_);

		DBG_NG << "animate_unit_advancement result = " << result << std::endl;
		u = resources::gameboard->units().find(params.loc_);
		// level 10 unit gives 80 XP and the highest mainline is level 5
		if (u.valid() && u->experience() > 80)
		{
			WRN_NG << "Unit has too many (" << u->experience() << ") XP left; cascade leveling goes on still." << std::endl;
		}
	}
	ERR_NG << "unit at " << params.loc_ << "tried to advance more than 20 times. Advancing was aborted" << std::endl;
}

unit_ptr get_advanced_unit(const unit &u, const std::string& advance_to)
{
	const unit_type *new_type = unit_types.find(advance_to);
	if (!new_type) {
		throw game::game_error("Could not find the unit being advanced"
			" to: " + advance_to);
	}
	unit_ptr new_unit(new unit(u));
	new_unit->set_experience(new_unit->experience_overflow());
	new_unit->advance_to(*new_type);
	new_unit->heal_fully();
	new_unit->set_state(unit::STATE_POISONED, false);
	new_unit->set_state(unit::STATE_SLOWED, false);
	new_unit->set_state(unit::STATE_PETRIFIED, false);
	new_unit->set_user_end_turn(false);
	new_unit->set_hidden(false);
	return new_unit;
}


/**
 * Returns the AMLA-advanced version of a unit (with traits and items retained).
 */
unit_ptr get_amla_unit(const unit &u, const config &mod_option)
{
	unit_ptr amla_unit(new unit(u));
	amla_unit->set_experience(amla_unit->experience_overflow());
	amla_unit->add_modification("advancement", mod_option);
	return amla_unit;
}


void advance_unit(map_location loc, const advancement_option &advance_to, bool fire_event)
{
	unit_map::unit_iterator u = resources::gameboard->units().find(loc);
	if(!u.valid()) {
		return;
	}
	// original_type is not a reference, since the unit may disappear at any moment.
	std::string original_type = u->type_id();

	// "advance" event.
	if(fire_event)
	{
		LOG_NG << "Firing advance event at " << loc <<".\n";
		resources::game_events->pump().fire("advance",loc);

		if (!u.valid() || u->experience() < u->max_experience() ||
			u->type_id() != original_type)
		{
			LOG_NG << "WML has invalidated the advancing unit. Aborting.\n";
			return;
		}
		// In case WML moved the unit:
		loc = u->get_location();
	}

	// This is not normally necessary, but if a unit loses power when leveling
	// (e.g. loses "jamming" or ambush), it could be discovered as a result of
	// the advancement.
	std::vector<int> not_seeing = actions::get_sides_not_seeing(*u);

	// Create the advanced unit.
	bool use_amla = boost::get<std::string>(&advance_to) == nullptr;
	unit_ptr new_unit = use_amla ? get_amla_unit(*u, *boost::get<const config*>(advance_to)) :
	                           get_advanced_unit(*u, boost::get<std::string>(advance_to));
	new_unit->set_location(loc);
	if ( !use_amla )
	{
		statistics::advance_unit(*new_unit);
		preferences::encountered_units().insert(new_unit->type_id());
		LOG_CF << "Added '" << new_unit->type_id() << "' to the encountered units.\n";
	}
	resources::gameboard->units().erase(loc);
	resources::whiteboard->on_kill_unit();
	u = resources::gameboard->units().insert(new_unit).first;

	// Update fog/shroud.
	actions::shroud_clearer clearer;
	clearer.clear_unit(loc, *new_unit);

	// "post_advance" event.
	if(fire_event)
	{
		LOG_NG << "Firing post_advance event at " << loc << ".\n";
		resources::game_events->pump().fire("post_advance",loc);
	}

	// "sighted" event(s).
	clearer.fire_events();
	if ( u.valid() )
		actions::actor_sighted(*u, &not_seeing);

	resources::whiteboard->on_gamestate_change();
}
