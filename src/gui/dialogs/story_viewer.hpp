/*
	Copyright (C) 2017 - 2024
	by Charles Dang <exodia339@gmail.com>
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

#include "config.hpp"
#include "storyscreen/controller.hpp"
#include "storyscreen/part.hpp"

namespace gui2::dialogs
{

/** Dialog to view the storyscreen. */
class story_viewer : public modal_dialog
{
public:
	story_viewer(const std::string& scenario_name, const config& cfg_parsed);

	~story_viewer();

	static void display(const std::string& scenario_name, const config& story)
	{
		try {
			story_viewer viewer(scenario_name, story);
			if(viewer.controller_.max_parts() > 0) {
				viewer.show();
			}
		} catch(const std::out_of_range&) {}
	}

	/** top_level_drawable hook to animate the view */
	virtual void update() override;

private:
	virtual const std::string& window_id() const override;

	virtual void pre_show() override;
	virtual void post_show() override;

	void clear_image_timer();

	void update_current_part_ptr();

	void display_part();

	using floating_image_list = std::vector<storyscreen::floating_image>;
	void draw_floating_image(floating_image_list::const_iterator image_iter, int this_part_index);

	enum NAV_DIRECTION {
		DIR_FORWARD,
		DIR_BACKWARDS
	};

	void nav_button_callback( NAV_DIRECTION direction);

	void key_press_callback(const SDL_Keycode key);

	void set_next_draw();
	void begin_fade_draw(bool fade_in);
	void halt_fade_draw();

	void flag_stack_as_dirty();

	storyscreen::controller controller_;

	int part_index_;

	storyscreen::controller::part_pointer_type current_part_;

	std::size_t timer_id_;
	std::size_t next_draw_;

	int fade_step_;

	enum FADE_STATE {
		FADING_IN,
		FADING_OUT,
		NOT_FADING
	};

	FADE_STATE fade_state_;

	bool game_was_already_hidden_;
};

} // namespace dialogs
