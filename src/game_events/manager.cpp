/*
	Copyright (C) 2003 - 2024
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

#include "game_events/manager.hpp"

#include "game_events/handlers.hpp"
#include "game_events/manager_impl.hpp"
#include "game_events/pump.hpp"

#include "game_data.hpp"
#include "log.hpp"
#include "resources.hpp"
#include "serialization/string_utils.hpp"

static lg::log_domain log_engine("engine");
#define DBG_NG LOG_STREAM(debug, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)
#define WRN_NG LOG_STREAM(warn, log_engine)

static lg::log_domain log_event_handler("event_handler");
#define LOG_EH LOG_STREAM(info, log_event_handler)
#define DBG_EH LOG_STREAM(debug, log_event_handler)

namespace
{
	// Event handlers can't be destroyed as long as at least one of these locks exist.
	class event_handler_list_lock
	{
	public:
		event_handler_list_lock()
		{
			++num_locks_;
		}

		~event_handler_list_lock()
		{
			--num_locks_;
		}

		static bool none()
		{
			return num_locks_ == 0u;
		}
	private:
		static unsigned int num_locks_;
	};

	unsigned int event_handler_list_lock::num_locks_ = 0u;
}

namespace game_events
{
/** Create an event handler. */
void manager::add_event_handler_from_wml(const config& handler, game_lua_kernel& lk, bool is_menu_item)
{
	auto new_handler = event_handlers_->add_event_handler(
		handler["name"],
		handler["id"],
		!handler["first_time_only"].to_bool(true),
		handler["priority"].to_double(0.),
		is_menu_item
	);
	if(new_handler.valid()) {
		new_handler->read_filters(handler);

		// Strip out anything that's used by the event system itself.
		config args;
		for(const auto& [attr, val] : handler.attribute_range()) {
			if(attr == "id" || attr == "name" || attr == "first_time_only" || attr == "priority" || attr.compare(0, 6, "filter") == 0) {
				continue;
			}
			args[attr] = val;
		}
		for(auto [key, cfg] : handler.all_children_view()) {
			if(key.compare(0, 6, "filter") != 0) {
				args.add_child(key, cfg);
			}
		}
		new_handler->set_arguments(args);
		new_handler->register_wml_event(lk);
		DBG_EH << "Registered WML event "
			<< (new_handler->names_raw().empty() ? "" : "'" + new_handler->names_raw() + "'")
			<< (new_handler->id().empty() ? "" : "{id=" + new_handler->id() + "}")
			<< (new_handler->repeatable() ? " (repeating" : " (first time only")
			<< "; priority " + std::to_string(new_handler->priority())
			<< (is_menu_item ? "; menu item)" : ")")
			<< " with the following actions:\n"
			<< args.debug();
	} else {
		LOG_EH << "Content of failed event:\n" << handler.debug();
	}
}

pending_event_handler manager::add_event_handler_from_lua(const std::string& name, const std::string& id, bool repeat, double priority, bool is_menu_item)
{
	return event_handlers_->add_event_handler(name, id, repeat, priority, is_menu_item);
}

/** Removes an event handler. */
void manager::remove_event_handler(const std::string& id)
{
	event_handlers_->remove_event_handler(id);
}

/** Gets an event handler by id */
const handler_ptr manager::get_event_handler_by_id(const std::string& id)
{
	return event_handlers_->get_event_handler_by_id(id);
}

/* ** manager ** */

manager::manager()
	: event_handlers_(new event_handlers())
	, unit_wml_ids_()
	, pump_(new game_events::wml_event_pump(*this))
	, wml_menu_items_()
{
}

void manager::read_scenario(const config& scenario_cfg, game_lua_kernel& lk)
{
	for(const config& ev : scenario_cfg.child_range("event")) {
		add_event_handler_from_wml(ev, lk);
	}

	for(const std::string& id : utils::split(scenario_cfg["unit_wml_ids"])) {
		unit_wml_ids_.insert(id);
	}

	wml_menu_items_.set_menu_items(scenario_cfg);

	// Create the event handlers for menu items.
	wml_menu_items_.init_handlers(lk);
}

manager::~manager()
{
}

void manager::add_events(const config::const_child_itors& cfgs, game_lua_kernel& lk, const std::string& type)
{
	if(!type.empty()) {
		if(std::find(unit_wml_ids_.begin(), unit_wml_ids_.end(), type) != unit_wml_ids_.end()) {
			return;
		}

		unit_wml_ids_.insert(type);
	}

	for(const config& new_ev : cfgs) {
		if(type.empty() && new_ev["id"].empty()) {
			WRN_NG << "attempt to add an [event] with empty id= from [unit], ignoring ";
			continue;
		}

		add_event_handler_from_wml(new_ev, lk);
	}
}

void manager::write_events(config& cfg, bool include_nonserializable) const
{
	for(const handler_ptr& eh : event_handlers_->get_active()) {
		if(!eh || eh->is_menu_item()) {
			continue;
		}

		// Silently skip disabled events if this function is invoked mid-event, such as via
		// [inspect] (the inspector writes the events to a local config) or if an out-of-sync
		// error occurs in MP. If the event in question is first-time-only, it will already
		// have been flagged as disabled by this point (such events are disabled before their
		// actions are run). If a disabled event is encountered outside an event context,
		// however, assert. That means something went wrong with event list cleanup.
		// Also silently skip them when including nonserializable events, which can happen
		// if viewing the inspector with :inspect after removing an event from the Lua console.
		if(eh->disabled() && (is_event_running() || include_nonserializable)) {
			continue;
		} else {
			assert(!eh->disabled());
		}

		config event_cfg;
		eh->write_config(event_cfg, include_nonserializable);
		if(!event_cfg.empty()) {
			cfg.add_child("event", std::move(event_cfg));
		}
	}

	cfg["unit_wml_ids"] = utils::join(unit_wml_ids_);
	wml_menu_items_.to_config(cfg);
}

void manager::execute_on_events(const std::string& event_id, const manager::event_func_t& func)
{
	const std::string standardized_event_id = event_handlers::standardize_name(event_id);
	const game_data* gd = resources::gamedata;
	// Copy the list so that new events added during processing are not executed.
	auto active_handlers = event_handlers_->get_active();

	{
		// Ensure that event handlers won't be cleaned up while we're iterating them.
		event_handler_list_lock lock;

		for (unsigned i = 0; i < active_handlers.size(); ++i) {
			handler_ptr handler = nullptr;

			try {
				handler = active_handlers.at(i);
			}
			catch (const std::out_of_range&) {
				continue;
			}

			// Shouldn't happen, but we're just being safe.
			if (!handler || handler->disabled()) {
				continue;
			}

			// Could be more than one.
			for(const std::string& name : handler->names(gd)) {
				if(standardized_event_id == name) {
					func(*this, handler);
					break;
				}
			}
		}
	}

	// Clean up expired ptrs. This saves us effort later since it ensures every ptr is valid.
	if(event_handler_list_lock::none()) {
		event_handlers_->clean_up_expired_handlers(standardized_event_id);
	}
}

bool manager::is_event_running() const
{
	// If there is an event handler list lock, an event is being processed.
	return !event_handler_list_lock::none();
}

game_events::wml_event_pump& manager::pump()
{
	return *pump_;
}

} // end namespace game_events
