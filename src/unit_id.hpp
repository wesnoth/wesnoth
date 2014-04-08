/*
   Copyright (C) 2008 - 2014 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/


#ifndef UNIT_ID_HPP_INCLUDED
#define UNIT_ID_HPP_INCLUDED

#include <ctime>

#include <boost/noncopyable.hpp>

namespace n_unit {

	class id_manager : private boost::noncopyable {
		private:
			size_t next_id_;
			size_t fake_id_;
			static id_manager manager_;
			id_manager();
		public:
			static id_manager& instance();
			/** returns id for unit that is created */
			size_t next_id();

			size_t next_fake_id();

			/** Used for saving id to savegame */
			size_t get_save_id();
			void set_save_id(size_t);
			/** Clears id counter after game */
			void clear();
			void reset_fake();
	};

}

#endif
