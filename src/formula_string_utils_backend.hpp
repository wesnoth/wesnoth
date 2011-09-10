/* $Id$ */
/*
   Copyright (C) 2003 by David White <dave@whitevine.net>
   Copyright (C) 2005 - 2011 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef FORMULA_STRING_UTILS_BACKEND_HPP_INCLUDED
#define FORMULA_STRING_UTILS_BACKEND_HPP_INCLUDED

#include "exceptions.hpp"

#include "formula_string_utils.hpp"

#include "config.hpp"
#include "log.hpp"
#include "formula.hpp"
#include "gettext.hpp"
#include "util.hpp"    // for str_cast
#include "foreach.hpp"

///todo remove next 1 line
#include <boost/variant.hpp>

#include <boost/shared_ptr.hpp>

#include "utils/lru_cache.hpp"

static lg::log_domain log_interpolate("interpolate");
#define ERR_INTERP LOG_STREAM(err, log_interpolate)
#define WARN_INTERP LOG_STREAM(warn, log_interpolate)
#define DEBUG_INTERP LOG_STREAM(debug, log_interpolate)


/** @file contains a parser, a code generator and an interpolator to interpolate variables into WMLstrings.
t_tokenizer tokenizes strings into possible variable name components and control characters
t_instructions runs the mini-language that  interpolates wml_variables into strings.
t_operations constitutes the mini-language: push, append, interpolate and formula.
t_parse takes a vector of tokens and parses it into a script to do the interpolation

At each step results are cached so that at run-time a given string is only tokenized and parsed into instructions once.
At each interpolation the variable is retrieved and interpolated as necessary.
 */

namespace wml_interpolation {

typedef n_token::t_token t_token;
typedef std::vector<t_token> t_tokens;


/// Control tokens for parsing
static const t_token z_empty("", false);

static const unsigned int CACHE_SIZE = 10000;

inline bool  is_alnum_or_underscore (char c){
	return ( (((c) & ~0x7f) == 0)/*isascii(c)*/ &&  (isalnum(c) || (c == '_'))); }

///Checks if this token could be a part of a variable name
///@pre a has a least 1 character
///assumes that the whole token is characterized by the first character, which is true of
///parsed tokens but might not be true of returned by interpolation tokens
inline bool  is_good_for_varname (t_token const & a){
	return is_alnum_or_underscore(static_cast<std::string const &>(a)[0] ) ;}



///Break string/token into relevant tokens
/** @class t_tokenizer breaks strings/tokens into vectors of tokens.
	Tokens are either a part of a variable name (alnum or _),
	a valid control character $|.[]()'#
	or everything else.
 */
class t_tokenizer {
	t_tokens tokens_;  /// tokens for output
	bool is_done_, is_valid_varname_;
	std::string input_string_; ///inputs string
	size_t curr_pos_, end_, varname_start_;
public:
	/** Initialize the tokenizer with a std::string or a t_token
	@param[in] in a string/token static_castible to a string
	*/
	template <typename T> t_tokenizer(T const & in);

	/**Binds to a new string and tokenizes the string
	@param[in] in a string/token static_castible to a string
	*/
	template <typename T> t_tokens const & tokenize(T const & in);

	///Tokenizes the string
	t_tokens const & tokenize();

private:
	///Grabs a variable name from the start pos to the current position
	void grab_name();

	///Process the next character
	void process_char();
};








/**@class
   Each t_operation is a functor that does 2 things
   1. When provided with a token stack and a variable set it performs an operation :
   push, append, interpolate (variable) or formula (interpolation).
   2. When provided with 2 operations, it returns a single operation that in the same as
   the two operations it possible.
 */

struct t_operation;
typedef boost::shared_ptr<t_operation> t_operation_ptr;
typedef std::vector<t_operation_ptr> t_operations;

struct t_operation_noop;
struct t_operation_push;
struct t_operation_append;
struct t_operation_interp;
struct t_operation_formula;

struct t_operation {

	virtual ~t_operation() {}

	virtual void operator()(t_tokens & /*stack*/,  variable_set const & /*variable_set*/) const = 0;

	virtual std::string const string() const = 0;
	friend std::ostream & operator<<(std::ostream &out, t_operation const & a){
		return out << a.string(); }

	///t_reduce_result returns either (new_operation,true) on success or (void_op, false)
	typedef std::pair<t_operation_ptr, bool> t_reduce_result;

	///Takes 2 operations and possibly reduces them to 1
	virtual t_reduce_result reduce(t_operation_ptr const & second_op) const = 0;

	///Takes 2 operations and possibly reduces them to 1
	virtual t_reduce_result reduce_flipped(t_operation_noop const & /*first_op*/) const {
		return std::make_pair(void_op_,false); }
	virtual t_reduce_result reduce_flipped(t_operation_push const & /*first_op*/) const {
		return std::make_pair(void_op_,false); }
	virtual t_reduce_result reduce_flipped(t_operation_append const & /*first_op*/) const {
		return std::make_pair(void_op_,false); }
	virtual t_reduce_result reduce_flipped(t_operation_interp const & /*first_op*/) const {
		return std::make_pair(void_op_,false); }
	virtual t_reduce_result reduce_flipped(t_operation_formula const & /*first_op*/) const {
		return std::make_pair(void_op_,false); }

	virtual bool operator==(t_operation_ptr const & second_op) const = 0;
	virtual bool operator!=(t_operation_ptr const & second_op) const {
		return !(this->operator==(second_op)); }

	virtual bool operator==(t_operation_noop const & /*first_op*/) const { return false; }
	virtual bool operator==(t_operation_push const & /*first_op*/) const { return false; }
	virtual bool operator==(t_operation_append const & /*first_op*/) const { return false; }
	virtual bool operator==(t_operation_interp const & /*first_op*/) const { return false; }
	virtual bool operator==(t_operation_formula const & /*first_op*/) const { return false; }

private:
	static const t_operation_ptr void_op_; ///Operation ptr returned on failure, deferencing will cause a crash
};

///No op
struct t_operation_noop : public t_operation {
	virtual void operator()(t_tokens & /*stack*/,  variable_set const & /*variable_set*/) const { }
	std::string const string() const ;

	virtual std::pair<t_operation_ptr, bool> reduce(t_operation_ptr const & second_op) const {
		return std::make_pair(second_op, true); }

	virtual bool operator==(t_operation_ptr const & second_op) const {
		return second_op->operator==(*this); }

	virtual bool operator==(t_operation_noop const & /*first_op*/) const { return true; }
};


///@class t_operation_push pushes a token on the stack
class t_operation_push : public t_operation {
	t_token x_;
public:
	t_operation_push(t_token const & a) : x_(a){}

	virtual void operator()(t_tokens & stack,  variable_set const & /*variable_set*/) const {
		stack.push_back(x_);
		DEBUG_INTERP << string() << "\n"; }

	t_token const &x() const {return x_;}

	std::string const string() const;

	virtual t_reduce_result reduce(t_operation_ptr const & second_op) const {
		return second_op->reduce_flipped(*this); }

	virtual bool operator==(t_operation_ptr const & second_op) const {
		return second_op->operator==(*this); }

	virtual bool operator==(t_operation_push const & first_op) const { return first_op.x_ == x_; }
};


///@class t_operation_appends appends a token with the top item on the stack.  It caches results.
class t_operation_append : public t_operation {

	t_token x_;

	///@class this functor generates the result to cache in the event of a cache miss
	struct t_make_append_result {
		t_token operator()(std::pair<t_token, t_token> const & x) const {
			return t_token(static_cast<std::string const &>(x.first) +  static_cast<std::string const &>(x.second)); }
	};

	///LRU cache of append operations
	typedef  n_lru_cache::t_lru_cache<std::pair<t_token, t_token>, t_token, t_make_append_result> t_append_cache;
	static t_append_cache cache_;

public:
	t_operation_append(t_token const & a) : x_(a){}

	///Checks the cache for the append result and replaces the top token on the stack with the result
	virtual void operator()(t_tokens & stack,  variable_set const & /*variable_set*/) const {
		if(!stack.empty()){
			t_token const &res = cache_.check(std::make_pair(stack.back(), x_));
			if(res != stack.back()){
				DEBUG_INTERP << string(res) << "\n";
				stack.pop_back();
				stack.push_back(res); } }

		else {
			stack.push_back(x_); }
	}

	t_token const &x() const {return x_;}

	virtual t_reduce_result reduce(t_operation_ptr const & second_op) const {
		return second_op->reduce_flipped(*this); }

	/// push->append is reduces to a push operation
	virtual t_reduce_result reduce_flipped(t_operation_push const & first_op) const {
		t_operation_ptr op(new t_operation_push(t_token(static_cast<std::string const &>(first_op.x())
														+  static_cast<std::string const &>(x()))));
		return std::make_pair(op, true); }

	/// append->append is reduces to an append operation
	virtual t_reduce_result reduce_flipped(t_operation_append const & first_op) const {
		t_operation_ptr op(new t_operation_append(t_token(static_cast<std::string const &>(first_op.x())
														+  static_cast<std::string const &>(x()))));
		return std::make_pair(op, true); }

	virtual bool operator==(t_operation_ptr const & second_op) const {
		return second_op->operator==(*this); }

	virtual bool operator==(t_operation_append const & first_op) const { return first_op.x_ == x_; }

	std::string const string() const ;

private:
	std::string const string(std::string const & res) const;

};

/**@class t_operation_interp looksup the top token on the stack in the variable_set
   @param[in] variable_set
 */
class t_operation_interp : public t_operation {

public :

	///Grab the top item on the stack, look it up in the variable set and append it to the second item on the stack
	virtual void operator()(t_tokens & stack,  variable_set const & variable_set) const {

		t_token var_name(stack.back());
		config::attribute_value stuffing(variable_set.get_variable_const(var_name ));
		DEBUG_INTERP << string(var_name, stuffing) << "\n";

		stack.pop_back();
		t_operation_append append(stuffing.token());
		append(stack, variable_set);
	}

	virtual t_reduce_result reduce(t_operation_ptr const & second_op) const {
		return second_op->reduce_flipped(*this); }

	virtual bool operator==(t_operation_ptr const & second_op) const {
		return second_op->operator==(*this); }

	virtual bool operator==(t_operation_interp const & /*first_op*/) const { return true; }

	std::string const string() const ;
private:
	std::string const string(std::string const & var, std::string const & res) const ;
};


struct t_operation_formula : public t_operation {

	virtual void operator()(t_tokens & stack,  variable_set const & variable_set) const ;

	virtual t_reduce_result reduce(t_operation_ptr const & second_op) const {
		return second_op->reduce_flipped(*this); }

	std::string const string() const ;

	virtual bool operator==(t_operation_ptr const & second_op) const {
		return second_op->operator==(*this); }

	virtual bool operator==(t_operation_formula const & /*first_op*/) const { return true; }

private:
	std::string const string(std::string const & var, std::string const & res) const ;
};


/**  @class A vector of operations that can be run.
 */
class t_instructions {
	t_operations ops_; /// the operations
public:

	t_instructions()
		: ops_()
	{
	}

	///Optimize the instructions by trying to pairwise reduce the instruction set.
	void optimize(t_token const & unparsed) ;

	///runs the instructions and places the results in @param[out] x
	void run(t_token & x, variable_set const & set) const {
		if ( ops_.empty() ) {
			//DEBUG_INTERP << "Nothing to interpolate in "<<x<<"\n";
			return; }
		run_core(x, set);
	}

	void run_core(t_token & x, variable_set const & set) const ;

	t_operations & ops() { return ops_; }
	t_operations const & ops() const { return ops_; }

	friend std::ostream & operator<<(std::ostream &out, t_instructions const & a);
};



/** @class t_parse parses tokens into instructions to perform the interpolation
 */
struct t_parse {

	n_token::t_token token_; ///The input token
	t_tokens tokens_; ///Its tokens

	t_instructions complete_parse_;

	friend std::ostream & operator<<(std::ostream &out, t_parse const & a);

	t_parse(n_token::t_token const & unparsed) ;
	t_parse(t_parse const & a );
	t_parse & operator=(t_parse const & a) ;

	///Calculate a parse
	t_instructions & parse();

	///Peeks at the next token
	t_token const & peek_next(t_tokens::iterator const & curr_pos );

	///Keep concatenating components until you see a $ which indicates some kind of interpolation
	t_tokens::iterator do_parse_plain(t_tokens::iterator const & start_pos );

	///Parse the instructions to generate the variable name for interpolation
	t_tokens::iterator do_parse_interp(t_tokens::iterator const & start_pos ) ;

	/** do_parse formul	a parses a formula invoked by $ (...) syntax
	 */
	t_tokens::iterator do_parse_formula(t_tokens::iterator const & start_pos );
};


/**  @class Parses and possibly stores variables for subsequent interpolation */
class t_parse_and_interpolator {

	/// @class Generate a parse for an  uncached token
	struct t_parser {
		t_instructions operator()(n_token::t_token const & unparsed){
			t_parse aparse(unparsed);
			t_instructions  instructions(aparse.parse());
			instructions.optimize(unparsed);
			return instructions;
		}
	};

	/// A Least Recently Used (LRU) cache of previous parse results
	///These are independent of the interpolated data
	typedef n_lru_cache::t_lru_cache<n_token::t_token, t_instructions, t_parser> t_all_parsed;
	static t_all_parsed all_parsed_;

	n_token::t_token maybe_parsed_; ///input token and result
	t_instructions const * complete_parse_; ///instructions to interpolate this token
	bool is_done_;

public:
	t_parse_and_interpolator (n_token::t_token const & unparsed)
		: maybe_parsed_(unparsed), complete_parse_(NULL), is_done_(false){}

	t_parse_and_interpolator (t_parse_and_interpolator const & a);
	t_parse_and_interpolator & operator=(t_parse_and_interpolator const & a);

	friend std::ostream & operator<<(std::ostream &out, t_parse_and_interpolator const & a);
	// {
		// out << ((a.is_done_) ? "parsed=" : "unparsed=") << a.maybe_parsed_;
		// return out; }

	bool valid() const { return complete_parse_ != NULL; }
	bool is_done() const { return valid() && is_done_; }

	/// The core loop.  Parse and Interpolate
	/// The parse may be fetched from the cache and the same also with the interpolated result (after variable lookup)
	n_token::t_token const & parse_and_interpolate(const variable_set& set) {
		parse();
		return interpolate(set); }

	///Fetch the parsed instructions from the cache
	t_parse_and_interpolator & parse(){
		complete_parse_ = & all_parsed_.check(maybe_parsed_);
		is_done_ = false;
		return *this; }

	///Run the parsed instructions to do the interpolation.
	t_token const & interpolate(const variable_set& set){
		assert(complete_parse_);
		if(!is_done_){
			complete_parse_->run(maybe_parsed_, set); }
		return maybe_parsed_; }

};

}


#endif
