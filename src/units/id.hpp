/*
   Copyright (C) 2008 - 2018 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project https://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include <ctime>

namespace n_unit {

	struct unit_id
	{
		unit_id() : value(0) {}
		explicit unit_id(std::size_t val) : value(val) {}
		static const std::size_t highest_bit = static_cast<std::size_t>(1) << (sizeof(std::size_t) * 8 - 1);
		std::size_t value;

		bool is_fake() const { return (value & highest_bit) != 0; }
		bool is_empty() const { return !value; }

		static unit_id create_real(std::size_t val) { return unit_id(val); }
		static unit_id create_fake(std::size_t val) { return unit_id(val | highest_bit); }

		friend bool operator <(unit_id a, unit_id b) { return a.value < b.value; }
		friend bool operator <=(unit_id a, unit_id b) { return a.value <= b.value; }
		friend bool operator ==(unit_id a, unit_id b) { return a.value == b.value; }
		friend bool operator >=(unit_id a, unit_id b) { return a.value >= b.value; }
		friend bool operator >(unit_id a, unit_id b) { return a.value > b.value; }
	};

	class id_manager
	{
	private:
		std::size_t next_id_;
		std::size_t fake_id_;
		static id_manager manager_;
	public:
		id_manager(std::size_t next_id) : next_id_(next_id) , fake_id_(0) {}
		/** returns id for unit that is created */
		unit_id next_id();

		unit_id next_fake_id();

		/** Used for saving id to savegame */
		std::size_t get_save_id() const;
		void set_save_id(std::size_t);
		/** Clears id counter after game */
		void clear();
		void reset_fake();
		static id_manager& global_instance() {return manager_;}
	};

}
