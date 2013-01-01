/* $Id$ */
/*
   Copyright (C) 2008 - 2013 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_ADDON_CONNECT_HPP_INCLUDED
#define GUI_DIALOGS_ADDON_CONNECT_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"

namespace gui2 {

/** Addon connect dialog. */
class taddon_connect
	: public tdialog
{
public:
	/**
	 * Constructor.
	 *
	 * @param host_name [in]      The initial value for the host_name.
	 * @param host_name [out]     The final value of the host_name if the
	 *                            dialog returns @ref twindow::OK or 3
	 *                            undefined otherwise.
	 * @param allow_updates       Sets @ref allow_updates_.
	 * @param allow_remove        Sets @ref allow_remove_.
	 */
	taddon_connect(std::string& host_name
			, const bool allow_updates
			, const bool allow_remove);

private:

	/** Enable the update addon button? */
	bool allow_updates_;

	/** Enable the addon remove button? */
	bool allow_remove_;

	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);
};

} // namespace gui2

#endif
