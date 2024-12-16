/*
	Copyright (C) 2010 - 2024
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

namespace gui2::dialogs
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
	 *                              the dialog returns retval::OK
	 *                              undefined otherwise.
	 * @param immutable           Sets immutable_toggle attribute.
	 * @param visible_fog         Sets visible_fog_toggle attribute.
	 * @param visible_shroud      Sets visible_shroud_toggle attribute.
	 * @param color               Sets slider color.
	 * @param category            Sets category attribute.
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
	void register_color_component(const std::string& widget_id, uint8_t color_t::* component);
	virtual const std::string& window_id() const override;
	virtual void pre_show() override;
};
} // namespace dialogs
