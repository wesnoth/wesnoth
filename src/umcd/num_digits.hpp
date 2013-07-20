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

#ifndef UMCD_NUM_DIGITS_HPP
#define UMCD_NUM_DIGITS_HPP

template <std::size_t v>
struct num_digits_impl2
{
   static const std::size_t value = 1 + num_digits_impl2<v/10>::value;
};

template <>
struct num_digits_impl2<10>
{
   static const std::size_t value = 2;
};

template <>
struct num_digits_impl2<0>
{
   static const std::size_t value = 0;
};

template <std::size_t v>
struct num_digits_impl
{
   static const std::size_t value = num_digits_impl2<v>::value;
};

template <>
struct num_digits_impl<0>
{
   static const std::size_t value = 1;
};

template <std::size_t v>
struct num_digits
{
   static const std::size_t value = num_digits_impl<v>::value;
};

template <std::size_t v>
const std::size_t num_digits<v>::value;

#endif // UMCD_NUM_DIGITS_HPP
