/*
   Copyright (C) 2008 - 2014 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_AUXILIARY_WINDOW_BUILDER_PASSWORD_BOX_HPP_INCLUDED
#define GUI_AUXILIARY_WINDOW_BUILDER_PASSWORD_BOX_HPP_INCLUDED

#include "gui/auxiliary/window_builder/control.hpp"

namespace gui2
{

namespace implementation
{

// copy & paste from tbuilder_text_box...
// does it make more sense to inherit from it?
struct tbuilder_password_box : public tbuilder_control
{
public:
	explicit tbuilder_password_box(const config& cfg);

	using tbuilder_control::build;

	twidget* build() const;

private:
	std::string history_;
};

} // namespace implementation

} // namespace gui2

#endif
