/*
   Copyright (C) 2009 - 2017 by Yurii Chernyi <terraninfo@terraninfo.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_GAMESTATE_INSPECTOR_HPP_INCLUDED
#define GUI_DIALOGS_GAMESTATE_INSPECTOR_HPP_INCLUDED

#include "gui/dialogs/modal_dialog.hpp"
#include "variable.hpp"

namespace game_events {class manager; }
class display_context;

namespace gui2
{
namespace dialogs
{

class gamestate_inspector : public modal_dialog
{
public:
	class model;
	class view;
	class controller;
	gamestate_inspector(const config& vars, const game_events::manager& events, const display_context& dc, const std::string& title = "");

private:
	/** Inherited from modal_dialog. */
	void pre_show(window& window);

	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	std::shared_ptr<view> view_;
	std::shared_ptr<model> model_;
	std::shared_ptr<controller> controller_;
	std::string title_;
	const config& vars_;
	const game_events::manager& events_;
	const display_context& dc_;
};
} // namespace dialogs
} // namespace gui2

#endif /* ! GUI_DIALOGS_GAMESTATE_INSPECTOR_HPP_INCLUDED */
