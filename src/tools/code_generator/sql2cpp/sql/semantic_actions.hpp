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

#ifndef SQL_SEMANTIC_ACTIONS_HPP
#define SQL_SEMANTIC_ACTIONS_HPP

#include "tools/code_generator/sql2cpp/sql/ast.hpp"

#include <boost/make_shared.hpp>
#include <boost/ref.hpp>

namespace sql{

class semantic_actions
{
public:
	template <class T, class U>
	void make_ptr(boost::shared_ptr<U>& res) const
	{
		res = boost::make_shared<T>();
	}
	
	template <class T, class U, class A>
	void make_ptr(boost::shared_ptr<U>& res, A const& arg) const
	{
		res = boost::make_shared<T>(arg);
	}
	
	template <class T, class U, class A1, class A2>
	void make_ptr(boost::shared_ptr<U>& res, A1 const& arg1, A2 const& arg2) const
	{
		res = boost::make_shared<T>(arg1, arg2);
	}

	template <class T, class U, class A1, class A2, class A3>
	void make_ptr(boost::shared_ptr<U>& res, A1 const& arg1, A2 const& arg2, A3 const& arg3) const
	{
		res = boost::make_shared<T>(arg1, arg2, arg3);
	}

	void make_unsigned_numeric(boost::shared_ptr<type::numeric_type>& res) const
	{
		res->is_unsigned = true;
	}

	/**
	@param success is set to false to make the parser fails.
	*/
	void get_table_by_name(std::vector<ast::table>::iterator &res, 
		std::vector<ast::table>& tables, 
		const std::string& name,
		bool &success) const
	{
		res = tables.begin();
		while(res != tables.end() && !(res->table_identifier == name))
		{
			++res;
		}
		if(res == tables.end())
		{
			std::cerr << "Try to alter the table " << name << " without having previously defined it." << std::endl;
			success = false;
		}
		else
		{
			success = true;
		}
	}

	/**
	@post We replace the constraint if it already exists in the table constraints, otherwise we add it.
	*/
	void alter_table_add_constraint(ast::table& table, 
		const boost::shared_ptr<sql::base_constraint>& constraint_to_add) const
	{
		bool to_add = true;
		for(std::size_t i = 0; i < table.constraints.size() && to_add; ++i)
		{
			if(table.constraints[i]->name == constraint_to_add->name)
			{
				to_add = false;
				table.constraints[i] = constraint_to_add;
			}
		}
		if(to_add)
		{
			table.constraints.push_back(constraint_to_add);
		}
	}
};

} // namespace sql
#endif // SQL_SEMANTIC_ACTIONS_HPP