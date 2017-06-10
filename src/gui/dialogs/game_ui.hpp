/*
   Copyright (C) 2017 by Charles Dang <exodia339@gmail.com>
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

class config;
class CVideo;
class display;

namespace gui2
{
namespace dialogs
{
class game_ui : public modal_dialog
{
public:
	game_ui();

	DEFINE_SIMPLE_DISPLAY_WRAPPER(game_ui)

private:
	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const override;

	/** Inherited from modal_dialog. */
	virtual void pre_show(window& window) override;

	/** Inherited from modal_dialog. */
	virtual void post_show(window& window) override;

	::display* disp_; // TODO: needed?

	/** Reference to the entire master game config object. */
	const config& game_config_;

	/** Reference to the current scenario's config. */
	const config& scenario_;
};

} // namespace dialogs
} // namespace gui2
