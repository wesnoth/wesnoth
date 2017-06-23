/*
   Copyright (C) 2017 by the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include <iterator>  //input_iterator_tag
#include <utility>   //pair
#include <cstddef>   //ptrdiff_t
#include <cassert>   //assert

#include "unicode_types.hpp"

namespace ucs4
{
	template<typename string_type, typename update_implementation>
	class iterator_base
	{
	public:
		typedef std::input_iterator_tag iterator_category;
		typedef ucs4::char_t value_type;
		typedef ptrdiff_t difference_type;
		typedef ucs4::char_t* pointer;
		typedef ucs4::char_t& reference;

		iterator_base(const string_type& str)
			: current_char(0)
			, string_end(str.end())
			, current_substr(std::make_pair(str.begin(), str.begin()))
		{
			update();
		}

		iterator_base(typename string_type::const_iterator const &begin, typename string_type::const_iterator const &end)
			: current_char(0)
			, string_end(end)
			, current_substr(std::make_pair(begin, begin))
		{
			update();
		}

		static iterator_base begin(const string_type& str)
		{
			return iterator_base(str.begin(), str.end());
		}

		static iterator_base end(const string_type& str)
		{
			return iterator_base(str.end(), str.end());
		}

		bool operator==(const iterator_base& a) const
		{
			return current_substr.first == a.current_substr.first;
		}

		bool operator!=(const iterator_base& a) const
		{
			return ! (*this == a);
		}

		iterator_base& operator++()
		{
			current_substr.first = current_substr.second;
			update();
			return *this;
		}

		ucs4::char_t operator*() const
		{
			return current_char;
		}

		bool next_is_end() const
		{
			if(current_substr.second == string_end)
				return true;
			return false;
		}

		const std::pair<typename string_type::const_iterator, typename string_type::const_iterator>& substr() const
		{
			return current_substr;
		}
	private:
		void update()
		{
			assert(current_substr.first == current_substr.second);
			if(current_substr.first == string_end)
				return;
			current_char = update_implementation::read(current_substr.second, string_end);
		}

		ucs4::char_t current_char;
		typename string_type::const_iterator string_end;
		std::pair<typename string_type::const_iterator, typename string_type::const_iterator> current_substr;
	};

}
