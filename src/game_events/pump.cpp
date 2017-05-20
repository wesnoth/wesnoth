/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
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

#include "game_events/pump.hpp"
#include "game_events/conditional_wml.hpp"
#include "game_events/handlers.hpp"

#include "display_chat_manager.hpp"
#include "game_config.hpp"
#include "game_data.hpp"
#include "game_display.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "play_controller.hpp"
#include "resources.hpp"
#include "scripting/game_lua_kernel.hpp"
#include "side_filter.hpp"
#include "units/map.hpp"
#include "units/unit.hpp"
#include "variable.hpp"
#include "whiteboard/manager.hpp"

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

// This file is in the game_events namespace.
namespace game_events
{
namespace context
{
/// State when processing a particular flight of events or commands.
struct state
{
	bool mutated;
	bool skip_messages;

	explicit state(bool s, bool m = true)
		: mutated(m)
		, skip_messages(s)
	{
	}
};

class scoped
{
public:
	scoped(std::stack<context::state>& contexts, bool m = true);
	~scoped();

private:
	std::stack<context::state>& contexts_;
};
}

struct pump_impl
{
	std::vector<queued_event> events_queue;

	/// The value returned by wml_tracking();
	size_t internal_wml_tracking;

	std::stringstream wml_messages_stream;

	std::stack<context::state> contexts_;

	unsigned instance_count;

	manager* my_manager;

	pump_impl(manager& man)
		: events_queue()
		, internal_wml_tracking(0)
		, wml_messages_stream()
		, contexts_()
		, instance_count(0)
		, my_manager(&man)
	{
		contexts_.push(context::state(false));
	}
};

namespace
{ // Types
class pump_manager
{
public:
	pump_manager(pump_impl&);
	~pump_manager();

	/// Allows iteration through the queued events.
	queued_event& next()
	{
		return queue_[pumped_count_++];
	}
	/// Indicates the iteration is over.
	bool done() const
	{
		return pumped_count_ >= queue_.size();
	}

	unsigned count()
	{
		return impl_.instance_count;
	}

private:
	pump_impl& impl_;
	int x1_, x2_, y1_, y2_;

	/**
	 * Tracks the events to process.
	 * This isolates these events from any events that might be generated during the processing.
	 */
	std::vector<queued_event> queue_;

	/** Tracks how many events have been processed. */
	size_t pumped_count_;
};
} // end anonymous namespace (types)

namespace
{ // Support functions

pump_manager::pump_manager(pump_impl& impl)
	: impl_(impl)
	, x1_(resources::gamedata->get_variable("x1"))
	, x2_(resources::gamedata->get_variable("x2"))
	, y1_(resources::gamedata->get_variable("y1"))
	, y2_(resources::gamedata->get_variable("y2"))
	, queue_()
	, pumped_count_(0) // Filled later with a swap().
{
	queue_.swap(impl_.events_queue);
	++impl_.instance_count;
}

pump_manager::~pump_manager()
{
	--impl_.instance_count;

	// Not sure what the correct thing to do is here. In princple,
	// discarding all events (i.e. clearing events_queue) seems like
	// the right thing to do in the face of an exception. However, the
	// previous functionality preserved the queue, so for now we will
	// restore it.
	if(!done()) {
		// The remaining events get inserted at the beginning of events_queue.
		std::vector<queued_event> temp;
		impl_.events_queue.swap(temp);
		impl_.events_queue.insert(impl_.events_queue.end(), queue_.begin() + pumped_count_, queue_.end());
		impl_.events_queue.insert(impl_.events_queue.end(), temp.begin(), temp.end());
	}

	// Restore the old values of the game variables.
	resources::gamedata->get_variable("y2") = y2_;
	resources::gamedata->get_variable("y1") = y1_;
	resources::gamedata->get_variable("x2") = x2_;
	resources::gamedata->get_variable("x1") = x1_;
}
}

/**
 * Returns true iff the given event passes all its filters.
 */
bool wml_event_pump::filter_event(const event_handler& handler, const queued_event& ev)
{
	const unit_map& units = resources::gameboard->units();
	unit_map::const_iterator unit1 = units.find(ev.loc1);
	unit_map::const_iterator unit2 = units.find(ev.loc2);
	vconfig filters(handler.get_config());

	for(const vconfig& condition : filters.get_children("filter_condition")) {
		if(!conditional_passed(condition)) {
			return false;
		}
	}

	for(const vconfig& f : filters.get_children("filter_side")) {
		side_filter ssf(f, &resources::controller->gamestate());
		if(!ssf.match(resources::controller->current_side()))
			return false;
	}

	for(const vconfig& f : filters.get_children("filter")) {
		if(!ev.loc1.matches_unit_filter(unit1, f)) {
			return false;
		}
	}

	vconfig::child_list special_filters = filters.get_children("filter_attack");
	bool special_matches = special_filters.empty();
	if(!special_matches && unit1 != units.end()) {
		const bool matches_unit = ev.loc1.matches_unit(unit1);
		const config& attack = ev.data.child("first");
		for(const vconfig& f : special_filters) {
			if(f.empty()) {
				special_matches = true;
			} else if(!matches_unit) {
				return false;
			}

			special_matches = special_matches || matches_special_filter(attack, f);
		}
	}

	if(!special_matches) {
		return false;
	}

	for(const vconfig& f : filters.get_children("filter_second")) {
		if(!ev.loc2.matches_unit_filter(unit2, f)) {
			return false;
		}
	}

	special_filters = filters.get_children("filter_second_attack");
	special_matches = special_filters.empty();
	if(!special_matches && unit2 != units.end()) {
		const bool matches_unit = ev.loc2.matches_unit(unit2);
		const config& attack = ev.data.child("second");
		for(const vconfig& f : special_filters) {
			if(f.empty()) {
				special_matches = true;
			} else if(!matches_unit) {
				return false;
			}

			special_matches = special_matches || matches_special_filter(attack, f);
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
bool wml_event_pump::process_event(handler_ptr& handler_p, const queued_event& ev)
{
	DBG_EH << "processing event " << ev.name << " with id=" << ev.id << "\n";

	// We currently never pass a null pointer to this function, but to
	// guard against future modifications:
	if(!handler_p) {
		return false;
	}

	unit_map& units = resources::gameboard->units();
	scoped_xy_unit first_unit("unit", ev.loc1, units);
	scoped_xy_unit second_unit("second_unit", ev.loc2, units);
	scoped_weapon_info first_weapon("weapon", ev.data.child("first"));
	scoped_weapon_info second_weapon("second_weapon", ev.data.child("second"));

	if(!filter_event(*handler_p, ev)) {
		return false;
	}

	// The event hasn't been filtered out, so execute the handler.
	++impl_->internal_wml_tracking;
	context::scoped evc(impl_->contexts_);
	assert(resources::lua_kernel != nullptr);
	handler_p->handle_event(ev, handler_p, *resources::lua_kernel);
	// NOTE: handler_p may be null at this point!

	if(ev.name == "select") {
		resources::gamedata->last_selected = ev.loc1;
	}

	if(resources::screen != nullptr) {
		resources::screen->maybe_rebuild();
	}

	return context_mutated();
}

/**
 * Helper function for show_wml_messages(), which gathers
 * the messages from a stringstream.
 */
void wml_event_pump::fill_wml_messages_map(std::map<std::string, int>& msg_map, std::stringstream& source)
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
void wml_event_pump::show_wml_messages(std::stringstream& source, const std::string& caption, bool to_cerr)
{
	// Get all unique messages in messages,
	// with the number of encounters for these messages
	std::map<std::string, int> messages;
	fill_wml_messages_map(messages, source);

	// Show the messages collected
	for(std::map<std::string, int>::const_iterator itor = messages.begin(); itor != messages.end(); ++itor) {
		std::stringstream msg;
		msg << itor->first;
		if(itor->second > 1) {
			msg << " (" << itor->second << ")";
		}

		resources::screen->get_chat_manager().add_chat_message(
			time(nullptr), caption, 0, msg.str(), events::chat_handler::MESSAGE_PUBLIC, false);

		if(to_cerr) {
			std::cerr << caption << ": " << msg.str() << '\n';
		}
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
void wml_event_pump::show_wml_errors()
{
	static const std::string caption("Invalid WML found");

	show_wml_messages(lg::wml_error(), caption, true);
}

/**
 * Shows a summary of the messages generated so far by WML.
 * Identical messages are shown once, with (between parentheses)
 * the number of times that message was encountered.
 * The order in which the messages are shown does not need
 * to be the order in which these messages are encountered.
 */
void wml_event_pump::show_wml_messages()
{
	static const std::string caption("WML");

	show_wml_messages(impl_->wml_messages_stream, caption, false);
}

void wml_event_pump::put_wml_message(
		lg::logger& logger, const std::string& prefix, const std::string& message, bool in_chat)
{
	logger(log_wml) << message << std::endl;
	if(in_chat) {
		impl_->wml_messages_stream << prefix << message << std::endl;
	}
}

context::scoped::scoped(std::stack<context::state>& contexts, bool m)
	: contexts_(contexts)
{
	// The default context at least should always be on the stack
	assert(contexts_.size() > 0);

	bool skip_messages = (contexts_.size() > 1) && contexts_.top().skip_messages;
	contexts_.push(context::state(skip_messages, m));
}

context::scoped::~scoped()
{
	assert(contexts_.size() > 1);
	bool mutated = contexts_.top().mutated;
	contexts_.pop();
	contexts_.top().mutated |= mutated;
}

bool wml_event_pump::context_mutated()
{
	assert(impl_->contexts_.size() > 0);
	return impl_->contexts_.top().mutated;
}

void wml_event_pump::context_mutated(bool b)
{
	assert(impl_->contexts_.size() > 0);
	impl_->contexts_.top().mutated = b;
}

bool wml_event_pump::context_skip_messages()
{
	assert(impl_->contexts_.size() > 0);
	return impl_->contexts_.top().skip_messages;
}

void wml_event_pump::context_skip_messages(bool b)
{
	assert(impl_->contexts_.size() > 0);
	impl_->contexts_.top().skip_messages = b;
}

/**
 * Helper function which determines whether a wml_message text can
 * really be pushed into the wml_messages_stream, and does it.
 */
void wml_event_pump::put_wml_message(const std::string& logger, const std::string& message, bool in_chat)
{
	if(logger == "err" || logger == "error") {
		put_wml_message(lg::err(), _("Error: "), message, in_chat);
	} else if(logger == "warn" || logger == "wrn" || logger == "warning") {
		put_wml_message(lg::warn(), _("Warning: "), message, in_chat);
	} else if((logger == "debug" || logger == "dbg") && !lg::debug().dont_log(log_wml)) {
		put_wml_message(lg::debug(), _("Debug: "), message, in_chat);
	} else if(!lg::info().dont_log(log_wml)) {
		put_wml_message(lg::info(), _("Info: "), message, in_chat);
	}
}

bool wml_event_pump::fire(
		const std::string& event, const entity_location& loc1, const entity_location& loc2, const config& data)
{
	raise(event, loc1, loc2, data);
	return (*this)();
}

bool wml_event_pump::fire(const std::string& event,
		const std::string& id,
		const entity_location& loc1,
		const entity_location& loc2,
		const config& data)
{
	raise(event, id, loc1, loc2, data);
	return (*this)();
}

void wml_event_pump::raise(const std::string& event,
		const std::string& id,
		const entity_location& loc1,
		const entity_location& loc2,
		const config& data)
{
	if(resources::screen == nullptr)
		return;

	DBG_EH << "raising event name=" << event << ", id=" << id << "\n";

	impl_->events_queue.emplace_back(event, id, loc1, loc2, data);
}

bool wml_event_pump::operator()()
{
	// Quick aborts:
	if(resources::screen == nullptr) {
		return false;
	}

	assert(resources::lua_kernel != nullptr);
	if(impl_->events_queue.empty()) {
		DBG_EH << "Processing queued events, but none found.\n";
		return false;
	}

	if(impl_->instance_count >= game_config::max_loop) {
		ERR_NG << "game_events pump waiting to process new events because "
			   << "recursion level would exceed maximum: " << game_config::max_loop << '\n';
		return false;
	}

	if(!lg::debug().dont_log("event_handler")) {
		std::stringstream ss;
		for(const queued_event& ev : impl_->events_queue) {
			ss << "name=" << ev.name << ", "
			   << "id=" << ev.id << "; ";
		}
		DBG_EH << "processing queued events: " << ss.str() << "\n";
	}

	const size_t old_wml_track = impl_->internal_wml_tracking;

	// Ensure the whiteboard doesn't attempt to build its future unit map
	// while events are being processed.
	wb::real_map real_unit_map;

	pump_manager pump_instance(*impl_);
	context::scoped evc(impl_->contexts_, false);
	// Loop through the events we need to process.
	while(!pump_instance.done()) {
		queued_event& ev = pump_instance.next();

		if(ev.name.empty() && ev.id.empty()) {
			continue;
		}

		const std::string& event_name = ev.name;
		const std::string& event_id = ev.id;

		// Clear the unit cache, since the best clearing time is hard to figure out
		// due to status changes by WML. Every event will flush the cache.
		unit::clear_status_caches();

		{ // Block for context::scoped
			context::scoped inner_evc(impl_->contexts_, false);
			if(resources::lua_kernel->run_event(ev)) {
				++impl_->internal_wml_tracking;
			}
		}

		assert(impl_->my_manager);

		resources::gamedata->get_variable("x1") = ev.loc1.filter_loc().wml_x();
		resources::gamedata->get_variable("y1") = ev.loc1.filter_loc().wml_y();
		resources::gamedata->get_variable("x2") = ev.loc2.filter_loc().wml_x();
		resources::gamedata->get_variable("y2") = ev.loc2.filter_loc().wml_y();

		if(event_id.empty()) {
			// Handle events of this name.
			impl_->my_manager->execute_on_events(event_name, [&](game_events::manager&, handler_ptr ptr) {
				DBG_EH << "processing event " << event_name << " with id=" << ptr->get_config()["id"] << "\n";

				// Let this handler process our event.
				process_event(ptr, ev);

				// NOTE: ptr may be null at this point!
			});
		} else {
			// Get the handler directly via ID
			handler_ptr cur_handler = impl_->my_manager->get_event_handler_by_id(event_id);

			if(cur_handler) {
				DBG_EH << "processing event " << event_name << " with id=" << cur_handler->get_config()["id"] << "\n";
				process_event(cur_handler, ev);
			}
		}

		// Flush messages when finished iterating over event_handlers.
		flush_messages();
	}

	if(old_wml_track != impl_->internal_wml_tracking) {
		// Notify the whiteboard of any event.
		// This is used to track when moves, recruits, etc. happen.
		resources::whiteboard->on_gamestate_change();
	}

	return context_mutated();
}

void wml_event_pump::flush_messages()
{
	// Dialogs can only be shown if the display is not locked
	if(resources::screen && !resources::screen->video().update_locked()) {
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
size_t wml_event_pump::wml_tracking()
{
	return impl_->internal_wml_tracking;
}

wml_event_pump::wml_event_pump(manager& man)
	: impl_(new pump_impl(man))
{
}

wml_event_pump::~wml_event_pump()
{
}

} // end namespace game_events
