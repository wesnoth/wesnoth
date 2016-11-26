/*
   Copyright (C) 2014 - 2016 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_THEME_LIST_HPP_INCLUDED
#define GUI_DIALOGS_THEME_LIST_HPP_INCLUDED

#include "gui/dialogs/modal_dialog.hpp"
#include "synced_user_choice.hpp"
#include "generic_event.hpp"
namespace gui2
{
class label;
class window;
namespace dialogs
{
class synched_choice_wait : public modal_dialog, public events::observer
{
public:
	explicit synched_choice_wait(user_choice_manager& mgr);
	~synched_choice_wait();
private:
	user_choice_manager& mgr_;
	label* message_;
	window* window_;
	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from modal_dialog. */
	void pre_show(window& window);

	virtual void handle_generic_event(const std::string& event_name);
};
} // namespace dialogs
} // namespace gui2

#endif
