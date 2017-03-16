/*
   Copyright (C) 2014 - 2016 by Chris Beck <render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/lua_interpreter.hpp"

#include "gui/auxiliary/field.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/core/window_builder.hpp"
#include "gui/dialogs/helper.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/scroll_label.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/window.hpp"

#include "desktop/clipboard.hpp"
#include "game_config.hpp"
#include "game_errors.hpp"
#include "gettext.hpp"
#include "resources.hpp" //for help fetching lua kernel pointers
#include "scripting/plugins/manager.hpp" //needed for the WHICH_KERNEL version of display
#include "scripting/game_lua_kernel.hpp"	//needed for the WHICH_KERNEL version of display
#include "scripting/lua_kernel_base.hpp"
#include "serialization/string_utils.hpp"
#include "serialization/unicode.hpp"
#include "lexical_cast.hpp"
#include "log.hpp"
#include "font/pango/escape.hpp"

#include <sstream>
#include <string>
#include <vector>
#include "utils/functional.hpp"

#ifdef HAVE_HISTORY
#include "filesystem.hpp"
#include <readline/history.h>
#endif

static lg::log_domain log_lua_int("lua/interpreter");
#define DBG_LUA LOG_STREAM(debug, log_lua_int)
#define LOG_LUA LOG_STREAM(info, log_lua_int)
#define WRN_LUA LOG_STREAM(warn, log_lua_int)
#define ERR_LUA LOG_STREAM(err, log_lua_int)

class CVideo;

namespace gui2
{
namespace dialogs
{

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 3_lua_interpretter
 *
 * == Settings manager ==
 *
 * This shows the settings manager
 *
 */


REGISTER_DIALOG(lua_interpreter)

// Model, View, Controller definitions

class lua_interpreter::view {
private:
	scroll_label* msg_label; //the view is extremely simple, it's pretty much just this one widget that gets updated

public:
	view() : msg_label(nullptr) {}

	/** Bind the scroll label widget to my pointer, and configure */
	void bind(window& window) {
		msg_label = &find_widget<scroll_label>(&window, "msg", false);
		msg_label->set_use_markup(true);
		msg_label->set_vertical_scrollbar_mode(scrollbar_container::ALWAYS_VISIBLE);
		msg_label->set_label("");
	}

	/** Update the scroll label contents */
	void update_contents(const std::string & str)
	{
		assert(msg_label);

		msg_label->set_label(str);
		msg_label->scroll_vertical_scrollbar(scrollbar_base::END);
	}

	void pg_up()
	{
		assert(msg_label);
		msg_label->scroll_vertical_scrollbar(scrollbar_base::JUMP_BACKWARDS);
	}

	void pg_down()
	{
		assert(msg_label);
		msg_label->scroll_vertical_scrollbar(scrollbar_base::JUMP_FORWARD);
	}
};

/**
 * The lua model is responsible to interact with the lua kernel base and keep track of what should be displayed in the console.
 * It registers its stringstream with the lua kernel when it is created, and unregisters when it is destroyed.
 *
 * It is responsible to execute commands as strings, or add dialog messages for the user. It is also responsible to ask
 * the lua kernel for help with tab completion.
 */
class lua_interpreter::lua_model {
private:
	lua_kernel_base & L_;
	std::stringstream log_;

	// Lua kernel sends log strings to this function
	// This is arranged by set_external_log call
	void log_function(const std::string & str) {
		log_ << font::escape_text(str);
	}

public:
	lua_model (lua_kernel_base & lk)
		: L_(lk)
		, log_()
	{
		DBG_LUA << "constructing a lua_interpreter::model\n";
		//DBG_LUA << "incoming:\n" << lk.get_log().rdbuf() << "\n.\n";
		log_ << lk.get_log().str() << std::flush;
		L_.set_external_log([this](const std::string & str) { this->log_function(str); }); //register our log to get commands and output from the lua interpreter
		//DBG_LUA << "recieved:\n" << log_.str() << "\n.\n";

		DBG_LUA << "finished constructing a lua_interpreter::model\n";
	}

	~lua_model()
	{
		DBG_LUA << "destroying a lua_interpreter::model\n";
		L_.set_external_log(nullptr); //deregister our log since it's about to be destroyed
	}

	/** Ask the lua kernel to execute a command. No throw of game::lua_error, instead the error message is formatted and printed to console.*/
	bool execute(const std::string & cmd);

	/** Add a message from the dialog, formatted in blue to distinguish from issued commands.
	 * This message gets put in the interpreter log, but does not get entered in the kernel log, so if the window is closed this message will
	 * not appear the next time it is opened.
	 **/
	void add_dialog_message(const std::string & msg);

	std::string get_log() const { return log_.str(); } ///< Get the log string
	std::string get_name() const { return L_.my_name(); } ///< Get a string describing the name of lua kernel

	/// Clear the console log
	void clear_log() {
		L_.clear_log();
		log_.str("");
		log_.clear();
	}

	//* Tab completion: Get list of presently defined global variables */
	std::vector<std::string> get_globals() { return L_.get_global_var_names(); }
	//* Tab completion: Get list of attributes for variable corresponding to this path. */
	std::vector<std::string> get_attribute_names(const std::string & s) { return L_.get_attribute_names(s); }
};

/**
 * The input_model keeps track of what commands were executed before, and figures out what
 * should be displayed when the user presses up / down arrows in the input.
 * It is essentially part of the model, but it isn't connected to the lua kernel so I have implemented it
 * separately. Putatively it could all be refactored so that there is a single model with private subclass "lua_model"
 * and also a "command_history_model" but I have decided simply to not implement it that way.
 */
class lua_interpreter::input_model {
private:
	std::string prefix_;
	bool end_of_history_;
#ifdef HAVE_HISTORY
	std::string filename_;
#endif

public:
	input_model()
	: prefix_()
	, end_of_history_(true)
#ifdef HAVE_HISTORY
	, filename_(filesystem::get_user_config_dir() + "/lua_command_history")
	{
		using_history();
		read_history (filename_.c_str());
	}
#else
	{}
#endif

#ifdef HAVE_HISTORY
	~input_model()
	{
		try {
			const size_t history_max = 500;
			if (filesystem::file_exists(filename_)) {
				append_history (history_max,filename_.c_str());
			} else {
				write_history (filename_.c_str());
			}

			history_truncate_file (filename_.c_str(), history_max);
		} catch (...) { std::cerr << "Swallowed an exception when trying to write lua command line history\n";}
	}
#endif
	void add_to_history (std::string str) {
		prefix_ = "";
		(void) str;
#ifdef HAVE_HISTORY
		add_history(str.c_str());
#endif
		end_of_history_ = true;

	}

	void maybe_update_prefix (const std::string & text) {
		LOG_LUA << "maybe update prefix\n";
		LOG_LUA << "prefix_: '"<< prefix_ << "'\t text='"<< text << "'\n";

		if (!end_of_history_) return;

		prefix_ = text;
		LOG_LUA << "updated prefix\n";
	}

	std::string search( int direction ) {
#ifdef HAVE_HISTORY
		LOG_LUA << "searching in direction " << direction << " from position " << where_history() << "\n";

		HIST_ENTRY * e = nullptr;
		if (end_of_history_) {
			// if the direction is > 0, do nothing because searching down only takes place when we are in the history records.
			if (direction < 0) {
				history_set_pos(history_length);

				if (prefix_.size() > 0) {
					int result = history_search_prefix(prefix_.c_str(), direction);
					if (result == 0) {
						e = current_history();
					}
				} else {
					e = previous_history();
				}
			}
		} else {
			e = (direction > 0) ? next_history() : previous_history();
			if (prefix_.size() > 0 && e) {
				int result = history_search_prefix(prefix_.c_str(), direction);
				if (result == 0) {
					e = current_history();
				} else {
					e = nullptr;		// if the search misses, it leaves the state as it was, which might not have been on an entry matching prefix.
					end_of_history_ = true;	// we actually want to force it to be null and treat as off the end of history in this case.
				}
			}
		}

		if (e) {
			LOG_LUA << "found something at " << where_history() << "\n";
			std::string ret = e->line;
			end_of_history_ = false;
			return ret;
		}
#endif

		LOG_LUA << "didn't find anything\n";

		// reset, set history to the end and prefix_ to empty, and return the current prefix_ for the user to edit
		end_of_history_ = true;
		(void) direction;
		std::string temp = prefix_;
		prefix_ = "";
		return temp;
	}

	std::string clear_history() {
#ifdef HAVE_HISTORY
		::clear_history();
		write_history (filename_.c_str());
		return "Cleared history.";
#else
		return "History is disabled, you did not compile with GNU history support.";
#endif
	}

	std::string list_history() {
#ifdef HAVE_HISTORY
		HIST_ENTRY **the_list;

		the_list = history_list ();
		if (the_list) {
			if (!*the_list) {
				return "History is empty.";
			}

			std::string result;
			for (int i = 0; the_list[i]; i++) {
				result += lexical_cast<std::string, int>(i+history_base);
				result += ": ";
				result += the_list[i]->line;
				result += "\n";
			}
			return result;
		} else {
			return "Couldn't find history.";
		}
#else
		return "History is disabled, you did not compile with GNU history support.";
#endif
	}

	// Does history expansion in a command line. A return value of true indicates an error,
	// the error message will be returned in the string argument. A return value of false
	// indicates success and that execution should proceed.
	bool do_history_expansion (std::string & cmd) {
#ifdef HAVE_HISTORY
		// Do history expansions
		std::unique_ptr<char[]> cmd_cstr(new char[cmd.length()+1]);
		strcpy (cmd_cstr.get(), cmd.c_str());

		char * expansion;

		int result = history_expand(cmd_cstr.get(), &expansion);

		if (result < 0 || result == 2) {
			cmd = expansion; // return error message in cmd var
			free(expansion);
			return true;
		}

		cmd = expansion;
		free(expansion);
#endif
		(void) cmd;
		return false;
	}
};

/**
 * The controller is responsible to hold all the input widgets, and a pointer to the model and view.
 * It is responsible to bind the input signals to appropriate handler methods, which it holds.
 * It is also responsible to ask the view to update based on the output of the model, typically in
 * response to some input.
 */
class lua_interpreter::controller {
private:
	button* copy_button;
	button* clear_button;

	text_box* text_entry;
	std::string text_entry_;

	const std::unique_ptr<lua_interpreter::lua_model> lua_model_;
	const std::unique_ptr<lua_interpreter::input_model> input_model_;
	const std::unique_ptr<lua_interpreter::view> view_;

	void execute();
	void tab();
	void search(int direction);
public:
	controller(lua_kernel_base & lk)
		: copy_button(nullptr)
		, clear_button(nullptr)
		, text_entry(nullptr)
		, text_entry_()
		, lua_model_(new lua_interpreter::lua_model(lk))
		, input_model_(new lua_interpreter::input_model())
		, view_(new lua_interpreter::view())
	{}

	/** Bind my pointers to the widgets found in the window */
	void bind(window& window);

	void handle_copy_button_clicked(window & window);
	void handle_clear_button_clicked(window & window);

	void input_keypress_callback(bool& handled,
						   bool& halt,
						   const SDL_Keycode key,
						   window& window);

	void update_view(); ///< Update the view based on the model

	friend class lua_interpreter;
};

// Model impl

/** Execute a command, and report any errors encountered. */
bool lua_interpreter::lua_model::execute (const std::string & cmd)
{
	LOG_LUA << "lua_interpreter::model::execute...\n";

	try {
		L_.interactive_run(cmd.c_str());
		return true;
	} catch (game::lua_error & e) {
		add_dialog_message(std::string(e.what()));
		return false;
	}
}

/** Add a dialog message, which will appear in blue. */
void lua_interpreter::lua_model::add_dialog_message(const std::string & msg) {
	log_ << "<span color='#8888FF'>" << font::escape_text(msg) << "</span>\n";
}

// View impl

// Controller impl

/** Update the view based on the model. */
void lua_interpreter::controller::update_view()
{
	LOG_LUA << "lua_interpreter update_view...\n";
	assert(lua_model_);
	assert(view_);

	view_->update_contents(lua_model_->get_log());

	LOG_LUA << "lua_interpreter update_view finished\n";
}

/** Find all the widgets managed by the controller and connect them to handler methods. */
void lua_interpreter::controller::bind(window& window)
{
	LOG_LUA << "Entering lua_interpreter::controller::bind" << std::endl;
	assert(view_);
	view_->bind(window);

	text_entry = &find_widget<text_box>(&window, "text_entry", false);
	//text_entry->set_text_changed_callback(
	//		std::bind(&view::filter, this, std::ref(window)));
	window.keyboard_capture(text_entry);
	window.set_click_dismiss(false);
	window.set_enter_disabled(true);

	connect_signal_pre_key_press(
			*text_entry,
			std::bind(&lua_interpreter::controller::input_keypress_callback,
						this,
						_3,
						_4,
						_5,
						std::ref(window)));

	copy_button = &find_widget<button>(&window, "copy", false);
	connect_signal_mouse_left_click(
			*copy_button,
			std::bind(&lua_interpreter::controller::handle_copy_button_clicked,
						this,
						std::ref(window)));

	clear_button = &find_widget<button>(&window, "clear", false);
	connect_signal_mouse_left_click(
			*clear_button,
			std::bind(&lua_interpreter::controller::handle_clear_button_clicked,
						this,
						std::ref(window)));

	if (!desktop::clipboard::available()) {
		copy_button->set_active(false);
		copy_button->set_tooltip(_("Clipboard support not found, contact your packager"));
	}

	LOG_LUA << "Exiting lua_interpreter::controller::bind" << std::endl;
}

/** Copy text to the clipboard */
void lua_interpreter::controller::handle_copy_button_clicked(window & /*window*/)
{
	assert(lua_model_);
	desktop::clipboard::copy_to_clipboard(lua_model_->get_log(), false);
}

/** Clear the text */
void lua_interpreter::controller::handle_clear_button_clicked(window & /*window*/)
{
	assert(lua_model_);
	lua_model_->clear_log();
	assert(view_);
	view_->update_contents("");
}

/** Handle return key (execute) or tab key (tab completion) */
void lua_interpreter::controller::input_keypress_callback(bool& handled,
							   bool& halt,
							   const SDL_Keycode key,
							   window& /*window*/)
{
	assert(lua_model_);
	assert(text_entry);

	LOG_LUA << "keypress_callback\n";
	if(key == SDLK_RETURN || key == SDLK_KP_ENTER) { // handle executing whatever is in the command entry field
		LOG_LUA << "executing...\n";
		execute();
		handled = true;
		halt = true;
		LOG_LUA << "finished executing\n";
	} else if(key == SDLK_TAB) {	// handle tab completion
		tab();
		handled = true;
		halt = true;
	} else if(key == SDLK_UP) {
		search(-1);
		handled = true;
		halt = true;
	} else if(key == SDLK_DOWN) {
		search(1);
		handled = true;
		halt = true;
	} else if(key == SDLK_PAGEUP) {
		view_->pg_up();
		handled = true;
		halt = true;
	} else if(key == SDLK_PAGEDOWN) {
		view_->pg_down();
		handled = true;
		halt = true;
	}

}

void lua_interpreter::controller::execute()
{
	std::string cmd = text_entry->get_value();
	if (cmd.size() == 0) return; //don't bother with empty string

	cmd.erase(cmd.find_last_not_of(" \n\r\t")+1); //right trim the string

	LOG_LUA << "Executing '"<< cmd << "'\n";

	if (cmd.size() >= 13 && (cmd.substr(0,13) == "history clear" || cmd.substr(0,13) == "clear history")) {
		lua_model_->add_dialog_message(input_model_->clear_history());
		text_entry->set_value("");
		update_view();
		return;
	}

	if (cmd.size() >= 7 && (cmd.substr(0,7) == "history")) {
		lua_model_->add_dialog_message(input_model_->list_history());
		text_entry->set_value("");
		update_view();
		return;
	}

	if (input_model_->do_history_expansion(cmd)) {
		lua_model_->add_dialog_message(cmd);
		update_view();
		return;
	}

	if (lua_model_->execute(cmd)) {
		input_model_->add_to_history(cmd);
		text_entry->set_value("");
	}
	update_view();
}

void lua_interpreter::controller::tab()
{
	std::string text = text_entry->get_value();

	std::string prefix;
	size_t prefix_end_pos = text.find_last_of(" (");
	if (prefix_end_pos != std::string::npos) {
		prefix = text.substr(0, prefix_end_pos + 1);
		text = text.substr(prefix_end_pos + 1);
	}

	std::vector<std::string> matches;

	if (text.find('.') == std::string::npos) {
		matches = lua_model_->get_globals();
		matches.push_back("and");
		matches.push_back("break");
		matches.push_back("else");
		matches.push_back("elseif");
		matches.push_back("end");
		matches.push_back("false");
		matches.push_back("for");
		matches.push_back("function");
		matches.push_back("local");
		matches.push_back("nil");
		matches.push_back("not");
		matches.push_back("repeat");
		matches.push_back("return");
		matches.push_back("then");
		matches.push_back("true");
		matches.push_back("until");
		matches.push_back("while");
	} else {
		matches = lua_model_->get_attribute_names(text);
	}

	//bool line_start = utils::word_completion(text, matches);
	if (text.size() > 0) { // this if is to avoid wierd behavior in word_completion, where it thinks nothing matches the empty string
		utils::word_completion(text, matches);
	}

	if(matches.empty()) {
		return;
	}

	//if(matches.size() == 1) {
		//text.append(" "); //line_start ? ": " : " ");
	//} else {
	if (matches.size() > 1) {
		//std::string completion_list = utils::join(matches, " ");

		const size_t wrap_limit = 80;
		std::string buffer;

		for (size_t idx = 0; idx < matches.size(); ++idx) {
			if (buffer.size() + 1 + matches.at(idx).size() > wrap_limit) {
				lua_model_->add_dialog_message(buffer);
				buffer = matches.at(idx);
			} else {
				if (buffer.size()) {
					buffer += (" " + matches.at(idx));
				} else {
					buffer = matches.at(idx);
				}
			}
		}

		lua_model_->add_dialog_message(buffer);
		update_view();
	}
	text_entry->set_value(prefix + text);
}

void lua_interpreter::controller::search(int direction)
{
	std::string current_text = text_entry->get_value();
	input_model_->maybe_update_prefix(current_text);
	text_entry->set_value(input_model_->search(direction));

#ifndef HAVE_HISTORY
	lua_model_->add_dialog_message("History is disabled, you did not compile with GNU history support.");
	update_view();
#endif

}

// Dialog implementation

/** Display a new console, using given video and lua kernel */
void lua_interpreter::display(CVideo& video, lua_kernel_base * lk) {
#ifndef ALWAYS_HAVE_LUA_CONSOLE
	if(!game_config::debug) {
		return;
	}
#endif
	if (!lk) {
		ERR_LUA << "Tried to open console with a null lua kernel pointer.\n";
		return;
	}

	lua_interpreter(*lk).show(video);
}

/** Helper function to assist those callers which don't want to include resources.hpp */
void lua_interpreter::display(CVideo& video, lua_interpreter::WHICH_KERNEL which) {
	if (which == lua_interpreter::APP) {
		display(video, plugins_manager::get()->get_kernel_base());
	} else if (which == lua_interpreter::GAME) {
		display(video, resources::lua_kernel);
	}
}

/** Call inherited method */
window* lua_interpreter::build_window(CVideo& video)
{
	return build(video, window_id());
}

/** Bind the controller, initialize one of the static labels with info about this kernel, and update the view. */
void lua_interpreter::pre_show(window& window)
{
	LOG_LUA << "Entering lua_interpreter::view::pre_show" << std::endl;
	register_text("text_entry", false, controller_->text_entry_, true);
	controller_->bind(window);

	label *kernel_type_label = &find_widget<label>(&window, "kernel_type", false);
	kernel_type_label->set_label(controller_->lua_model_->get_name());

	controller_->update_view();
	//window.invalidate_layout(); // workaround for assertion failure
	LOG_LUA << "Exiting lua_interpreter::view::pre_show" << std::endl;
}

lua_interpreter::lua_interpreter(lua_kernel_base & lk)
		: controller_(new lua_interpreter::controller(lk))
{
	LOG_LUA << "entering lua_interpreter ctor...\n";
	LOG_LUA << "finished lua_interpreter ctor...\n";
}

} // namespace dialogs
} // namespace gui2
