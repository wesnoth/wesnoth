/*
   Copyright (C) 2009 - 2015 by Yurii Chernyi <terraninfo@terraninfo.net>
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

#include "gui/dialogs/dialog.hpp"
#include "../../variable.hpp"

#include <boost/shared_ptr.hpp>

namespace gui2
{

class tgamestate_inspector : public tdialog
{
public:
	class model;
	class view;
	class controller;
	explicit tgamestate_inspector(const vconfig& cfg);

	boost::shared_ptr<view> get_view();

private:
	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	boost::shared_ptr<view> view_;
};
}

#endif /* ! GUI_DIALOGS_GAMESTATE_INSPECTOR_HPP_INCLUDED */
