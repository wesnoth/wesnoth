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
#include "gui/widgets/text_box_base.hpp"

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
	/**
	 * RNG mode selection values.
	 *
	 * @note The contents of this enum must match the order of the options
	 *       defined in the WML for the "rng_menu" widget of this dialog.
	 */
	enum RNG_MODE
	{
		RNG_DEFAULT,
		RNG_SAVE_SEED,
		RNG_BIASED,
	};

	explicit campaign_selection(ng::create_engine& eng)
		: engine_(eng)
		, choice_(-1)
		, rng_mode_(RNG_DEFAULT)
		, mod_states_()
		, page_ids_()
		, difficulties_()
		, current_difficulty_()
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

	RNG_MODE get_rng_mode() const
	{
		return rng_mode_;
	}

	const std::string& get_difficulty() const
	{
		return current_difficulty_;
	}

private:
	/** Called when another campaign is selected. */
	void campaign_selected();

	/** Called when the difficulty selection changes. */
	void difficulty_selected();

	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const override;

	/** Inherited from modal_dialog. */
	virtual void pre_show(window& window) override;

	/** Inherited from modal_dialog. */
	virtual void post_show(window& window) override;

	void sort_campaigns(CAMPAIGN_ORDER order, bool ascending);

	void add_campaign_to_tree(const config& campaign) const;

	void toggle_sorting_selection(CAMPAIGN_ORDER order);

	void mod_toggled();

	void filter_text_changed(const std::string& text);

	ng::create_engine& engine_;

	/** The chosen campaign. */
	int choice_;

	/** whether the player checked the "Deterministic" checkbox. */
	RNG_MODE rng_mode_;

	boost::dynamic_bitset<> mod_states_;

	std::vector<std::string> page_ids_;

	std::vector<std::string> difficulties_;

	std::string current_difficulty_;

	CAMPAIGN_ORDER current_sorting_;

	bool currently_sorted_asc_;

	std::vector<std::string> last_search_words_;
};

} // namespace dialogs
} // namespace gui2
