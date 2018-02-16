/*
   Copyright (C) 2010 - 2018 by Iris Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

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
#include "color.hpp"

namespace gui2
{
namespace dialogs
{

class editor_edit_label : public modal_dialog
{
public:
	/**
	 * Constructor.
	 *
	 * @param[in, out] text       The parameter's usage is:
	 *                            - Input: The initial value of the label.
	 *                            - Output: The label text the user entered if
	 *                              the dialog returns @ref window::OK
	 *                              undefined otherwise.
	 */
	editor_edit_label(std::string& text,
					   bool& immutable,
					   bool& visible_fog,
					   bool& visible_shroud,
					   color_t& color,
					   std::string& category);

	/** The execute function. See @ref modal_dialog for more information. */
	DEFINE_SIMPLE_EXECUTE_WRAPPER(editor_edit_label)

private:
	color_t& color_store;
	int load_color_component(uint8_t color_t::* component);
	void save_color_component(uint8_t color_t::* component, const int value);
	void register_color_component(std::string widget_id, uint8_t color_t::* component);
	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const override;
	/** Inherited from modal_dialog. */
	virtual void pre_show(window& window) override;
};
} // namespace dialogs
} // namespace gui2
