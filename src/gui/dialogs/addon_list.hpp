/*
   Copyright (C) 2008 - 2015 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_ADDON_LIST_HPP_INCLUDED
#define GUI_DIALOGS_ADDON_LIST_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"

#include "gui/widgets/pane.hpp"

class config;
#include "config.hpp" // needed for config::const_child_itors

namespace gui2
{

class pane;

/** Shows the list of addons on the server. */
class taddon_list : public tdialog
{
public:
	explicit taddon_list(const config& cfg)
		: cfg_(cfg), cfg_iterators_(cfg_.child_range("campaign"))
	{
	}

private:
	/**
	 * Collapses the description of an addon.
	 *
	 * @param grid                The grid of the item whose description to
	 *                            collapse.
	 */
	void collapse(tgrid& grid);

	/**
	 * Expands the description of an addon.
	 *
	 * @param grid                The grid of the item whose description to
	 *                            expand.
	 */
	void expand(tgrid& grid);

	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

	/**
	 * Creates a single campaign.
	 *
	 * @param pane                The pane to add the campaigns to.
	 * @param campaign            A config object containing the campaign info
	 *                            as send by campaignd.
	 */
	void create_campaign(tpane& pane, const config& campaign);

	/** Config which contains the list with the campaigns. */
	const config& cfg_;

	/**
	 * Debug iterators for testing with --new-widgets
	 */
	config::const_child_itors cfg_iterators_;

	/**
	 * Debug function to load a single campaign.
	 *
	 * @param pane                The pane to add the campaigns to.
	 */
	void load(tpane& pane);
};

} // namespace gui2

#endif
