/* $Id$ */
/*
   Copyright (C) 2003 - 2010 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GAME_EVENTS_H_INCLUDED
#define GAME_EVENTS_H_INCLUDED

#include "config.hpp"
#include "map.hpp"
#include "map_location.hpp"
#include "serialization/string_utils.hpp"
#include "variable.hpp"
#include "unit_map.hpp"

#include <vector>
#include <map>

/**
 * @file game_events.hpp
 * Define the game's events mechanism.
 *
 * Events might be units moving or fighting, or when victory or defeat occurs.
 * A scenario's configuration file will define actions to take when certain events occur.
 * This module is responsible for making sure that when the events occur, the actions take place.
 *
 * Note that game events have nothing to do with SDL events,
 * like mouse movement, keyboard events, etc.
 * See events.hpp for how they are handled.
 */

/**
 * Changes a terrain location.
 * Ensures that villages are properly lost and that new terrains are discovered.
 */
void change_terrain(const map_location &loc, const t_translation::t_terrain &t,
	gamemap::tmerge_mode mode, bool replace_if_failed);

namespace game_events
{

	// The game event manager loads the scenario configuration object,
	// and ensures that events are handled according to the
	// scenario configuration for its lifetime.
	//
	// Thus, a manager object should be created when a scenario is played,
	// and destroyed at the end of the scenario.
	struct manager {
		// Note that references will be maintained,
		// and must remain valid for the life of the object.
		manager(const config& scenario_cfg);
		~manager();

	private:
		variable::manager variable_manager;
	};

	struct entity_location : public map_location {
		entity_location(map_location loc, const size_t id=0);
		explicit entity_location(unit_map::iterator itor);
		bool requires_unit() const;
		bool matches_unit(const unit& u) const;
		private:
		size_t id_;
	};


	struct queued_event {
		queued_event(const std::string& name, const game_events::entity_location& loc1,
				const game_events::entity_location& loc2,
				const config& data)
			: name(name), loc1(loc1), loc2(loc2),data(data) {}

		std::string name;
		game_events::entity_location loc1;
		game_events::entity_location loc2;
		config data;
	};

	class event_handler
	{
		public:
			event_handler(const vconfig& cfg, bool is_menu_item=false) :
				first_time_only_(utils::string_bool(cfg["first_time_only"],true)),
				disabled_(false), is_menu_item_(is_menu_item), cfg_(cfg)
			{}

			void read(const vconfig& cfg) { cfg_ = cfg; }
			void write(config& cfg) const
			{
				if(disabled_)
					return;

				cfg = cfg_.get_config();
			}

			bool matches_name(const std::string& name) const;

			bool first_time_only() const { return first_time_only_; }

			void disable() { disabled_ = true; }
			bool disabled() const { return disabled_; }

			bool is_menu_item() const;
			const vconfig::child_list first_arg_filters() const
			{
				return cfg_.get_children("filter");
			}
			const vconfig::child_list first_special_filters() const
			{
				vconfig::child_list kids;
				kids = cfg_.get_children("filter_attack");
				return kids;

			}

			const vconfig::child_list second_arg_filters() const
			{
				return cfg_.get_children("filter_second");
			}
			const vconfig::child_list second_special_filters() const
			{
				vconfig::child_list kids;
				kids = cfg_.get_children("filter_second_attack");
				return kids;
			}

			void handle_event(const queued_event& event_info);

			const vconfig& get_vconfig() const { return cfg_; }
		private:
			bool first_time_only_;
			bool disabled_;
			bool is_menu_item_;
			vconfig cfg_;
	};

	/**
	 * Runs the action handler associated to the command sequence @a cfg.
	 */
	void handle_event_commands(const queued_event &event_info, const vconfig &cfg);

	/**
	 * Runs the action handler associated to @a cmd with parameters @a cfg.
	 */
	void handle_event_command(const std::string &cmd,
		const queued_event &event_info, const vconfig &cfg);

	void write_events(config& cfg);
	void add_events(const config::const_child_itors &cfgs,const std::string& id);

	bool unit_matches_filter(unit_map::const_iterator itor, const vconfig& filter);

	/** Used for [wml_message]. */
	void handle_wml_log_message(const config& cfg);
	/** Used for [deprecated_message]. */
	void handle_deprecated_message(const config& cfg);

	/**
	 * Function to fire an event.
	 *
	 * Events may have up to two arguments, both of which must be locations.
	 */
	bool fire(const std::string& event,
			const entity_location& loc1=map_location::null_location,
			const entity_location& loc2=map_location::null_location,
			const config& data=config());

	void raise(const std::string& event,
			const entity_location& loc1=map_location::null_location,
			const entity_location& loc2=map_location::null_location,
			const config& data=config());

	bool conditional_passed(const unit_map* units,
			const vconfig& cond, bool backwards_compat=true);

	/**
	 * Handles newly-created handlers. Flushes WML messages and errors.
	 */
	void commit();

	bool pump();

	/**
	 * Abstract class for a WML action handler.
	 */
	struct action_handler
	{
		virtual void handle(const queued_event &event_info, const vconfig &cfg) = 0;
		virtual ~action_handler() {}
	};

	/**
	 * Registers a WML action_handler for the lifetime of the current event manager.
	 * The handler is automatically destroyed at the end, if still registered.
	 * The previous handler is stored at the memory pointed by @a previous if
	 * nonnull, deleted otherwise.
	 */
	void register_action_handler(const std::string &tag, action_handler *handler,
		action_handler **previous = NULL);

} // end namespace game_events

#endif
