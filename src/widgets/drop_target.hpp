/*
   Copyright (C) 2008 - 2016 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef DROP_TARGET_H_INCLUDED
#define DROP_TARGET_H_INCLUDED

#include "SDL.h"

#include <map>

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

namespace gui {

	class drop_target;
	typedef boost::shared_ptr<drop_target> drop_target_ptr;

	typedef int drop_target_group;
	class drop_group_manager;
	typedef boost::shared_ptr<drop_group_manager> drop_group_manager_ptr;

	/**
	 * Handles droping for drag able ui items.
	 * Widget class just has to inherit from drop_target,
	 * call constructor and handle_drop.
	 **/
	class drop_target : public boost::noncopyable {
		typedef std::multimap<drop_target_group, drop_target*> drop_groups;
		typedef std::map<drop_target_group, int> target_id;
		static drop_groups groups_;
		static target_id next_id_;
		static drop_target_group next_group_;

		int next_free_id(const drop_target_group& group) const;
		bool hit_rect(const SDL_Rect& hit_loc, const int not_id) const;
		drop_groups::iterator find_this() const;

		const SDL_Rect& loc_;
		const int id_;
		drop_group_manager_ptr group_;

		static drop_target_group create_group();
		static void delete_group(const drop_target_group id);

		protected:
		/**
		 * Used to check if static storages are empty.
		 * Only for testing.
		 **/
		static bool empty()	{ return groups_.empty() && next_id_.empty(); }
		/**
		 * Called by widget object when droping happens.
		 *
		 * Droping over multiple widget objects in same group
		 * is undefined.
		 *
		 * @return: id which widget was hit when droping
		 **/
		int handle_drop();

		public:
		/**
		 * Registers drop target and saves reference to location.
		 **/
		drop_target(const drop_group_manager_ptr group, const SDL_Rect& loc);
		~drop_target();

		int get_id() const;
		/**
		 * Checks if id matches id for this object.
		 * Used by for_each/boost:bind
		 **/
		bool is_this_id(const int id) const;

		friend class drop_group_manager;

	};

	/**
	 * Used to create and destroy drop groups.
	 * To create drop_target widgets one has to have
	 * drop_group_manager stored in drop_group_manager_ptr.
	 **/
	class drop_group_manager : public boost::noncopyable {
		const drop_target_group group_id_;
		public:
		drop_group_manager();
		~drop_group_manager();

		drop_target_group get_group_id() const;
	};

}
#endif
