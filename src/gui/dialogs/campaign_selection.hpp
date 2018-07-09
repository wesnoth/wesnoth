/*
   Copyright (C) 2009 - 2018 by Mark de Wever <koraq@xs4all.nl>
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

#include "game_initialization/create_engine.hpp"

#include <boost/dynamic_bitset.hpp>

namespace gui2
{
namespace dialogs
{

class campaign_selection : public modal_dialog
{
	enum CAMPAIGN_ORDER {RANK, DATE, NAME};
public:
	explicit campaign_selection(ng::create_engine& eng)
		: engine_(eng)
		, choice_(-1)
		, deterministic_(false)
		, mod_states_()
		, page_ids_()
		, current_sorting_(RANK)
		, currently_sorted_asc_(true)
	{
		set_restore(true);
	}

	/***** ***** ***** setters / getters for members ***** ****** *****/

	int get_choice() const
	{
		return choice_;
	}

	bool get_deterministic() const
	{
		return deterministic_;
	}

private:
	/** Called when another campaign is selected. */
	void campaign_selected(window& window);

	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const override;

	/** Inherited from modal_dialog. */
	virtual void pre_show(window& window) override;

	/** Inherited from modal_dialog. */
	virtual void post_show(window& window) override;

	void sort_campaigns(window& window, CAMPAIGN_ORDER order, bool ascending);

	void add_campaign_to_tree(window& window, const config& campaign);

	void toggle_sorting_selection(window& window, CAMPAIGN_ORDER order);

	void mod_toggled(window& window);

	ng::create_engine& engine_;

	/** The chosen campaign. */
	int choice_;

	/** whether the player checked the "Deterministic" checkbox. */
	bool deterministic_;

	boost::dynamic_bitset<> mod_states_;

	std::vector<std::string> page_ids_;

	CAMPAIGN_ORDER current_sorting_;

	bool currently_sorted_asc_;
};

} // namespace dialogs
} // namespace gui2
