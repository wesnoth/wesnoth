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

#ifndef UTILS_INTERNED_H_INCLUDED
#define UTILS_INTERNED_H_INCLUDED

/**
 * @file
 */

#include <string>
#include <vector>
#include <iosfwd>
//@todo when C++0x is supported switch to #include <unordered_map>
#include <boost/unordered_map.hpp>

//debug
#include <iostream> //std::cerr

//#include <utility>
//using namespace std::rel_ops; //conflicts with other source files i.e. tree_view.cpp

#include "utils/reference_counter.hpp"
#include "utils/count_logger.hpp"

namespace n_interned {

template <typename T, typename T_hasher = boost::hash<T> > class t_interned_token ;



/**
   @class t_interned_token
   @brief t_interned_token stores unique copies of an object of type T and returns a reference to an object of type T
   that allows fast O(1) equality comparison, copying and hashing.
   A unique copy of the T is available as a const reference.
   It is typically reference counted unless explicitly requested otherwise upon construction (i.e for static T)

   The copying of interned_tokens is faster than the creation of interned_tokens, because a reference object isn't created.
   Hence use static interned_tokens as replacements for constants in the code, as follows:
   static const t_interned_token<T> z_some_T_thing(reference_T_thing, false);

   or use generate_safe_static_const_t_interned.  @see below
 */
template <typename T, typename T_hasher >
class t_interned_token {
public:
	inline ~t_interned_token();

	///default constructor.
	t_interned_token();

	///Create  token
	///@param[in] is_ref_counted determines if all tokens of this value will be ref counted.
	///Static tokens should not be ref counted.
	explicit t_interned_token(T const & a, bool is_ref_counted = true);

	///Fast Copying
	inline t_interned_token(t_interned_token const & a);
	///Fast Copying
	inline t_interned_token & operator=(t_interned_token const &a);

	inline bool valid() const;
	inline bool empty() const {return !valid();}

	void disable_count() {
		is_ref_counted_ = false;
		iter_->second.disable_count(); }


	///Cast to the constant member
	operator T const &() const { return iter_->first; }

	///Access to the constant member
	T const & operator*() const  { return iter_->first; }

	///Access constant members of T
	T const * operator->() const  { return &iter_->first; }

	///Fast equality operators
	friend inline bool operator==(t_interned_token const &a, t_interned_token const &b) { return (a.iter_==b.iter_); }
	///Fast equality operators
	friend inline bool operator!=(t_interned_token const &a, t_interned_token const &b) { return !(a == b); }

	///slower equality operators
	friend inline bool operator==(t_interned_token const &a, T const &b) { return a.iter_->first == b;}
	///slower equality operators
	friend inline bool operator==(T const &b, t_interned_token const &a) { return b == a.iter_->first;}
	///slower equality operators
	friend inline bool operator!=(t_interned_token const &a, T const &b) { return !(a == b); }
	///slower equality operators
	friend inline bool operator!=(T const &b, t_interned_token const &a) { return !(b == a); }

	///slower comparison operators
	friend inline bool operator<(t_interned_token const &a, t_interned_token const &b) {return  (a.iter_->first) < ( b.iter_->first); }
	///slower comparison operators
	friend inline bool operator<(T const &a, t_interned_token const &b) {return  a < ( b.iter_->first); }
	///slower comparison operators
	friend inline bool operator<(t_interned_token const &a, T const &b) {return  (a.iter_->first) <  b; }

	///Fast Hashing
	friend inline std::size_t hash_value(t_interned_token const & a){
		boost::hash<t_interned const *> h;
		assert(a.valid());
		return  h( &a.iter_->second); }

#ifdef DEBUG
	///For debugging purposes count the number of identical tokens created by
	///redundant construction of objects.
	///If the numbers are large enough that construction and hashing of the reference objects is creating a slowdown
	///then these should then be changed to static objects/copies of tokens to avoid the
	///repeated cost of constructing the objects.
	static const unsigned int is_collect_num_stats_ = 0;

	///When debugging this assert stops the code when this many copies are made in order to find the offending code.
	/// 0 prevents the assert from being active
	static const unsigned int assert_stop_at_ = 0;
#endif

	friend std::ostream & operator<<(std::ostream &out, t_interned_token const & a) {
		return out << a.iter_->first; }
	friend std::istream & operator>>(std::istream& is, t_interned_token & a) {
		T t;
		std::istream & ret = is >> t;
		a = t_interned_token(t);
		return ret;
	}


private:
	typedef n_ref_counter::t_ref_counter<signed long> t_interned;
	typedef boost::unordered_map<T, t_interned, T_hasher > t_stash;
	///The token table
	///This forces correct static initialization order.
	///Do not inline.
	static t_stash & the_stash();

	///Initialize first default constructed value.  The default value is only constructed once.
	typename t_stash::value_type * first_default_constructed();

	///Increment the reference count
	inline void inc_ref();
	///Decrement the reference count
	inline void dec_ref();

	///Iterator to the (value,reference count)
	typename t_stash::value_type * iter_;
	bool is_ref_counted_;
};


/**
   @function generate_safe_static_const_t_interned
   @brief generate_safe_static_const<T> produces const reference to a T object that
   will never be destrotyed.  The created object is both static initialization and static de-initialization safe

   @note usage inside a function body type and zzz with be a safe copy of yyy
   static const T & zzz = generate_safe_static_const_t_interned.get( yyy )

   @note Do not use this to create objects that are not static, that will be a memory leak

 */

template <typename T>
//don't inline
T const & generate_safe_static_const_t_interned(T const & x);



//Static member initialization


template <typename T, typename T_hasher >
typename t_interned_token<T, T_hasher>::t_stash & t_interned_token<T, T_hasher>::the_stash(){
	/// @note Do not inline this function.  It forces an ordered static initialization of the_stash_ before static tokens
	static t_stash * the_stash_ = new t_stash();
	return *the_stash_;
}

template <typename T, typename T_hasher >
typename t_interned_token<T, T_hasher>::t_stash::value_type * t_interned_token<T, T_hasher>::first_default_constructed() {
	 t_stash & the_stash_ = the_stash();
	 t_interned default_value_(t_interned::NOT_COUNTED);
	 T a = T();

	 std::pair<typename t_stash::iterator, bool> maybe_inserted = the_stash_.insert(std::make_pair(a, default_value_));

	 return &*(maybe_inserted.first);
}
template <typename T, typename T_hasher >
t_interned_token<T, T_hasher>::t_interned_token()
	: iter_(NULL)
	, is_ref_counted_(false)
{
	static typename t_stash::value_type * default_iter = first_default_constructed();
	iter_ = default_iter;
}

template <typename T, typename T_hasher >
t_interned_token<T, T_hasher>::t_interned_token(T const & a , bool is_ref_counted)
	: iter_(NULL)
	, is_ref_counted_(is_ref_counted)
{

	static t_stash & the_stash_ = the_stash();
	static t_interned default_value_(0);

	std::pair<typename t_stash::iterator, bool> maybe_inserted = the_stash_.insert(std::make_pair(a, default_value_));
	assert( maybe_inserted.first != the_stash_.end() );

	iter_ = &*(maybe_inserted.first);
	if(!is_ref_counted_){
		//Mark this token as no longer ref counted
		iter_->second.disable_count();
	}
	inc_ref();

#ifdef DEBUG
	/// Use the collected stats to find redundant object construction and change them to static non-reference counted tokens
	/// to avoid their repeated hashing and reference counting
	if(is_collect_num_stats_ > 0){
		static n_count_logger::t_count_logger<T> count_bad_token_ref("Number of tokens constructed from fully constructed objects and then reference counted. \nIf number is high and performance is slow make offending constructions static.",is_collect_num_stats_);
		static n_count_logger::t_count_logger<T> count_bad_token_static("Number of tokens constructed from fully constructed objects. \nIf number is high and performance is slow make offending objects static and not reference counted." , is_collect_num_stats_);
		if(is_ref_counted_ && iter_->second > 0){
			unsigned int joe = count_bad_token_ref.inc(a);
			(void) joe;
			if((assert_stop_at_ > 0) && (joe > assert_stop_at_) ){assert(false);}
		}else{
			unsigned int joe =count_bad_token_static.inc(a);
			(void) joe;
			if((assert_stop_at_ > 0) && (joe > assert_stop_at_) ){assert(false);}
		}
	}
#endif //DEBUG

}

template <typename T, typename T_hasher >
t_interned_token<T, T_hasher>::t_interned_token(t_interned_token<T, T_hasher> const & a)
	: iter_(a.iter_), is_ref_counted_(a.is_ref_counted_) {
	inc_ref();
}

template <typename T, typename T_hasher >
t_interned_token<T, T_hasher> & t_interned_token<T, T_hasher>::operator=(t_interned_token const &a){
	if(*this == a){ return *this; }

	dec_ref();

	iter_ = a.iter_;
	is_ref_counted_ = a.is_ref_counted_;

	inc_ref();

	return *this;
}


template <typename T, typename T_hasher >
t_interned_token<T, T_hasher>::~t_interned_token() {
	dec_ref();
}

template <typename T, typename T_hasher >
void t_interned_token<T, T_hasher>::inc_ref() {
	if(is_ref_counted_ && valid() && (++(iter_->second) < 0 )) {
		is_ref_counted_ = false; }
}
template <typename T, typename T_hasher >
void t_interned_token<T, T_hasher>::dec_ref() {
	if(is_ref_counted_ && valid() && ( (--(iter_->second)) == 0)) {
		static t_stash & the_stash_ = the_stash();
		///@todo upgrade to quick_erase when boost 1.42 is supported
		the_stash_.erase(iter_->first);  }
}

template <typename T, typename T_hasher >
bool t_interned_token<T, T_hasher>::valid() const { return true; }


template <typename T>
T const & generate_safe_static_const_t_interned(T const & x) {
	//This pointer should never be deleted.
	//This is intentional and not a memory leak
	//It prevents both static initialization and de-initialization crashes
	T * never_delete = new T(x);
	never_delete->disable_count();
	return *never_delete;
}




}//end namepace

#endif
