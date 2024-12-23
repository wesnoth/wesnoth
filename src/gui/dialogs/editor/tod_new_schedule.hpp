/*
	Copyright (C) 2023 - 2024
	by Subhraman Sarkar (babaissarkar) <suvrax@gmail.com>
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

namespace gui2::dialogs
{
/**
 * Dialog that takes new schedule ID and name from the player.
 * custom_tod.cpp is the main editor window for time schedules.
 * This is launched when the user presses OK there.
 */
class tod_new_schedule : public modal_dialog
{
public:
	tod_new_schedule(std::string& schedule_id, t_string& schedule_name);

	/** The execute function. See @ref modal_dialog for more information. */
	DEFINE_SIMPLE_EXECUTE_WRAPPER(tod_new_schedule);

private:
	virtual void post_show() override;
	virtual void pre_show() override;

	virtual const std::string& window_id() const override;

	std::string& schedule_id_;
	t_string& schedule_name_;

	/* Callback for enabling or disabling OK button */
	void button_state_change();
};

} // namespace gui2::dialogs
