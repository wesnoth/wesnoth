/* $Id$ */
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


template <typename T_key, typename T_value, typename T_compute_value>
class t_lru_cache{

public:
	///Key value for the cache
	///@pre supports operator== and boost::hash_value
	///@pre doesn't contain any references as it is stored in a STL container
	typedef T_key t_key;
	///Value for the cache
	///@pre doesn't contain any references as it is stored in a STL container
	typedef T_value t_value;
	///A functor that computes the value from key
	typedef T_compute_value t_comp;

private:
	typedef std::list<t_key> t_lru_list;
	typedef typename t_lru_list::iterator lru_iterator;
	typedef lru_iterator iterator;
	typedef std::pair<t_value, lru_iterator> t_valpos_pair;
	typedef boost::unordered_map<t_key, t_valpos_pair> t_key_to_value;
	typedef typename t_key_to_value::iterator kv_iterator;

	///A map of keys to values
	t_key_to_value key_to_valpos_;
	///A list of the least recently used keys
	t_lru_list lru_list_;
	///The functor used to compute values from keys
	t_comp comp_;
	///The maximum size of cache before old items are discarded
	size_t max_size_;
	///A placeholder for insertion (see code).
	t_value const dummy_item_;

	///Counters for debugging
	unsigned int stat_hit_, stat_miss_, stat_erased_;
public:
	///Initialize the cache
	///@param[in] c a functor to compute values from keys
	///@param[in] max_size the maximum size of the cache before items are discarded
	///@param[in] dummy_value a dummy value used internally to insert fake values into the cache
	t_lru_cache(t_comp const & c, size_t max_size, t_value const & dummy_value = t_value())
		: key_to_valpos_(), lru_list_(), comp_(c), max_size_(max_size), dummy_item_(dummy_value)
		, stat_hit_(0), stat_miss_(0), stat_erased_(0){}
	t_lru_cache(t_lru_cache const & a)
		: key_to_valpos_(a.key_to_valpos_), lru_list_(a.lru_list_), comp_(a.comp_)
		, max_size_(a.max_size_), dummy_item_(a.dummy_item_)
		, stat_hit_(a.stat_hit_), stat_miss_(a.stat_miss_), stat_erased_(a.stat_erased_){}
	~t_lru_cache(){
		//std::cerr<<"lru cache hits="<<stat_hit_<<" misses="<<stat_miss_<<" erased="<<stat_erased_<<"\n";
	}
	void set_comp(t_comp const & a){comp_ = a;}
	void set_max_size(size_t m){max_size_=m; right_size();}

	///Check the cache for the value associated with key k.
	t_value & check(t_key const & k){ return check(k, comp_); }

	///Check the cache for the value associated with key k.  Use lcomp to compute the value if not in the cache
	template <typename X_comp>
	t_value & check(t_key const & k, X_comp & lcomp){
		///Try to insert the dummy value.
		///If it fails the returned value is a cache hit.
		///If it succeeds then calculate the real cache value and store the result

		std::pair<kv_iterator, bool> ins(
				key_to_valpos_.insert(std::make_pair(k, std::make_pair(dummy_item_, lru_list_.begin() ))) );

		bool found = ! ins.second ;
		kv_iterator & kv_it(ins.first);
		//cache hit
		if(found){
			//++stat_hit_;
			mark_used(kv_it);
			return kv_it->second.first;
		}

		//cache miss
		//++stat_miss_;
		t_value val = lcomp(k);
		mark_used(k);
		t_valpos_pair & vp( kv_it->second);
		vp.first=val;
		vp.second=lru_list_.begin();

		//resize the cache
		right_size();

		return vp.first;

	}
	void invalidate(){
		lru_list_.clear();
		key_to_valpos_.clear();
	}
	void invalidate(t_key const & k){
		kv_iterator found (key_to_valpos_.find(k));
		if(found != key_to_valpos_.end()){ //Cache hit
			lru_list_.erase(found->second.second);
			///todo Change to quick_erase when wesnoth supports boost 1.4?6
			key_to_valpos_.erase(found);
		}
	}

	///For debugging only.  Check for a cache hit.
	///@note Don't use this in real code, just call check.  It is faster
	bool debugging_test_cache(t_key const & k) const {
		typename t_key_to_value::const_iterator found (key_to_valpos_.find(k));
		return (found != key_to_valpos_.end()); }
	std::pair<bool, t_key> debugging_newest_item() const {
		if (lru_list_.empty()){ return std::make_pair(false, t_key()); }
		return std::make_pair(true, *lru_list_.begin()); }
	std::pair<bool, t_key> debugging_oldest_item() const {
		if (lru_list_.empty()){ return std::make_pair(false, t_key()); }
		return std::make_pair(true, lru_list_.back()); }

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
		size_t size(key_to_valpos_.size());
		if(size > max_size_){
			size_t num_extra(size - max_size_);
			for(size_t i=0; i<num_extra; ++i){
				//				++stat_erased_;
				invalidate(lru_list_.back());
			}
		}
	}
};

}//end namepace

#endif
