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

#include <boost/fusion/include/std_pair.hpp> 

#include <boost/algorithm/string.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/lexical_cast.hpp>

#include "tools/code_generator/sql2cpp/sql_type.hpp"
#include "tools/code_generator/sql2cpp/sql_type_constraint.hpp"
#include "tools/code_generator/sql2cpp/sql_constraint.hpp"


#include <iostream>
#include <fstream>
#include <string>
#include <set>
#include <utility>

template <class synthesized, class inherited = void>
struct attribute
{
	typedef synthesized s_type;
	typedef inherited i_type;
	typedef s_type type(i_type);
};

template <class synthesized>
struct attribute <synthesized, void>
{
	typedef synthesized s_type;
	typedef s_type type();
};

template <class inherited>
struct attribute <void, inherited>
{
	typedef inherited i_type;
	typedef void type(i_type);
};


namespace sql{

class semantic_actions
{
public:
	typedef attribute<boost::shared_ptr<sql::type::base_type> > column_type_attribute;
	typedef attribute<boost::shared_ptr<sql::type::numeric_type> > numeric_type_attribute;
	typedef attribute<std::string> default_value_attribute;
	typedef attribute<boost::shared_ptr<sql::base_type_constraint> > type_constraint_attribute;
	typedef attribute<ast::sql_column> column_attribute;
	typedef attribute<std::vector<ast::sql_column> > create_table_columns_attribute;
	typedef attribute<ast::sql_table> create_table_attribute;
	typedef attribute<ast::sql_table> create_statement_attribute;

	// Alter statement.
	typedef attribute<void, std::vector<ast::sql_table>&> alter_statement_attribute;
	typedef attribute<void, std::vector<ast::sql_table>&> alter_table_attribute;
	typedef attribute<void, ast::sql_table&> alter_table_add_attribute;
	
	typedef attribute<void, std::vector<ast::sql_table>&> statement_attribute;
	typedef attribute<std::vector<ast::sql_table> > program_attribute;
	typedef attribute<std::vector<boost::shared_ptr<sql::constraint::base_constraint> > > table_constraints_attribute;
	typedef attribute<boost::shared_ptr<sql::constraint::base_constraint> > constraint_definition_attribute;
	typedef attribute<boost::shared_ptr<sql::constraint::base_constraint>, std::string> primary_key_constraint_attribute;
	typedef attribute<boost::shared_ptr<sql::constraint::base_constraint>, std::string> foreign_key_constraint_attribute;
	typedef attribute<sql::constraint::key_references> reference_definition_attribute;

	template<class T>
	void make_column_type(boost::shared_ptr<sql::type::base_type>& res) const
	{
		res = boost::make_shared<T>();
	}

	void make_varchar_type(boost::shared_ptr<sql::type::base_type>& res, std::size_t length) const
	{
		res = boost::make_shared<sql::type::varchar>(length);
	}

	template<class T>
	void make_numeric_type(boost::shared_ptr<sql::type::numeric_type>& res) const
	{
		res = boost::make_shared<T>();
		res->is_unsigned = false;
	}
   
	void make_unsigned_numeric(boost::shared_ptr<sql::type::numeric_type>& res) const
	{
		res->is_unsigned = true;
	}

	template <class T>
	void make_type_constraint(boost::shared_ptr<sql::base_type_constraint>& res) const
	{
		res = boost::make_shared<T>();
	}

	void make_default_value_constraint(boost::shared_ptr<sql::base_type_constraint>& res, const std::string& default_value) const
	{
		res = boost::make_shared<sql::default_value>(default_value);
	}

	template <class T>
	void make_constraint(boost::shared_ptr<sql::constraint::base_constraint>& res, const std::string& name) const
	{
		res = boost::make_shared<T>(boost::ref(name));
	}

	void make_pk_constraint(boost::shared_ptr<sql::constraint::base_constraint>& res, 
							const std::string& name, 
							const std::vector<std::string>& keys) const
	{
		res = boost::make_shared<sql::constraint::primary_key>(name, keys);
	}

	void make_fk_constraint(boost::shared_ptr<sql::constraint::base_constraint>& res, 
							const std::string& name, 
							const std::vector<std::string>& keys,
							const sql::constraint::key_references& refs) const
	{
		res = boost::make_shared<sql::constraint::foreign_key>(name, keys, refs);
	}

	/**
	@param success is set to false to make the parser fails.
	*/
	void get_table_by_name(std::vector<ast::sql_table>::iterator &res, 
		std::vector<ast::sql_table>& tables, 
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
	void alter_table_add_constraint(ast::sql_table& table, 
		const boost::shared_ptr<sql::constraint::base_constraint>& constraint_to_add)
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