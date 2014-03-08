/* $Id$ */
/*
   Copyright (C) 2003 - 2013 by David White <dave@whitevine.net>
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
 * Processing of WML-events.
 */

#include "global.hpp"

#include "actions.hpp"
#include "ai/manager.hpp"
#include "dialogs.hpp"
#include "game_display.hpp"
#include "game_events.hpp"
#include "game_preferences.hpp"
#include "gettext.hpp"
#include "gui/dialogs/gamestate_inspector.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "gui/dialogs/wml_message.hpp"
#include "gui/widgets/window.hpp"
#include "help.hpp"
#include "log.hpp"
#include "map.hpp"
#include "map_label.hpp"
#include "map_exception.hpp"
#include "pathfind/teleport.hpp"
#include "replay.hpp"
#include "reports.hpp"
#include "resources.hpp"
#include "scripting/lua.hpp"
#include "side_filter.hpp"
#include "sound.hpp"
#include "soundsource.hpp"
#include "terrain_filter.hpp"
#include "unit_display.hpp"
#include "unit_helper.hpp"
#include "wml_exception.hpp"
#include "play_controller.hpp"
#include "persist_var.hpp"
#include "whiteboard/manager.hpp"

#include <boost/scoped_array.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <algorithm>
#include <iomanip>
#include <iostream>

#include <boost/foreach.hpp>

static lg::log_domain log_engine("engine");
#define DBG_NG LOG_STREAM(debug, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)
#define WRN_NG LOG_STREAM(warn, log_engine)
#define ERR_NG LOG_STREAM(err, log_engine)

static lg::log_domain log_display("display");
#define DBG_DP LOG_STREAM(debug, log_display)
#define LOG_DP LOG_STREAM(info, log_display)

static lg::log_domain log_wml("wml");
#define DBG_WML LOG_STREAM(debug, log_wml)
#define LOG_WML LOG_STREAM(info, log_wml)
#define WRN_WML LOG_STREAM(warn, log_wml)
#define ERR_WML LOG_STREAM(err, log_wml)

static lg::log_domain log_config("config");
#define ERR_CF LOG_STREAM(err, log_config)

static lg::log_domain log_event_handler("event_handler");
#define DBG_EH LOG_STREAM(debug, log_event_handler)

/**
 * State when processing a flight of events or commands.
 */
struct event_context
{
	bool mutated;
	bool skip_messages;
	event_context(bool s): mutated(true), skip_messages(s) {}
};

static event_context default_context(false);
static event_context *current_context = &default_context;

/**
 * Context state with automatic lifetime handling.
 */
struct scoped_context
{
	event_context *old_context;
	event_context new_context;

	scoped_context()
		: old_context(current_context)
		, new_context(old_context != &default_context && old_context->skip_messages)
	{
		current_context = &new_context;
	}

	~scoped_context()
	{
		old_context->mutated |= new_context.mutated;
		current_context = old_context;
	}
};

static bool screen_needs_rebuild;

namespace {

	std::stringstream wml_messages_stream;

	bool manager_running = false;
	int floating_label = 0;

	typedef std::pair< std::string, config* > wmi_command_change;
	std::vector< wmi_command_change > wmi_command_changes;

	const gui::msecs prevent_misclick_duration = 10;
	const gui::msecs average_frame_time = 30;

	class pump_manager {
		public:
		pump_manager() :
			x1_(resources::state_of_game->get_variable("x1")),
			x2_(resources::state_of_game->get_variable("x2")),
			y1_(resources::state_of_game->get_variable("y1")),
			y2_(resources::state_of_game->get_variable("y2"))
		{
			++instance_count;
		}
		~pump_manager() {
			resources::state_of_game->get_variable("x1") = x1_;
			resources::state_of_game->get_variable("x2") = x2_;
			resources::state_of_game->get_variable("y1") = y1_;
			resources::state_of_game->get_variable("y2") = y2_;
			--instance_count;
		}
		static unsigned count() {
			return instance_count;
		}
		private:
		static unsigned instance_count;
		int x1_, x2_, y1_, y2_;
	};
	unsigned pump_manager::instance_count=0;

} // end anonymous namespace (1)

#ifdef _MSC_VER
// std::getline might be broken in Visual Studio so show a warning
#if _MSC_VER < 1300
#ifndef GETLINE_PATCHED
#pragma message("warning: the std::getline implementation in your compiler might be broken.")
#pragma message(" http://support.microsoft.com/default.aspx?scid=kb;EN-US;q240015")
#endif
#endif
#endif

/**
 * Helper function which determines whether a wml_message text can
 * really be pushed into the wml_messages_stream, and does it.
 */
static void put_wml_message(const std::string& logger, const std::string& message)
{
	if (logger == "err" || logger == "error") {
		ERR_WML << message << "\n";
		wml_messages_stream << _("Error: ") << message << "\n";
	} else if (logger == "warn" || logger == "wrn" || logger == "warning") {
		WRN_WML << message << "\n";
		wml_messages_stream << _("Warning: ") << message << "\n";
	} else if ((logger == "debug" || logger == "dbg") && !lg::debug.dont_log(log_wml)) {
		DBG_WML << message << "\n";
		wml_messages_stream << _("Debug: ") << message << "\n";
	} else if (!lg::info.dont_log(log_wml)) {
		LOG_WML << message << "\n";
		wml_messages_stream << _("Info: ") << message << "\n";
	}
}

/**
 * Helper function for show_wml_errors(), which gathers
 * the messages from a stringstream.
 */
static void fill_wml_messages_map(std::map<std::string, int>& msg_map, std::stringstream& source)
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
 * Shows a summary of the errors encountered in WML thusfar,
 * to avoid a lot of the same messages to be shown.
 * Identical messages are shown once, with (between braces)
 * the number of times that message was encountered.
 * The order in which the messages are shown does not need
 * to be the order in which these messages are encountered.
 * Messages are always written to std::cerr.
 */
static void show_wml_errors()
{
	// Get all unique messages in messages,
	// with the number of encounters for these messages
	std::map<std::string, int> messages;
	fill_wml_messages_map(messages, lg::wml_error);

	// Show the messages collected
	const std::string caption = "Invalid WML found";
	for(std::map<std::string, int>::const_iterator itor = messages.begin();
			itor != messages.end(); ++itor) {

		std::stringstream msg;
		msg << itor->first;
		if(itor->second > 1) {
			msg << " (" << itor->second << ")";
		}

		resources::screen->add_chat_message(time(NULL), caption, 0, msg.str(),
				events::chat_handler::MESSAGE_PUBLIC, false);
		std::cerr << caption << ": " << msg.str() << '\n';
	}
}

static void show_wml_messages()
{
	// Get all unique messages in messages,
	// with the number of encounters for these messages
	std::map<std::string, int> messages;
	fill_wml_messages_map(messages, wml_messages_stream);

	// Show the messages collected
	const std::string caption = "WML";
	for(std::map<std::string, int>::const_iterator itor = messages.begin();
			itor != messages.end(); ++itor) {

		std::stringstream msg;
		msg << itor->first;
		if(itor->second > 1) {
			msg << " (" << itor->second << ")";
		}

		resources::screen->add_chat_message(time(NULL), caption, 0, msg.str(),
				events::chat_handler::MESSAGE_PUBLIC, false);
	}
}

typedef void (*wml_handler_function)(
	const game_events::queued_event &event_info, const vconfig &cfg);

typedef std::map<std::string, wml_handler_function> static_wml_action_map;
/** Map of the default action handlers known of the engine. */
static static_wml_action_map static_wml_actions;

/**
 * WML_HANDLER_FUNCTION macro handles auto registeration for wml handlers
 *
 * @param pname wml tag name
 * @param pei the variable name of game_events::queued_event object inside function
 * @param pcfg the variable name of config object inside function
 *
 * You are warned! This is evil macro magic!
 *
 * The following code registers a [foo] tag:
 * \code
 * // comment out unused parameters to prevent compiler warnings
 * WML_HANDLER_FUNCTION(foo, event_info, cfg)
 * {
 *    // code for foo
 * }
 * \endcode
 *
 * Generated code looks like this:
 * \code
 * void wml_action_foo(...);
 * struct wml_func_register_foo {
 *   wml_func_register_foo() {
 *     static_wml_actions["foo"] = &wml_func_foo;
 *   } wml_func_register_foo;
 * void wml_func_foo(...)
 * {
 *    // code for foo
 * }
 * \endcode
 */
#define WML_HANDLER_FUNCTION(pname, pei, pcfg) \
	static void wml_func_##pname(const game_events::queued_event &pei, const vconfig &pcfg); \
	struct wml_func_register_##pname \
	{ \
		wml_func_register_##pname() \
		{ static_wml_actions[#pname] = &wml_func_##pname; } \
	}; \
	static wml_func_register_##pname wml_func_register_##pname##_aux;  \
	static void wml_func_##pname(const game_events::queued_event& pei, const vconfig& pcfg)

namespace game_events {

	static bool matches_special_filter(const config &cfg, const vconfig& filter);

	static bool internal_conditional_passed(const vconfig& cond, bool& backwards_compat)
	{
		static std::vector<std::pair<int,int> > default_counts = utils::parse_ranges("1-99999");

		// If the if statement requires we have a certain unit,
		// then check for that.
		const vconfig::child_list& have_unit = cond.get_children("have_unit");
		backwards_compat = backwards_compat && have_unit.empty();
		for(vconfig::child_list::const_iterator u = have_unit.begin(); u != have_unit.end(); ++u) {
			if(resources::units == NULL)
				return false;
			std::vector<std::pair<int,int> > counts = (*u).has_attribute("count")
				? utils::parse_ranges((*u)["count"]) : default_counts;
			int match_count = 0;
			BOOST_FOREACH(const unit &i, *resources::units)
			{
				if(i.hitpoints() > 0 && unit_matches_filter(i, *u)) {
					++match_count;
					if(counts == default_counts) {
						// by default a single match is enough, so avoid extra work
						break;
					}
				}
			}
			if ((*u)["search_recall_list"].to_bool())
			{
				for(std::vector<team>::iterator team = resources::teams->begin();
						team!=resources::teams->end(); ++team)
				{
					if(counts == default_counts && match_count) {
						break;
					}
					const std::vector<unit>& avail_units = team->recall_list();
					for(std::vector<unit>::const_iterator unit = avail_units.begin(); unit!=avail_units.end(); ++unit) {
						if(counts == default_counts && match_count) {
							break;
						}
						scoped_recall_unit auto_store("this_unit", team->save_id(), unit - avail_units.begin());
						if (unit_matches_filter(*unit, *u)) {
							++match_count;
						}
					}
				}
			}
			if(!in_ranges(match_count, counts)) {
				return false;
			}
		}

		// If the if statement requires we have a certain location,
		// then check for that.
		const vconfig::child_list& have_location = cond.get_children("have_location");
		backwards_compat = backwards_compat && have_location.empty();
		for(vconfig::child_list::const_iterator v = have_location.begin(); v != have_location.end(); ++v) {
			std::set<map_location> res;
			terrain_filter(*v, *resources::units).get_locations(res);

			std::vector<std::pair<int,int> > counts = (*v).has_attribute("count")
				? utils::parse_ranges((*v)["count"]) : default_counts;
			if(!in_ranges<int>(res.size(), counts)) {
				return false;
			}
		}

		// Check against each variable statement,
		// to see if the variable matches the conditions or not.
		const vconfig::child_list& variables = cond.get_children("variable");
		backwards_compat = backwards_compat && variables.empty();

		BOOST_FOREACH(const vconfig &values, variables)
		{
			const std::string name = values["name"];
			config::attribute_value value = resources::state_of_game->get_variable_const(name);
			std::string str_value = value.str();
			double num_value = value.to_double();

#define TEST_ATTR(name, test) do { \
			if (values.has_attribute(name)) { \
				config::attribute_value attr = values[name]; \
				if (!(test)) return false; \
			} \
			} while (0)

#define TEST_STR_ATTR(name, test) do { \
			if (values.has_attribute(name)) { \
				std::string attr_str = values[name]; \
				if (!(test)) return false; \
			} \
			} while (0)

#define TEST_NUM_ATTR(name, test) do { \
			if (values.has_attribute(name)) { \
				double attr_num = values[name].to_double(); \
				if (!(test)) return false; \
			} \
			} while (0)

			TEST_STR_ATTR("equals",                str_value == attr_str);
			TEST_STR_ATTR("not_equals",            str_value != attr_str);
			TEST_NUM_ATTR("numerical_equals",      num_value == attr_num);
			TEST_NUM_ATTR("numerical_not_equals",  num_value != attr_num);
			TEST_NUM_ATTR("greater_than",          num_value >  attr_num);
			TEST_NUM_ATTR("less_than",             num_value <  attr_num);
			TEST_NUM_ATTR("greater_than_equal_to", num_value >= attr_num);
			TEST_NUM_ATTR("less_than_equal_to",    num_value <= attr_num);
			TEST_ATTR("boolean_equals",     value.to_bool() == attr.to_bool());
			TEST_ATTR("boolean_not_equals", value.to_bool() != attr.to_bool());
			TEST_STR_ATTR("contains", value.str().find(attr_str) != std::string::npos);

#undef TEST_ATTR
#undef TEST_STR_ATTR
#undef TEST_NUM_ATTR
		}
		return true;
	}

	bool conditional_passed(const vconfig& cond, bool backwards_compat)
	{
		bool allow_backwards_compat = backwards_compat = backwards_compat &&
			cond["backwards_compat"].to_bool(true);
		bool matches = internal_conditional_passed(cond, allow_backwards_compat);

		// Handle [and], [or], and [not] with in-order precedence
		int or_count = 0;
		vconfig::all_children_iterator cond_i = cond.ordered_begin();
		vconfig::all_children_iterator cond_end = cond.ordered_end();
		while(cond_i != cond_end)
		{
			const std::string& cond_name = cond_i.get_key();
			const vconfig& cond_filter = cond_i.get_child();

			// Handle [and]
			if(cond_name == "and")
			{
				matches = matches && conditional_passed(cond_filter, backwards_compat);
				backwards_compat = false;
			}
			// Handle [or]
			else if(cond_name == "or")
			{
				matches = matches || conditional_passed(cond_filter, backwards_compat);
				++or_count;
			}
			// Handle [not]
			else if(cond_name == "not")
			{
				matches = matches && !conditional_passed(cond_filter, backwards_compat);
				backwards_compat = false;
			}
			++cond_i;
		}
		// Check for deprecated [or] syntax
		if(matches && or_count > 1 && allow_backwards_compat)
		{
			///@deprecated r18803 [or] syntax
			lg::wml_error << "possible deprecated [or] syntax: now forcing re-interpretation\n";
			/**
			 * @todo For now we will re-interpret it according to the old
			 * rules, but this should be later to prevent re-interpretation
			 * errors.
			 */
			const vconfig::child_list& orcfgs = cond.get_children("or");
			for(unsigned int i=0; i < orcfgs.size(); ++i) {
				if(conditional_passed(orcfgs[i])) {
					return true;
				}
			}
			return false;
		}
		return matches;
	}

	void handle_wml_log_message(const config& cfg)
	{
		const std::string& logger = cfg["logger"];
		const std::string& msg = cfg["message"];

		put_wml_message(logger,msg);
	}

	void handle_deprecated_message(const config& cfg)
	{
		// Note: no need to translate the string, since only used for deprecated things.
		const std::string& message = cfg["message"];
		lg::wml_error << message << '\n';
	}

	std::vector<int> get_sides_vector(const vconfig& cfg, const bool only_ssf, const bool only_side)
	{
		if(only_ssf) {
			side_filter filter(cfg);
			return filter.get_teams();
		}

		const config::attribute_value sides = cfg["side"];
		const vconfig &ssf = cfg.child("filter_side");

		if (!ssf.null() && !only_side) {
			if(!sides.empty()) { WRN_NG << "ignoring duplicate side filter information (inline side=)\n"; }
			side_filter filter(ssf);
			return filter.get_teams();
		}

		if (sides.blank()) {
			if(only_side) put_wml_message("error", "empty side= is deprecated, use side=1");
			//To deprecate the current default (side=1), require one of the currently two ways
			//of specifying a side - putting inline side= or [filter_side].
			else put_wml_message("error", "empty side= and no [filter_side] is deprecated, use either side=1 or [filter_side]");
			std::vector<int> result;
			result.push_back(1); // we make sure the current default is maintained
			return result;
		}
		// uncomment if the decision will be made to make [filter_side] the only & final syntax for specifying sides
		// put_wml_message("error","specifying side via inline side= is deprecated, use [filter_side] ");
		side_filter filter(sides.str());
		return filter.get_teams();
	}

} // end namespace game_events (1)

namespace {

	std::set<std::string> used_items;

} // end anonymous namespace (2)

static bool events_init() { return resources::screen != 0; }

namespace {

	std::deque<game_events::queued_event> events_queue;


} // end anonymous namespace (3)

static map_location cfg_to_loc(const vconfig& cfg,int defaultx = 0, int defaulty = 0)
{
	int x = cfg["x"].to_int(defaultx) - 1;
	int y = cfg["y"].to_int(defaulty) - 1;

	return map_location(x, y);
}

namespace {

	class t_event_handlers {
		typedef std::vector<game_events::event_handler> t_active;
	public:
		typedef t_active::iterator iterator;
		typedef t_active::const_iterator const_iterator;
	private:

		t_active active_; ///Active event handlers
		t_active insert_buffer_; ///Event handlers added while pumping events
		std::set<std::string> remove_buffer_; ///Event handlers removed while pumping events
		bool buffering_;


		void log_handler(std::stringstream& ss,
			const std::vector<game_events::event_handler>& handlers,
			const std::string& msg) {

			BOOST_FOREACH(const game_events::event_handler& h, handlers){
				const config& cfg = h.get_config();
				ss << "name=" << cfg["name"] << ", with id=" << cfg["id"] << "; ";
			}
			DBG_EH << msg << " handlers are now " << ss.str() << "\n";
			ss.str(std::string());
		}

		void log_handlers() {
			if(lg::debug.dont_log("event_handler")) return;

			std::stringstream ss;
			log_handler(ss, active_, "active");
			log_handler(ss, insert_buffer_, "insert buffered");
			BOOST_FOREACH(const std::string& h, remove_buffer_){
				ss << "id=" << h << "; ";
			}
			DBG_EH << "remove buffered handlers are now " << ss.str() << "\n";
		}

	public:

		t_event_handlers()
			: active_() , insert_buffer_() , remove_buffer_() , buffering_(false) { }

		/**
		 * Adds an event handler.  An event with a nonempty ID will not
		 * be added if an event with that ID already exists.  This method
		 * respects this class's buffering functionality.
		 */
		void add_event_handler(game_events::event_handler const & new_handler) {
			if(buffering_) {
				DBG_EH << "buffering event handler for name=" << new_handler.get_config()["name"] <<
				" with id " << new_handler.get_config()["id"] << "\n";
				insert_buffer_.push_back(new_handler);
				log_handlers();
			}

			else {
				const config & cfg = new_handler.get_config();
				std::string id = cfg["id"];
				if(!id.empty()) {
					BOOST_FOREACH( game_events::event_handler const & eh, active_) {
						config const & temp_config( eh.get_config());
						if(id == temp_config["id"]) {
							DBG_EH << "ignoring event handler for name=" << cfg["name"] <<
								" with id " << id << "\n";
							return;
						}
					}
				}
				DBG_EH << "inserting event handler for name=" << cfg["name"] <<
					" with id=" << id << "\n";
				active_.push_back(new_handler);
				log_handlers();
			}
		}

		/**
		 * Removes an event handler, identified by its ID.  Events with
		 * empty IDs cannot be removed.  This method respects this class's
		 * buffering functionality.
		 */
		void remove_event_handler(std::string const & id) {
			if(id == "") { return; }

			DBG_EH << "removing event handler with id " << id << "\n";

			if(buffering_) { remove_buffer_.insert(id); }

			t_active &temp = buffering_ ? insert_buffer_ : active_;

			t_active::iterator i = temp.begin();
			while(i < temp.end()) {
				config const & temp_config = (*i).get_config();
				std::string event_id = temp_config["id"];
				if(event_id != "" && event_id == id) {
					i = temp.erase(i); }
				else {
					++i; }
			}
			log_handlers();
		}

		/**
		 * Starts buffering.  While buffering, any calls to add_event_handler
		 * and remove_event_handler will not take effect until commit_buffer
		 * is called.  This function is idempotent - starting a buffer
		 * when already buffering will not start a second buffer.
		 */
		void start_buffering() {
			buffering_ = true;
			DBG_EH << "starting buffering...\n";
		}

		void stop_buffering() {
			DBG_EH << "stopping buffering...\n";
			buffering_ = false;
		}

		/**
		 * Commits all buffered events
		 */
		void commit_buffer() {
			DBG_EH << "committing buffered event handlers, buffering: " << buffering_ << "\n";
			if(buffering_)
				return;

			// Commit any event removals
			BOOST_FOREACH(std::string const & i ,  remove_buffer_ ){
				remove_event_handler( i ); }
			remove_buffer_.clear();

			// Commit any spawned events-within-events
			BOOST_FOREACH( game_events::event_handler const & i ,  insert_buffer_ ){
				add_event_handler( i ); }
			insert_buffer_.clear();

			log_handlers();
		}

		void clear(){
			active_.clear();
			insert_buffer_.clear();
			remove_buffer_.clear();
			buffering_ = false;
		}


		iterator begin() { return active_.begin(); }
		const_iterator begin() const { return active_.begin(); }

		iterator end() { return active_.end(); }
		const_iterator end() const { return active_.end(); }

	};

	t_event_handlers event_handlers;

} // end anonymous namespace (4)

static void toggle_shroud(const bool remove, const vconfig& cfg)
{
	std::vector<int> sides = game_events::get_sides_vector(cfg);
	size_t index;

	BOOST_FOREACH(const int &side_num, sides)
	{
		index = side_num - 1;
		team &t = (*resources::teams)[index];
		std::set<map_location> locs;
		terrain_filter filter(cfg, *resources::units);
		filter.restrict_size(game_config::max_loop);
		filter.get_locations(locs, true);

		BOOST_FOREACH(map_location const &loc, locs)
		{
			if (remove) {
				t.clear_shroud(loc);
			} else {
				t.place_shroud(loc);
			}
		}
	}

	resources::screen->labels().recalculate_shroud();
	resources::screen->recalculate_minimap();
	resources::screen->invalidate_all();
}

WML_HANDLER_FUNCTION(remove_shroud, /*event_info*/, cfg)
{
	toggle_shroud(true,cfg);
}

WML_HANDLER_FUNCTION(place_shroud, /*event_info*/,cfg)
{
	toggle_shroud(false,cfg );
}

WML_HANDLER_FUNCTION(tunnel, /*event_info*/, cfg)
{
	const bool remove = utils::string_bool(cfg["remove"], false);
	if (remove) {
		const std::vector<std::string> ids = utils::split(cfg["id"]);
		BOOST_FOREACH(const std::string &id, ids) {
			resources::tunnels->remove(id);
		}
	} else if (cfg.get_children("source").empty() ||
		cfg.get_children("target").empty() ||
		cfg.get_children("filter").empty()) {
		ERR_WML << "[tunnel] is missing a mandatory tag:\n"
			 << cfg.get_config().debug();
	} else {
		pathfind::teleport_group tunnel(cfg, false);
		resources::tunnels->add(tunnel);

		if (utils::string_bool(cfg["bidirectional"], true)) {
			tunnel = pathfind::teleport_group(cfg, true);
			resources::tunnels->add(tunnel);
		}
	}
}

WML_HANDLER_FUNCTION(teleport, event_info, cfg)
{
	unit_map::iterator u = resources::units->find(event_info.loc1);

	// Search for a valid unit filter, and if we have one, look for the matching unit
	const vconfig filter = cfg.child("filter");
	if(!filter.null()) {
		for (u = resources::units->begin(); u != resources::units->end(); ++u){
			if(game_events::unit_matches_filter(*u, filter))
				break;
		}
	}

	if (u == resources::units->end()) return;

	// We have found a unit that matches the filter
	const map_location dst = cfg_to_loc(cfg);
	if (dst == u->get_location() || !resources::game_map->on_board(dst)) return;

	const unit* pass_check = NULL;
	//@deprecated ignore_passability 1.9.10
	const config::attribute_value ignore_passability = cfg["ignore_passability"];
	if (!ignore_passability.blank()) {
		WRN_NG << "[teleport]ignore_passability= is deprecated, use check_passability=\n";
			if (!ignore_passability.to_bool(false))
				pass_check = &*u;
	}
	else if (cfg["check_passability"].to_bool(true))
		pass_check = &*u;
	const map_location vacant_dst = find_vacant_tile(*resources::game_map, *resources::units, dst, pathfind::VACANT_ANY, pass_check);
	if (!resources::game_map->on_board(vacant_dst)) return;

	int side = u->side();
	if (cfg["clear_shroud"].to_bool(true)) {
		clear_shroud(side);
	}

	map_location src_loc = u->get_location();

	std::vector<map_location> teleport_path;
	teleport_path.push_back(src_loc);
	teleport_path.push_back(vacant_dst);
	bool animate = cfg["animate"].to_bool();
	unit_display::move_unit(teleport_path, *u, *resources::teams, animate);

	resources::units->move(src_loc, vacant_dst);
	unit::clear_status_caches();

	u = resources::units->find(vacant_dst);
	u->set_standing();

	if (resources::game_map->is_village(vacant_dst)) {
		get_village(vacant_dst, side);
	}

	resources::screen->invalidate_unit_after_move(src_loc, dst);

	resources::screen->draw();
}

WML_HANDLER_FUNCTION(volume, /*event_info*/, cfg)
{

	int vol;
	float rel;
	std::string music = cfg["music"];
	std::string sound = cfg["sound"];

	if(!music.empty()) {
		vol = preferences::music_volume();
		rel = atof(music.c_str());
		if (rel >= 0.0 && rel < 100.0) {
			vol = static_cast<int>(rel*vol/100.0);
		}
		sound::set_music_volume(vol);
	}

	if(!sound.empty()) {
		vol = preferences::sound_volume();
		rel = atof(sound.c_str());
		if (rel >= 0.0 && rel < 100.0) {
			vol = static_cast<int>(rel*vol/100.0);
		}
		sound::set_sound_volume(vol);
	}

}

static void color_adjust(const vconfig& cfg) {
	game_display &screen = *resources::screen;
	screen.adjust_color_overlay(cfg["red"], cfg["green"], cfg["blue"]);
	screen.invalidate_all();
	screen.draw(true,true);
}

WML_HANDLER_FUNCTION(color_adjust, /*event_info*/, cfg)
{
	color_adjust(cfg);
}

//Function handling old name
///@deprecated 1.9.2 'colour_adjust' instead of 'color_adjust'
//deprecation message added 1.9.10
WML_HANDLER_FUNCTION(colour_adjust, /*event_info*/, cfg)
{
	WRN_NG << "[colour_adjust] is deprecated, use [color_adjust]\n";
	color_adjust(cfg);
}

WML_HANDLER_FUNCTION(scroll, /*event_info*/, cfg)
{
	game_display &screen = *resources::screen;
	screen.scroll(cfg["x"], cfg["y"]);
	screen.draw(true,true);
}

// store time of day config in a WML variable; useful for those who
// are too lazy to calculate the corresponding time of day for a given turn,
// or if the turn / time-of-day sequence mutates in a scenario.
WML_HANDLER_FUNCTION(store_time_of_day, /*event_info*/, cfg)
{
	const map_location loc = cfg_to_loc(cfg, -999, -999);
	int turn = cfg["turn"];
	// using 0 will use the current turn
	const time_of_day& tod = resources::tod_manager->get_time_of_day(loc,turn);

	std::string variable = cfg["variable"];
	if(variable.empty()) {
		variable = "time_of_day";
	}

	variable_info store(variable, true, variable_info::TYPE_CONTAINER);

	config tod_cfg;
	tod.write(tod_cfg);

	(*store.vars).add_child(store.key, tod_cfg);
}

WML_HANDLER_FUNCTION(inspect, /*event_info*/, cfg)
{
	gui2::tgamestate_inspector inspect_dialog(cfg);
	inspect_dialog.show(resources::screen->video());
}

WML_HANDLER_FUNCTION(modify_ai, /*event_info*/, cfg)
{
	std::vector<int> sides = game_events::get_sides_vector(cfg);
	BOOST_FOREACH(const int &side_num, sides)
	{
		ai::manager::modify_active_ai_for_side(side_num,cfg.get_parsed_config());
	}
}

WML_HANDLER_FUNCTION(modify_side, /*event_info*/, cfg)
{
	std::vector<team> &teams = *resources::teams;

	std::string team_name = cfg["team_name"];
	std::string user_team_name = cfg["user_team_name"];
	std::string controller = cfg["controller"];
	std::string recruit_str = cfg["recruit"];
	std::string shroud_data = cfg["shroud_data"];
	const config& parsed = cfg.get_parsed_config();
	const config::const_child_itors &ai = parsed.child_range("ai");
	/**
	 * @todo also allow client to modify a side's color if it is possible
	 * to change it on the fly without causing visual glitches
	 */
	std::string switch_ai = cfg["switch_ai"];

	std::vector<int> sides = game_events::get_sides_vector(cfg);
	size_t team_index;

	BOOST_FOREACH(const int &side_num, sides)
	{
		team_index = side_num - 1;
		LOG_NG << "modifying side: " << side_num << "\n";
		if(!team_name.empty()) {
			LOG_NG << "change side's team to team_name '" << team_name << "'\n";
			teams[team_index].change_team(team_name,
					user_team_name);
		} else if(!user_team_name.empty()) {
			LOG_NG << "change side's user_team_name to '" << user_team_name << "'\n";
			teams[team_index].change_team(teams[team_index].team_name(),
					user_team_name);
		}
		// Modify recruit list (override)
		if (!recruit_str.empty()) {
			std::vector<std::string> recruit = utils::split(recruit_str);
			if (recruit.size() == 1 && recruit.back() == "")
				recruit.clear();

			teams[team_index].set_recruits(std::set<std::string>(recruit.begin(),recruit.end()));
		}
		// Modify income
		config::attribute_value income = cfg["income"];
		if (!income.empty()) {
			teams[team_index].set_base_income(income.to_int() + game_config::base_income);
		}
		// Modify total gold
		config::attribute_value gold = cfg["gold"];
		if (!gold.empty()) {
			teams[team_index].set_gold(gold);
		}
		// Set controller
		if (!controller.empty()) {
			teams[team_index].change_controller(controller);
		}
		// Set shroud
		config::attribute_value shroud = cfg["shroud"];
		if (!shroud.empty()) {
			teams[team_index].set_shroud(shroud.to_bool(true));
		}
		// Merge shroud data
		if (!shroud_data.empty()) {
			teams[team_index].merge_shroud_map_data(shroud_data);
		}
		// Set whether team is hidden in status table
		config::attribute_value hidden = cfg["hidden"];
		if (!hidden.empty()) {
			teams[team_index].set_hidden(hidden.to_bool(true));
		}
		// Set fog
		config::attribute_value fog = cfg["fog"];
		if (!fog.empty()) {
			teams[team_index].set_fog(fog.to_bool(true));
		}
		// Set income per village
		config::attribute_value village_gold = cfg["village_gold"];
		if (!village_gold.empty()) {
			teams[team_index].set_village_gold(village_gold);
		}
		// Redeploy ai from location (this ignores current AI parameters)
		if (!switch_ai.empty()) {
			ai::manager::add_ai_for_side_from_file(side_num,switch_ai,true);
		}
		// Override AI parameters
		if (ai.first != ai.second) {
			ai::manager::modify_active_ai_config_old_for_side(side_num,ai);
		}
		// Add shared view to current team
		config::attribute_value share_view = cfg["share_view"];
		if (!share_view.empty()){
			teams[team_index].set_share_view(share_view.to_bool(true));
			team::clear_caches();
			resources::screen->recalculate_minimap();
			resources::screen->invalidate_all();
		}
		// Add shared maps to current team
		// IMPORTANT: this MUST happen *after* share_view is changed
		config::attribute_value share_maps = cfg["share_maps"];
		if (!share_maps.empty()){
			teams[team_index].set_share_maps(share_maps.to_bool(true));
			team::clear_caches();
			resources::screen->recalculate_minimap();
			resources::screen->invalidate_all();
		}

	}
}

WML_HANDLER_FUNCTION(modify_turns, /*event_info*/, cfg)
{
	config::attribute_value value = cfg["value"];
	std::string add = cfg["add"];
	config::attribute_value current = cfg["current"];
	tod_manager& tod_man = *resources::tod_manager;
	if(!add.empty()) {
		tod_man.modify_turns(add);
	} else if(!value.empty()) {
		tod_man.set_number_of_turns(value.to_int(-1));
	}
	// change current turn only after applying mods
	if(!current.empty()) {
		const unsigned int current_turn_number = tod_man.turn();
		int new_turn_number = current.to_int(current_turn_number);
		const unsigned int new_turn_number_u = static_cast<unsigned int>(new_turn_number);
		if(new_turn_number_u < 1 || (new_turn_number > tod_man.number_of_turns() && tod_man.number_of_turns() != -1)) {
			ERR_NG << "attempted to change current turn number to one out of range (" << new_turn_number << ")\n";
		} else if(new_turn_number_u != current_turn_number) {
			tod_man.set_turn(new_turn_number_u);
			resources::screen->new_turn();
		}
	}
}

namespace {

game_display::fake_unit *create_fake_unit(const vconfig& cfg)
{
	std::string type = cfg["type"];
	std::string variation = cfg["variation"];
	std::string img_mods = cfg["image_mods"];

	size_t side_num = cfg["side"].to_int(1) - 1;
	if (side_num >= resources::teams->size()) side_num = 0;

	unit_race::GENDER gender = string_gender(cfg["gender"]);
	const unit_type *ut = unit_types.find(type);
	if (!ut) return NULL;
	game_display::fake_unit * fake_unit = new game_display::fake_unit(unit(ut, side_num + 1, false, gender));

	if(!variation.empty()) {
		config mod;
		config &effect = mod.add_child("effect");
		effect["apply_to"] = "variation";
		effect["name"] = variation;
		fake_unit->add_modification("variation",mod);
	}

	if(!img_mods.empty()) {
		config mod;
		config &effect = mod.add_child("effect");
		effect["apply_to"] = "image_mod";
		effect["add"] = img_mods;
		fake_unit->add_modification("image_mod",mod);
	}

	return fake_unit;
}

std::vector<map_location> fake_unit_path(const unit& fake_unit, const std::vector<std::string>& xvals, const std::vector<std::string>& yvals)
{
	gamemap *game_map = resources::game_map;
	std::vector<map_location> path;
	map_location src;
	map_location dst;
	for(size_t i = 0; i != std::min(xvals.size(),yvals.size()); ++i) {
		if(i==0){
			src.x = atoi(xvals[i].c_str())-1;
			src.y = atoi(yvals[i].c_str())-1;
			if (!game_map->on_board(src)) {
				ERR_CF << "invalid move_unit_fake source: " << src << '\n';
				break;
			}
			path.push_back(src);
			continue;
		}
		pathfind::shortest_path_calculator calc(fake_unit,
				(*resources::teams)[fake_unit.side()-1],
				*resources::units,
				*resources::teams,
				*game_map);

		dst.x = atoi(xvals[i].c_str())-1;
		dst.y = atoi(yvals[i].c_str())-1;
		if (!game_map->on_board(dst)) {
			ERR_CF << "invalid move_unit_fake destination: " << dst << '\n';
			break;
		}

		pathfind::plain_route route = pathfind::a_star_search(src, dst, 10000, &calc,
			game_map->w(), game_map->h());

		if (route.steps.empty()) {
			WRN_NG << "Could not find move_unit_fake route from " << src << " to " << dst << ": ignoring complexities\n";
			pathfind::emergency_path_calculator calc(fake_unit, *game_map);

			route = pathfind::a_star_search(src, dst, 10000, &calc,
					game_map->w(), game_map->h());
			if(route.steps.empty()) {
				// This would occur when trying to do a MUF of a unit
				// over locations which are unreachable to it (infinite movement
				// costs). This really cannot fail.
				WRN_NG << "Could not find move_unit_fake route from " << src << " to " << dst << ": ignoring terrain\n";
				pathfind::dummy_path_calculator calc(fake_unit, *game_map);
				route = a_star_search(src, dst, 10000, &calc, game_map->w(), game_map->h());
				assert(!route.steps.empty());
			}
		}
		// we add this section to the end of the complete path
		// skipping section's head because already included
		// by the previous iteration
		path.insert(path.end(),
				route.steps.begin()+1, route.steps.end());

		src = dst;
	}
	return path;
}

} //end of anonymous namespace

// Moving a 'unit' - i.e. a dummy unit
// that is just moving for the visual effect
WML_HANDLER_FUNCTION(move_unit_fake, /*event_info*/, cfg)
{
	util::unique_ptr<unit> dummy_unit(create_fake_unit(cfg));
	if(!dummy_unit.get())
		return;

	const std::string x = cfg["x"];
	const std::string y = cfg["y"];

	const std::vector<std::string> xvals = utils::split(x);
	const std::vector<std::string> yvals = utils::split(y);

	const std::vector<map_location>& path = fake_unit_path(*dummy_unit, xvals, yvals);
	if (!path.empty())
		unit_display::move_unit(path, *dummy_unit, *resources::teams);
}

WML_HANDLER_FUNCTION(move_units_fake, /*event_info*/, cfg)
{
	LOG_NG << "Processing [move_units_fake]\n";

	const vconfig::child_list unit_cfgs = cfg.get_children("fake_unit");
	size_t num_units = unit_cfgs.size();
	boost::scoped_array<util::unique_ptr<game_display::fake_unit> > units(
		new util::unique_ptr<game_display::fake_unit>[num_units]);
	std::vector<std::vector<map_location> > paths;
	paths.reserve(num_units);
	game_display* disp = game_display::get_singleton();

	LOG_NG << "Moving " << num_units << " units\n";

	size_t longest_path = 0;

	BOOST_FOREACH(const vconfig& config, unit_cfgs) {
		const std::vector<std::string> xvals = utils::split(config["x"]);
		const std::vector<std::string> yvals = utils::split(config["y"]);
		int skip_steps = config["skip_steps"];
		game_display::fake_unit *u = create_fake_unit(config);
		units[paths.size()].reset(u);
		paths.push_back(fake_unit_path(*u, xvals, yvals));
		if(skip_steps > 0)
			paths.back().insert(paths.back().begin(), skip_steps, paths.back().front());
		longest_path = std::max(longest_path, paths.back().size());
		DBG_NG << "Path " << paths.size() - 1 << " has length " << paths.back().size() << '\n';

		u->set_location(paths.back().front());
		u->place_on_game_display(disp);
	}

	LOG_NG << "Units placed, longest path is " << longest_path << " long\n";

	std::vector<map_location> path_step(2);
	path_step.resize(2);
	for(size_t step = 1; step < longest_path; ++step) {
		DBG_NG << "Doing step " << step << "...\n";
		for(size_t un = 0; un < num_units; ++un) {
			if(step >= paths[un].size() || paths[un][step - 1] == paths[un][step])
				continue;
			DBG_NG << "Moving unit " << un << ", doing step " << step << '\n';
			path_step[0] = paths[un][step - 1];
			path_step[1] = paths[un][step];
			unit_display::move_unit(path_step, *units[un], *resources::teams);
			units[un]->set_location(path_step[1]);
			units[un]->set_standing();
		}
	}

	LOG_NG << "Units moved\n";

	for(size_t un = 0; un < num_units; ++un) {
		units[un]->remove_from_game_display();
	}

	LOG_NG << "Units removed\n";
}

WML_HANDLER_FUNCTION(set_variable, /*event_info*/, cfg)
{
	game_state *state_of_game = resources::state_of_game;

	const std::string name = cfg["name"];
	if(name.empty()) {
		ERR_NG << "trying to set a variable with an empty name:\n" << cfg.get_config().debug();
		return;
	}
	config::attribute_value &var = state_of_game->get_variable(name);

	config::attribute_value literal = cfg.get_config()["literal"]; // no $var substitution
	if (!literal.blank()) {
		var = literal;
	}

	config::attribute_value value = cfg["value"];
	if (!value.blank()) {
		var = value;
	}

	const std::string to_variable = cfg["to_variable"];
	if(to_variable.empty() == false) {
		var = state_of_game->get_variable(to_variable);
	}

	config::attribute_value add = cfg["add"];
	if (!add.empty()) {
		var = var.to_double() + add.to_double();
	}

	config::attribute_value sub = cfg["sub"];
	if (!sub.empty()) {
		var = var.to_double() - sub.to_double();
	}

	config::attribute_value multiply = cfg["multiply"];
	if (!multiply.empty()) {
		var = var.to_double() * multiply.to_double();
	}

	config::attribute_value divide = cfg["divide"];
	if (!divide.empty()) {
		if (divide.to_double() == 0) {
			ERR_NG << "division by zero on variable " << name << "\n";
			return;
		}
		var = var.to_double() / divide.to_double();
	}

	config::attribute_value modulo = cfg["modulo"];
	if (!modulo.empty()) {
		if (modulo.to_double() == 0) {
			ERR_NG << "division by zero on variable " << name << "\n";
			return;
		}
		var = std::fmod(var.to_double(), modulo.to_double());
	}

	config::attribute_value round_val = cfg["round"];
	if (!round_val.empty()) {
		double value = var.to_double();
		if (round_val == "ceil") {
			var = int(std::ceil(value));
		} else if (round_val == "floor") {
			var = int(std::floor(value));
		} else {
			// We assume the value is an integer.
			// Any non-numerical values will be interpreted as 0
			// Which is probably what was intended anyway
			int decimals = round_val.to_int();
			value *= std::pow(10.0, decimals); //add $decimals zeroes
			value = round_portable(value); // round() isn't implemented everywhere
			value *= std::pow(10.0, -decimals); //and remove them
			var = value;
		}
	}

	config::attribute_value ipart = cfg["ipart"];
	if (!ipart.empty()) {
		double result;
		std::modf(ipart.to_double(), &result);
		var = int(result);
	}

	config::attribute_value fpart = cfg["fpart"];
	if (!fpart.empty()) {
		double ignore;
		var = std::modf(fpart.to_double(), &ignore);
	}

	config::attribute_value string_length_target = cfg["string_length"];
	if (!string_length_target.blank()) {
		var = int(string_length_target.str().size());
	}

	// Note: maybe we add more options later, eg. strftime formatting.
	// For now make the stamp mandatory.
	const std::string time = cfg["time"];
	if(time == "stamp") {
		var = int(SDL_GetTicks());
	}

	// Random generation works as follows:
	// rand=[comma delimited list]
	// Each element in the list will be considered a separate choice,
	// unless it contains "..". In this case, it must be a numerical
	// range (i.e. -1..-10, 0..100, -10..10, etc).
	const std::string rand = cfg["rand"];

	// The new random generator, the logic is a copy paste of the old random.
	if(rand.empty() == false) {
		assert(state_of_game);

		std::string random_value;

		std::string word;
		std::vector<std::string> words;
		std::vector<std::pair<long,long> > ranges;
		int num_choices = 0;
		std::string::size_type pos = 0, pos2 = std::string::npos;
		std::stringstream ss(std::stringstream::in|std::stringstream::out);
		while (pos2 != rand.length()) {
			pos = pos2+1;
			pos2 = rand.find(",", pos);

			if (pos2 == std::string::npos)
				pos2 = rand.length();

			word = rand.substr(pos, pos2-pos);
			words.push_back(word);
			std::string::size_type tmp = word.find("..");


			if (tmp == std::string::npos) {
				// Treat this element as a string
				ranges.push_back(std::pair<int, int>(0,0));
				num_choices += 1;
			}
			else {
				// Treat as a numerical range
				const std::string first = word.substr(0, tmp);
				const std::string second = word.substr(tmp+2,
						rand.length());

				long low, high;
				ss << first + " " + second;
				ss >> low;
				ss >> high;
				ss.clear();

				if (low > high) {
					std::swap(low, high);
				}
				ranges.push_back(std::pair<long, long>(low,high));
				num_choices += (high - low) + 1;

				// Make 0..0 ranges work
				if (high == 0 && low == 0) {
					words.pop_back();
					words.push_back("0");
				}
			}
		}

		int choice = state_of_game->rng().get_next_random() % num_choices;
		int tmp = 0;
		for(size_t i = 0; i < ranges.size(); ++i) {
			tmp += (ranges[i].second - ranges[i].first) + 1;
			if (tmp > choice) {
				if (ranges[i].first == 0 && ranges[i].second == 0) {
					random_value = words[i];
				}
				else {
					tmp = (ranges[i].second - (tmp - choice)) + 1;
					ss << tmp;
					ss >> random_value;
				}
				break;
			}
		}

		var = random_value;
	}


	const vconfig::child_list join_elements = cfg.get_children("join");
	if(!join_elements.empty())
	{
		const vconfig join_element=join_elements.front();

		std::string array_name=join_element["variable"];
		std::string separator=join_element["separator"];
		std::string key_name=join_element["key"];

		if(key_name.empty())
		{
			key_name="value";
		}

		bool remove_empty = join_element["remove_empty"].to_bool();

		std::string joined_string;

		variable_info vi(array_name, true, variable_info::TYPE_ARRAY);
		bool first = true;
		BOOST_FOREACH(const config &cfg, vi.as_array())
		{
			std::string current_string = cfg[key_name];
			if (remove_empty && current_string.empty()) continue;
			if (first) first = false;
			else joined_string += separator;
			joined_string += current_string;
		}

		var=joined_string;
	}

}


WML_HANDLER_FUNCTION(set_variables, /*event_info*/, cfg)
{
	const t_string& name = cfg["name"];
	variable_info dest(name, true, variable_info::TYPE_CONTAINER);
	if(name.empty()) {
		ERR_NG << "trying to set a variable with an empty name:\n" << cfg.get_config().debug();
		return;
	}

	std::string mode = cfg["mode"]; // replace, append, merge, or insert
	if(mode == "extend") {
		mode = "append";
	} else if(mode != "append" && mode != "merge") {
		if(mode == "insert") {
			size_t child_count = dest.vars->child_count(dest.key);
			if(dest.index >= child_count) {
				while(dest.index >= ++child_count) {
					//inserting past the end requires empty data
					dest.vars->append(config(dest.key));
				}
				//inserting at the end is handled by an append
				mode = "append";
			}
		} else {
			mode = "replace";
		}
	}

	const vconfig::child_list values = cfg.get_children("value");
	const vconfig::child_list literals = cfg.get_children("literal");
	const vconfig::child_list split_elements = cfg.get_children("split");

	config data;

	if(cfg.has_attribute("to_variable"))
	{
		variable_info tovar(cfg["to_variable"], false, variable_info::TYPE_CONTAINER);
		if(tovar.is_valid) {
			if(tovar.explicit_index) {
				data.add_child(dest.key, tovar.as_container());
			} else {
				variable_info::array_range range = tovar.as_array();
				while(range.first != range.second)
				{
					data.add_child(dest.key, *range.first++);
				}
			}
		}
	} else if(!values.empty()) {
		for(vconfig::child_list::const_iterator i=values.begin(); i!=values.end(); ++i)
		{
			data.add_child(dest.key, (*i).get_parsed_config());
		}
	} else if(!literals.empty()) {
		for(vconfig::child_list::const_iterator i=literals.begin(); i!=literals.end(); ++i)
		{
			data.add_child(dest.key, i->get_config());
		}
	} else if(!split_elements.empty()) {
		const vconfig split_element=split_elements.front();

		std::string split_string=split_element["list"];
		std::string separator_string=split_element["separator"];
		std::string key_name=split_element["key"];
		if(key_name.empty())
		{
			key_name="value";
		}

		bool remove_empty = split_element["remove_empty"].to_bool();

		char* separator = separator_string.empty() ? NULL : &separator_string[0];

		std::vector<std::string> split_vector;

		//if no separator is specified, explode the string
		if(separator == NULL)
		{
			for(std::string::iterator i=split_string.begin(); i!=split_string.end(); ++i)
			{
				split_vector.push_back(std::string(1, *i));
			}
		}
		else {
			split_vector=utils::split(split_string, *separator, remove_empty ? utils::REMOVE_EMPTY | utils::STRIP_SPACES : utils::STRIP_SPACES);
		}

		for(std::vector<std::string>::iterator i=split_vector.begin(); i!=split_vector.end(); ++i)
		{
			data.add_child(dest.key)[key_name]=*i;
		}
	}
	if(mode == "replace")
	{
		if(dest.explicit_index) {
			dest.vars->remove_child(dest.key, dest.index);
		} else {
			dest.vars->clear_children(dest.key);
		}
	}
	if(!data.empty())
	{
		if(mode == "merge")
		{
			if(dest.explicit_index) {
				// merging multiple children into a single explicit index
				// requires that they first be merged with each other
				data.merge_children(dest.key);
				dest.as_container().merge_with(data.child(dest.key));
			} else {
				dest.vars->merge_with(data);
			}
		} else if(mode == "insert" || dest.explicit_index) {
			BOOST_FOREACH(const config &child, data.child_range(dest.key))
			{
				dest.vars->add_child_at(dest.key, child, dest.index++);
			}
		} else {
			dest.vars->append(data);
		}
	}
}

WML_HANDLER_FUNCTION(role, /*event_info*/, cfg)
{
	bool found = false;

	// role= represents the instruction, so we can't filter on it
	config item = cfg.get_config();
	item.remove_attribute("role");
	vconfig filter(item);

	// try to match units on the gamemap before the recall lists
	std::vector<std::string> types = utils::split(filter["type"]);
	const bool has_any_types = !types.empty();
	std::vector<std::string>::iterator ti = types.begin(),
		ti_end = types.end();
	// loop to give precendence based on type order
	do {
		if (has_any_types) {
			item["type"] = *ti;
		}
		unit_map::iterator itor;
		BOOST_FOREACH(unit &u, *resources::units) {
			if (game_events::unit_matches_filter(u, filter)) {
				u.set_role(cfg["role"]);
				found = true;
				break;
			}
		}
	} while(!found && has_any_types && ++ti != ti_end);
	if(!found) {
		// next try to match units on the recall lists
		std::set<std::string> player_ids;
		std::vector<std::string> sides = utils::split(cfg["side"]);
		const bool has_any_sides = !sides.empty();
		BOOST_FOREACH(std::string const& side_str, sides) {
			size_t side_num = lexical_cast_default<size_t>(side_str,0);
			if(side_num > 0 && side_num <= resources::teams->size()) {
				player_ids.insert((resources::teams->begin() + (side_num - 1))->save_id());
			}
		}
		// loop to give precendence based on type order
		std::vector<std::string>::iterator ti = types.begin();
		do {
			if (has_any_types) {
				item["type"] = *ti;
			}
			std::vector<team>::iterator pi,
				pi_end = resources::teams->end();
			for (pi = resources::teams->begin(); pi != pi_end; ++pi)
			{
				std::string const& player_id = pi->save_id();
				// Verify the filter's side= includes this player
				if(has_any_sides && !player_ids.count(player_id)) {
					continue;
				}
				// Iterate over the player's recall list to find a match
				for(size_t i=0; i < pi->recall_list().size(); ++i) {
					unit& u = pi->recall_list()[i];
					scoped_recall_unit auto_store("this_unit", player_id, i);
					if (u.matches_filter(filter, map_location())) {
						u.set_role(cfg["role"]);
						found=true;
						break;
					}
				}
			}
		} while(!found && has_any_types && ++ti != ti_end);
	}
}

WML_HANDLER_FUNCTION(sound_source, /*event_info*/, cfg)
{
	soundsource::sourcespec spec(cfg.get_parsed_config());
	resources::soundsources->add(spec);
}

WML_HANDLER_FUNCTION(remove_sound_source, /*event_info*/, cfg)
{
	resources::soundsources->remove(cfg["id"]);
}

void change_terrain(const map_location &loc, const t_translation::t_terrain &t,
	gamemap::tmerge_mode mode, bool replace_if_failed)
{
	gamemap *game_map = resources::game_map;

	t_translation::t_terrain
		old_t = game_map->get_terrain(loc),
		new_t = game_map->merge_terrains(old_t, t, mode, replace_if_failed);
	if (new_t == t_translation::NONE_TERRAIN) return;
	preferences::encountered_terrains().insert(new_t);

	if (game_map->is_village(old_t) && !game_map->is_village(new_t)) {
		int owner = village_owner(loc, *resources::teams);
		if (owner != -1)
			(*resources::teams)[owner].lose_village(loc);
	}

	game_map->set_terrain(loc, new_t);
	screen_needs_rebuild = true;

	BOOST_FOREACH(const t_translation::t_terrain &ut, game_map->underlying_union_terrain(loc)) {
		preferences::encountered_terrains().insert(ut);
	}
}

// Creating a mask of the terrain
WML_HANDLER_FUNCTION(terrain_mask, /*event_info*/, cfg)
{
	map_location loc = cfg_to_loc(cfg, 1, 1);

	gamemap mask(*resources::game_map);

	try {
		mask.read(cfg["mask"], false);
	} catch(incorrect_map_format_error&) {
		ERR_NG << "terrain mask is in the incorrect format, and couldn't be applied\n";
		return;
	} catch(twml_exception& e) {
		e.show(*resources::screen);
		return;
	}
	bool border = cfg["border"].to_bool();
	resources::game_map->overlay(mask, cfg.get_parsed_config(), loc.x, loc.y, border);
	screen_needs_rebuild = true;
}

static bool try_add_unit_to_recall_list(const map_location& loc, const unit& u)
{
	if((*resources::teams)[u.side()-1].persistent()) {
		(*resources::teams)[u.side()-1].recall_list().push_back(u);
		return true;
	} else {
		ERR_NG << "unit with id " << u.id() << ": location (" << loc.x << "," << loc.y <<") is not on the map, and player "
			<< u.side() << " has no recall list.\n";
		return false;
	}
}

// If we should spawn a new unit on the map somewhere
WML_HANDLER_FUNCTION(unit, /*event_info*/, cfg)
{
	config parsed_cfg = cfg.get_parsed_config();

	config::attribute_value to_variable = cfg["to_variable"];
	if (!to_variable.blank())
	{
		parsed_cfg.remove_attribute("to_variable");
		unit new_unit(parsed_cfg, true, resources::state_of_game);
		config &var = resources::state_of_game->get_variable_cfg(to_variable);
		var.clear();
		new_unit.write(var);
		if (const config::attribute_value *v = parsed_cfg.get("x")) var["x"] = *v;
		if (const config::attribute_value *v = parsed_cfg.get("y")) var["y"] = *v;
		return;
	}

	int side = parsed_cfg["side"].to_int(1);


	if ((side<1)||(side > static_cast<int>(resources::teams->size()))) {
		ERR_NG << "wrong side in [unit] tag - no such side: "<<side<<" ( number of teams :"<<resources::teams->size()<<")"<<std::endl;
		DBG_NG << parsed_cfg.debug();
		return;
	}
	team &tm = resources::teams->at(side-1);

	unit_creator uc(tm,resources::game_map->starting_position(side));

	uc
		.allow_add_to_recall(true)
		.allow_discover(true)
		.allow_get_village(true)
		.allow_invalidate(true)
		.allow_rename_side(true)
		.allow_show(true);

	uc.add_unit(parsed_cfg, &cfg);

}

// If we should recall units that match a certain description
WML_HANDLER_FUNCTION(recall, /*event_info*/, cfg)
{
	LOG_NG << "recalling unit...\n";
	config temp_config(cfg.get_config());
	// Prevent the recall unit filter from using the location as a criterion

	/**
	 * @todo FIXME: we should design the WML to avoid these types of
	 * collisions; filters should be named consistently and always have a
	 * distinct scope.
	 */
	temp_config["x"] = "recall";
	temp_config["y"] = "recall";
	vconfig unit_filter(temp_config);
	const vconfig leader_filter = cfg.child("secondary_unit");

	for(int index = 0; index < int(resources::teams->size()); ++index) {
		LOG_NG << "for side " << index + 1 << "...\n";
		const std::string player_id = (*resources::teams)[index].save_id();

		if((*resources::teams)[index].recall_list().size() < 1) {
			DBG_NG << "recall list is empty when trying to recall!\n"
				   << "player_id: " << player_id << " side: " << index+1 << "\n";
			continue;
		}

		std::vector<unit>& avail = (*resources::teams)[index].recall_list();
		std::vector<unit_map::unit_iterator> leaders = resources::units->find_leaders(index + 1);

		for(std::vector<unit>::iterator u = avail.begin(); u != avail.end(); ++u) {
			DBG_NG << "checking unit against filter...\n";
			scoped_recall_unit auto_store("this_unit", player_id, u - avail.begin());
			if (u->matches_filter(unit_filter, map_location())) {
				DBG_NG << u->id() << " matched the filter...\n";
				const unit to_recruit(*u);
				const unit* pass_check = &to_recruit;
				if(!cfg["check_passability"].to_bool(true)) pass_check = NULL;
				const map_location cfg_loc = cfg_to_loc(cfg);

				//TODO fendrin: comment this monster
				BOOST_FOREACH(unit_map::const_unit_iterator leader, leaders) {
					DBG_NG << "...considering " + leader->id() + " as the recalling leader...\n";
					map_location loc = cfg_loc;
					if ( (leader_filter.null() || leader->matches_filter(leader_filter, leader->get_location())) &&
							(u->matches_filter(vconfig(leader->recall_filter()), map_location())) ) {
						DBG_NG << "...matched the leader filter and is able to recall the unit.\n";
						if(!resources::game_map->on_board(loc))
							loc = leader->get_location();
						if(pass_check || (resources::units->count(loc) > 0))
							loc = pathfind::find_vacant_tile(*resources::game_map,
									*resources::units, loc, pathfind::VACANT_ANY, pass_check);
						if(resources::game_map->on_board(loc)) {
							DBG_NG << "...valid location for the recall found. Recalling.\n";
							avail.erase(u);	// Erase before recruiting, since recruiting can fire more events
							place_recruit(to_recruit, loc, leader->get_location(), true,
									cfg["show"].to_bool(true), cfg["fire_event"].to_bool(false), true, true);
							return;
						}
					}
				}
				if (resources::game_map->on_board(cfg_loc)) {
					map_location loc = cfg_loc;
					if(pass_check || (resources::units->count(loc) > 0))
						loc = pathfind::find_vacant_tile(*resources::game_map,
								*resources::units, loc, pathfind::VACANT_ANY, pass_check);
					// Check if we still have a valid location
					if (resources::game_map->on_board(loc)) {
						DBG_NG << "No usable leader found, but found usable location. Recalling.\n";
						avail.erase(u);	// Erase before recruiting, since recruiting can fire more events
						map_location null_location = map_location::null_location;
						place_recruit(to_recruit, loc, null_location, true,
								cfg["show"].to_bool(true), cfg["fire_event"].to_bool(false), true, true);
						return;
					}
				}
			}
		}
	}
	//TODO I don't know about that error throwing. Sometimes a unit is just not available,
	//the designer needs to check with [have_unit] or fetch the recall event.
	ERR_NG << "Trying to recall unit failed!\n";
}

WML_HANDLER_FUNCTION(object, event_info, cfg)
{
	const vconfig filter = cfg.child("filter");

	std::string id = cfg["id"];

	// If this item has already been used
	if(id != "" && used_items.count(id))
		return;

	std::string image = cfg["image"];
	std::string caption = cfg["name"];
	std::string text;

	map_location loc;
	if(!filter.null()) {
		BOOST_FOREACH(const unit &u, *resources::units) {
			if (game_events::unit_matches_filter(u, filter)) {
				loc = u.get_location();
				break;
			}
		}
	}

	if(loc.valid() == false) {
		loc = event_info.loc1;
	}

	const unit_map::iterator u = resources::units->find(loc);

	std::string command_type = "then";

	if (u != resources::units->end() && (filter.null() || game_events::unit_matches_filter(*u, filter)))
	{
		text = cfg["description"].str();

		if(cfg["delayed_variable_substitution"].to_bool(false))
			u->add_modification("object", cfg.get_config());
		else
			u->add_modification("object", cfg.get_parsed_config());

		resources::screen->select_hex(event_info.loc1);
		resources::screen->invalidate_unit();

		// Mark this item as used up.
		used_items.insert(id);
	} else {
		text = cfg["cannot_use_message"].str();
		command_type = "else";
	}

	if (!cfg["silent"].to_bool())
	{
		// Redraw the unit, with its new stats
		resources::screen->draw();

		try {
			gui2::show_transient_message(resources::screen->video(), caption, text, image, true);
		} catch(utils::invalid_utf8_exception&) {
			// we already had a warning so do nothing.
		}
	}

	BOOST_FOREACH(const vconfig &cmd, cfg.get_children(command_type)) {
		handle_event_commands(event_info, cmd);
	}
}

WML_HANDLER_FUNCTION(print, /*event_info*/, cfg)
{
	// Remove any old message.
	if (floating_label)
		font::remove_floating_label(floating_label);

	// Display a message on-screen
	std::string text = cfg["text"];
	if(text.empty())
		return;

	int size = cfg["size"].to_int(font::SIZE_SMALL);
	int lifetime = cfg["duration"].to_int(50);
	SDL_Color color = create_color(cfg["red"], cfg["green"], cfg["blue"]);

	const SDL_Rect& rect = resources::screen->map_outside_area();

	font::floating_label flabel(text);
	flabel.set_font_size(size);
	flabel.set_color(color);
	flabel.set_position(rect.w/2,rect.h/2);
	flabel.set_lifetime(lifetime);
	flabel.set_clip_rect(rect);

	floating_label = font::add_floating_label(flabel);
}

WML_HANDLER_FUNCTION(deprecated_message, /*event_info*/, cfg)
{
	game_events::handle_deprecated_message( cfg.get_parsed_config() );
}

WML_HANDLER_FUNCTION(wml_message, /*event_info*/, cfg)
{
	game_events::handle_wml_log_message( cfg.get_parsed_config() );
}


typedef std::map<map_location, int> recursion_counter;

class recursion_preventer {
	static recursion_counter counter_;
	static const int max_recursion = 10;
	map_location loc_;
	bool too_many_recursions_;

	public:
	recursion_preventer(map_location& loc) :
		loc_(loc),
		too_many_recursions_(false)
	{
		recursion_counter::iterator inserted = counter_.insert(std::make_pair(loc_, 0)).first;
		++inserted->second;
		too_many_recursions_ = inserted->second >= max_recursion;
	}
	~recursion_preventer()
	{
		recursion_counter::iterator itor = counter_.find(loc_);
		if (--itor->second == 0)
		{
			counter_.erase(itor);
		}
	}
	bool too_many_recursions() const
	{
		return too_many_recursions_;
	}
};

recursion_counter recursion_preventer::counter_ = recursion_counter();

typedef boost::scoped_ptr<recursion_preventer> recursion_preventer_ptr;

WML_HANDLER_FUNCTION(kill, event_info, cfg)
{
	bool secondary_unit = cfg.has_child("secondary_unit");
	game_events::entity_location killer_loc(map_location(0, 0));
	if(cfg["fire_event"].to_bool() && secondary_unit)
	{
		secondary_unit = false;
		for(unit_map::const_unit_iterator unit = resources::units->begin();
			unit != resources::units->end(); ++unit) {
				if(game_events::unit_matches_filter(*unit, cfg.child("secondary_unit")))
				{
					killer_loc = game_events::entity_location(*unit);
					secondary_unit = true;
					break;
				}
		}
		if(!secondary_unit) {
			WRN_NG << "failed to match [secondary_unit] in [kill] with a single on-board unit\n";
		}
	}

	//Find all the dead units first, because firing events ruins unit_map iteration
	std::vector<unit *> dead_men_walking;
	// unit_map::iterator uit(resources::units->begin()), uend(resources::units->end());
	// for(;uit!=uend; ++uit){
	BOOST_FOREACH(unit & u, *resources::units){
		if(game_events::unit_matches_filter(u, cfg)){
			dead_men_walking.push_back(&u);
		}
	}

	BOOST_FOREACH(unit * un, dead_men_walking) {
		map_location loc(un->get_location());
		bool fire_event = false;
		game_events::entity_location death_loc(*un);
		if(!secondary_unit) {
			killer_loc = game_events::entity_location(*un);
		}
		if (cfg["fire_event"].to_bool())
			{
				// Prevent infinite recursion of 'die' events
				fire_event = true;
				recursion_preventer_ptr recursion_prevent;

				if (event_info.loc1 == death_loc && (event_info.name == "die" || event_info.name == "last breath"))
					{
						recursion_prevent.reset(new recursion_preventer(death_loc));

						if(recursion_prevent->too_many_recursions())
							{
								fire_event = false;

								ERR_NG << "tried to fire 'die' or 'last breath' event on primary_unit inside its own 'die' or 'last breath' event with 'first_time_only' set to false!\n";
							}
					}
			}

		if (fire_event) {
			game_events::fire("last breath", death_loc, killer_loc);
		}

		if (cfg["animate"].to_bool()) {
			resources::screen->scroll_to_tile(loc);
			unit_map::iterator iun = resources::units->find(loc);
			if (iun != resources::units->end() && iun.valid()) {
				unit_display::unit_die(loc, *iun);
			}
		} else {
			// Make sure the unit gets (fully) cleaned off the screen.
			resources::screen->invalidate(loc);
			unit_map::iterator iun = resources::units->find(loc);
			if ( iun != resources::units->end()  &&  iun.valid() )
				iun->invalidate(loc);
		}
		resources::screen->redraw_minimap();

		if (fire_event) {
			game_events::fire("die", death_loc, killer_loc);
			unit_map::iterator iun = resources::units->find(death_loc);
			if (iun != resources::units->end() && death_loc.matches_unit(*iun)) {
				resources::units->erase(iun);
			}
		}
		else resources::units->erase(loc);

	}

	// If the filter doesn't contain positional information,
	// then it may match units on all recall lists.
	t_string const& cfg_x = cfg["x"];
	t_string const& cfg_y = cfg["y"];
	if((cfg_x.empty() || cfg_x == "recall")
	&& (cfg_y.empty() || cfg_y == "recall"))
	{
		//remove the unit from the corresponding team's recall list
		for(std::vector<team>::iterator pi = resources::teams->begin();
				pi!=resources::teams->end(); ++pi)
		{
			std::vector<unit>& avail_units = pi->recall_list();
			for(std::vector<unit>::iterator j = avail_units.begin(); j != avail_units.end();) {
				scoped_recall_unit auto_store("this_unit", pi->save_id(), j - avail_units.begin());
				if (j->matches_filter(cfg, map_location())) {
					j = avail_units.erase(j);
				} else {
					++j;
				}
			}
		}
	}
}

// Setting of menu items
WML_HANDLER_FUNCTION(set_menu_item, /*event_info*/, cfg)
{
	/*
	   [set_menu_item]
	   id=test1
	   image="buttons/group_all.png"
	   description="Summon Troll"
	   [show_if]
	   [not]
	   [have_unit]
	   x,y=$x1,$y1
	   [/have_unit]
	   [/not]
	   [/show_if]
	   [filter_location]
	   [/filter_location]
	   [command]
	   {LOYAL_UNIT $side_number (Troll) $x1 $y1 (Myname) ( _ "Myname")}
	   [/command]
	   [/set_menu_item]
	   */
	std::string id = cfg["id"];
	wml_menu_item*& mref = resources::state_of_game->wml_menu_items[id];
	if(mref == NULL) {
		mref = new wml_menu_item(id);
	}
	if(cfg.has_attribute("image")) {
		mref->image = cfg["image"].str();
	}
	if(cfg.has_attribute("description")) {
		mref->description = cfg["description"];
	}
	if(cfg.has_attribute("needs_select")) {
		mref->needs_select = cfg["needs_select"].to_bool();
	}
	if(cfg.has_child("show_if")) {
		mref->show_if = cfg.child("show_if").get_config();
	}
	if(cfg.has_child("filter_location")) {
		mref->filter_location = cfg.child("filter_location").get_config();
	}
	if(cfg.has_child("command")) {
		config* new_command = new config(cfg.child("command").get_config());
		wmi_command_changes.push_back(wmi_command_change(id, new_command));
	}
}

struct unstore_unit_advance_choice: mp_sync::user_choice
{
	int nb_options;
	map_location loc;
	bool use_dialog;

	unstore_unit_advance_choice(int o, const map_location &l, bool d)
		: nb_options(o), loc(l), use_dialog(d)
	{}

	virtual config query_user() const
	{
		int selected;
		if (use_dialog) {
			DBG_NG << "dialog requested\n";
			selected = dialogs::advance_unit_dialog(loc);
		} else {
			// VITAL this is NOT done using the synced RNG
			selected = rand() % nb_options;
		}
		config cfg;
		cfg["value"] = selected;
		return cfg;
	}

	virtual config random_choice(rand_rng::simple_rng &rng) const
	{
		config cfg;
		cfg["value"] = rng.get_next_random() % nb_options;
		return cfg;
	}
};

// Unit serialization to variables
WML_HANDLER_FUNCTION(unstore_unit, /*event_info*/, cfg)
{
	const config &var = resources::state_of_game->get_variable_cfg(cfg["variable"]);

	try {
		config tmp_cfg(var);
		const unit u(tmp_cfg, false);

		preferences::encountered_units().insert(u.type_id());
		map_location loc = cfg_to_loc(
			(cfg.has_attribute("x") && cfg.has_attribute("y")) ? cfg : vconfig(var));
		const bool advance = cfg["advance"].to_bool(true);
		if(resources::game_map->on_board(loc)) {
			if (cfg["find_vacant"].to_bool()) {
				const unit* pass_check = NULL;
				if (cfg["check_passability"].to_bool(true)) pass_check = &u;
				loc = pathfind::find_vacant_tile(
						*resources::game_map,
						*resources::units,
						loc,
						pathfind::VACANT_ANY,
						pass_check);
			}

			resources::units->erase(loc);
			resources::units->add(loc, u);

			std::string text = cfg["text"];
			play_controller *controller = resources::controller;
			if(!text.empty() && !controller->is_skipping_replay())
			{
				// Print floating label
				resources::screen->float_label(loc, text, cfg["red"], cfg["green"], cfg["blue"]);
			}

			const int side = controller->current_side();
			if (advance &&
			    unit_helper::will_certainly_advance(resources::units->find(loc)))
			{
				int total_opt = unit_helper::number_of_possible_advances(u);
				bool use_dialog = side == u.side() &&
					(*resources::teams)[side - 1].is_human();
				config selected = mp_sync::get_user_choice("choose",
					unstore_unit_advance_choice(total_opt, loc, use_dialog));
				dialogs::animate_unit_advancement(loc, selected["value"], cfg["fire_event"].to_bool(false));
			}
		} else {
			if(advance && u.advances()) {
				WRN_NG << "Cannot advance units when unstoring to the recall list.\n";
			}

			team& t = (*resources::teams)[u.side()-1];

			if(t.persistent()) {

				// Test whether the recall list has duplicates if so warn.
				// This might be removed at some point but the uniqueness of
				// the description is needed to avoid the recall duplication
				// bugs. Duplicates here might cause the wrong unit being
				// replaced by the wrong unit.
				if(t.recall_list().size() > 1) {
					std::vector<size_t> desciptions;
					for(std::vector<unit>::const_iterator citor =
							t.recall_list().begin();
							citor != t.recall_list().end(); ++citor) {

						const size_t desciption =
							citor->underlying_id();
						if(std::find(desciptions.begin(), desciptions.end(),
									desciption) != desciptions.end()) {

							lg::wml_error << "Recall list has duplicate unit "
								"underlying_ids '" << desciption
								<< "' unstore_unit may not work as expected.\n";
						} else {
							desciptions.push_back(desciption);
						}
					}
				}

				// Avoid duplicates in the list.
				/**
				 * @todo it would be better to change recall_list() from
				 * a vector to a map and use the underlying_id as key.
				 */
				const size_t key = u.underlying_id();
				for(std::vector<unit>::iterator itor =
						t.recall_list().begin();
						itor != t.recall_list().end(); ++itor) {

					LOG_NG << "Replaced unit '"
						<< key << "' on the recall list\n";
					if(itor->underlying_id() == key) {
						t.recall_list().erase(itor);
						break;
					}
				}
				t.recall_list().push_back(u);
			} else {
				ERR_NG << "Cannot unstore unit: recall list is empty for player " << u.side()
					<< " and the map location is invalid.\n";
			}
		}

		// If we unstore a leader make sure the team gets a leader if not the loading
		// in MP might abort since a side without a leader has a recall list.
		if(u.can_recruit()) {
			(*resources::teams)[u.side() - 1].have_leader();
		}

	} catch (game::game_error &e) {
		ERR_NG << "could not de-serialize unit: '" << e.message << "'\n";
	}
}

/* [store_villages] : store villages into an array
 * Keys:
 * - variable (mandatory): variable to store in
 * - side: if present, the village should be owned by this side (0=unowned villages)
 * - terrain: if present, filter the village types against this list of terrain types
 */
WML_HANDLER_FUNCTION(store_villages, /*event_info*/, cfg)
{
	log_scope("store_villages");
	std::string variable = cfg["variable"];
	if (variable.empty()) {
		variable="location";
	}
	config to_store;
	variable_info varinfo(variable, true, variable_info::TYPE_ARRAY);

	std::vector<map_location> locs = resources::game_map->villages();

	for(std::vector<map_location>::const_iterator j = locs.begin(); j != locs.end(); ++j) {
		bool matches = terrain_filter(cfg, *resources::units).match(*j);
		if(matches) {
			config &loc_store = to_store.add_child(varinfo.key);
			j->write(loc_store);
			resources::game_map->write_terrain(*j, loc_store);
			int side = village_owner(*j, *resources::teams) + 1;
			loc_store["owner_side"] = side;
		}
	}
	varinfo.vars->clear_children(varinfo.key);
	varinfo.vars->append(to_store);
}

WML_HANDLER_FUNCTION(end_turn, /*event_info*/, /*cfg*/)
{
	resources::controller->force_end_turn();
}

WML_HANDLER_FUNCTION(endlevel, /*event_info*/, cfg)
{
	end_level_data &data = resources::controller->get_end_level_data();
	if(data.disabled) {
		WRN_NG << "repeated [endlevel] execution, ignoring\n";
		return;
	}
	data.disabled = true;

	game_state *state_of_game = resources::state_of_game;
	unit_map *units = resources::units;

	// Remove 0-hp units from the unit map to avoid the following problem:
	// In case a die event triggers an endlevel the dead unit is still as a
	// 'ghost' in linger mode. After save loading in linger mode the unit
	// is fully visible again.
	unit_map::iterator u = units->begin();
	while (u != units->end()) {
		if (u->hitpoints() <= 0) {
			units->erase(u++);
		} else {
			++u;
		}
	}

	std::string next_scenario = cfg["next_scenario"];
	if (!next_scenario.empty()) {
		state_of_game->classification().next_scenario = next_scenario;
	}

	std::string end_of_campaign_text = cfg["end_text"];
	if (!end_of_campaign_text.empty()) {
		state_of_game->classification().end_text = end_of_campaign_text;
	}

	config::attribute_value end_of_campaign_text_delay = cfg["end_text_duration"];
	if (!end_of_campaign_text_delay.empty()) {
		state_of_game->classification().end_text_duration =
			end_of_campaign_text_delay.to_int(state_of_game->classification().end_text_duration);
	}


	std::string result = cfg["result"];
	VALIDATE_WITH_DEV_MESSAGE(
			  result.empty() || result == "victory" || result == "defeat"
			, _("Invalid value in the result key for [end_level]")
			, "result = '"  + result + "'.");
	data.custom_endlevel_music = cfg["music"].str();
	data.carryover_report = cfg["carryover_report"].to_bool(true);
	data.prescenario_save = cfg["save"].to_bool(true);
	data.replay_save = cfg["replay_save"].to_bool(true);
	data.linger_mode = cfg["linger_mode"].to_bool(true)
		&& !resources::teams->empty();
	data.reveal_map = cfg["reveal_map"].to_bool(true);
	data.gold_bonus = cfg["bonus"].to_bool(true);
	data.carryover_percentage = cfg["carryover_percentage"].to_int(game_config::gold_carryover_percentage);
	data.carryover_add = cfg["carryover_add"].to_bool();

	if(result == "defeat") {
		data.carryover_add = false;
		resources::controller->force_end_level(DEFEAT);
	} else {
		resources::controller->force_end_level(VICTORY);
	}
}

WML_HANDLER_FUNCTION(redraw, /*event_info*/, cfg)
{
	game_display &screen = *resources::screen;

	const config::attribute_value clear_shroud_av = cfg["clear_shroud"];
	const config::attribute_value side = cfg["side"];
	bool clear_shroud_bool = clear_shroud_av.to_bool(false);
	if(clear_shroud_av.blank() && !side.blank()) {
		//Backwards compat, behavior of the tag was to clear shroud in case that side= is given.
		clear_shroud_bool = true;
	}

	if (clear_shroud_bool) {
		side_filter filter(cfg);
		BOOST_FOREACH(const int side, filter.get_teams()){
			clear_shroud(side);
		}
		screen.recalculate_minimap();
	}
	if (screen_needs_rebuild) {
		screen_needs_rebuild = false;
		screen.recalculate_minimap();
		screen.rebuild_all();
	}
	screen.invalidate_all();
	screen.draw(true,true);
}

WML_HANDLER_FUNCTION(animate_unit, event_info, cfg)
{
	const events::command_disabler disable_commands;
	unit_display::wml_animation(cfg, event_info.loc1);
}

WML_HANDLER_FUNCTION(label, /*event_info*/, cfg)
{
	game_display &screen = *resources::screen;

	terrain_label label(screen.labels(), cfg.get_config());

	screen.labels().set_label(label.location(), label.text(),
		label.team_name(), label.color(), label.visible_in_fog(), label.visible_in_shroud(), label.immutable());
}

WML_HANDLER_FUNCTION(heal_unit, event_info, cfg)
{
	unit_map* units = resources::units;

	const vconfig healers_filter = cfg.child("filter_second");
	std::vector<unit*> healers;
	if (!healers_filter.null()) {
		BOOST_FOREACH(unit& u, *units) {
			if (game_events::unit_matches_filter(u, healers_filter) && u.has_ability_type("heals")) {
				healers.push_back(&u);
			}
		}
	}

	const config::attribute_value amount = cfg["amount"];
	const config::attribute_value moves = cfg["moves"];
	const bool restore_attacks = cfg["restore_attacks"].to_bool(false);
	const bool restore_statuses = cfg["restore_statuses"].to_bool(true);
	const bool animate = cfg["animate"].to_bool(false);

	const vconfig healed_filter = cfg.child("filter");
	bool only_unit_at_loc1 = healed_filter.null();
	bool heal_amount_to_set = true;
	for(unit_map::unit_iterator u  = units->begin(); u != units->end(); ++u) {
		if (only_unit_at_loc1)
		{
			u = units->find(event_info.loc1);
			if(!u.valid()) return;
		}
		else if (!game_events::unit_matches_filter(*u, healed_filter)) continue;

		int heal_amount = u->max_hitpoints() - u->hitpoints();
		if(amount.blank() || amount == "full") u->set_hitpoints(u->max_hitpoints());
		else {
			heal_amount = lexical_cast_default<int, config::attribute_value> (amount, heal_amount);
			const int new_hitpoints = std::max(1, std::min(u->max_hitpoints(), u->hitpoints() + heal_amount));
			heal_amount = new_hitpoints - u->hitpoints();
			u->set_hitpoints(new_hitpoints);
		}

		if(!moves.blank()) {
			if(moves == "full") u->set_movement(u->total_movement());
			else {
				// set_movement doesn't set below 0
				u->set_movement(std::min<int>(
					u->total_movement(),
					u->movement_left() + lexical_cast_default<int, config::attribute_value> (moves, 0)
					));
			}
		}

		if(restore_attacks) u->set_attacks(u->max_attacks());

		if(restore_statuses)
		{
			u->set_state(unit::STATE_POISONED, false);
			u->set_state(unit::STATE_SLOWED, false);
			u->set_state(unit::STATE_PETRIFIED, false);
			u->set_state(unit::STATE_UNHEALABLE, false);
		}

		if (heal_amount_to_set)
		{
			heal_amount_to_set = false;
			resources::state_of_game->get_variable("heal_amount") = heal_amount;
		}

		if(animate) unit_display::unit_healing(*u, u->get_location(), healers, heal_amount);
		if(only_unit_at_loc1) return;
	}
}

// Allow undo sets the flag saying whether the event has mutated the game to false
WML_HANDLER_FUNCTION(allow_undo,/*event_info*/,/*cfg*/)
{
	current_context->mutated = false;
}

WML_HANDLER_FUNCTION(open_help,  /*event_info*/, cfg)
{
	game_display &screen = *resources::screen;
	t_string topic_id = cfg["topic"];
	help::show_help(screen, topic_id.to_serialized());
}
// Helper namespace to do some subparts for message function
namespace {

/**
 * Helper to handle the speaker part of the message.
 *
 * @param event_info              event_info of message.
 * @param cfg                     cfg of message.
 *
 * @returns                       The unit who's the speaker or units->end().
 */
unit_map::iterator handle_speaker(
		const game_events::queued_event& event_info,
		const vconfig& cfg,
		bool scroll)
{
	unit_map *units = resources::units;
	game_display &screen = *resources::screen;

	unit_map::iterator speaker = units->end();
	const std::string speaker_str = cfg["speaker"];

	if(speaker_str == "unit") {
		speaker = units->find(event_info.loc1);
	} else if(speaker_str == "second_unit") {
		speaker = units->find(event_info.loc2);
	} else if(speaker_str != "narrator") {
		for(speaker = units->begin(); speaker != units->end(); ++speaker){
			if (game_events::unit_matches_filter(*speaker, cfg))
				break;
		}
	}
	if(speaker != units->end()) {
		LOG_NG << "set speaker to '" << speaker->name() << "'\n";
		const map_location &spl = speaker->get_location();
		screen.highlight_hex(spl);
		if(scroll) {
			LOG_DP << "scrolling to speaker..\n";
			const int offset_from_center = std::max<int>(0, spl.y - 1);
			screen.scroll_to_tile(map_location(spl.x, offset_from_center));
		}
		screen.highlight_hex(spl);
	} else if(speaker_str == "narrator") {
		LOG_NG << "no speaker\n";
		screen.highlight_hex(map_location::null_location);
	} else {
		return speaker;
	}

	screen.draw(false);
	LOG_DP << "done scrolling to speaker...\n";
	return speaker;
}

/**
 * Helper to handle the image part of the message.
 *
 * @param cfg                     cfg of message.
 * @param speaker                 The speaker of the message.
 *
 * @returns                       The image to show.
 */
std::string get_image(const vconfig& cfg, unit_map::iterator speaker)
{
	std::string image = cfg["image"];
	if (image.empty() && speaker != resources::units->end())
	{
		image = speaker->big_profile();
#ifndef LOW_MEM
		if(image == speaker->absolute_image()) {
			image += speaker->image_mods();
		}
#endif
	}
	return image;
}

/**
 * Helper to handle the caption part of the message.
 *
 * @param cfg                     cfg of message.
 * @param speaker                 The speaker of the message.
 *
 * @returns                       The caption to show.
 */
std::string get_caption(const vconfig& cfg, unit_map::iterator speaker)
{
	std::string caption = cfg["caption"];
	if (caption.empty() && speaker != resources::units->end()) {
		caption = speaker->name();
		if(caption.empty()) {
			caption = speaker->type_name();
		}
	}
	return caption;
}

} // namespace

struct message_user_choice : mp_sync::user_choice
{
	vconfig cfg;
	unit_map::iterator speaker;
	vconfig text_input_element;
	bool has_text_input;
	const std::vector<std::string> &options;

	message_user_choice(const vconfig &c, const unit_map::iterator &s,
		const vconfig &t, bool ht, const std::vector<std::string> &o)
		: cfg(c), speaker(s), text_input_element(t)
		, has_text_input(ht), options(o)
	{}

	virtual config query_user() const
	{
		std::string image = get_image(cfg, speaker);
		std::string caption = get_caption(cfg, speaker);

		size_t right_offset = image.find("~RIGHT()");
		bool left_side = right_offset == std::string::npos;
		if (!left_side) {
			image.erase(right_offset);
		}

		// Parse input text, if not available all fields are empty
		std::string text_input_label = text_input_element["label"];
		std::string text_input_content = text_input_element["text"];
		unsigned input_max_size = text_input_element["max_length"].to_int(256);
		if (input_max_size > 1024 || input_max_size < 1) {
			lg::wml_error << "invalid maximum size for input "
				<< input_max_size << '\n';
			input_max_size = 256;
		}

		int option_chosen = -1;
		int dlg_result = gui2::show_wml_message(left_side,
			resources::screen->video(), caption, cfg["message"],
			image, false, has_text_input, text_input_label,
			&text_input_content, input_max_size, options,
			&option_chosen);

		/* Since gui2::show_wml_message needs to do undrawing the
		   chatlines can get garbled and look dirty on screen. Force a
		   redraw to fix it. */
		/** @todo This hack can be removed once gui2 is finished. */
		resources::screen->invalidate_all();
		resources::screen->draw(true,true);

		if (dlg_result == gui2::twindow::CANCEL) {
			current_context->skip_messages = true;
		}

		config cfg;
		if (!options.empty()) cfg["value"] = option_chosen;
		if (has_text_input) cfg["text"] = text_input_content;
		return cfg;
	}

	virtual config random_choice(rand_rng::simple_rng &) const
	{
		return config();
	}
};

// Display a message dialog
WML_HANDLER_FUNCTION(message, event_info, cfg)
{
	// Check if there is any input to be made, if not the message may be skipped
	const vconfig::child_list menu_items = cfg.get_children("option");

	const vconfig::child_list text_input_elements = cfg.get_children("text_input");
	const bool has_text_input = (text_input_elements.size() == 1);

	bool has_input= (has_text_input || !menu_items.empty() );

	// skip messages during quick replay
	play_controller *controller = resources::controller;
	if(!has_input && (
			 controller->is_skipping_replay() ||
			 current_context->skip_messages
			 ))
	{
		return;
	}

	// Check if this message is for this side
	std::string side_for_raw = cfg["side_for"];
	if (!side_for_raw.empty())
	{
		/* Always ignore side_for when the message has some input
		   boxes, but display the error message only if side_for is
		   used for an inactive side. */
		bool side_for_show = has_input;
		if (has_input && side_for_raw != str_cast(resources::controller->current_side()))
			lg::wml_error << "[message]side_for= cannot query any user input out of turn.\n";

		std::vector<std::string> side_for =
			utils::split(side_for_raw, ',', utils::STRIP_SPACES | utils::REMOVE_EMPTY);
		std::vector<std::string>::iterator itSide;
		size_t side;

		// Check if any of side numbers are human controlled
		for (itSide = side_for.begin(); itSide != side_for.end(); ++itSide)
		{
			side = lexical_cast_default<size_t>(*itSide);
			// Make sanity check that side number is good
			// then check if this side is human controlled.
			if (side > 0 && side <= resources::teams->size() &&
				(*resources::teams)[side-1].is_human())
			{
				side_for_show = true;
				break;
			}
		}
		if (!side_for_show)
		{
			DBG_NG << "player isn't controlling side which should get message\n";
			return;
		}
	}

	unit_map::iterator speaker = handle_speaker(event_info, cfg, cfg["scroll"].to_bool(true));
	if (speaker == resources::units->end() && cfg["speaker"] != "narrator") {
		// No matching unit found, so the dialog can't come up.
		// Continue onto the next message.
		WRN_NG << "cannot show message\n";
		return;
	}

	std::vector<std::string> options;
	std::vector<vconfig::child_list> option_events;

	for(vconfig::child_list::const_iterator mi = menu_items.begin();
			mi != menu_items.end(); ++mi) {
		std::string msg_str = (*mi)["message"];
		if (!mi->has_child("show_if")
			|| game_events::conditional_passed(mi->child("show_if")))
		{
			options.push_back(msg_str);
			option_events.push_back((*mi).get_children("command"));
		}
	}

	has_input = !options.empty() || has_text_input;
	if (!has_input && get_replay_source().is_skipping()) {
		// No input to get and the user is not interested either.
		return;
	}

	if (cfg.has_attribute("sound")) {
		sound::play_sound(cfg["sound"]);
	}

	if(text_input_elements.size()>1) {
		lg::wml_error << "too many text_input tags, only one accepted\n";
	}

	const vconfig text_input_element = has_text_input ?
		text_input_elements.front() : vconfig::empty_vconfig();

	int option_chosen = 0;
	std::string text_input_result;

	DBG_DP << "showing dialog...\n";

	message_user_choice msg(cfg, speaker, text_input_element, has_text_input,
		options);
	if (!has_input)
	{
		/* Always show the dialog if it has no input, whether we are
		   replaying or not. */
		msg.query_user();
	}
	else
	{
		config choice = mp_sync::get_user_choice("input", msg, 0, true);
		option_chosen = choice["value"];
		text_input_result = choice["text"].str();
	}

	// Implement the consequences of the choice
	if(options.empty() == false) {
		if(size_t(option_chosen) >= menu_items.size()) {
			std::stringstream errbuf;
			errbuf << "invalid choice (" << option_chosen
				<< ") was specified, choice 0 to " << (menu_items.size() - 1)
				<< " was expected.\n";
			replay::process_error(errbuf.str());
			return;
		}

		BOOST_FOREACH(const vconfig &cmd, option_events[option_chosen]) {
			handle_event_commands(event_info, cmd);
		}
	}
	if(has_text_input) {
		std::string variable_name=text_input_element["variable"];
		if(variable_name.empty())
			variable_name="input";
		resources::state_of_game->set_variable(variable_name, text_input_result);
	}
}

// Adding/removing new time_areas dynamically with Standard Location Filters.
WML_HANDLER_FUNCTION(time_area, /*event_info*/, cfg)
{
	log_scope("time_area");

	bool remove = cfg["remove"].to_bool();
	std::string ids = cfg["id"];

	if(remove) {
		const std::vector<std::string> id_list =
			utils::split(ids, ',', utils::STRIP_SPACES | utils::REMOVE_EMPTY);
		BOOST_FOREACH(const std::string& id, id_list) {
			resources::tod_manager->remove_time_area(id);
			LOG_NG << "event WML removed time_area '" << id << "'\n";
		}
	}
	else {
		std::string id;
		if(ids.find(',') != std::string::npos) {
			id = utils::split(ids,',',utils::STRIP_SPACES | utils::REMOVE_EMPTY).front();
			ERR_NG << "multiple ids for inserting a new time_area; will use only the first\n";
		} else {
			id = ids;
		}
		std::set<map_location> locs;
		terrain_filter filter(cfg, *resources::units);
		filter.restrict_size(game_config::max_loop);
		filter.get_locations(locs, true);
		config parsed_cfg = cfg.get_parsed_config();
		resources::tod_manager->add_time_area(id, locs, parsed_cfg);
		LOG_NG << "event WML inserted time_area '" << id << "'\n";
	}
}

//Replacing the current time of day schedule
WML_HANDLER_FUNCTION(replace_schedule, /*event_info*/, cfg)
{
	if(cfg.get_children("time").empty()) {
		ERR_NG << "attempted to to replace ToD schedule with empty schedule\n";
	} else {
		resources::tod_manager->replace_schedule(cfg.get_parsed_config());
		resources::screen->new_turn();
		LOG_NG << "replaced ToD schedule\n";
	}
}

WML_HANDLER_FUNCTION(allow_end_turn, /*event_info*/, /*cfg*/)
{
	resources::state_of_game->set_allow_end_turn(true);
}

WML_HANDLER_FUNCTION(disallow_end_turn, /*event_info*/, /*cfg*/)
{
	resources::state_of_game->set_allow_end_turn(false);
}

// Adding new events
WML_HANDLER_FUNCTION(event, /*event_info*/, cfg)
{
	if (cfg["remove"].to_bool(false)) {
		event_handlers.remove_event_handler(cfg["id"]);
	} else if (!cfg["delayed_variable_substitution"].to_bool(true)) {
		event_handlers.add_event_handler(game_events::event_handler(cfg.get_parsed_config()));
	} else {
		event_handlers.add_event_handler(game_events::event_handler(cfg.get_config()));
	}
}

// Experimental map replace
WML_HANDLER_FUNCTION(replace_map, /*event_info*/, cfg)
{
	gamemap *game_map = resources::game_map;

	gamemap map(*game_map);
	try {
		map.read(cfg["map"], false);
	} catch(incorrect_map_format_error&) {
		lg::wml_error << "replace_map: Unable to load map " << cfg["map"] << "\n";
		return;
	} catch(twml_exception& e) {
		e.show(*resources::screen);
		return;
	}
	if (map.total_width() > game_map->total_width()
	|| map.total_height() > game_map->total_height()) {
		if (!cfg["expand"].to_bool()) {
			lg::wml_error << "replace_map: Map dimension(s) increase but expand is not set\n";
			return;
		}
	}
	if (map.total_width() < game_map->total_width()
	|| map.total_height() < game_map->total_height()) {
		if (!cfg["shrink"].to_bool()) {
			lg::wml_error << "replace_map: Map dimension(s) decrease but shrink is not set\n";
			return;
		}
		unit_map *units = resources::units;
		unit_map::iterator itor;
		for (itor = units->begin(); itor != units->end(); ) {
			if (!map.on_board(itor->get_location())) {
				if (!try_add_unit_to_recall_list(itor->get_location(), *itor)) {
					lg::wml_error << "replace_map: Cannot add a unit that would become off-map to the recall list\n";
				}
				units->erase(itor++);
			} else {
				++itor;
			}
		}
	}
	*game_map = map;
	resources::screen->reload_map();
	screen_needs_rebuild = true;
	ai::manager::raise_map_changed();
}

// Experimental data persistence
WML_HANDLER_FUNCTION(set_global_variable,/**/,pcfg)
{
	if (get_replay_source().at_end() || (network::nconnections() != 0))
		verify_and_set_global_variable(pcfg);
}
WML_HANDLER_FUNCTION(get_global_variable,/**/,pcfg)
{
	verify_and_get_global_variable(pcfg);
}
WML_HANDLER_FUNCTION(clear_global_variable,/**/,pcfg)
{
	if (get_replay_source().at_end() || (network::nconnections() != 0))
		verify_and_clear_global_variable(pcfg);
}

/** Handles all the different types of actions that can be triggered by an event. */

static void commit_wmi_commands() {
	// Commit WML Menu Item command changes
	while(wmi_command_changes.size() > 0) {
		wmi_command_change wcc = wmi_command_changes.front();
		const bool is_empty_command = wcc.second->empty();

		wml_menu_item*& mref = resources::state_of_game->wml_menu_items[wcc.first];
		const bool has_current_handler = !mref->command.empty();

		mref->command = *(wcc.second);
		mref->command["name"] = mref->name;
		mref->command["first_time_only"] = false;

		if(has_current_handler) {
			if(is_empty_command) {
				mref->command.add_child("allow_undo");
			}
			BOOST_FOREACH(game_events::event_handler& hand, event_handlers) {
				if(hand.is_menu_item() && hand.matches_name(mref->name)) {
					LOG_NG << "changing command for " << mref->name << " to:\n" << *wcc.second;
					hand = game_events::event_handler(mref->command, true);
				}
			}
		} else if(!is_empty_command) {
			LOG_NG << "setting command for " << mref->name << " to:\n" << *wcc.second;
			event_handlers.add_event_handler(game_events::event_handler(mref->command, true));
		}

		delete wcc.second;
		wmi_command_changes.erase(wmi_command_changes.begin());
	}
}

static bool process_event(game_events::event_handler& handler, const game_events::queued_event& ev)
{
	if(handler.disabled())
		return false;

	unit_map *units = resources::units;
	unit_map::iterator unit1 = units->find(ev.loc1);
	unit_map::iterator unit2 = units->find(ev.loc2);
	bool filtered_unit1 = false, filtered_unit2 = false;
	scoped_xy_unit first_unit("unit", ev.loc1.x, ev.loc1.y, *units);
	scoped_xy_unit second_unit("second_unit", ev.loc2.x, ev.loc2.y, *units);
	scoped_weapon_info first_weapon("weapon", ev.data.child("first"));
	scoped_weapon_info second_weapon("second_weapon", ev.data.child("second"));
	vconfig filters(handler.get_config());


	BOOST_FOREACH(const vconfig &condition, filters.get_children("filter_condition"))
	{
		if (!game_events::conditional_passed(condition)) {
			return false;
		}
	}

	BOOST_FOREACH(const vconfig &f, filters.get_children("filter"))
	{
		if (unit1 == units->end() || !game_events::unit_matches_filter(*unit1, f)) {
			return false;
		}
		if (!f.empty()) {
			filtered_unit1 = true;
		}
	}

	BOOST_FOREACH(const vconfig &f, filters.get_children("filter_side"))
	{
		side_filter ssf(f);
		const int current_side = resources::controller->current_side();
		if(!ssf.match(current_side)) return false;
	}

	vconfig::child_list special_filters = filters.get_children("filter_attack");
	bool special_matches = special_filters.empty();
	BOOST_FOREACH(const vconfig &f, special_filters)
	{
		if (unit1 != units->end() && game_events::matches_special_filter(ev.data.child("first"), f)) {
			special_matches = true;
		}
		if (!f.empty()) {
			filtered_unit1 = true;
		}
	}
	if(!special_matches) {
		return false;
	}

	BOOST_FOREACH(const vconfig &f, filters.get_children("filter_second"))
	{
		if (unit2 == units->end() || !game_events::unit_matches_filter(*unit2, f)) {
			return false;
		}
		if (!f.empty()) {
			filtered_unit2 = true;
		}
	}

	special_filters = filters.get_children("filter_second_attack");
	special_matches = special_filters.empty();
	BOOST_FOREACH(const vconfig &f, special_filters)
	{
		if (unit2 != units->end() && game_events::matches_special_filter(ev.data.child("second"), f)) {
			special_matches = true;
		}
		if (!f.empty()) {
			filtered_unit2 = true;
		}
	}
	if(!special_matches) {
		return false;
	}
	if (ev.loc1.requires_unit() && filtered_unit1 &&
	    (unit1 == units->end() || !ev.loc1.matches_unit(*unit1))) {
		// Wrong or missing entity at src location
		return false;
	}
	if (ev.loc2.requires_unit() && filtered_unit2 &&
	    (unit2 == units->end() || !ev.loc2.matches_unit(*unit2))) {
		// Wrong or missing entity at dst location
		return false;
	}

	// The event hasn't been filtered out, so execute the handler.
	scoped_context evc;
	handler.handle_event(ev);

	if(ev.name == "select") {
		resources::state_of_game->last_selected = ev.loc1;
	}

	if (screen_needs_rebuild) {
		screen_needs_rebuild = false;
		game_display *screen = resources::screen;
		screen->recalculate_minimap();
		screen->invalidate_all();
		screen->rebuild_all();
	}


	return current_context->mutated;
}

namespace game_events {

	event_handler::event_handler(const config &cfg, bool imi) :
		first_time_only_(cfg["first_time_only"].to_bool(true)),
		disabled_(false), is_menu_item_(imi), cfg_(cfg)
	{}

	void event_handler::handle_event(const game_events::queued_event& event_info)
	{
		if (first_time_only_)
		{
			disabled_ = true;
		}

		if (is_menu_item_) {
			DBG_NG << cfg_["name"] << " will now invoke the following command(s):\n" << cfg_;
		}

		handle_event_commands(event_info, vconfig(cfg_));
	}

	void handle_event_commands(const game_events::queued_event& event_info, const vconfig &cfg)
	{
		resources::lua_kernel->run_wml_action("command", cfg, event_info);
	}

	void handle_event_command(const std::string &cmd,
		const game_events::queued_event &event_info, const vconfig &cfg)
	{
		log_scope2(log_engine, "handle_event_command");
		LOG_NG << "handling command '" << cmd << "' from "
			<< (cfg.is_volatile()?"volatile ":"") << "cfg 0x"
			<< std::hex << std::setiosflags(std::ios::uppercase)
			<< reinterpret_cast<uintptr_t>(&cfg.get_config()) << std::dec << "\n";

		if (!resources::lua_kernel->run_wml_action(cmd, cfg, event_info))
		{
			ERR_NG << "Couldn't find function for wml tag: "<< cmd <<"\n";
		}

		DBG_NG << "done handling command...\n";
	}

	bool event_handler::matches_name(const std::string &name) const
	{
		const t_string& t_my_names = cfg_["name"];
		const std::string& my_names = t_my_names;
		std::string::const_iterator itor,
			it_begin = my_names.begin(),
			it_end = my_names.end(),
			match_it = name.begin(),
			match_begin = name.begin(),
			match_end = name.end();
		int skip_count = 0;
		for(itor = it_begin; itor != it_end; ++itor) {
			bool do_eat = false,
				do_skip = false;
			switch(*itor) {
			case ',':
				if(itor - it_begin - skip_count == match_it - match_begin && match_it == match_end) {
					return true;
				}
				it_begin = itor + 1;
				match_it = match_begin;
				skip_count = 0;
				continue;
			case '\f':
			case '\n':
			case '\r':
			case '\t':
			case '\v':
				do_skip = (match_it == match_begin || match_it == match_end);
				break;
			case ' ':
				do_skip = (match_it == match_begin || match_it == match_end);
				// fall through to case '_'
			case '_':
				do_eat = (match_it != match_end && (*match_it == ' ' || *match_it == '_'));
				break;
			default:
				do_eat = (match_it != match_end && *match_it == *itor);
				break;
			}
			if(do_eat) {
				++match_it;
			} else if(do_skip) {
				++skip_count;
			} else {
				itor = std::find(itor, it_end, ',');
				if(itor == it_end) {
					return false;
				}
				it_begin = itor + 1;
				match_it = match_begin;
				skip_count = 0;
			}
		}
		if(itor - it_begin - skip_count == match_it - match_begin && match_it == match_end) {
			return true;
		}
		return false;
	}

	bool matches_special_filter(const config &cfg, const vconfig& filter)
	{
		if (!cfg) {
			WRN_NG << "attempt to filter attack for an event with no attack data.\n";
			// better to not execute the event (so the problem is more obvious)
			return false;
		}
		const attack_type attack(cfg);
		bool matches = attack.matches_filter(filter.get_parsed_config());

		// Handle [and], [or], and [not] with in-order precedence
		vconfig::all_children_iterator cond_i = filter.ordered_begin();
		vconfig::all_children_iterator cond_end = filter.ordered_end();
		while(cond_i != cond_end)
		{
			const std::string& cond_name = cond_i.get_key();
			const vconfig& cond_filter = cond_i.get_child();

			// Handle [and]
			if(cond_name == "and")
			{
				matches = matches && matches_special_filter(cfg, cond_filter);
			}
			// Handle [or]
			else if(cond_name == "or")
			{
				matches = matches || matches_special_filter(cfg, cond_filter);
			}
			// Handle [not]
			else if(cond_name == "not")
			{
				matches = matches && !matches_special_filter(cfg, cond_filter);
			}
			++cond_i;
		}
		return matches;
	}

	bool unit_matches_filter(const unit &u, const vconfig& filter)
	{
		return u.matches_filter(filter, u.get_location());
	}

	static std::set<std::string> unit_wml_ids;

	manager::manager(const config& cfg)
		: variable_manager()
	{
		assert(!manager_running);
		BOOST_FOREACH(const config &ev, cfg.child_range("event")) {
			event_handlers.add_event_handler(game_events::event_handler(ev));
		}
		BOOST_FOREACH(const std::string &id, utils::split(cfg["unit_wml_ids"])) {
			unit_wml_ids.insert(id);
		}

		resources::lua_kernel = new LuaKernel(cfg);
		manager_running = true;

		BOOST_FOREACH(static_wml_action_map::value_type &action, static_wml_actions) {
			resources::lua_kernel->set_wml_action(action.first, action.second);
		}

		const std::string used = cfg["used_items"];
		if(!used.empty()) {
			const std::vector<std::string>& v = utils::split(used);
			for(std::vector<std::string>::const_iterator i = v.begin(); i != v.end(); ++i) {
				used_items.insert(*i);
			}
		}
		int wmi_count = 0;
		typedef std::pair<std::string, wml_menu_item *> item;
		BOOST_FOREACH(const item &itor, resources::state_of_game->wml_menu_items) {
			if (!itor.second->command.empty()) {
				event_handlers.add_event_handler(game_events::event_handler(itor.second->command, true));
			}
			++wmi_count;
		}
		if(wmi_count > 0) {
			LOG_NG << wmi_count << " WML menu items found, loaded." << std::endl;
		}
	}

	void write_events(config& cfg)
	{
		assert(manager_running);
		BOOST_FOREACH(const game_events::event_handler &eh, event_handlers) {
			if (eh.disabled() || eh.is_menu_item()) continue;
			cfg.add_child("event", eh.get_config());
		}

		std::stringstream used;
		std::set<std::string>::const_iterator u;
		for(u = used_items.begin(); u != used_items.end(); ++u) {
			if(u != used_items.begin())
				used << ",";

			used << *u;
		}

		cfg["used_items"] = used.str();
		std::stringstream ids;
		for(u = unit_wml_ids.begin(); u != unit_wml_ids.end(); ++u) {
			if(u != unit_wml_ids.begin())
				ids << ",";

			ids << *u;
		}

		cfg["unit_wml_ids"] = ids.str();

		if (resources::soundsources)
			resources::soundsources->write_sourcespecs(cfg);

		resources::lua_kernel->save_game(cfg);
	}

	manager::~manager() {
		assert(manager_running);
		manager_running = false;
		events_queue.clear();
		event_handlers.clear();
		reports::reset_generators();
		delete resources::lua_kernel;
		resources::lua_kernel = NULL;
		unit_wml_ids.clear();
		used_items.clear();
	}

	void raise(const std::string& event,
			const entity_location& loc1,
			const entity_location& loc2,
			const config& data)
	{
		assert(manager_running);
		if(!events_init())
			return;

		DBG_EH << "raising event: " << event << "\n";

		events_queue.push_back(game_events::queued_event(event,loc1,loc2,data));
	}

	bool fire(const std::string& event,
			const entity_location& loc1,
			const entity_location& loc2,
			const config& data)
	{
		assert(manager_running);
		raise(event,loc1,loc2,data);
		return pump();
	}

	void add_events(const config::const_child_itors &cfgs, const std::string& type)
	{
		if(!type.empty()) {
			if(std::find(unit_wml_ids.begin(),unit_wml_ids.end(),type) != unit_wml_ids.end()) return;
			unit_wml_ids.insert(type);
		}
		BOOST_FOREACH(const config &new_ev, cfgs) {
			if(type.empty() && new_ev["id"].empty())
			{
				WRN_NG << "attempt to add an [event] with empty id=, ignoring \n";
				continue;
			}
			event_handlers.add_event_handler(game_events::event_handler(new_ev));
		}
	}

	void commit()
	{
		DBG_EH << "committing new event handlers, number of pump_instances: " <<
			pump_manager::count() << "\n";
		commit_wmi_commands();
		event_handlers.commit_buffer();
		// Dialogs can only be shown if the display is not locked
		if (!resources::screen->video().update_locked()) {
			show_wml_errors();
			show_wml_messages();
		}
	}

	bool pump()
	{
		//ensure the whiteboard doesn't attempt to build its future unit map
		//for the duration of this method
		wb::real_map real_unit_map;

		assert(manager_running);
		if(!events_init())
			return false;

		pump_manager pump_instance;
		if(pump_manager::count() >= game_config::max_loop) {
			ERR_NG << "game_events::pump() waiting to process new events because "
				<< "recursion level would exceed maximum " << game_config::max_loop << '\n';
			return false;
		}

		if(!lg::debug.dont_log("event_handler")) {
			std::stringstream ss;
			BOOST_FOREACH(const game_events::queued_event& ev, events_queue) {
				ss << "name=" << ev.name << "; ";
			}
			DBG_EH << "processing queued events: " << ss.str() << "\n";
		}

		//Notify the whiteboard of any event; this is used to track when moves, recruits, etc. happen
		if(!events_queue.empty()) {
			resources::whiteboard->on_gamestate_change();
		}

		bool result = false;
		while(events_queue.empty() == false) {
			if(pump_manager::count() <= 1)
				event_handlers.start_buffering();
			game_events::queued_event ev = events_queue.front();
			events_queue.pop_front();	// pop now for exception safety
			const std::string& event_name = ev.name;

			// Clear the unit cache, since the best clearing time is hard to figure out
			// due to status changes by WML. Every event will flush the cache.
			unit::clear_status_caches();

			/* bool lua_event_triggered = */ resources::lua_kernel->run_event(ev);

			bool init_event_vars = true;

			BOOST_FOREACH(game_events::event_handler& handler, event_handlers) {
				if(!handler.matches_name(event_name))
					continue;
				// Set the variables for the event
				if (init_event_vars) {
					resources::state_of_game->get_variable("x1") = ev.loc1.x + 1;
					resources::state_of_game->get_variable("y1") = ev.loc1.y + 1;
					resources::state_of_game->get_variable("x2") = ev.loc2.x + 1;
					resources::state_of_game->get_variable("y2") = ev.loc2.y + 1;
					init_event_vars = false;
				}

				DBG_EH << "processing event " << event_name << " with id="<<
					handler.get_config()["id"] << "\n";
				if(process_event(handler, ev))
				{
					result = true;
				}
			}

			if(pump_manager::count() <= 1)
				event_handlers.stop_buffering();
			// Only commit new handlers when finished iterating over event_handlers.
			commit();
		}

		return result;
	}

	entity_location::entity_location(const map_location &loc, size_t id)
		: map_location(loc), id_(id)
	{}

	entity_location::entity_location(const unit &u)
		: map_location(u.get_location()), id_(u.underlying_id())
	{}

	bool entity_location::matches_unit(const unit& u) const
	{
		return id_ == u.underlying_id();
	}

	bool entity_location::requires_unit() const
	{
		return id_ > 0;
	}

} // end namespace game_events (2)
