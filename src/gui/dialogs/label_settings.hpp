/*
   Copyright (C) 2017 by the Battle for Wesnoth Project http://www.wesnoth.org/

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
#include <map>
#include "display_context.hpp"
#include "tstring.hpp"

namespace gui2
{
namespace dialogs
{

class label_settings : public modal_dialog {
public:
	label_settings(display_context& dc);

	/**
	 * The execute function.
	 *
	 * See @ref modal_dialog for more information.
	 */
	static bool execute(display_context& dc, CVideo& video);
private:
	std::map<std::string, bool> all_labels;
	std::map<std::string, t_string> labels_display;
	display_context& viewer;

	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const override;

	/** Inherited from modal_dialog. */
	virtual void pre_show(window& window) override;

	/** Callback for toggling a checkbox state. */
	void toggle_category(widget& box, std::string category);
};
} // namespace dialogs
} // namespace gui2
