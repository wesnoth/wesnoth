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

#ifndef GUI_DIALOGS_LUA_INT_HPP_INCLUDED
#define GUI_DIALOGS_LUA_INT_HPP_INCLUDED

#include "gui/dialogs/modal_dialog.hpp"

class lua_kernel_base;

namespace gui2
{
namespace dialogs
{

class lua_interpreter : public modal_dialog
{
public:
	class lua_model;
	class input_model;
	class view;
	class controller;

	lua_interpreter(lua_kernel_base & lk);

	/** Inherited from modal_dialog. */
	window* build_window(CVideo& video);

	/** Inherited from modal_dialog. */
	void pre_show(window& window);

	enum WHICH_KERNEL { APP, GAME };
	static void display(CVideo& video, lua_kernel_base * lk);
	static void display(CVideo& video, WHICH_KERNEL which);
private:
	const std::unique_ptr<controller> controller_;

	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;
};

} // namespace dialogs
} // namespace gui2

#endif /* ! GUI_DIALOGS_LUA_INT_HPP_INCLUDED */
