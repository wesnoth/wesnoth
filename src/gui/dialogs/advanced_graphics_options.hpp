/*
   Copyright (C) 2012 - 2018 by Chris Beck <render787@gmail.com>
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
#include "gui/widgets/group.hpp"
#include "preferences/general.hpp"

namespace gui2
{
namespace dialogs
{

class advanced_graphics_options : public modal_dialog
{
public:
	/** Constructor. */
	advanced_graphics_options();

	/**
	 * The display function.
	 *
	 * See @ref modal_dialog for more information.
	 */
	DEFINE_SIMPLE_DISPLAY_WRAPPER(advanced_graphics_options)

	// These names must match the infixes of the widget ids in advanced_graphics_options.cfg
	static const std::vector<std::string> scale_cases;

private:
	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const override;

	/** Inherited from modal_dialog. */
	virtual void pre_show(window& window) override;

	/** Inherited from modal_dialog. */
	virtual void post_show(window& window) override;

	void setup_scale_case(const std::string&, window&);
	void update_scale_case(const std::string&);

	using SCALING_ALGORITHM = preferences::SCALING_ALGORITHM;

	SCALING_ALGORITHM get_scale_pref(const std::string& pref_id);

	std::map<std::string, group<SCALING_ALGORITHM>> groups_;
};

} // namespace dialogs
} // end namespace gui2
