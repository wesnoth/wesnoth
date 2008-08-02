/* $Id:$ */
/*
   Copyright (C) 2008 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
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
		const drop_target_group group_;
		protected:
		int handle_drop();

		public:
		drop_target(const drop_target_group& group, const SDL_Rect& loc);
		~drop_target();
		drop_target(const drop_target& o);
		int get_id() const;
		bool is_this_id(const int id) const;
		static drop_target_group create_group();
	};
}
#endif
