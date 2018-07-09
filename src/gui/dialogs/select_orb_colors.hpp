/*
   Copyright (C) 2017-2018 by the Battle for Wesnoth Project https://www.wesnoth.org/

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
#include "gui/widgets/group.hpp"

#include <map>

namespace gui2
{
namespace dialogs
{
class select_orb_colors : public modal_dialog
{
public:
	select_orb_colors();

	/**
	 * The display function.
	 *
	 * See @ref modal_dialog for more information.
	 */
	DEFINE_SIMPLE_DISPLAY_WRAPPER(select_orb_colors)

private:
	void setup_orb_group(const std::string& base_id, bool& shown, const std::string& initial);

	void reset_orb_group(const std::string& base_id, bool& shown, const std::string& initial);

	void toggle_orb_callback(bool& storage);
	void reset_orb_callback();

	bool show_unmoved_, show_partial_, show_moved_, show_ally_, show_enemy_;

	std::map<std::string, group<std::string>> groups_;

	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const override;

	/** Inherited from modal_dialog. */
	virtual void pre_show(window& window) override;

	/** Inherited from modal_dialog. */
	virtual void post_show(window& window) override;
};

} // namespace dialogs
} // namespace gui2
