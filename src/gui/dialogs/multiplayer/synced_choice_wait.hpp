/*
	Copyright (C) 2014 - 2024
	by Iris Morelle <shadowm2006@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

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

	DEFINE_SIMPLE_DISPLAY_WRAPPER(synched_choice_wait)

private:
	user_choice_manager& mgr_;
	label* message_;

	virtual const std::string& window_id() const override;

	virtual void pre_show() override;

	virtual void handle_generic_event(const std::string& event_name) override;
};
} // namespace dialogs
} // namespace gui2
