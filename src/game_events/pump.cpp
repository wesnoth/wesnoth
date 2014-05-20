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

/**
 * @file
 * Handles the current state of WML-events. This includes raising and firing,
 * as well as tracking the context for event firing.
 */

#include "../global.hpp"
#include "pump.hpp"
#include "conditional_wml.hpp"
#include "handlers.hpp"

#include "../game_config.hpp"
#include "../game_display.hpp"
#include "../gamestatus.hpp"
#include "../gettext.hpp"
#include "../log.hpp"
#include "../play_controller.hpp"
#include "../resources.hpp"
#include "../scripting/lua.hpp"
#include "../side_filter.hpp"
#include "../unit.hpp"
#include "../unit_map.hpp"
#include "../whiteboard/manager.hpp"
#include "../variable.hpp"

#include <boost/foreach.hpp>
#include <iomanip>
#include <iostream>


static lg::log_domain log_engine("engine");
#define DBG_NG LOG_STREAM(debug, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)
#define ERR_NG LOG_STREAM(err, log_engine)

static lg::log_domain log_wml("wml");
#define DBG_WML LOG_STREAM(debug, log_wml)
#define LOG_WML LOG_STREAM(info, log_wml)
#define WRN_WML LOG_STREAM(warn, log_wml)
#define ERR_WML LOG_STREAM(err, log_wml)

static lg::log_domain log_event_handler("event_handler");
#define DBG_EH LOG_STREAM(debug, log_event_handler)

// std::getline might be broken in Visual Studio so show a warning
#ifdef _MSC_VER
#if _MSC_VER < 1300
#ifndef GETLINE_PATCHED
#pragma message("warning: the std::getline implementation in your compiler might be broken.")
#pragma message(" http://support.microsoft.com/default.aspx?scid=kb;EN-US;q240015")
#endif
#endif
#endif


// This file is in the game_events namespace.
namespace game_events {

namespace { // Types
	class pump_manager {
	public:
		pump_manager();
		~pump_manager();

		/// Allows iteration through the queued events.
		queued_event & next() { return queue_[pumped_count_++]; }
		/// Indicates the iteration is over.
		bool done() const { return pumped_count_ >= queue_.size(); }

		static unsigned count() {
			return instance_count;
		}

	private:
		static unsigned instance_count;
		int x1_, x2_, y1_, y2_;
		/// Tracks the events to process.
		/// This isolates these events from any events that might be generated
		/// during the processing.
		std::vector<queued_event> queue_;
		/// Tracks how many events have been processed.
		size_t pumped_count_;
	};
	unsigned pump_manager::instance_count=0;
} // end anonymous namespace (types)

namespace { // Variables
	std::vector<queued_event> events_queue;

	/// The value returned by wml_tracking();
	size_t internal_wml_tracking = 0;

	std::stringstream wml_messages_stream;
} // end anonymous namespace (variables)

namespace { // Support functions

	pump_manager::pump_manager() :
		x1_(resources::gamedata->get_variable("x1")),
		x2_(resources::gamedata->get_variable("x2")),
		y1_(resources::gamedata->get_variable("y1")),
		y2_(resources::gamedata->get_variable("y2")),
		queue_(), // Filled later with a swap().
		pumped_count_(0)
	{
		queue_.swap(events_queue);
		++instance_count;
	}

	pump_manager::~pump_manager() {
		--instance_count;

		// Not sure what the correct thing to do is here. In princple,
		// discarding all events (i.e. clearing events_queue) seems like
		// the right thing to do in the face of an exception. However, the
		// previous functionality preserved the queue, so for now we will
		// restore it.
		if ( !done() ) {
			// The remainig events get inserted at the beginning of events_queue.
			std::vector<queued_event> temp;
			events_queue.swap(temp);
			events_queue.insert(events_queue.end(), queue_.begin() + pumped_count_, queue_.end());
			events_queue.insert(events_queue.end(), temp.begin(), temp.end());
		}

		// Restore the old values of the game variables.
		resources::gamedata->get_variable("y2") = y2_;
		resources::gamedata->get_variable("y1") = y1_;
		resources::gamedata->get_variable("x2") = x2_;
		resources::gamedata->get_variable("x1") = x1_;
	}


	inline bool events_init()
	{
		return resources::screen != NULL;
	}

	/**
	 * Returns true iff the given event passes all its filters.
	 */
	bool filter_event(const event_handler& handler, const queued_event& ev)
	{
		const unit_map *units = resources::units;
		unit_map::const_iterator unit1 = units->find(ev.loc1);
		unit_map::const_iterator unit2 = units->find(ev.loc2);
		vconfig filters(handler.get_config());

		BOOST_FOREACH(const vconfig &condition, filters.get_children("filter_condition"))
		{
			if (!conditional_passed(condition)) {
				return false;
			}
		}

		BOOST_FOREACH(const vconfig &f, filters.get_children("filter_side"))
		{
			side_filter ssf(f);
			if ( !ssf.match(resources::controller->current_side()) )
				return false;
		}

		BOOST_FOREACH(const vconfig &f, filters.get_children("filter"))
		{
			if ( !ev.loc1.matches_unit_filter(unit1, f) ) {
				return false;
			}
		}

		vconfig::child_list special_filters = filters.get_children("filter_attack");
		bool special_matches = special_filters.empty();
		if ( !special_matches  &&  unit1 != units->end() )
		{
			const bool matches_unit = ev.loc1.matches_unit(unit1);
			const config & attack = ev.data.child("first");
			BOOST_FOREACH(const vconfig &f, special_filters)
			{
				if ( f.empty() )
					special_matches = true;
				else if ( !matches_unit )
					return false;

				special_matches = special_matches ||
					              matches_special_filter(attack, f);
			}
		}
		if(!special_matches) {
			return false;
		}

		BOOST_FOREACH(const vconfig &f, filters.get_children("filter_second"))
		{
			if ( !ev.loc2.matches_unit_filter(unit2, f) ) {
				return false;
			}
		}

		special_filters = filters.get_children("filter_second_attack");
		special_matches = special_filters.empty();
		if ( !special_matches  &&  unit2 != units->end() )
		{
			const bool matches_unit = ev.loc2.matches_unit(unit2);
			const config & attack = ev.data.child("second");
			BOOST_FOREACH(const vconfig &f, special_filters)
			{
				if ( f.empty() )
					special_matches = true;
				else if ( !matches_unit )
					return false;

				special_matches = special_matches ||
					              matches_special_filter(attack, f);
			}
		}
		if(!special_matches) {
			return false;
		}

		// All filters passed.
		return true;
	}

	/**
	 * Processes an event through a single event handler.
	 * This includes checking event filters, but not checking that the event
	 * name matches.
	 *
	 * @param[in,out]  handler_p  The handler to offer the event to.
	 *                            This may be reset during processing.
	 * @param[in]      ev         The event information.
	 *
	 * @returns true if the game state changed.
	 */
	bool process_event(handler_ptr& handler_p, const queued_event& ev)
	{
		// We currently never pass a null pointer to this function, but to
		// guard against future modifications:
		if ( !handler_p )
			return false;

		unit_map *units = resources::units;
		scoped_xy_unit first_unit("unit", ev.loc1.x, ev.loc1.y, *units);
		scoped_xy_unit second_unit("second_unit", ev.loc2.x, ev.loc2.y, *units);
		scoped_weapon_info first_weapon("weapon", ev.data.child("first"));
		scoped_weapon_info second_weapon("second_weapon", ev.data.child("second"));

		if ( !filter_event(*handler_p, ev) )
			return false;

		// The event hasn't been filtered out, so execute the handler.
		++internal_wml_tracking;
		context::scoped evc;
		handler_p->handle_event(ev, handler_p);
		// NOTE: handler_p may be null at this point!

		if(ev.name == "select") {
			resources::gamedata->last_selected = ev.loc1;
		}

		if ( context::screen_needs_rebuild() ) {
			context::screen_needs_rebuild(false);
			game_display *screen = resources::screen;
			screen->recalculate_minimap();
			screen->invalidate_all();
			screen->rebuild_all();
		}

		return context::mutated();
	}

	/**
	 * Helper function for show_wml_messages(), which gathers
	 * the messages from a stringstream.
	 */
	void fill_wml_messages_map(std::map<std::string, int>& msg_map, std::stringstream& source)
	{
		while(true) {
			std::string msg;
			std::getline(source, msg);

			if(source.eof()) {
				break;
			}

			if(msg == "") {
				continue;
			}

			if(msg_map.find(msg) == msg_map.end()) {
				msg_map[msg] = 1;
			} else {
				msg_map[msg]++;
			}
		}
		// Make sure the eof flag is cleared otherwise no new messages are shown
		source.clear();
	}

	/**
	 * Shows a summary of messages/errors generated so far by WML.
	 * Identical messages are shown once, with (between parentheses)
	 * the number of times that message was encountered.
	 * The order in which the messages are shown does not need
	 * to be the order in which these messages are encountered.
	 * Messages are also written to std::cerr if to_cerr is true.
	 */
	void show_wml_messages(std::stringstream& source, const std::string & caption,
	                       bool to_cerr)
	{
		// Get all unique messages in messages,
		// with the number of encounters for these messages
		std::map<std::string, int> messages;
		fill_wml_messages_map(messages, source);

		// Show the messages collected
		for(std::map<std::string, int>::const_iterator itor = messages.begin();
		    itor != messages.end(); ++itor )
		{
			std::stringstream msg;
			msg << itor->first;
			if(itor->second > 1) {
				msg << " (" << itor->second << ")";
			}

			resources::screen->add_chat_message(time(NULL), caption, 0, msg.str(),
					events::chat_handler::MESSAGE_PUBLIC, false);
			if ( to_cerr )
				std::cerr << caption << ": " << msg.str() << '\n';
		}
	}

	/**
	 * Shows a summary of the errors encountered in WML so far,
	 * to avoid a lot of the same messages to be shown.
	 * Identical messages are shown once, with (between parentheses)
	 * the number of times that message was encountered.
	 * The order in which the messages are shown does not need
	 * to be the order in which these messages are encountered.
	 * Messages are always written to std::cerr.
	 */
	void show_wml_errors()
	{
		static const std::string caption("Invalid WML found");

		show_wml_messages(lg::wml_error, caption, true);
	}

	/**
	 * Shows a summary of the messages generated so far by WML.
	 * Identical messages are shown once, with (between parentheses)
	 * the number of times that message was encountered.
	 * The order in which the messages are shown does not need
	 * to be the order in which these messages are encountered.
	 */
	void show_wml_messages()
	{
		static const std::string caption("WML");

		show_wml_messages(wml_messages_stream, caption, false);
	}

	void put_wml_message(lg::logger& logger, const std::string& prefix, const std::string& message, bool in_chat)
	{
		logger(log_wml) << message << "\n";
		if (in_chat)
		{
			wml_messages_stream << prefix << message << "\n";
		}
	}


} // end anonymous namespace (support functions)


// Static members of context.
context::event_context context::default_context_(false);
context::event_context *context::current_context_ = &default_context_;
bool context::rebuild_screen_ = false;


context::scoped::scoped() :
	old_context_(context::current_context_),
	new_context_(old_context_ != &context::default_context_  &&
	             old_context_->skip_messages)
{
	context::current_context_ = &new_context_;
}

context::scoped::~scoped()
{
	old_context_->mutated |= new_context_.mutated;
	context::current_context_ = old_context_;
}


/**
 * Helper function which determines whether a wml_message text can
 * really be pushed into the wml_messages_stream, and does it.
 */
void put_wml_message(const std::string& logger, const std::string& message, bool in_chat)
{
	if (logger == "err" || logger == "error") {
		put_wml_message(lg::err, _("Error: "), message, in_chat );
	} else if (logger == "warn" || logger == "wrn" || logger == "warning") {
		put_wml_message(lg::warn, _("Warning: "), message, in_chat );
	} else if ((logger == "debug" || logger == "dbg") && !lg::debug.dont_log(log_wml)) {
		put_wml_message(lg::debug, _("Debug: "), message, in_chat );
	} else if (!lg::info.dont_log(log_wml)) {
		put_wml_message(lg::info, _("Info: "), message, in_chat );
	}
}

void run_lua_commands(char const *lua_code)
{
	resources::lua_kernel->run(lua_code);
}

void handle_event_commands(const queued_event& event_info, const vconfig &cfg)
{
	resources::lua_kernel->run_wml_action("command", cfg, event_info);
}

void handle_event_command(const std::string &cmd,
                          const queued_event &event_info, const vconfig &cfg)
{
	log_scope2(log_engine, "handle_event_command");
	LOG_NG << "handling command '" << cmd << "' from cfg 0x"
		<< std::hex << std::setiosflags(std::ios::uppercase)
		<< reinterpret_cast<uintptr_t>(&cfg.get_config()) << std::dec << "\n";

	if (!resources::lua_kernel->run_wml_action(cmd, cfg, event_info))
	{
		ERR_NG << "Couldn't find function for wml tag: "<< cmd <<"\n";
	}

	DBG_NG << "done handling command...\n";
}


bool fire(const std::string& event,
          const entity_location& loc1,
          const entity_location& loc2,
          const config& data)
{
	raise(event,loc1,loc2,data);
	return pump();
}

void raise(const std::string& event,
           const entity_location& loc1,
           const entity_location& loc2,
           const config& data)
{
	if(!events_init())
		return;

	DBG_EH << "raising event: " << event << "\n";

	events_queue.push_back(queued_event(event, loc1, loc2, data));
}

bool pump()
{
	// Quick aborts:
	if(!events_init())
		return false;
	assert(resources::lua_kernel != NULL);
	if ( events_queue.empty() ) {
		DBG_EH << "Processing queued events, but none found.\n";
		return false;
	}
	if(pump_manager::count() >= game_config::max_loop) {
		ERR_NG << "game_events::pump() waiting to process new events because "
		       << "recursion level would exceed maximum: " << game_config::max_loop << '\n';
		return false;
	}
	if(!lg::debug.dont_log("event_handler")) {
		std::stringstream ss;
		BOOST_FOREACH(const queued_event& ev, events_queue) {
			ss << "name=" << ev.name << "; ";
		}
		DBG_EH << "processing queued events: " << ss.str() << "\n";
	}

	const size_t old_wml_track = internal_wml_tracking;
	bool result = false;
	// Ensure the whiteboard doesn't attempt to build its future unit map
	// while events are being processed.
	wb::real_map real_unit_map;

	pump_manager pump_instance;

	// Loop through the events we need to process.
	while ( !pump_instance.done() )
	{
		queued_event & ev = pump_instance.next();
		const std::string& event_name = ev.name;

		// Clear the unit cache, since the best clearing time is hard to figure out
		// due to status changes by WML. Every event will flush the cache.
		unit::clear_status_caches();

		if ( resources::lua_kernel->run_event(ev) )
			++internal_wml_tracking;

		// Initialize an iteration over event handlers matching this event.
		manager::iteration handler_iter(event_name);

		// If there are any matching event handlers, initialize variables.
		// Note: Initializing variables all the time would not be
		//       functionally wrong, merely inefficient. So we do not have
		//       to cache *handler_iter here.
		if ( *handler_iter ) {
			resources::gamedata->get_variable("x1") = ev.loc1.filter_x() + 1;
			resources::gamedata->get_variable("y1") = ev.loc1.filter_y() + 1;
			resources::gamedata->get_variable("x2") = ev.loc2.filter_x() + 1;
			resources::gamedata->get_variable("y2") = ev.loc2.filter_y() + 1;
		}

		// While there is a potential handler for this event name.
		while ( handler_ptr cur_handler = *handler_iter ) {
			DBG_EH << "processing event " << event_name << " with id="<<
			          cur_handler->get_config()["id"] << "\n";
			// Let this handler process our event.
			if ( process_event(cur_handler, ev) )
			{
				// Game state changed.
				result = true;
			}
			// NOTE: cur_handler may be null at this point!

			++handler_iter;
		}

		// Flush messages when finished iterating over event_handlers.
		flush_messages();
	}

	if ( old_wml_track != internal_wml_tracking )
		// Notify the whiteboard of any event.
		// This is used to track when moves, recruits, etc. happen.
		resources::whiteboard->on_gamestate_change();

	return result;
}

/** Clears all events tha have been raised (and not pumped). */
void clear_events()
{
	events_queue.clear();
}

void flush_messages()
{
	// Dialogs can only be shown if the display is not locked
	if (!resources::screen->video().update_locked()) {
		show_wml_errors();
		show_wml_messages();
	}
}


/**
 * This function can be used to detect when no WML/Lua has been executed.
 *
 * If two calls to this function return the same value, then one can
 * assume that the usual game mechanics have been followed, and code does
 * not have to account for all the things WML/Lua can do. If the return
 * values are different, then something unusual might have happened between
 * those calls.
 *
 * This is not intended as a precise metric. Rather, it is motivated by
 * how large the number of fired WML events is, compared to the (typical)
 * number of WML event handlers. It is intended for code that can benefit
 * from caching some aspect of the game state and that cannot rely on
 * [allow_undo] not being used when that state changes.
 */
size_t wml_tracking()
{
	return internal_wml_tracking;
}

} // end namespace game_events

