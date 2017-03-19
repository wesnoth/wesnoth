/*
   Copyright (C) 2008 - 2017 by the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/sp_options_configure.hpp"

#include "gui/widgets/settings.hpp"

namespace gui2
{
namespace dialogs
{

REGISTER_DIALOG(sp_options_configure)

sp_options_configure::sp_options_configure(ng::create_engine& create_engine, ng::configure_engine& config_engine)
	: create_engine_(create_engine)
	, config_engine_(config_engine)
	, options_manager_()
{
	set_restore(true);
}

void sp_options_configure::pre_show(window& window)
{
	options_manager_.reset(new mp_options_helper(window, create_engine_));
	options_manager_->update_all_options();
}

void sp_options_configure::post_show(window& window)
{
	if(window.get_retval() == window::OK) {
		config_engine_.set_options(options_manager_->get_options_config());
	}
}

} // namespace dialogs
} // namespace gui2
