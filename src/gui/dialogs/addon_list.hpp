/*
   Copyright (C) 2008 - 2016 by Mark de Wever <koraq@xs4all.nl>
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

#include "addon/info.hpp"
#include "addon/state.hpp"

#include "gui/dialogs/dialog.hpp"
#include "gui/widgets/generator.hpp"
#include "gui/widgets/pane.hpp"

#include "config.hpp" // needed for config::const_child_itors

namespace gui2
{
class ttext_;
class ttext_box;
class pane;
class tselectable_;

/** Shows the list of addons on the server. */
class taddon_list : public tdialog
{
public:
	explicit taddon_list(const config& cfg);

private:
	void register_sort_button(twindow& window, const std::string& id, const tgenerator_::torder_func& up, const tgenerator_::torder_func& down);
	void register_sort_button_alphabetical(twindow& window, const std::string& id, const std::string& prop_id);
	void register_sort_button_numeric(twindow& window, const std::string& id, const std::string& prop_id);

	void on_order_button_click(twindow& window, const tgenerator_::torder_func& up, const tgenerator_::torder_func& down, twidget& w);
	void on_filtertext_changed(ttext_* textbox, const std::string& text);

	std::vector<tselectable_*> orders_;

	void on_addon_select(twindow& window);
	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

	/** Config which contains the list with the campaigns. */
	const config& cfg_;

	/**
	 * Debug iterators for testing with --new-widgets
	 */
	config::const_child_itors cfg_iterators_;

	addons_list addons_;

	addons_tracking_list tracking_info_;

	std::vector<std::string> ids_;
	
	void browse_url_callback(ttext_box& url_box);
	void copy_url_callback(ttext_box& url_box);
};

} // namespace gui2

#endif
