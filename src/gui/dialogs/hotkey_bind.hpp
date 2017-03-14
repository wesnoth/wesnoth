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

#ifndef GUI_DIALOGS_HOTKEY_BIND_HPP_INCLUDED
#define GUI_DIALOGS_HOTKEY_BIND_HPP_INCLUDED

#include "gui/dialogs/modal_dialog.hpp"
#include "hotkey/hotkey_item.hpp"

#include <SDL_keycode.h>

#include <string>

namespace gui2
{
namespace dialogs
{

class hotkey_bind : public modal_dialog
{
public:
	explicit hotkey_bind(const std::string& hotkey_id);

	hotkey::hotkey_ptr get_new_binding() const
	{
		return new_binding_;
	}

private:
	const std::string& hotkey_id_;

	hotkey::hotkey_ptr new_binding_;

	void key_press_callback(bool&, bool&, const SDL_Keycode key);

	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from modal_dialog. */
	void pre_show(window& window);

	/** Inherited from modal_dialog. */
	void post_show(window& window);
};

} // namespace dialogs
} // namespace gui2

#endif /* ! GUI_DIALOGS_HOTKEY_BIND_HPP_INCLUDED */
