#pragma once

#include "vision.hpp"
#include "../map_location.hpp"
#include "../unit_ptr.hpp"
#include "../synced_context.hpp"

#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/optional.hpp>
namespace actions {
	class undo_list;
}
namespace actions {

	/// Records information to be able to undo an action.
	/// Each type of action gets its own derived type.
	/// Base class for all entries in the undo stack, also contains non undoable actions like update_shroud or auto_shroud.
	struct undo_action_base : boost::noncopyable
	{
		/// Default constructor.
		/// This is the only way to get NULL view_info.
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
		undo_action()
			: undo_action_base()
			, replay_data()
			, unit_id_diff(synced_context::get_unit_id_diff())
			, umc_commands_undo()
			, umc_commands_redo()
		{
			umc_commands_undo.swap(synced_context::get_undo_commands());
			umc_commands_redo.swap(synced_context::get_redo_commands());
		}
		undo_action(const config& cfg)
			: undo_action_base()
			, replay_data(cfg.child_or_empty("replay_data"))
			, unit_id_diff(cfg["unit_id_diff"])
			, umc_commands_undo()
			, umc_commands_redo()
		{
			read_tconfig_vector(umc_commands_undo, cfg, "undo_actions");
			read_tconfig_vector(umc_commands_redo, cfg, "redo_actions");
		}
		// Virtual destructor to support derived classes.
		virtual ~undo_action() {}

		/// Writes this into the provided config.
		virtual void write(config & cfg) const
		{
			cfg.add_child("replay_data", replay_data);
			cfg["unit_id_diff"] = unit_id_diff;
			write_tconfig_vector(umc_commands_undo, cfg, "undo_actions");
			write_tconfig_vector(umc_commands_redo, cfg, "redo_actions");
			undo_action_base::write(cfg);
		}

		/// Undoes this action.
		/// @return true on success; false on an error.
		virtual bool undo(int side) = 0;
		/// Redoes this action.
		/// @return true on success; false on an error.
		virtual bool redo(int side) = 0;
		/// the replay data to do this action, this is only !empty() when this action is on the redo stack
		/// we need this because we don't recalculate the redos like they would be in real game,
		/// but even undoable commands can have "dependent" (= user_input) commands, which we save here.
		config replay_data;
		/// the difference in the unit ids
		/// TODO: does it really make sense to allow undoing if the unit id counter has changed?
		int unit_id_diff;
		/// actions wml (specified by wml) that should be executed when undoing this command.
		typedef boost::ptr_vector<config> tconfig_vector;
		tconfig_vector umc_commands_undo;
		tconfig_vector umc_commands_redo;
		void execute_undo_umc_wml();
		void execute_redo_umc_wml();
		
		static void read_tconfig_vector(tconfig_vector& vec, const config& cfg, const std::string& tag);
		static void write_tconfig_vector(const tconfig_vector& vec, config& cfg, const std::string& tag);
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
		/// Redoes this action.
		virtual bool redo(int)
		{
			return true;
		}
	};

}