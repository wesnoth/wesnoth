/*
   Copyright (C) 2003 - 2015 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef SERIALIZATION_UNICODE_CAST_HPP_INCLUDED
#define SERIALIZATION_UNICODE_CAST_HPP_INCLUDED

#include "ucs4_convert_impl.hpp"
#include <iostream>   //for std::cerr
#include <iterator>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_arithmetic.hpp>

namespace ucs4_convert_impl
{
	/**
	 * Transforms an output iterator to a writer for ucs4_convert_impl functions.
	 */
	template<typename oitor_t>
	struct iteratorwriter
	{
		oitor_t& out_;
		iteratorwriter(oitor_t& out) : out_(out) {}

		bool can_push(size_t /*count*/)
		{
			return true;
		}
		template<typename value_type>
		void push(value_type val)
		{
			*out_++  = val;
		}
	};
	template<typename Tret, typename Tcheck>
	struct enableif 
	{ 
		typedef Tcheck ignore; 
		typedef Tret type; 
	};
}

/**
 * @tparam TD Output, a collection type.
 * @tparam TS Input, a collection type.
 *
 * @return An instance of TD.
 */
template<typename TD , typename TS>
typename ucs4_convert_impl::enableif<TD, typename TS::value_type>::type unicode_cast(const TS& source)
//TD unicode_cast(const TS& source)
{
	using namespace ucs4_convert_impl;
	typedef typename convert_impl<typename TD::value_type>::type t_impl_writer;
	typedef typename convert_impl<typename TS::value_type>::type t_impl_reader;
	typedef typename std::back_insert_iterator<TD> t_outputitor;
	typedef typename TS::const_iterator t_inputitor;

	TD res;
	try
	{	
		t_outputitor inserter(res);
		iteratorwriter<t_outputitor> dst(inserter);
		t_inputitor i1 = source.begin();
		t_inputitor i2 = source.end();

		while(i1 != i2) {
			t_impl_writer::write (dst, t_impl_reader::read(i1, i2));
		}
	}
	catch(utf8::invalid_utf8_exception&) 
	{
		// TODO: use a ERR_.. stream but i dont know whether i can so to in header easily.
		std::cerr << "Failed to convert a string from " << t_impl_reader::get_name() << " to " << t_impl_writer::get_name() << "\n";
		return res;
	}
	return res;
}

/**
 * @tparam TD Output, a collection type.
 *
 * @return An instance of TD.
 */
template<typename TD>
TD unicode_cast(ucs4::char_t onechar)
{
	using namespace ucs4_convert_impl;
	typedef typename convert_impl<typename TD::value_type>::type t_impl_writer;
	typedef convert_impl<ucs4::char_t>::type t_impl_reader;
	typedef typename std::back_insert_iterator<TD> t_outputitor;
	
	TD res;
	try
	{
		t_outputitor inserter(res);
		iteratorwriter<t_outputitor> dst(inserter);
		t_impl_writer::write (dst, onechar);
	}
	catch(utf8::invalid_utf8_exception&) 
	{
		// TODO: use a ERR_.. stream but i dont know whether i can so to in header easily.
		std::cerr << "Failed to convert a string from " << t_impl_reader::get_name() << " to " << t_impl_writer::get_name() << "\n";
		return res;
	}
	return res;
}
#endif
