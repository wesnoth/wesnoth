/*
	Copyright (C) 2017 - 2024
	by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

#include "utils/optional_fwd.hpp"
#include "map/location.hpp"
#include "synced_context.hpp"
#include "config.hpp"

namespace actions {

	struct undo_action;

	/*
	 * Contains all steps that are needed to undo a user action.
	 */
	class undo_action_container
	{
		using t_step_ptr = std::unique_ptr<undo_action>;
		using t_steps = std::vector<t_step_ptr>;

		t_steps steps_;
		int unit_id_diff_;

	public:

		undo_action_container();

		bool empty() const { return steps_.empty(); }
		t_steps& steps() { return steps_; }
		bool undo(int side);
		void write(config& cfg);
		/**
		 * Creates the list of undo steps based on a config.
		 * Throws bad_lexical_cast or config::error if it cannot parse the config properly.
		 */
		void read(const config& cfg);
		void add(t_step_ptr&& action);
		void set_unit_id_diff(int id_diff)
		{
			unit_id_diff_ = id_diff;
		}

		using t_factory = std::function<t_step_ptr(const config&)>;
		using t_factory_map = std::map<std::string, t_factory>;
		static t_factory_map& get_factories();

		template<typename T>
		struct subaction_factory
		{
			subaction_factory()
			{
				std::string name = T::get_type_impl();
				get_factories()[name] = [](const config& cfg) {
					auto res = std::make_unique<T>(cfg);
					return res;
				};
			}
		};
	};

	/**
	 * Records information to be able to undo an action.
	 * Each type of step gets its own derived type.
	 */
	struct undo_action
	{
		undo_action() {}
		// Virtual destructor to support derived classes.
		virtual ~undo_action() {}

		/**
		 * Undoes this action.
		 * @return true on success; false on an error.
		 */
		virtual bool undo(int side) = 0;
		/** Writes this into the provided config. */
		virtual void write(config& cfg) const
		{
			cfg["type"] = this->get_type();
		}
		virtual const char* get_type() const = 0;
	};

	class  undo_event : public undo_action
	{
	public:

		undo_event(int fcn_idx, const config& args, const game_events::queued_event& ctx);
		undo_event(const config& cmds, const game_events::queued_event& ctx);

		undo_event(const config& cfg);

		virtual bool undo(int side);
		virtual void write(config& cfg) const;


		static const char* get_type_impl() { return "event"; }
		virtual const char* get_type() const { return get_type_impl(); }

	private:
		undo_event(const config& first, const config& second, const config& weapons, const config& cmds);

		utils::optional<int> lua_idx;
		config commands, data;
		map_location loc1, loc2, filter_loc1, filter_loc2;
		std::size_t uid1, uid2;
		std::string id1, id2;
	};
}
