/*
   Copyright (C) 2017 by Charles Dang <exodia339@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_OUTRO_HPP_INCLUDED
#define GUI_DIALOGS_OUTRO_HPP_INCLUDED

#include "gui/dialogs/modal_dialog.hpp"

class CVideo;

namespace gui2
{
namespace dialogs
{

/** Dialog to display 'The End' at the end of a campaign. */
class outro : public modal_dialog
{
public:
	outro(const std::string& text, unsigned int duration);

	/**
	 * Displays a simple fading screen with any user-provided text.
	 * Used after the end of single-player campaigns.
	 *
	 * @param text     Text to display, centered on the screen.
	 *
	 * @param duration In milliseconds, for how much time the text will
	 *                 be displayed on screen.
	 */
	static void display(const std::string& text, unsigned int duration, CVideo& video)
	{
		outro(text, duration).show(video);
	}

private:
	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from modal_dialog. */
	void pre_show(window& window);

	/** Inherited from modal_dialog. */
	void post_show(window& window);

	void timer_callback(window& window);

	std::string text_;

	unsigned int duration_;
	int fade_step_;

	bool fading_in_;

	size_t timer_id_;
	size_t timer_id_secondary_;
};

} // namespace dialogs
} // namespace gui2

#endif
