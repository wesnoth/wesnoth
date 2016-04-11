/*
 Part of the Battle for Wesnoth Project http://www.wesnoth.org/

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

#include "gui/dialogs/dialog.hpp"
#include "gui/widgets/group.hpp"
#include <map>

namespace gui2
{
class tlabel;
class ttoggle_button;

class tlogging : public tdialog
{
public:
	/** Constructor. */
	tlogging();

	/**
	 * The display function.
	 *
	 * See @ref tdialog for more information.
	 */
	static void display(CVideo& video)
	{
		tlogging().show(video);
	}


private:
	void set_logger(const std::basic_string<char> log_domain);

	std::map<std::string, tgroup<std::string> > groups_;
	std::vector<std::string> domain_list, widget_id;
	std::string domain_list_end;

	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from tdialog. */
	void pre_show(twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);

};

} // end namespace gui2

#endif
