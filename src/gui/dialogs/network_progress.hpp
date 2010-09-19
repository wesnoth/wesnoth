/* $Id$ */
/*
   Copyright (C) 2010 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_NETWORK_PROGRESS_HPP_INCLUDED
#define GUI_DIALOGS_NETWORK_PROGRESS_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"
#include "tstring.hpp"
#include "network_progress_data.hpp"

namespace gui2 {

class tnetwork_progress : public tdialog
{
public:
	tnetwork_progress();
	
	~tnetwork_progress();
	
	void set_progress_object(network::progress_data& pd);

private:
	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);

	void poll_progress(twindow& window);
	
	void abort(twindow& window);

private:
	/** Inherited from tdialog, implemented by REGISTER_WINDOW. */
	virtual const std::string& window_id() const;

	network::progress_data* pd_;
	unsigned long poll_progress_timer_;
};

} // namespace gui2

#endif

