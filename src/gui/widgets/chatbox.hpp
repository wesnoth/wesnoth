/*
   Copyright (C) 2016 by the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "gui/widgets/container.hpp"
#include "gui/dialogs/lobby/data.hpp"
#include "gui/dialogs/lobby/info.hpp"

#include "chat_events.hpp"

#include <string>


class config;

namespace gui2
{

// ------------ WIDGET -----------{

class tbutton;
class tlistbox;
class tlabel;
class tmulti_page;
class ttext_box;

namespace implementation
{
	struct tbuilder_chatbox;
}


struct tlobby_chat_window
{
	tlobby_chat_window(const std::string& name, bool whisper)
		: name(name), whisper(whisper), pending_messages(0)
	{
	}
	std::string name;
	bool whisper;
	int pending_messages;
};

class tchatbox : public tcontainer_, public events::chat_handler
{
	friend struct implementation::tbuilder_chatbox;

public:
	tchatbox();

	/** See @ref tcontrol::set_active. */
	virtual void set_active(const bool active) override;

	/** See @ref tcontrol::get_active. */
	virtual bool get_active() const override;

	/** See @ref tcontrol::get_state. */
	virtual unsigned get_state() const override { return 0; };

	void send_to_server(const ::config& cfg) override;

	void set_active_window_changed_callback(const std::function<void(void)>& f) { active_window_changed_callback_ = f; }

	void set_lobby_info(lobby_info& i) { lobby_info_ = &i; }

protected:
	/**
	 * Initializes the interneral sub-widget pointers.
	 * Should be called when building the window, so the pointers
	 * are initilized when set_displayed_type() is called.
	 */
	void finalize_setup();

	virtual void user_relation_changed(const std::string& name) override;

	/** inherited form chat_handler */
	virtual void add_chat_message(const time_t& time,
		const std::string& speaker,
		int side,
		const std::string& message,
		events::chat_handler::MESSAGE_TYPE type
		= events::chat_handler::MESSAGE_PRIVATE) override;

	/** inherited form chat_handler */
	virtual void add_whisper_sent(const std::string& receiver,
		const std::string& message) override;

	/** inherited form chat_handler */
	virtual void add_whisper_received(const std::string& sender,
		const std::string& message) override;

	/** inherited form chat_handler */
	virtual void add_chat_room_message_sent(const std::string& room,
		const std::string& message) override;

	/** inherited form chat_handler */
	virtual void add_chat_room_message_received(const std::string& room,
		const std::string& speaker,
		const std::string& message) override;


private:
	tlistbox* roomlistbox_;

	tmulti_page* chat_log_container_;

	ttext_box* chat_input_;

	std::vector<tlobby_chat_window> open_windows_;

	size_t active_window_;

	std::function<void(void)> active_window_changed_callback_;

	class lobby_info* lobby_info_;

	class lobby_info& lobby_info() { return *lobby_info_; }

	/** See @ref tcontrol::get_control_type. */
	virtual const std::string& get_control_type() const override;

	/** See @ref tcontainer_::set_self_active. */
	virtual void set_self_active(const bool active) override;
	
	void chat_input_keypress_callback(bool& handled, bool& halt, const SDLKey key);

	void append_to_chatbox(const std::string& text, const bool force_scroll = false);

	void append_to_chatbox(const std::string& text, size_t id, const bool force_scroll = false);
	/**
	* @return true if the whisper window for "name" is the active window
	*/
	bool whisper_window_active(const std::string& name);

	/**
	* @return true if the room window for "room" is the active window
	*/
	bool room_window_active(const std::string& room);

	/**
	* Mark the whisper window for "name" as having one more pending message
	*/
	void increment_waiting_whsipers(const std::string& name);

	/**
	* Mark the room window for "room" as having one more pending message
	*/
	void increment_waiting_messages(const std::string& room);

	/**
	* Add a whisper message to the whisper window
	*/
	void add_whisper_window_whisper(const std::string& sender,
		const std::string& message);

	/**
	* Add a whisper message to the current window which is not the whisper
	* window
	* for "name".
	*/
	void add_active_window_whisper(const std::string& sender,
		const std::string& message,
		const bool force_scroll = false);

	/**
	* Add a message to the window for room "room"
	*/
	void add_room_window_message(const std::string& room,
		const std::string& sender,
		const std::string& message);

	/**
	* Add a message to the window for room "room"
	*/
	void add_active_window_message(const std::string& sender,
		const std::string& message,
		const bool force_scroll = false);

	void close_window(size_t idx);

	void send_message_button_callback();

public:

	/** inherited form chat_handler */
	virtual void send_chat_message(const std::string& message,
		bool /*allies_only*/) override;

	/**
	* Switch to the window given by a valid pointer (e.g. received from a call
	* to *_window_open)
	*/
	void switch_to_window(tlobby_chat_window* t);

	void switch_to_window(size_t id);

	void active_window_changed();

	/**
	* Get the room* corresponding to the currently active window, or nullptr
	* if a whisper window is active at the moment
	*/
	room_info* active_window_room();

	/**
	* Check if a room window for "room" is open, if open_new is true
	* then it will be created if not found.
	* @return valid ptr if the window was found or added, null otherwise
	*/
	tlobby_chat_window* room_window_open(const std::string& room,
		bool open_new);

	/**
	* Check if a whisper window for user "name" is open, if open_new is true
	* then it will be created if not found.
	* @return valid ptr if the window was found or added, null otherwise
	*/
	tlobby_chat_window* whisper_window_open(const std::string& name,
		bool open_new);

	/**
	* Helper function to find and open a new window, used by *_window_open
	*/
	tlobby_chat_window* search_create_window(const std::string& name, bool whisper, bool open_new);

	void do_notify(t_notify_mode mode) { do_notify(mode, "", ""); }
	void do_notify(t_notify_mode mode, const std::string & sender, const std::string & message) { do_mp_notify(mode, sender, message); }

	void close_window_button_callback(tlobby_chat_window& chat_window, bool& handled, bool& halt);


	void process_room_join(const ::config& data);

	void process_room_part(const ::config& data);

	void process_room_query_response(const ::config& data);

	void process_message(const ::config& data, bool whisper = false);

	bool process_network_data(const ::config& data);
};

// }---------- DEFINITION ---------{

struct tchatbox_definition : public tcontrol_definition
{

	explicit tchatbox_definition(const config& cfg);

	struct tresolution : public tresolution_definition_
	{
		explicit tresolution(const config& cfg);

		tbuilder_grid_ptr grid;
	};
};

// }---------- BUILDER -----------{

namespace implementation
{

struct tbuilder_chatbox : public tbuilder_control
{
public:
	explicit tbuilder_chatbox(const config& cfg);

	using tbuilder_control::build;

	twidget* build() const;

private:
};

} // namespace implementation

// }------------ END --------------

} // namespace gui2
