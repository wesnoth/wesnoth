/*
	Copyright (C) 2013 - 2024
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

class edit_text : public modal_dialog
{
public:
	/**
	 * Constructor.
	 *
	 * @param title               The dialog's title.
	 * @param label               Label of the text field.
	 * @param [in, out] text      The parameter's usage is:
	 *                            - Input: The initial value of the text field.
	 *                            - Output: The new unit name the user entered
	 *                              if the dialog returns retval::OK,
	 *                              undefined otherwise.
	 * @param disallow_empty      Whether to prevent the user from entering a string that is
	 *                            empty or consists only of whitespace.
	 */
	edit_text(const std::string& title,
			   const std::string& label,
			   std::string& text,
			   bool disallow_empty = false);

	/**
	 * Executes the dialog.
	 * See @ref modal_dialog for more information.
	 */
	DEFINE_SIMPLE_EXECUTE_WRAPPER(edit_text)

private:
	virtual const std::string& window_id() const override;

	virtual void pre_show() override;

	void on_text_change();

	bool disallow_empty_;
};
} // namespace dialogs
