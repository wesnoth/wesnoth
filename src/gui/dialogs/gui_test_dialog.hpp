/*
	Copyright (C) 2023 - 2024
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
/**
 * @ingroup GUIWindowDefinitionWML
 *
 * A test dialog for testing various gui2 features
 */
class gui_test_dialog : public modal_dialog
{
public:
	gui_test_dialog();

	/** The execute function. See @ref modal_dialog for more information. */
	DEFINE_SIMPLE_EXECUTE_WRAPPER(gui_test_dialog)

private:
	virtual void pre_show() override;
	virtual void post_show() override;

	virtual const std::string& window_id() const override;
};

} // namespace gui2::dialogs
