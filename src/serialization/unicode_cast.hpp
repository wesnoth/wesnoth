/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
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

#include "ucs4_convert_impl.hpp"
#include <iostream>   //for std::cerr
#include <iterator>

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
	typedef typename convert_impl<typename TD::value_type>::type impl_writer;
	typedef typename convert_impl<typename TS::value_type>::type impl_reader;
	typedef typename std::back_insert_iterator<TD> output_itor;
	typedef typename TS::const_iterator input_itor;

	TD res;
	try
	{
		output_itor inserter(res);
		iteratorwriter<output_itor> dst(inserter);
		input_itor i1 = source.begin();
		input_itor i2 = source.end();

		while(i1 != i2) {
			impl_writer::write (dst, impl_reader::read(i1, i2));
		}
	}
	catch(utf8::invalid_utf8_exception&)
	{
		// TODO: use a ERR_.. stream but I don't know whether I can do so in header easily.
		std::cerr << "Failed to convert a string from " << impl_reader::get_name() << " to " << impl_writer::get_name() << "\n";
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
	typedef typename convert_impl<typename TD::value_type>::type impl_writer;
	typedef convert_impl<ucs4::char_t>::type impl_reader;
	typedef typename std::back_insert_iterator<TD> output_itor;

	TD res;
	try
	{
		output_itor inserter(res);
		iteratorwriter<output_itor> dst(inserter);
		impl_writer::write (dst, onechar);
	}
	catch(utf8::invalid_utf8_exception&)
	{
		// TODO: use a ERR_.. stream but I don't know whether I can do so in header easily.
		std::cerr << "Failed to convert a string from " << impl_reader::get_name() << " to " << impl_writer::get_name() << "\n";
		return res;
	}
	return res;
}
