/*
   Copyright (C) 2008 - 2016 by Jörg Hinrichs <joerg.hinrichs@alice-dsl.de>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_LOAD_GAME_HPP_INCLUDED
#define GUI_DIALOGS_LOAD_GAME_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/text.hpp"
#include "save_index.hpp"
#include "tstring.hpp"

namespace gui2
{

class tgame_load : public tdialog
{
public:
	explicit tgame_load(const config& cache_config);

	const std::string& filename() const
	{
		return filename_;
	}
	bool change_difficulty() const
	{
		return change_difficulty_;
	}
	bool show_replay() const
	{
		return show_replay_;
	}
	bool cancel_orders() const
	{
		return cancel_orders_;
	}
	const config& summary()
	{
		return summary_;
	}

protected:
	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);

private:
	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	bool filter_text_changed(ttext_* textbox, const std::string& text);
	void list_item_clicked(twindow& window);
	void delete_button_callback(twindow& window);

	void display_savegame(twindow& window);
	void evaluate_summary_string(std::stringstream& str,
								 const config& cfg_summary);

	bool compare_name(unsigned i1, unsigned i2) const;
	bool compare_date(unsigned i1, unsigned i2) const;
	bool compare_name_rev(unsigned i1, unsigned i2) const;
	bool compare_date_rev(unsigned i1, unsigned i2) const;

	void fill_game_list(twindow& window,
						std::vector<savegame::save_info>& games);

	tfield_text* txtFilter_;
	tfield_bool* chk_change_difficulty_;
	tfield_bool* chk_show_replay_;
	tfield_bool* chk_cancel_orders_;

	std::string filename_;
	bool change_difficulty_;
	bool show_replay_;
	bool cancel_orders_;

	std::vector<savegame::save_info> games_;
	const config& cache_config_;

	std::vector<std::string> last_words_;

	config summary_;
};
}

#endif
