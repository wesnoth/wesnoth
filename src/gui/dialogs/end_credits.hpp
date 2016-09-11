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

#ifndef GUI_DIALOGS_END_CREDITS_HPP_INCLUDED
#define GUI_DIALOGS_END_CREDITS_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"

#include <memory>
#include <string>
#include <vector>

class display;

namespace gui2
{

class tend_credits : public tdialog
{
public:
	tend_credits(const std::vector<std::string>& text, const std::vector<std::string>& backgrounds);

	static void display(const std::vector<std::string>& text, const std::vector<std::string>& backgrounds, CVideo& video)
	{
		tend_credits(text, backgrounds).show(video);
	}

private:
	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from tdialog. */
	void pre_show(twindow& window);

	const std::vector<std::string>& text_;

	std::vector<std::string> backgrounds_;
};

} // namespace gui2

#endif /* ! GUI_DIALOGS_END_CREDITS_HPP_INCLUDED */
