/*
   Copyright (C) 2011 - 2014 by Yurii Chernyi <terraninfo@terraninfo.net>
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

#include "gui/auxiliary/find_widget.tpp"
#include "gui/dialogs/field.hpp"
#include "gui/dialogs/helper.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/window.hpp"

#include "desktop/clipboard.hpp"
#include "game_errors.hpp"
#include "gettext.hpp"
#include "resources.hpp" //for help fetching lua kernel pointers
#include "scripting/application_lua_kernel.hpp" //needed for the WHICH_KERNEL version of display
#include "scripting/game_lua_kernel.hpp"	//needed for the WHICH_KERNEL version of display
#include "scripting/lua_kernel_base.hpp"
#include "serialization/string_utils.hpp"
#include "serialization/unicode.hpp"
#include "log.hpp"
#include "text.hpp"

#include <ostream>
#include <sstream>
#include <string>
#include <vector>
#include <boost/assign/list_of.hpp>
#include <boost/bind.hpp>
#include <boost/scoped_ptr.hpp>

static lg::log_domain log_lua_int("lua/interpreter");
#define DBG_LUA LOG_STREAM(debug, log_lua_int)
#define LOG_LUA LOG_STREAM(info, log_lua_int)
#define WRN_LUA LOG_STREAM(warn, log_lua_int)
#define ERR_LUA LOG_STREAM(err, log_lua_int)

namespace gui2
{

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 3_chat_log
 *
 * == Settings manager ==
 *
 * This shows the settings manager
 *
 */


REGISTER_DIALOG(lua_interpreter)

void tlua_interpreter::display(CVideo& video, lua_kernel_base * lk) {
	if (!lk) {
		ERR_LUA << "Tried to open console with a null lua kernel pointer.\n";
		return;
	}

	tlua_interpreter(*lk).show(video);
}

void tlua_interpreter::display(CVideo& video, tlua_interpreter::WHICH_KERNEL which) {
	if (which == tlua_interpreter::APP) {
		display(video, resources::app_lua_kernel);
	} else if (which == tlua_interpreter::GAME) {
		display(video, resources::lua_kernel);
	}
}

twindow* tlua_interpreter::build_window(CVideo& video)
{
	return build(video, window_id());
}


void tlua_interpreter::pre_show(CVideo& /*video*/, twindow& window)
{
	LOG_LUA << "Entering tlua_interpreter::view::pre_show" << std::endl;
	bind(window);
	update_contents();
	//window.invalidate_layout(); // workaround for assertion failure
	LOG_LUA << "Exiting tlua_interpreter::view::pre_show" << std::endl;
}

void tlua_interpreter::bind(twindow& window)
{
	LOG_LUA << "Entering tlua_interpreter::bind" << std::endl;
	msg_label = &find_widget<tscroll_label>(&window, "msg", false);
	msg_label->set_use_markup(true);
	msg_label->set_vertical_scrollbar_mode(tscrollbar_container::always_visible);
	msg_label->set_label("");

	register_text("text_entry", false, text_entry_, true);
	text_entry = &find_widget<ttext_box>(&window, "text_entry", false);
	//text_entry->set_text_changed_callback(
	//		boost::bind(&view::filter, this, boost::ref(window)));
	window.keyboard_capture(text_entry);
	window.set_click_dismiss(false);
	window.set_enter_disabled(true);

	connect_signal_pre_key_press(
			*text_entry,
			boost::bind(&tlua_interpreter::input_keypress_callback,
						this,
						_3,
						_4,
						_5,
						boost::ref(window)));


	copy_button = &find_widget<tbutton>(&window, "copy", false);
	connect_signal_mouse_left_click(
			*copy_button,
			boost::bind(&tlua_interpreter::handle_copy_button_clicked,
						this,
						boost::ref(window)));

	if (!desktop::clipboard::available()) {
		copy_button->set_active(false);
		copy_button->set_tooltip(_("Clipboard support not found, contact your packager."));
	}

	LOG_LUA << "Exiting tlua_interpreter::bind" << std::endl;
}


class tlua_interpreter::model {
private:
	lua_kernel_base & L_;
	std::stringstream log_;

public:
	model (lua_kernel_base & lk)
		: L_(lk)
		, log_()
	{
		DBG_LUA << "constructing a tlua_interpreter::model\n";
		//DBG_LUA << "incoming:\n" << lk.get_log().rdbuf() << "\n.\n";
		log_ << lk.get_log().str() << std::flush;
		L_.set_external_log(&log_); //register our log to get commands and output from the lua interpreter
		//DBG_LUA << "recieved:\n" << log_.str() << "\n.\n";
		
		DBG_LUA << "finished constructing a tlua_interpreter::model\n";

	}

	~model()
	{
		DBG_LUA << "destroying a tlua_interpreter::model\n";
		L_.set_external_log(NULL); //deregister our log since it's about to be destroyed
	}

	bool execute(const std::string & cmd); // no throw of lua_error. should handle formatting any syntax errors to the interpreter log.

	void add_dialog_message(const std::string & msg); //formats a message from the dialog, puts in interpreter log but not lua's log

	std::string get_log() const { return log_.str(); }

	std::vector<std::string> get_globals() { return L_.get_global_var_names(); }
	std::vector<std::string> get_attribute_names(const std::string & s) { return L_.get_attribute_names(s); }
};

bool tlua_interpreter::model::execute (const std::string & cmd)
{
	LOG_LUA << "tlua_interpreter::model::execute...\n";
	try {
		L_.throwing_run(cmd.c_str());
		return true;
	} catch (game::lua_error & e) {
		add_dialog_message(std::string(e.what()));
		return false;
	}
}

void tlua_interpreter::model::add_dialog_message(const std::string & msg) {
	log_ << "<span color='#8888FF'>" << font::escape_text(msg) << "</span>\n";
}

tlua_interpreter::tlua_interpreter(lua_kernel_base & lk)
		: model_(new tlua_interpreter::model(lk))
		, msg_label(NULL)
		, copy_button(NULL)
		, text_entry(NULL)
{
	LOG_LUA << "entering tlua_interpreter ctor...\n";
	LOG_LUA << "finished tlua_interpreter ctor...\n";
}

void tlua_interpreter::update_contents()
{
	LOG_LUA << "tlua_interpreter update_contents...\n";
	if (msg_label) {
		msg_label->set_label(model_->get_log());
		msg_label->scroll_vertical_scrollbar(tscrollbar_::END);
	}
	LOG_LUA << "tlua_interpreter update_contents finished\n";
}

void tlua_interpreter::handle_copy_button_clicked(twindow & /*window*/)
{
	desktop::clipboard::copy_to_clipboard(model_->get_log(), false);
}

void tlua_interpreter::input_keypress_callback(bool& handled,
							   bool& halt,
							   const SDLKey key,
							   twindow& /*window*/)
{
	LOG_LUA << "keypress_callback\n";
	if(key == SDLK_RETURN || key == SDLK_KP_ENTER) {
		LOG_LUA << "executing...\n";
		if (model_->execute(text_entry->get_value())) {
			text_entry->save_to_history();
		}
		update_contents();
		text_entry->set_value("");
		handled = true;
		halt = true;
		LOG_LUA << "finished executing\n";
	} else if(key == SDLK_TAB) {
		std::string text = text_entry->get_value();

		std::string prefix;
		size_t idx = text.find_last_of(" (");
		if (idx != std::string::npos) {
			prefix = text.substr(0, idx+1);
			text = text.substr(idx+1);
		}

		std::vector<std::string> matches;

		if (text.find('.') == std::string::npos) {
			matches = model_->get_globals();
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
			matches = model_->get_attribute_names(text);
		}

		//bool line_start = utils::word_completion(text, matches);
		if (text.size() > 0) { // this if is to avoid wierd behavior in word_completion, where it thinks nothing matches the empty string
			utils::word_completion(text, matches);
		}

		if(matches.empty()) {
			handled = true; //there's no point in putting tabs in the command line
			halt = true;
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
					model_->add_dialog_message(buffer);
					buffer = matches.at(idx);
				} else {
					if (buffer.size()) {
						buffer += (" " + matches.at(idx));
					} else {
						buffer = matches.at(idx);
					}
				}
			}

			model_->add_dialog_message(buffer);
			update_contents();
		}
		text_entry->set_value(prefix + text);
		handled = true;
		halt = true;
	}
}

} // end of namespace gui2
