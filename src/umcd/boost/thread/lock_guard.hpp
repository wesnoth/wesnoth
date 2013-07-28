/* Distributed under the Boost Software License, Version 1.0. (See
	accompanying file LICENSE_1_0.txt or copy at
	http://www.boost.org/LICENSE_1_0.txt)
	(C) Copyright 2007 Anthony Williams
	(C) Copyright 2011-2012 Vicente J. Botet Escriba
*/
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

// Caution: Delete this file when the supported Boost version will be >= 1.53.

#ifndef UMCD_LOCK_GUARD_HPP
#define UMCD_LOCK_GUARD_HPP

template <typename Mutex>
class lock_guard
{
private:
	Mutex& m;

public:
	typedef Mutex mutex_type;

	explicit lock_guard(Mutex& m_) :
		m(m_)
	{
		m.lock();
	}

	~lock_guard()
	{
		m.unlock();
	}
};

#endif // UMCD_LOCK_GUARD_HPP