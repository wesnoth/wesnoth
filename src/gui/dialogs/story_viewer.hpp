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

#ifndef GUI_DIALOGS_STORY_VIEWER_HPP_INCLUDED
#define GUI_DIALOGS_STORY_VIEWER_HPP_INCLUDED

#include "gui/dialogs/modal_dialog.hpp"

#include "config.hpp"
#include "storyscreen/controller.hpp"
#include "storyscreen/part.hpp"

class CVideo;

namespace gui2
{
namespace dialogs
{

/** Dialog to view the storyscreen. */
class story_viewer : public modal_dialog
{
public:
	story_viewer(const std::string& scenario_name, const config& cfg_parsed);

	~story_viewer();

	static void display(const std::string& scenario_name, const config& story, CVideo& video)
	{
		story_viewer(scenario_name, story).show(video);
	}

private:
	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const override;

	/** Inherited from modal_dialog. */
	virtual void pre_show(window& window) override;

	void update_current_part_ptr();

	void display_part(window& window);

	using floating_image_list = std::vector<storyscreen::floating_image>;
	void draw_floating_image(window& window, floating_image_list::const_iterator image_iter, int this_part_index);

	enum NAV_DIRECTION {
		DIR_FORWARD,
		DIR_BACKWARDS
	};

	void nav_button_callback(window& window, NAV_DIRECTION direction);

	void key_press_callback(window& window, const SDL_Keycode key);

	void set_next_draw();
	void begin_fade_draw(bool fade_in);
	void halt_fade_draw();

	void draw_callback(window& window);

	storyscreen::controller controller_;

	int part_index_;

	storyscreen::controller::part_pointer_type current_part_;

	size_t timer_id_;
	size_t next_draw_;

	int fade_step_;

	enum FADE_STATE {
		FADING_IN,
		FADING_OUT,
		NOT_FADING
	};

	FADE_STATE fade_state_;
};

} // namespace dialogs
} // namespace gui2

#endif
