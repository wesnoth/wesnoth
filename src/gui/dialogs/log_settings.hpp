/*
   Copyright (C) 2017 by the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_LOGGING_HPP_INCLUDED
#define GUI_LOGGING_HPP_INCLUDED

#include "gui/dialogs/modal_dialog.hpp"
#include "gui/widgets/group.hpp"
#include <map>

namespace gui2
{
class label;
class toggle_button;
namespace dialogs
{

class log_settings : public modal_dialog
{
public:
	/** Constructor. */
	log_settings();

	/**
	 * The display function.
	 *
	 * See @ref modal_dialog for more information.
	 */
	static void display(CVideo& video)
	{
		log_settings().show(video);
	}


private:
	void set_logger(const std::basic_string<char> log_domain);

	std::map<std::string, group<std::string> > groups_;
	std::vector<std::string> domain_list_, widget_id_;

	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from modal_dialog. */
	void pre_show(window& window);

	/** Inherited from modal_dialog. */
	void post_show(window& window);

};

} // namespace dialogs
} // end namespace gui2

#endif
