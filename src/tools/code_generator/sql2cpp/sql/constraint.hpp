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

#ifndef SQL_CONSTRAINT_HPP
#define SQL_CONSTRAINT_HPP

#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>

namespace sql{

struct constraint_visitor;

struct base_constraint
{
	base_constraint(const std::string& name)
	: name(name)
	{}

	virtual ~base_constraint() {}

	virtual void accept(const boost::shared_ptr<constraint_visitor>& visitor) const = 0;

	std::string name;
};

template <class sql_constraint>
struct base_constraint_crtp : base_constraint
{
	base_constraint_crtp(const std::string& name)
	: base_constraint(name)
	{}
	
	virtual void accept(const boost::shared_ptr<constraint_visitor>& visitor) const;
};

struct primary_key : base_constraint_crtp<primary_key>
{
	typedef base_constraint_crtp<primary_key> base;

	primary_key(const std::string& name, const std::vector<std::string>& keys)
	: base(name)
	, keys(keys)
	{}

	std::vector<std::string> keys;
};

struct foreign_key : base_constraint_crtp<foreign_key>
{
	typedef base_constraint_crtp<foreign_key> base;

	foreign_key(const std::string& name, const std::vector<std::string>& keys, const ast::key_references& refs)
	: base(name)
	, keys(keys)
	, refs(refs)
	{}

	std::vector<std::string> keys;
	ast::key_references refs;
};

struct constraint_visitor
{
	virtual ~constraint_visitor() {}

	virtual void visit(const primary_key&) = 0;
	virtual void visit(const foreign_key&) = 0;
};

template <class constraint_type>
void base_constraint_crtp<constraint_type>::accept(const boost::shared_ptr<constraint_visitor>& visitor) const
{
	visitor->visit(*static_cast<const constraint_type*>(this));
}

} // namespace sql

#endif // SQL_CONSTRAINT_HPP
