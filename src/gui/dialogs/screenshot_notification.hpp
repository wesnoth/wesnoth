/*
   Copyright (C) 2013 - 2018 by Iris Morelle <shadowm2006@gmail.com>
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
namespace dialogs
{

class screenshot_notification : public modal_dialog
{
public:
	/**
	 * Constructor.
	 *
	 * @param path     Path to the screenshot file created. The caller should
	 *                 ensure the file exists, otherwise it will be displayed
	 *                 with size 0.
	 */
	screenshot_notification(const std::string& path);

	/**
	 * The display function.
	 *
	 * See @ref modal_dialog for more information.
	 */
	DEFINE_SIMPLE_DISPLAY_WRAPPER(screenshot_notification)

private:
	const std::string path_;
	const std::string screenshots_dir_path_;

	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const override;

	/** Inherited from modal_dialog. */
	virtual void pre_show(window& window) override;
};
} // namespace dialogs
} // namespace gui2
