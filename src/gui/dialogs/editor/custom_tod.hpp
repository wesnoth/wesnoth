/*
   Copyright (C) 2008 - 2017 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_CUSTOM_TOD_HPP_INCLUDED
#define GUI_DIALOGS_CUSTOM_TOD_HPP_INCLUDED

#include "display.hpp"
#include "time_of_day.hpp"
#include "gui/widgets/image.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/dialogs/modal_dialog.hpp"

#include <vector>

namespace gui2
{

class label;
class image;
class slider;
class text_box;

namespace dialogs
{

class custom_tod : public modal_dialog
{
public:
	custom_tod(display& display,
				const std::vector<time_of_day>& tods);

	static bool execute(display& display, const std::vector<time_of_day>& tods)
	{
		return custom_tod(display, tods).show(display.video());
	}

private:
	/** Available time_of_days */
	std::vector<time_of_day> tods_;

	/** Callback for the next tod button */
	void do_next_tod(window& window);
	void do_prev_tod(window& window);

	void do_new_tod(window& window);
	void do_delete_tod(window& window);

	void select_file(const std::string& filename,
					 const std::string& default_dir,
					 const std::string& attribute,
					 window& window);

	void update_tod_display(window& window);

	void update_lawful_bonus(window& window);

	void set_selected_tod(time_of_day tod);
	const time_of_day& get_selected_tod() const;

	void update_selected_tod_info(window& window);

	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const override;

	/** Inherited from modal_dialog. */
	virtual void pre_show(window& window) override;

	/** Inherited from modal_dialog. */
	virtual void post_show(window& window) override;

	/** Current map generator index */
	int current_tod_;

	/** Text boxes for name and id*/
	text_box* current_tod_name_;
	text_box* current_tod_id_;

	/** Images for the current tod*/
	image* current_tod_image_;
	image* current_tod_mask_;

	/** Labels for the current tod*/
	label* current_tod_sound_;
	label* current_tod_number_;

	field_integer* lawful_bonus_field_;
	slider* tod_red_field_;
	slider* tod_green_field_;
	slider* tod_blue_field_;

	/**
	 * The display to update when the ToD changes.
	 */
	display& display_;
};

} // namespace dialogs
} // namespace gui2

#endif
