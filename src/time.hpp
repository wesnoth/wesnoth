/* $Id$ */
/*
   Copyright (C) 2008 - 2010 by Pauli Nieminen <paniemin@cc.hut.fi>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef TIME_HPP_INCLUDED
#define TIME_HPP_INCLUDED
#include <cstdlib>

namespace ntime {

	class source
	{
		size_t frame_time_;
		size_t current_time_;
		static source time_source_;
		source();
		source(const source&);
		void operator=(const source&);

	public:
		/**
		 * Called in begin of each frame
		 * @return How many milliseconds this frame took?
		 */
		size_t start_frame(const bool limit = true);
		void set_frame_rate(const size_t fps);
		void set_frame_time(const size_t ms);

		size_t get_time() const;

		static source& get_source();

	};
}

#endif
