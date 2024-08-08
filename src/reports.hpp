/*
	Copyright (C) 2003 - 2024
	by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

#include "display_context.hpp"

#include <vector>

#include "utils/optional_reference.hpp"

//this module is responsible for outputting textual reports of
//various game and unit statistics

class gamemap;
class team;
class tod_manager;
class unit_map;

class display;

namespace wb {
	class manager;
}

namespace events {
	class mouse_handler;
}

class reports
{
public:

	class context
	{
	public:
		context(const display_context & dc, const display & disp, const tod_manager & tod, std::shared_ptr<wb::manager> wb, utils::optional_reference<events::mouse_handler> mhb) : dc_(dc), disp_(disp), tod_(tod), wb_(wb), mhb_(mhb) {}

		const std::vector<team> & teams() const { return dc_.teams(); }
		const unit_map & units() const { return dc_.units(); }
		const gamemap & map() const { return dc_.map(); }

		const display_context & dc() const { return dc_; }
		const display & screen() const { return disp_; }
		const tod_manager & tod() const { return tod_; }
		std::shared_ptr<wb::manager> wb() const { return wb_; }
		utils::optional_reference<events::mouse_handler> mhb() const { return mhb_; }

	private:
		const display_context& dc_;
		const display& disp_;
		const tod_manager& tod_;
		std::shared_ptr<wb::manager> wb_;
		utils::optional_reference<events::mouse_handler> mhb_;
	};

	struct generator
	{
		virtual config generate(const context& ct) = 0;
		virtual ~generator() {}
	};

	void register_generator(const std::string &name, generator *);

	config generate_report(const std::string &name, const context& ct, bool only_static = false);

	const std::set<std::string> &report_list();

	using generator_function = std::function<config(const reports::context&)>;

	typedef std::map<std::string, std::shared_ptr<reports::generator>> dynamic_report_generators;

private:

	std::set<std::string> all_reports_;

	dynamic_report_generators dynamic_generators_;

};
