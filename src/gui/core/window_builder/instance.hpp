/*
   Copyright (C) 2012 - 2018 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/core/window_builder.hpp"

#include "config.hpp"

namespace gui2
{

namespace implementation
{

struct builder_instance : public builder_widget
{
	explicit builder_instance(const config& cfg);

	widget* build() const;

	widget* build(const replacements_map& replacements) const;

	/**
	 * Holds a copy of the cfg parameter in the constructor.
	 *
	 * This is used when instantiating a spacer, it can still use the
	 * parameters originally sent.
	 */
	config configuration;
};

} // namespace implementation

} // namespace gui2
