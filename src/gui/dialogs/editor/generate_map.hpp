/*
   Copyright (C) 2008 - 2017 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_EDITOR_GENERATE_MAP_HPP_INCLUDED
#define GUI_DIALOGS_EDITOR_GENERATE_MAP_HPP_INCLUDED

#include "gui/dialogs/modal_dialog.hpp"
#include <boost/optional/optional.hpp>
#include <cstdint>

class map_generator;
class display;

namespace gui2
{

class label;
class text_box;

namespace dialogs
{

/** The dialog for selecting which random generator to use in the editor. */
class editor_generate_map : public modal_dialog
{
public:
	editor_generate_map();

	void set_map_generators(std::vector<map_generator*> mg)
	{
		map_generators_ = mg;
	}

	std::vector<map_generator*> get_map_generators()
	{
		return map_generators_;
	}

	map_generator* get_selected_map_generator();

	void select_map_generator(map_generator* mg);

	boost::optional<uint32_t> get_seed();

private:
	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const override;

	/** Inherited from modal_dialog. */
	virtual void pre_show(window& window) override;

	/** Callback for generator list selection changes. */
	void do_generator_selected(window& window);

	/** Callback for the generator settings button. */
	void do_settings();

	/** Available map generators */
	std::vector<map_generator*> map_generators_;

	/** Last used map generator, must be in map_generators_ */
	map_generator* last_map_generator_;

	/** Current map generator index */
	int current_map_generator_;

	/** random seed integer input*/
	std::string random_seed_;
};

} // namespace dialogs
} // namespace gui2

#endif
