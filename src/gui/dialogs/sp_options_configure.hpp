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

#pragma once

#include "gui/dialogs/modal_dialog.hpp"
#include "gui/dialogs/multiplayer/plugin_executor.hpp"
#include "gui/dialogs/multiplayer/mp_options_helper.hpp"

#include "game_initialization/create_engine.hpp"
#include "game_initialization/configure_engine.hpp"

namespace gui2
{
namespace dialogs
{

class sp_options_configure : public modal_dialog, private plugin_executor
{
public:
	sp_options_configure(ng::create_engine& create_engine, ng::configure_engine& config_engine);

	/**
	 * Execute function. We only want to show the dialog if the campaign has options or if
	 * there are active mods and at least one of those mods has custom options.
	 */
	static bool execute(ng::create_engine& create_engine, ng::configure_engine& config_engine, CVideo& video)
	{
		// Check campaign options.
		const auto& campaign_mods = create_engine.current_level().data().child_range("options");

		const bool have_campaign_options = std::any_of(campaign_mods.begin(), campaign_mods.end(), [](config& mod) {
			return !mod.empty();
		});
	
		// Check active mod options.
		bool have_mod_options = false;

		for(const auto& mod : create_engine.active_mods_data()) {
			if(!(*mod->cfg).has_child("options")) {
				continue;
			}

			const auto& opt_range = (*mod->cfg).child_range("options");

			if(std::any_of(opt_range.begin(), opt_range.end(), [](const config& options) {
				return !options.empty();
			})) {
				have_mod_options = true;
				break;
			}
		}

		// If we have no valid options whatsoever, just bypass this dialog.
		if(!have_campaign_options && !have_mod_options) {
			return true;
		}

		return sp_options_configure(create_engine, config_engine).show(video);
	}

private:
	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const override;

	/** Inherited from modal_dialog. */
	virtual void pre_show(window& window) override;

	/** Inherited from modal_dialog. */
	virtual void post_show(window& window) override;

	ng::create_engine& create_engine_;
	ng::configure_engine& config_engine_;

	std::unique_ptr<mp_options_helper> options_manager_;
};

} // namespace dialogs
} // namespace gui2
