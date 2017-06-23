/*
   Copyright (C) 2017 by the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "gui/dialogs/modeless_dialog.hpp"

#include <SDL_keycode.h>

#include <functional>
#include <memory>

namespace gui2
{
class text_box;

namespace dialogs
{

/**
 * A simple command console class.
 *
 * This class doesn't explicitly handle any processing of commands itself. Instead, it consists
 * of a singleton modeless dialog that can be invoked from any area of the game using the static
 * @display function. A callback function should be passed in that will handle processing of the
 * command input.
 *
 * @todo This dialog's design definitely needs to be improved (multiline input area, for one).
 * Also might be worth looking if more actual command handling could be merged in here.
 */
class command_console : public modeless_dialog
{
private:
	/** Callback function signature. */
	using callback_t = std::function<void(std::string)>;

public:
	command_console(const command_console&) = delete;
	command_console& operator=(const command_console&) = delete;

	command_console(const std::string& prompt, callback_t callback);

	~command_console()
	{
		//hide();
	}

	//enum CONSOLE_MODE { MODE_NONE, MODE_SEARCH, MODE_MESSAGE, MODE_COMMAND, MODE_AI };

	/**
	 * Shows the command console. This resets the singleton instance and displays the
	 * dialog until dismissed by user action.
	 *
	 * Since this is a modeless dialog, we can't use the usual method of creating a temporary
	 * dialog object here. Modal dialogs have a display loop that keeps that open; modeless
	 * dialogs, however, don't, meaning the object would immediately be destroyed and hidden.
	 */
	static void display(CVideo& video, const std::string& prompt, callback_t callback = nullptr)
	{
		singleton_.reset(new command_console(prompt, callback));
		singleton_->show(video, true); // allow interaction
	}

private:
	/** Inherited from modeless_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const override;

	/** Inherited from modeless_dialog. */
	virtual void post_build(window& window) override;

	/** Inherited from modeless_dialog. */
	virtual void pre_show(window& window) override;

	void window_key_press_callback(const SDL_Keycode key);

	void input_key_press_callback(const SDL_Keycode key);

	void close();

	/** The input text box. */
	text_box* input_;

	/** Text action prompt. */
	std::string prompt_;

	/** The callback to process the inputted command. */
	callback_t command_callback_;

	/** Dialog singleton. */
	static std::unique_ptr<command_console> singleton_;
};

} // namespace dialogs
} // namespace gui2

