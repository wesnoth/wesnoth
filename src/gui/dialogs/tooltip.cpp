/*
	Copyright (C) 2011 - 2024
	by Mark de Wever <koraq@xs4all.nl>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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

#include "gui/core/gui_definition.hpp"
#include "gui/dialogs/modal_dialog.hpp"
#include "gui/dialogs/modeless_dialog.hpp"

static lg::log_domain log_config("config");
#define ERR_CFG LOG_STREAM(warn, log_config)

namespace gui2::dialogs
{

REGISTER_WINDOW(tooltip_large)

/**
 * At the moment two kinds of tips are known:
 * * tooltip
 * * helptip
 *
 * Generic window to show a floating tip window.
 * The class has several subclasses using the same format.
 */
class tooltip : public modeless_dialog
{
public:
	tooltip(const std::string& window_id, const t_string& message,
			const point& mouse, const SDL_Rect& source_rect)
		: modeless_dialog(window_id)
	{
		find_widget<styled_widget>("label").set_label(message);

		set_variable("mouse_x", wfl::variant(mouse.x));
		set_variable("mouse_y", wfl::variant(mouse.y));

		set_variable("source_x", wfl::variant(source_rect.x));
		set_variable("source_y", wfl::variant(source_rect.y));
		set_variable("source_w", wfl::variant(source_rect.w));
		set_variable("source_h", wfl::variant(source_rect.h));
	}
};

namespace tip
{

static std::unique_ptr<tooltip> tip;

void show(const std::string& window_id,
		  const t_string& message,
		  const point& mouse,
		  const SDL_Rect& source_rect)
{
	/*
	 * For now allow invalid tip names, might turn them to invalid wml messages
	 * later on.
	 */
	tip.reset(new tooltip(window_id, message, mouse, source_rect));
	try
	{
		tip->show();
	}
	catch(const window_builder_invalid_id&)
	{
		ERR_CFG << "Tip with the requested id '" << window_id
				<< "' doesn't exist, fall back to the default.";
		tip.reset(new tooltip("tooltip_large", message, mouse, source_rect));
		try
		{
			tip->show();
		}
		catch(const window_builder_invalid_id&)
		{
			ERR_CFG << "Default tooltip doesn't exist, no message shown.";
		}
	}
}

void remove()
{
	tip.reset();
}

} // namespace tip

} // namespace dialogs
