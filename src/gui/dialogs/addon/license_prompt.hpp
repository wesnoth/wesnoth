/*
   Copyright (C) 2020 by Iris Morelle <shadowm@wesnoth.org>
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

namespace gui2 {
namespace dialogs {

class addon_license_prompt : public modal_dialog
{
public:
	/** Constructor. */
	explicit addon_license_prompt(const std::string& license_terms);

	/**
	 * The execute function.
	 *
	 * See @ref modal_dialog for more information.
	 */
	DEFINE_SIMPLE_EXECUTE_WRAPPER(addon_license_prompt)

private:
	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const override;

	/** Inherited from modal_dialog. */
	virtual void pre_show(window& window) override;

	std::string license_terms_;
};

}} // end namespace gui2::dialogs
