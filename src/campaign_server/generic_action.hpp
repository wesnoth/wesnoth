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

#ifndef SERVER_GENERIC_ACTION_HPP
#define SERVER_GENERIC_ACTION_HPP


template <class ReturnType, class... Args>
class generic_action
{
public:
   typedef ReturnType return_type;

   virtual return_type execute(Args... args) = 0;
};

#endif // SERVER_GENERIC_ACTION_HPP
