/*
   Copyright (C) 2011 - 2017 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/tooltip.hpp"

#include "gui/dialogs/modal_dialog.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/dialogs/modeless_dialog.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"

static lg::log_domain log_config("config");
#define ERR_CFG LOG_STREAM(warn, log_config)

namespace gui2
{
namespace dialogs
{

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_tip
 *
 * == Tip float ==
 *
 * Generic window to show a floating tip window. The class has several
 * subclasses using the same format. For example there will be tooltips and
 * helptips, both using this class.
 *
 * @begin{table}{dialog_widgets}
 *
 * label & & styled_widget & m &
 *         This text contains the message to show in the tip. $
 *
 * @end{table}
 *
 * In the canvas of the windows used in this dialog the following variables are
 * defined:
 *
 * @begin{table}{formula}
 *     mouse_x & unsigned &          The x coordinate of the mouse pointer when
 *                                   the window was created. $
 *     mouse_y & unsigned &          The y coordinate of the mouse pointer when
 *                                   the window was created. $
 * @end{table}
 */

REGISTER_WINDOW(tooltip_large)

/**
 * Class to show the tips.
 *
 * At the moment two kinds of tips are known:
 * * tooltip
 * * helptip
 */
class tooltip : public modeless_dialog
{
public:
	tooltip() : modeless_dialog(), window_id_(), message_(), mouse_()
	{
		// To make Coverity happy
		source_rect_.x = 0;
		source_rect_.y = 0;
		source_rect_.w = 0;
		source_rect_.h = 0;
	}

	void set_window_id(const std::string& window_id)
	{
		window_id_ = window_id;
	}

	void set_message(const t_string& message)
	{
		message_ = message;
	}

	void set_mouse(const point& mouse)
	{
		mouse_ = mouse;
	}

	void set_source_rect(const SDL_Rect& rect)
	{
		source_rect_ = rect;
	}

private:
	/** The id of the window to use to show the tip. */
	std::string window_id_;

	/** The message to show. */
	t_string message_;

	/** The position of the mouse. */
	point mouse_;

	/** The size of the requestor. */
	SDL_Rect source_rect_;

	/** Inherited from modeless_dialog. */
	virtual const std::string& window_id() const override;

	/** Inherited from modeless_dialog. */
	virtual void pre_show(window& window) override;
};

void tooltip::pre_show(window& window)
{
	find_widget<styled_widget>(&window, "label", false).set_label(message_);

	window.set_variable("mouse_x", wfl::variant(mouse_.x));
	window.set_variable("mouse_y", wfl::variant(mouse_.y));

	window.set_variable("source_x", wfl::variant(source_rect_.x));
	window.set_variable("source_y", wfl::variant(source_rect_.y));
	window.set_variable("source_w", wfl::variant(source_rect_.w));
	window.set_variable("source_h", wfl::variant(source_rect_.h));
}

const std::string& tooltip::window_id() const
{
	return window_id_;
}

namespace tip
{

static tooltip& tip()
{
	/*
	 * Allocating a static tip object causes a segmentation fault when Wesnoth
	 * terminates. So instead create an object on the heap and never free it.
	 */
	static tooltip* t = new tooltip();
	return *t;
}

void show(const std::string& window_id,
		  const t_string& message,
		  const point& mouse,
		  const SDL_Rect& source_rect)
{
	/*
	 * For now allow invalid tip names, might turn them to invalid wml messages
	 * later on.
	 */
	tooltip& t = tip();
	t.set_window_id(window_id);
	t.set_message(message);
	t.set_mouse(mouse);
	t.set_source_rect(source_rect);
	try
	{
		t.show();
	}
	catch(window_builder_invalid_id&)
	{
		ERR_CFG << "Tip with the requested id '" << window_id
				<< "' doesn't exist, fall back to the default.\n";
		t.set_window_id("tooltip_large");
		try
		{
			t.show();
		}
		catch(window_builder_invalid_id&)
		{
			ERR_CFG << "Default tooltip doesn't exist, no message shown."
					<< std::endl;
		}
	}
}

void remove()
{
	tip().hide();
}

} // namespace tip

} // namespace dialogs
} // namespace gui2
