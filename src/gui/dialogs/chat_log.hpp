/*
   Copyright (C) 2011 - 2017 by Yurii Chernyi <terraninfo@terraninfo.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_CHAT_LOG_HPP_INCLUDED
#define GUI_DIALOGS_CHAT_LOG_HPP_INCLUDED

#include "gui/dialogs/modal_dialog.hpp"
#include "variable.hpp"

class replay;

namespace gui2
{
namespace dialogs
{

class chat_log : public modal_dialog
{
public:
	class model;
	class view;
	class controller;
	chat_log(const vconfig& cfg, const replay& replay);

	/** Inherited from modal_dialog. */
	window* build_window(CVideo& video);

	/** Inherited from modal_dialog. */
	virtual void pre_show(window& window) override;

	std::shared_ptr<view> get_view();

private:
	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const override;

	std::shared_ptr<view> view_;
};
} // namespace dialogs
} // namespace gui2

#endif /* ! GUI_DIALOGS_CHAT_LOG_HPP_INCLUDED */
