/*
   Copyright (C) 2010 - 2017 by the Battle for Wesnoth Project http://www.wesnoth.org/

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
#include "generators/default_map_generator.hpp"

namespace gui2
{

class label;

namespace dialogs
{

class generator_settings : public modal_dialog
{
public:
	explicit generator_settings(generator_data& data);

	/** The execute function. See @ref modal_dialog for more information. */
	DEFINE_SIMPLE_EXECUTE_WRAPPER(generator_settings)

private:
	virtual void pre_show(window& window) override;

	void adjust_minimum_size_by_players(window& window);

	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const override;

	/** We need to own these fields to access the underlying widget */
	field_integer* players_;
	field_integer* width_;
	field_integer* height_;

	std::function<void()> update_width_label_, update_height_label_;
};

} // namespace dialogs
} // namespace gui2
