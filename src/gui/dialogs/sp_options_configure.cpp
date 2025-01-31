/*
	Copyright (C) 2008 - 2024
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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


namespace gui2::dialogs
{

REGISTER_DIALOG(sp_options_configure)

sp_options_configure::sp_options_configure(ng::create_engine& create_engine, ng::configure_engine& config_engine)
	: modal_dialog(window_id())
	, create_engine_(create_engine)
	, config_engine_(config_engine)
	, options_manager_()
{
	set_show_even_without_video(true);
	set_allow_plugin_skip(false);
}

void sp_options_configure::pre_show()
{
	options_manager_.reset(new mp_options_helper(*this, create_engine_));
	options_manager_->update_all_options();

	plugins_context_.reset(new plugins_context("Campaign Configure"));
	plugins_context_->set_callback("launch", [this](const config&) { set_retval(retval::OK); }, false);
}

void sp_options_configure::post_show()
{
	if(get_retval() == retval::OK) {
		config_engine_.set_options(options_manager_->get_options_config());
	}
}

} // namespace dialogs
