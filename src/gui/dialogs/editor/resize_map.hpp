/*
   Copyright (C) 2008 - 2018 by Mark de Wever <koraq@xs4all.nl>
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

namespace gui2
{

class toggle_button;

namespace dialogs
{

class editor_resize_map : public modal_dialog
{
public:
	enum EXPAND_DIRECTION {
		EXPAND_BOTTOM_RIGHT,
		EXPAND_BOTTOM,
		EXPAND_BOTTOM_LEFT,
		EXPAND_RIGHT,
		EXPAND_CENTER,
		EXPAND_LEFT,
		EXPAND_TOP_RIGHT,
		EXPAND_TOP,
		EXPAND_TOP_LEFT
	};

	/**
	 * Constructor.
	 *
	 * @param [in, out] width     The parameter's usage is:
	 *                            - Input: The initial width of the map.
	 *                            - Output: The selected width of the map if
	 *                              the dialog returns @ref window::OK
	 *                              undefined otherwise.
	 *
	 * @param [in, out] height    The parameter's usage is:
	 *                            - Input: The initial height of the map.
	 *                            - Output: The selected height of the map if
	 *                              the dialog returns @ref window::OK
	 *                              undefined otherwise.
	 *
	 * @param [out] expand_direction
	 *                            The selected expand direction if the dialog
	 *                            returns  @ref window::OK undefined
	 *                            otherwise.
	 *
	 * @param [in, out] copy_edge_terrain
	 *                            The parameter's usage is:
	 *                            - Input: The initial value of the copy edge
	 *                              toggle.
	 *                            - Output: The final value of the copy edge
	 *                              toggle if the dialog returns @ref
	 *                              window::OK undefined otherwise.
	 */
	editor_resize_map(int& width,
					   int& height,
					   EXPAND_DIRECTION& expand_direction,
					   bool& copy_edge_terrain);

	/** The execute function. See @ref modal_dialog for more information. */
	DEFINE_SIMPLE_EXECUTE_WRAPPER(editor_resize_map)

private:
	/** The currently selected width. */
	field_integer* width_;

	/** The currently selected height. */
	field_integer* height_;

	/** The original width. */
	int old_width_;

	/** The original height. */
	int old_height_;

	/** The selected expansion direction. */
	EXPAND_DIRECTION& expand_direction_;

	/**
	 * The toggle buttons show the state of expand_direction_.
	 *
	 * Allows both so select a direction and visually show the effect of the
	 * selection.
	 */
	toggle_button* direction_buttons_[9];

	void update_expand_direction(window& window);

	void set_direction_icon(int index, std::string icon);

	/** Inherited from modal_dialog */
	virtual void pre_show(window& window) override;

	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const override;
};

} // namespace dialogs
} // namespace gui2
