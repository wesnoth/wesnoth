/*
	Copyright (C) 2017 - 2025
	by Amir Hassan <amir@viel-zu.org>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "gui/dialogs/surrender_quit.hpp"


namespace gui2::dialogs
{

REGISTER_DIALOG(surrender_quit)


surrender_quit::surrender_quit()
	: modal_dialog(window_id())
{
}

} // namespace dialogs
