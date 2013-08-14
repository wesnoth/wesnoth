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


#include "tools/code_generator/sql2cpp/cpp/semantic_actions.hpp"

#include "tools/code_generator/sql2cpp/cpp/type_visitors.hpp"
#include "tools/code_generator/sql2cpp/utility.hpp"

#include <boost/make_shared.hpp>
#include <boost/algorithm/string.hpp>

namespace cpp{

semantic_actions::semantic_actions(const std::string& header, std::ofstream& generated, const std::string& output_dir)
: header_(header) 
, generated_(generated)
, output_dir_(output_dir)
{}

void semantic_actions::type2string(std::string& res, sql::ast::column_type_ptr const& type)
{
	boost::shared_ptr<sql::type::type_visitor> visitor = boost::make_shared<type2string_visitor>(boost::ref(res));
	type->accept(visitor);
}

void semantic_actions::header(std::string& res)
{
	res = header_;
}

void semantic_actions::define_name(std::string& res, const std::string& class_name)
{
	res = "UMCD_POD_" + boost::to_upper_copy(class_name) + "_HPP";
}

void semantic_actions::includes(std::string& res, sql::ast::column_list const& class_members, std::set<std::string>& included)
{
	for(std::size_t i = 0; i < class_members.size(); ++i)
	{
		std::string preproc_include;
		boost::shared_ptr<sql::type::type_visitor> visitor = boost::make_shared<type2header_visitor>(boost::ref(preproc_include));
		class_members[i].sql_type->accept(visitor);
		if(included.insert(preproc_include).second && !preproc_include.empty())
		{
			res += preproc_include + "\n";
		}
	}
}

void semantic_actions::open_sink(const std::string& class_name)
{
	generated_.close();
	std::string filepath = output_dir_ + boost::to_lower_copy(class_name) + ".hpp";
	generated_.open(filepath.c_str());
	if(!generated_.is_open())
	{
		throw std::runtime_error("Could not open the file " + filepath); 
	}
}

} // namespace cpp
