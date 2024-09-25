/*
	Copyright (C) 2008 - 2024
	by Mark de Wever <koraq@xs4all.nl>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "gui/core/static_registry.hpp"

#include "gui/core/log.hpp"

#include <map>
#include <set>
#include <string>
#include <tuple>
#include <utility>

namespace gui2
{
std::set<std::string>& registered_window_types()
{
	static std::set<std::string> result;
	return result;
}

void register_window(const std::string& id)
{
	bool added = false;
	std::tie(std::ignore, added) = registered_window_types().emplace(id);

	if(!added) {
		WRN_GUI_P << "Window '" << id << "' already registered. Ignoring.";
	}
}

std::map<std::string, registered_widget_parser>& registered_widget_types()
{
	static std::map<std::string, registered_widget_parser> result;
	return result;
}

void register_widget(const std::string& type, widget_parser_t f, const char* key)
{
	registered_widget_types()[type] = {std::move(f), key};
}

std::map<std::string, widget_builder_func_t>& widget_builder_lookup()
{
	static std::map<std::string, widget_builder_func_t> result;
	return result;
}

void register_widget_builder(const std::string& type, widget_builder_func_t functor)
{
	widget_builder_lookup().emplace(type, functor);
}

} // namespace gui2
