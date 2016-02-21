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

#pragma once

#include "gui/auxiliary/window_builder/control.hpp"

namespace gui2
{

class tcontrol;

namespace implementation
{

struct tbuilder_combobox : public tbuilder_control
{
public:
	explicit tbuilder_combobox(const config& cfg);

	using tbuilder_control::build;

	twidget* build() const;

private:
	std::string retval_id_;
	int retval_;
	std::vector<std::string> options_;
};

} // namespace implementation

} // namespace gui2
