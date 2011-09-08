/* $Id: tstring.hpp 48153 2011-01-01 15:57:50Z mordante $ */
/*
   Copyright (C) 2004 - 2011 by Philippe Plantier <ayin@anathas.org>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef UTILS_LRUCACHE_H_INCLUDED
#define UTILS_LRUCACHE_H_INCLUDED

/**
 * @file
 */


#include <list>
#include <iosfwd>
//@todo when C++0x is supported switch to #include <unordered_map>
#include <boost/unordered_map.hpp>

//debug
#include <string>
#include <iostream> //std::cerr


namespace n_lru_cache {


template <typename T_key, typename T_value, typename T_comp>
class t_lru_cache{

public:
	typedef T_key t_key;
	typedef T_value t_value;
	typedef T_comp t_comp;

private:
	typedef std::list<t_key> t_lru_list;
	typedef typename t_lru_list::iterator lru_iterator;
	typedef std::pair<t_value, lru_iterator> t_valpos_pair;
	typedef boost::unordered_map<t_key, t_valpos_pair> t_key_to_value;
	typedef typename t_key_to_value::iterator kv_iterator;

	t_key_to_value key_to_valpos_;
	t_lru_list lru_list_;
	t_comp comp_;
	size_t max_size_;

	unsigned int stat_hit_, stat_miss_, stat_erased_;
public:

	typedef lru_iterator iterator;

	t_lru_cache(t_comp const & c, size_t max_size):key_to_valpos_(), lru_list_(), comp_(c), max_size_(max_size)
												  ,stat_hit_(0), stat_miss_(0), stat_erased_(0){}
	t_lru_cache(t_lru_cache const & a)
		: key_to_valpos_(a.key_to_valpos_), lru_list_(a.lru_list_), comp_(a.comp_), max_size_(a.max_size_)
		,stat_hit_(a.stat_hit_), stat_miss_(a.stat_miss_), stat_erased_(a.stat_erased_){}
	~t_lru_cache(){
		//std::cerr<<"lru cache hits="<<stat_hit_<<" misses="<<stat_miss_<<" erased="<<stat_erased_<<"\n";
	}
	void set_comp(t_comp const & a){comp_ = a;}
	void set_max_size(size_t m){max_size_=m; right_size();}

	t_value & check(t_key const & k){ return check(k, comp_);}
	template <typename X_comp>
	t_value & check(t_key const & k, X_comp & lcomp){
		kv_iterator found (key_to_valpos_.find(k));
		if(found != key_to_valpos_.end()){ //Cache hit	
			//			++stat_hit_;
			mark_used(found);
			return found->second.first;
		}
		//		++stat_miss_;
		t_value val = lcomp(k);
		mark_used(k);
		std::pair<kv_iterator, bool> ins(key_to_valpos_.insert(std::make_pair(k, std::make_pair(val,lru_list_.begin() ))) );

		right_size();
		return ins.first->second.first;
	}
	void invalidate(){
		lru_list_.clear();
		key_to_valpos_.clear();
	}
	void invalidate(t_key const & k){
		kv_iterator found (key_to_valpos_.find(k));
		if(found != key_to_valpos_.end()){ //Cache hit	
			lru_list_.erase(found->second.second);
			key_to_valpos_.erase(found);
		}
	}

	iterator const end()const {return lru_list_.end();}
private:
	void mark_used(kv_iterator const & kv_iter){
		lru_list_.erase(kv_iter->second.second);
		lru_list_.push_front(kv_iter->first);
		kv_iter->second.second = lru_list_.begin();
	}
	inline void mark_used(t_key const & k){
		lru_list_.push_front(k);
	}
	inline void right_size(){
		if(key_to_valpos_.size() > max_size_){
			//			++stat_erased_;
			invalidate(lru_list_.back());
		}
	}
};

}//end namepace

#endif
