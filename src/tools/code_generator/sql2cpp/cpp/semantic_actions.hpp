/*
	Copyright (C) 2013 by Pierre Talbot <ptalbot@mopong.net>
	Part of the Battle for Wesnoth Project http://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#ifndef CPP_SEMANTIC_ACTIONS_HPP
#define CPP_SEMANTIC_ACTIONS_HPP

#include "tools/code_generator/sql2cpp/sql/ast.hpp"

#include <fstream>
#include <string>
#include <set>

namespace cpp{

class semantic_actions
{
public:
	semantic_actions(const std::string& header, std::ofstream& generated, const std::string& output_dir);

	void type2string(std::string& res, sql::ast::column_type_ptr const& type);
	void header(std::string& res);
	void define_name(std::string& res, const std::string& class_name);
	void includes(std::string& res, sql::ast::column_list const& class_members, std::set<std::string>& included);
	void open_sink(const std::string& class_name);

private:
	std::string header_;
	std::ofstream& generated_;
	std::string output_dir_;
};

} // namespace cpp
#endif // CPP_SEMANTIC_ACTIONS_HPP