/*
   Copyright (C) 2004 - 2018 by Philippe Plantier <ayin@anathas.org>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

/**
 * @file
 */


#include <limits>

namespace n_ref_counter {


/**
   @class ref_counter
   @brief ref_counter is a reference counter.  If the counter overflows it stops counting.
   So any negative count disables reference counting.
**/
template <typename T_integral> class ref_counter {
	static_assert(std::numeric_limits<T_integral>::is_signed, "Reference counter must be a signed integer");

	T_integral count_;

public:
	enum {NEW=0, NOT_COUNTED = -1};

	explicit ref_counter(T_integral x = 0) : count_(x) {}
	ref_counter(ref_counter const &a) : count_(a.count_) {}
	ref_counter & operator=(ref_counter const &a){count_ = a.count_; return *this;}

	operator T_integral const () const {return count_;}

	T_integral const set(T_integral const a) { count_=a; return count_; }
	T_integral const inc(){
		if (count_ >= 0) { count_  += 1; }
		return count_; }
	T_integral const dec(){
		if( count_ > 0) { count_  -= 1; }
		return count_; }
	T_integral const enable_count(){
		if (count_ < 0) {count_ = 0;}
		return count_; }
	T_integral const disable_count(){
		count_= NOT_COUNTED;
		return count_; }

	T_integral const operator++(){return inc();}
	T_integral const operator++(int){T_integral ret(count_); inc(); return ret;}
	T_integral const operator--(){return dec();}
	T_integral const operator--(int){T_integral ret(count_); dec(); return ret;}
};


}//end namepace
