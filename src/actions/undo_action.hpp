/*
   Copyright (C) 2017 the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "vision.hpp"
#include "map/location.hpp"
#include "units/ptr.hpp"
#include "synced_context.hpp"
#include "game_events/pump.hpp" // for queued_event
#include "config.hpp"

namespace actions {
	class undo_list;
	
	struct undo_event {
		config commands, data;
		map_location loc1, loc2, filter_loc1, filter_loc2;
		size_t uid1, uid2;
		std::string id1, id2;
		undo_event(const config& cmds, const game_events::queued_event& ctx);
		undo_event(const config& first, const config& second, const config& weapons, const config& cmds);
	};

	/// Records information to be able to undo an action.
	/// Each type of action gets its own derived type.
	/// Base class for all entries in the undo stack, also contains non undoable actions like update_shroud or auto_shroud.
	struct undo_action_base
	{
		undo_action_base(const undo_action_base&) = delete;
		undo_action_base& operator=(const undo_action_base&) = delete;

		/// Default constructor.
		/// This is the only way to get nullptr view_info.
		undo_action_base()
		{ }
		// Virtual destructor to support derived classes.
		virtual ~undo_action_base() {}

		/// Writes this into the provided config.
		virtual void write(config & cfg) const
		{
			cfg["type"] = this->get_type();
		}

		virtual const char* get_type() const = 0;
	};

	/// actions that are undoable (this does not include update_shroud and auto_shroud)
	struct undo_action : undo_action_base
	{
		/// Default constructor.
		/// It is assumed that undo actions are contructed after the action is performed
		/// so that the unit id diff does not change after this contructor.
		undo_action();
		undo_action(const config& cfg);
		// Virtual destructor to support derived classes.
		virtual ~undo_action() {}

		/// Writes this into the provided config.
		virtual void write(config & cfg) const;

		/// Undoes this action.
		/// @return true on success; false on an error.
		virtual bool undo(int side) = 0;
		/// the difference in the unit ids
		/// TODO: does it really make sense to allow undoing if the unit id counter has changed?
		int unit_id_diff;
		/// actions wml (specified by wml) that should be executed when undoing this command.
		typedef std::vector<undo_event> event_vector;
		event_vector umc_commands_undo;
		void execute_undo_umc_wml();

		static void read_event_vector(event_vector& vec, const config& cfg, const std::string& tag);
		static void write_event_vector(const event_vector& vec, config& cfg, const std::string& tag);
	};

	/// entry for player actions that do not need any special code to be performed when undoing such as right-click menu items.
	struct undo_dummy_action : undo_action
	{
		undo_dummy_action ()
			: undo_action()
		{
		}
		explicit undo_dummy_action (const config & cfg)
			: undo_action(cfg)
		{
		}
		virtual const char* get_type() const { return "dummy"; }
		virtual ~undo_dummy_action () {};
		/// Undoes this action.
		virtual bool undo(int)
		{
			return true;
		}
	};

}
