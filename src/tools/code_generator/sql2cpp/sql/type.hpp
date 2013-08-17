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

#ifndef SQL_TYPE_HPP
#define SQL_TYPE_HPP

#include <boost/shared_ptr.hpp>

namespace sql{
namespace type{

struct type_visitor;

struct base_type
{
	base_type(){}
	virtual ~base_type(){}
	virtual void accept(const boost::shared_ptr<type_visitor>& visitor) const = 0;
};

/* inherit_from help to choose another intermediate type than base_type to inherit from.
That can help to have a type erasure also use to aggregate data (just like numeric_type
that represents a rule in the parser).
*/
template <class sql_type, class inherit_from = base_type>
struct base_type_crtp : inherit_from
{
	virtual void accept(const boost::shared_ptr<type_visitor>& visitor) const;
};

struct numeric_type : base_type
{
	bool is_unsigned;
	
	numeric_type()
	: is_unsigned(false)
	{}

	virtual void accept(const boost::shared_ptr<type_visitor>& visitor) const = 0;
};

struct smallint : base_type_crtp<smallint, numeric_type>
{};

struct integer : base_type_crtp<integer, numeric_type>
{};

struct text : base_type_crtp<text>
{};

struct date : base_type_crtp<date>
{};

struct varchar : base_type_crtp<varchar>
{
	std::size_t length;

	varchar(std::size_t length)
	: length(length)
	{}
};

struct type_visitor
{
	virtual ~type_visitor() {}
	virtual void visit(const smallint&) = 0;
	virtual void visit(const integer&) = 0;
	virtual void visit(const text&) = 0;
	virtual void visit(const date&) = 0;
	virtual void visit(const varchar&) = 0;
};

template <class sql_type, class inherit_from>
void base_type_crtp<sql_type, inherit_from>::accept(const boost::shared_ptr<type_visitor>& visitor) const
{
	visitor->visit(*static_cast<const sql_type*>(this));
}

}} // namespace sql::type

#endif // SQL_TYPE_HPP
