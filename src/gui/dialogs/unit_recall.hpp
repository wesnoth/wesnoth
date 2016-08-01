/*
   Copyright (C) 2016 by the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_UNIT_RECALL_HPP_INCLUDED
#define GUI_DIALOGS_UNIT_RECALL_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"
#include "gui/widgets/group.hpp"
#include "gui/widgets/generator.hpp"
#include "units/race.hpp"
#include "units/ptr.hpp"

#include <memory>
#include <string>
#include <vector>

class team;
class unit_type;

namespace gui2
{

class ttext_;

class tunit_recall : public tdialog
{
	typedef std::shared_ptr<std::vector<unit_const_ptr> > recalls_ptr_vector;

public:
	tunit_recall(recalls_ptr_vector& recall_list, team& team);

	int get_selected_index() const
	{
		return selected_index_;
	}

private:
	recalls_ptr_vector& recall_list_;

	team& team_;

	int selected_index_;

	std::vector<std::string> filter_options_;
	std::vector<std::string> last_words_;

	template<typename T>
	void init_sorting_option(std::vector<tgenerator_::torder_func>& order_funcs, T filter_on);

	/** Callbacks */
	void list_item_clicked(twindow& window);
	void filter_text_changed(ttext_* textbox, const std::string& text);
	void dismiss_unit(twindow& window);
	void show_help(twindow& window);

	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from tdialog. */
	void pre_show(twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);
};

} // namespace gui2

#endif /* ! GUI_DIALOGS_UNIT_RECALL_HPP_INCLUDED */
