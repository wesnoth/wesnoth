/* $Id$ */
/*
   Copyright (C) 2010 - 2011 by Ignacio R. Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_ADDON_DESCRIPTION_HPP_INCLUDED
#define GUI_DIALOGS_ADDON_DESCRIPTION_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"

#include <vector>

struct addon_info
{
	addon_info()
		: name()
		, description()
		, icon()
		, version()
		, author()
		, sizestr()
		, translations()
	{
	}

	std::string name;
	std::string description;
	std::string icon;
	std::string version;
	std::string author;
	std::string sizestr;
	std::vector<std::string> translations;
};

namespace gui2 {

class taddon_description : public tdialog
{
public:
	taddon_description(const addon_info& ainfo)
		: ainfo_(ainfo) {}

	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

private:
	addon_info ainfo_;
};

}

#endif
