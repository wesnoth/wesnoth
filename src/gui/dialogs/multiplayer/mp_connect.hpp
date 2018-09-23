/*
   Copyright (C) 2008 - 2018 by Mark de Wever <koraq@xs4all.nl>
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
namespace dialogs
{

class mp_connect : public modal_dialog
{
	/** The unit test needs to be able to test the mp_connect dialog. */
	friend modal_dialog* unit_test_mp_server_list();

public:
	mp_connect();

	/** The execute function. See @ref modal_dialog for more information. */
	DEFINE_SIMPLE_EXECUTE_WRAPPER(mp_connect)

private:
	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const override;

	/** Inherited from modal_dialog. */
	virtual void pre_show(window& window) override;

	/** The host name of the selected servef. */
	field_text* host_name_;

	/**
	 * The unit test needs to be able to test the mp_connect dialog.
	 *
	 * @returns                   A newly allocated mp_server_list.
	 */
	static modal_dialog* mp_server_list_for_unit_test();
};

} // namespace dialogs
} // namespace gui2
