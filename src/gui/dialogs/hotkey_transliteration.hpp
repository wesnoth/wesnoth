/*
   Copyright (C) 2014 - 2018 by Iris Morelle <shadowm2006@gmail.com>
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

namespace gui2
{
class label;
class button;
namespace dialogs
{

/**
 * @ingroup GUIWindowDefinitionWML
 *
 * A Preferences subdialog configuring mappings between multiple keyboard layouts.

 * Key               |Type          |Mandatory|Description
 * ------------------|--------------|---------|-----------
 */
class hotkey_transliteration : public modal_dialog
{
public:
	/** Constructor. */
	hotkey_transliteration();

	/**
     * The display function.
	 *
	 * See @ref modal_dialog for more information.
     */
	DEFINE_SIMPLE_DISPLAY_WRAPPER(hotkey_transliteration)

private:
	std::string cache_path_;

	button* clean_button_;
	button* purge_button_;
	label* size_label_;

	virtual const std::string& window_id() const override;

	virtual void pre_show(window& window) override;

	virtual void post_show(window& window) override;
};

} // namespace dialogs
} // namespace gui2
