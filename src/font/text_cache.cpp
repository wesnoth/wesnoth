/*
   Copyright (C) 2016 - 2017 by Chris Beck<render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "font/text_cache.hpp"
#include "sdl/surface.hpp"

#include <algorithm>

namespace font {

text_cache::text_list text_cache::cache_;

unsigned int text_cache::max_size_ = 50;

void text_cache::resize(unsigned int size)
{
	// DBG_FT << "Text cache: resize from: " << max_size_ << " to: "
	// 	<< size << " items in cache: " << cache_.size() << '\n';

	while(size < cache_.size()) {
		cache_.pop_back();
	}
	max_size_ = size;
}

text_surface &text_cache::find(text_surface const &t)
{
	static size_t lookup_ = 0, hit_ = 0;
	text_list::iterator it_bgn = cache_.begin(), it_end = cache_.end();
	text_list::iterator it = std::find(it_bgn, it_end, t);
	if (it != it_end) {
		cache_.splice(it_bgn, cache_, it);
		++hit_;
	} else {
		if (cache_.size() >= max_size_)
			cache_.pop_back();
		cache_.push_front(t);
	}
	if (++lookup_ % 1000 == 0) {
		// DBG_FT << "Text cache: " << lookup_ << " lookups, " << (hit_ / 10) << "% hits\n";
		hit_ = 0;
	}
	return cache_.front();
}

} // end namespace font
