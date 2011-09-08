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

#include <exception>

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

#include "formula_string_utils_backend.hpp"


namespace utils {

class string_map_variable_set : public variable_set
{
public:
	string_map_variable_set(const string_map& map) : map_(map) {};

	virtual config::attribute_value get_variable_const(const n_token::t_token &key) const {
		config::attribute_value val;
		const string_map::const_iterator itor = map_.find(key);
		if (itor != map_.end())
			val = itor->second;
		return val;
	}
	virtual config::attribute_value get_variable_const(const std::string &key) const{
		return get_variable_const(n_token::t_token(key));}
private:
	const string_map& map_;

};
}

// namespace {
// typedef n_token::t_token t_token;
// typedef std::vector<t_token> t_tokens;

// static const t_token z_empty("", false);
// static const t_token z_dollar("$", false);
// static const t_token z_bar("|", false);
// static const t_token z_dot(".", false);
// static const t_token z_lbracket("[", false);
// static const t_token z_rbracket("]", false);
// static const t_token z_lparen("(", false);
// static const t_token z_rparen(")", false);
// static const t_token z_single_quote("'", false);
// static const t_token z_sharp("#", false);

// static const unsigned int CACHE_SIZE = 10000;

// inline bool  is_alnum_or_underscore (char c){
// 	return ( (((c) & ~0x7f) == 0)/*isascii(c)*/ &&  (isalnum(c) || (c == '_'))); }

// ///Checks if this token could be a part of a variable name
// ///@pre a has a least 1 character
// ///assumes that the whole token is characterized by the first character, which is true of
// ///parsed tokens but might not be true of returned by interpolation tokens
// inline bool  is_good_for_varname (t_token const & a){
// 	return is_alnum_or_underscore(static_cast<std::string const &>(a)[0] ) ;}


// class wml_syntax_error : public std::exception {
// 	t_tokens tokens_;
// 	t_tokens::const_iterator pos_;
// 	std::string reason_;
// 	mutable std::string output_;
//  public:
// 	~wml_syntax_error() throw() {}
// 	wml_syntax_error(t_tokens const & ts, size_t const & pos, std::string const & r = "unknown reason") 
// 		: tokens_(ts), pos_(tokens_.begin() + pos), reason_(r), output_() {}
// 	virtual const char * what() const throw() {
// 		std::stringstream ss;
// 		ss << "WML Syntax error:: Variable in WML string cannot be evaluated due to " << reason_<< " in \n\"";
// 		t_tokens::const_iterator i = tokens_.begin();
// 		for(; i!=tokens_.end(); ++i){
// 			ss << *i; }
// 		ss << "\" at \n\"";
// 		for(i = tokens_.begin(); (i!= tokens_.end() && i != pos_) ; ++i){
// 			ss << *i; }
// 		if(i != tokens_.end()){
// 			ss << "\"  -->\"" << *i << "\"<-- ";
// 			if((++i) != tokens_.end()){
// 				ss << "\"";
// 				for(; i != tokens_.end(); ++i){
// 					ss << *i; } 
// 				ss << "\"";
// 			}
// 		} 
// 		else { ss << "\"<--"; }

// 		output_ = ss.str();
// 		return output_.c_str();
// 	}
// };


// ///Break string/token into relevant tokens
// class t_tokenizer {
// 	t_tokens tokens_;
// 	bool is_done_, is_valid_varname_;
// 	std::string input_string_;
// 	size_t curr_pos_, end_, varname_start_;
// public:
// 	template <typename T>
// 	t_tokenizer(T const & in) 
// 		: tokens_(), is_done_(false), is_valid_varname_(false), input_string_(static_cast<std::string const &>(in))
// 		, curr_pos_(0), end_(input_string_.size()) 
// 		, varname_start_(input_string_.npos){}

// 	template <typename T>
// 	t_tokens tokenize(T const & in) {
// 		input_string_ = (static_cast<std::string const &>(in));
// 		is_done_=false;
// 		return tokenize();
// 	}
// 	t_tokens tokenize() {
// 		if(!is_done_){
// 			tokens_.clear();
// 			curr_pos_=0;
// 			end_=input_string_.size();
// 			if(end_ == 0){
// 				tokens_.push_back(z_dollar);
// 				is_done_ = true; } 
// 			else {
// 				varname_start_=input_string_.npos;
// 				int guard(0);
// 				while (!is_done_ && (++guard < 1e6)){
// 					make_token(); } }
// 			assert(is_done_);
// 		}

// 		if(! lg::debug.dont_log(log_engine)){
// 			std::stringstream ss;
// 			ss << "tokenized [";
// 			foreach(t_token const &tok, tokens_){ ss << " \""<<tok<<"\"";}
// 			DEBUG_NG << ss.str() << "]\n";
// 		}

// 		return tokens_;
// 	}

// 	void grab_name(){
// 		if(varname_start_ != input_string_.npos){ 
// 			t_token next_token(input_string_.substr(varname_start_, curr_pos_- varname_start_));
// 			tokens_.push_back(next_token);
// 			varname_start_=input_string_.npos;
// 		}
// 	}

// 	void make_token(){
// 		if(curr_pos_ == end_){ grab_name(); is_done_=true; return;}
		
// 		//if the char at curr_pos is a control character then it is its own token
// 		//otherwise it is the start of a multicharacter token

// 		char curr_char = input_string_[curr_pos_];
// 		switch(curr_char){
// 		case '$' : grab_name(); tokens_.push_back(z_dollar); ++curr_pos_; return ;
// 		case '|' : grab_name(); tokens_.push_back(z_bar); ++curr_pos_; return ;
// 		case '[' : grab_name(); tokens_.push_back(z_lbracket); ++curr_pos_; return;
// 		case ']': grab_name(); tokens_.push_back(z_rbracket); ++curr_pos_; return;
// 		case '(': grab_name(); tokens_.push_back(z_lparen); ++curr_pos_; return;
// 		case ')': grab_name(); tokens_.push_back(z_rparen); ++curr_pos_; return;
// 		case '.': grab_name(); tokens_.push_back(z_dot); ++curr_pos_; return;
// 		case '#': grab_name(); tokens_.push_back(z_sharp); ++curr_pos_; return;
// 		case '\'': grab_name(); tokens_.push_back(z_single_quote); ++curr_pos_; return;
// 		default:
// 			//Multi-character tokens are either all valid variable name characters or all not valid variable name characters
// 			bool is_good_char = is_alnum_or_underscore(curr_char);
// 			if(is_good_char != is_valid_varname_){
// 				grab_name();
// 			}
// 			if(varname_start_ == input_string_.npos){
// 				varname_start_ = curr_pos_;
// 				is_valid_varname_ = is_good_char ;
// 			}
// 			++curr_pos_;
// 		}
// 	}
// };








// /**
//    t_operation is a functiod that does 2 things
//    1. When provided with a token stack and a variable set it performs an operation like interpolation.
//    2. When given a token stack an t_operation stack it performs its operation if it can do so without the
//    variable set, otherwise it puts an operation on the instruction stack.  This function can be called when the expression 
//    is parsed in order to reduce the operations to their smallest set.
//  */
// struct t_operation;
// typedef boost::shared_ptr<t_operation> t_operation_ptr;
// typedef std::vector<t_operation_ptr> t_operations;

// struct t_operation_push;
// struct t_operation_append;
// struct t_operation_interp;
// struct t_operation_formula;

// struct t_operation {
// 	virtual void operator()(t_tokens & /*stack*/,  variable_set const & /*variable_set*/) const = 0;

// 	virtual std::string const string() const = 0;
// 	friend std::ostream & operator<<(std::ostream &out, t_operation const & a){
// 		return out << a.string(); }

// 	typedef std::pair<t_operation_ptr, bool> t_reduce_result;
// 	///Takes 2 operations and possibly reduces them to 1
// 	virtual t_reduce_result reduce(t_operation_ptr const & second_op) const {
// 		return second_op->reduce_flipped(*this); }

// 	///Takes 2 operations and possibly reduces them to 1
// 	virtual t_reduce_result reduce_flipped(t_operation const & /*first_op*/) const {
// 		return std::make_pair(void_op_,false); }
// 	virtual t_reduce_result reduce_flipped(t_operation_push const & /*first_op*/) const {
// 		return std::make_pair(void_op_,false); }
// 	virtual t_reduce_result reduce_flipped(t_operation_append const & /*first_op*/) const {
// 		return std::make_pair(void_op_,false); }
// 	virtual t_reduce_result reduce_flipped(t_operation_interp const & /*first_op*/) const {
// 		return std::make_pair(void_op_,false); }
// 	virtual t_reduce_result reduce_flipped(t_operation_formula const & /*first_op*/) const {
// 		return std::make_pair(void_op_,false); }
	
// private:
// 	static const t_operation_ptr void_op_;
// };

// //static member
// t_operation_ptr const t_operation::void_op_;

// ///No op
// struct t_operation_noop : public t_operation {
// 	virtual void operator()(t_tokens & /*stack*/,  variable_set const & /*variable_set*/) const { }
// 	std::string const string() const {return "[noop] "; }

// 	virtual std::pair<t_operation_ptr, bool> reduce(t_operation_ptr const & second_op) const {
// 		return std::make_pair(second_op, true); }

// };

// class t_operation_push : public t_operation {
// 	t_token x_;
// public:
// 	t_operation_push(t_token const & a) : x_(a){}
	
// 	virtual void operator()(t_tokens & stack,  variable_set const & /*variable_set*/) const {
// 		stack.push_back(x_);
// 		DEBUG_NG << string() << "\n"; }

// 	t_token const &x() const {return x_;}

// 	std::string const string() const {std::stringstream ss; ss << "[push \"" <<x_<<"\"] "; return ss.str(); }

// 	virtual t_reduce_result reduce(t_operation_ptr const & second_op) const {
// 		return second_op->reduce_flipped(*this); }
// };

// class t_operation_append : public t_operation {

// 	t_token x_;
// 	struct t_make_append_result {
// 		t_token operator()(std::pair<t_token, t_token> const & x) const {
// 			return t_token(static_cast<std::string const &>(x.first) +  static_cast<std::string const &>(x.second)); }
// 	};

// 	typedef  n_lru_cache::t_lru_cache<std::pair<t_token, t_token>, t_token, t_make_append_result> t_append_cache;
// 	static t_append_cache cache_;

// public:
// 	t_operation_append(t_token const & a) : x_(a){}
	
// 	virtual void operator()(t_tokens & stack,  variable_set const & /*variable_set*/) const {
// 		if(!stack.empty()){
// 			t_token const &res = cache_.check(std::make_pair(stack.back(), x_)); 
// 			if(res != stack.back()){
// 				DEBUG_NG << string(res) << "\n";
// 				stack.pop_back();
// 				stack.push_back(res);
// 			}
// 		} else {
// 			stack.push_back(x_);
// 		}
// 	}
// 	t_token const &x() const {return x_;}

// 	std::string const string() const {std::stringstream ss; ss << "[append \"" <<x_<<"\"] "; return ss.str(); }

// 	virtual t_reduce_result reduce(t_operation_ptr const & second_op) const {
// 		return second_op->reduce_flipped(*this); }

// 	/// push->append is just a push operation 
// 	virtual t_reduce_result reduce_flipped(t_operation_push const & first_op) const {
// 		t_operation_ptr op(new t_operation_push(t_token(static_cast<std::string const &>(first_op.x()) 
// 														+  static_cast<std::string const &>(x()))));
// 		return std::make_pair(op, true); }

// 	/// append->append is just an append operation 
// 	virtual t_reduce_result reduce_flipped(t_operation_append const & first_op) const {
// 		t_operation_ptr op(new t_operation_append(t_token(static_cast<std::string const &>(first_op.x()) 
// 														+  static_cast<std::string const &>(x()))));
// 		return std::make_pair(op, true); }


// private:
// 	std::string const string(std::string const & res) const {
// 		std::stringstream ss; ss << "[append \"" <<x_<<"\"->\""<< res <<"\"] "; return ss.str(); }

// };

// //Static Member
// t_operation_append::t_append_cache t_operation_append::cache_(t_operation_append::t_make_append_result(), CACHE_SIZE);

// class t_operation_interp : public t_operation {

// 	///interpolates the attribute_value value into a token
// 	///todo remove this once you can type x.token()
// 	struct do_interp_visitor  :  public boost::static_visitor<void> {
// 		///todo put back thonsew 		
// 		// : public config::attribute_value::default_visitor {
// 		// using default_visitor::operator();
// 		t_token & self_;
// 		do_interp_visitor(t_token & t):self_(t) {}
// 		void operator()() { self_ = z_empty; }
// 		void operator()(bool const b) {self_ =  t_token(str_cast(b)); }
// 		void operator()(int const i) {self_ =  t_token(str_cast(i)); }
// 		void operator()(double const d) {self_ =  t_token( str_cast(d)); }
// 		void operator()(n_token::t_token const & tok){self_ = tok; }
// 		void operator()(const t_string &s){ self_=t_token(s);}
// 		///todo remove 3 lines
// 		void operator()(boost::blank const &){self_ = z_empty; }
// 		void operator()(std::string const & s){self_=t_token(s);}
// 	};

// public :
// 	virtual void operator()(t_tokens & stack,  variable_set const & variable_set) const {

// 		t_token var_name(stack.back());
// 		config::attribute_value stuffing(variable_set.get_variable_const(var_name ));
// 		DEBUG_NG << string(var_name, stuffing) << "\n"; 

// 		///todo replace next 4 lines with this->lhs_ = stuffing.token();	
// 		t_token rhs;
// 		do_interp_visitor visitor(rhs);
// 		///todo swap next 2
// 		//stuffing.apply_visitor(visitor);
// 		boost::apply_visitor(visitor, stuffing.to_value());
		
// 		stack.pop_back();
// 		t_operation_append append(rhs);		
// 		append(stack, variable_set);
// 	}

// 	virtual t_reduce_result reduce(t_operation_ptr const & second_op) const {
// 		return second_op->reduce_flipped(*this); }

// 	std::string const string() const {return "[interpolate] "; }
// private:
// 	std::string const string(std::string const & var, std::string const & res) const {
// 		std::stringstream ss; ss << "[interpolate \"" << var <<"\"->\""<< res <<"\"] "; return ss.str(); }
// };


// struct t_operation_formula : public t_operation {

// 	virtual void operator()(t_tokens & stack,  variable_set const & variable_set) const {
// 		t_token var_name(stack.back());
// 		try {
// 			const game_logic::formula form(static_cast<std::string const &>(var_name) );
// 			t_token rhs(form.evaluate().string_cast());
// 			stack.pop_back();
// 			t_operation_append append(rhs);		
// 			append(stack, variable_set);
// 			DEBUG_NG << string(var_name, static_cast<std::string const  &>(rhs) ) << "\n"; 
// 		} catch(game_logic::formula_error& e) {
// 			ERR_NG << "Formula in WML string cannot be evaluated due to "
// 				   << e.type << "\n\t--> \""
// 				   << e.formula << "\"\n";
// 		}
// 	}

// 	virtual t_reduce_result reduce(t_operation_ptr const & second_op) const {
// 		return second_op->reduce_flipped(*this); }

// 	std::string const string() const {return "[formula] "; }
// private:
// 	std::string const string(std::string const & var, std::string const & res) const {
// 		std::stringstream ss; ss << "[formula \"" << var <<"\"->\""<< res <<"\"] "; return ss.str(); }
// };

// /**
//    In the event of a nested expansion wml expects embedded invalid characters to terminate the
//    external expansion, but embedded $ do not result in recursive expansion of variables.
//    This is the only jump in the mini language.  Here is an example
//    $outer$inner, where $inner is _stop#_extra_stuff, and $outer_stop front in the symbol table
//    becomes front#extra_stuff
//  */
// // struct t_operation_nested_expansion : public t_operation {
// // 	virtual void operator()(t_tokens & stack,  variable_set const & variable_set) const {

// // 		t_token interpolated_variable(stack.back());
// // 		t_tokenizer tokenizer(interpolated_variable);
// // 		t_tokens tokens = tokenizer.tokenize();
// // 		if(tokens.size() > 1){
// // 			stack.pop();
// // 			t_tokens::iterator i(tokens.begin);
// // 			while(i!=tokens.end() && is_good_for_varname(*i)){
				
// // 				++i;
// // 			}
// // 		}
// // 		try {
// // 			const game_logic::formula form(static_cast<std::string const &>(var_name) );
// // 			t_token rhs(form.evaluate().string_cast());
// // 			stack.pop_back();
// // 			t_operation_append append(rhs);		
// // 			append(stack, variable_set);
// // 			DEBUG_NG << string(var_name, static_cast<std::string const  &>(rhs) ) << "\n"; 
// // 		} catch(game_logic::formula_error& e) {
// // 			ERR_NG << "Formula in WML string cannot be evaluated due to "
// // 				   << e.type << "\n\t--> \""
// // 				   << e.formula << "\"\n";
// // 		}
// // 	}
// // 	std::string const string() const {return "[nested expansion] "; }
// // private:
// // 	std::string const string(std::string const & var, std::string const & res) const {
// // 		std::stringstream ss; ss << "[formula \"" << var <<"\"->\""<< res <<"\"] "; return ss.str(); }
	
// // }

// ///Reduction functions all take 2 operations and reduce them to one if possible and push them on the stack
// // template<typename X, typename Y> void reduce(X const &x, Y const & y, t_operations & ops) {
// // 	std::cerr<<"R&R\n";
// // 	ops.push_back(x);
// // 	ops.push_back(y); }

// // template<typename Y> void reduce(t_operation_noop const &x, Y const & y, t_operations & ops) {
// // 	std::cerr<<"R&R2\n";
// // 	ops.push_back(y); }
// // template<typename X> void reduce(X const & x, t_operation_noop const & y, t_operations & ops) {
// // 	ops.push_back(x); }

// // void reduce(t_operation_push const &x, t_operation_append const & y, t_operations & ops) {
// // 	std::cerr<<"R&R3\n";
// // 	t_operation_ptr op(new t_operation_push(t_token(static_cast<std::string const &>(x.x()) 
// // 													+  static_cast<std::string const &>(y.x()))));
// // 	ops.push_back(op); }

// // void reduce(t_operation_append const &x, t_operation_append const & y, t_operations & ops) {
// // 	std::cerr<<"R&R4\n";
// // 	t_operation_ptr op(new t_operation_append(t_token(static_cast<std::string const &>(x.x()) 
// // 											  +  static_cast<std::string const &>(y.x()))));
// // 	ops.push_back(op); }


// // struct t_operation_replace : public t_operation {
// // 	t_token out_;
// // 	t_operation_replace(t_token const & a) : out_(a){}
// // 	virtual t_token operator()(t_token const & /*results*/,  variable_set const & /*variable_set*/) const { 
// // 		std::cerr<<"DDo replace  "; return out_;}
// // };


// // struct t_operation_prepend : public t_operation {
// // 	t_token lhs_;
// // 	struct t_make_prepend_result {
// // 		t_token operator()(std::pair<t_token, t_token> const & x) const {
// // 			return t_token(static_cast<std::string const &>(x.first) +  static_cast<std::string const &>(x.second)); }
// // 	};

// // 	typedef  n_lru_cache::t_lru_cache<std::pair<t_token, t_token>, t_token, t_make_prepend_result> t_prepend_cache;
// // 	static t_prepend_cache cache_;

// // 	t_operation_prepend(t_token const & a) : lhs_(a){}
	
// // 	virtual t_token operator()(t_token const & rhs,  variable_set const & /*variable_set*/) const { 
// // 		t_token const &res = cache_.check(std::make_pair(lhs_, rhs)); 
// // 		std::cerr<<"DDo prepend  "<<lhs_<<" "<<rhs<<" ="<<res<<" ";
// // 		return res; }
// // };

// //static member
// //t_operation_prepend::t_prepend_cache t_operation_prepend::cache_(t_operation_prepend::t_make_prepend_result(), CACHE_SIZE);


// // struct t_operation_interp : public t_operation {
// // 	t_token var_name_;

// // 	///interpolates the attribute_value value into a token
// // 	///todo remove this once you can type x.token()
// // 	struct do_interp_visitor  :  public boost::static_visitor<void> {
// // 		///todo put back thonsew 		
// // 		// : public config::attribute_value::default_visitor {
// // 		// using default_visitor::operator();
// // 		t_token & self_;
// // 		do_interp_visitor(t_token & t):self_(t) {}
// // 		void operator()() { self_ = z_empty; }
// // 		void operator()(bool const b) {self_ =  t_token(str_cast(b)); }
// // 		void operator()(int const i) {self_ =  t_token(str_cast(i)); }
// // 		void operator()(double const d) {self_ =  t_token( str_cast(d)); }
// // 		void operator()(n_token::t_token const & tok){self_ = tok; }
// // 		void operator()(const t_string &s){ self_=t_token(s);}
// // 		///todo remove 3 lines
// // 		void operator()(boost::blank const &){self_ = z_empty; }
// // 		void operator()(std::string const & s){self_=t_token(s);}
// // 	};

// // 	t_operation_interp(t_token const & a) : var_name_(a){}
	
// // 	virtual t_token operator()(t_token const & rhs,  variable_set const & variable_set) const { 
// // 	std::cerr<<"DDo interp  ";
// // 		config::attribute_value stuffing(variable_set.get_variable_const(var_name_));
// // 	std::cerr<<"varname in interp \""<<var_name_<<"\"->repval \""<<stuffing<<"\""; 

// // 		///todo replace next 4 with this->lhs_ = stuffing.token();
// // 		t_token lhs;
// // 		do_interp_visitor visitor(lhs);
// // 		///todo swap next 2
// // 		//stuffing.apply_visitor(visitor);
// // 		boost::apply_visitor(visitor, stuffing.to_value());
		
// // 		t_operation_prepend prepend(lhs);

// // 		return prepend(rhs, variable_set); 
// // 	}
// // };



// // struct t_operation_formula : public t_operation {
// // 	t_token var_name_;

// // 	t_operation_formula(t_token const & a) : var_name_(a){}
	
// // 	virtual t_token operator()(t_token const & rhs,  variable_set const & variable_set) const { 
// // 		std::cerr<<"DDo formula	  ";
// // 		try {
// // 			const game_logic::formula form(static_cast<std::string const &>(var_name_) );
// // 			t_token lhs(form.evaluate().string_cast());
// // 			t_operation_prepend prepend(lhs);
// // 			return prepend(rhs, variable_set); 
// // 		} catch(game_logic::formula_error& e) {
// // 			ERR_NG << "Formula in WML string cannot be evaluated due to "
// // 				   << e.type << "\n\t--> \""
// // 				   << e.formula << "\"\n";
// // 		}
// // 		return rhs;
// // 		}
// // };


// /**  A vector of operations that can be optimized and run
//  */
// class t_instructions {
// 	t_operations ops_;
// public:
// 	void optimize() {
// 		t_operations optimized;
// 		DEBUG_NG << "Optimizing these instructions : \n" << *this <<"\n";
// 		if(ops_.size() > 1){
// 			bool last_instruction_optimized(false);
// 			t_operations::iterator i_not (ops_.begin() + 1);

// 			while(i_not!=ops_.end()){
// 				//reduce either the last 2 instructions or the last instruction and the last optimized instruction
// 				t_operation_ptr const & first_op = (last_instruction_optimized == true) ? optimized.back() : *(i_not-1);

// 				t_operation::t_reduce_result reduced =first_op->reduce(*i_not);
					
// 				if(reduced.second){
// 					DEBUG_NG <<"Optimizing "<< first_op << " + " << (**i_not) << " to "<< *reduced.first<<"\n";
// 					if(last_instruction_optimized){ optimized.pop_back(); }
// 					last_instruction_optimized = true;
// 					optimized.push_back(reduced.first); } 
// 				else{
// 					DEBUG_NG <<"NOT Optimizing "<< *first_op << " + " << (**i_not) << " \n";
// 					if(last_instruction_optimized){ }
// 					else { optimized.push_back(first_op); }
// 					last_instruction_optimized = false; }

// 				++i_not;
// 			}

// 			if(! last_instruction_optimized){ optimized.push_back( ops_.back() ); } 

// 			ops_.swap(optimized);
// 		}
// 		DEBUG_NG << "Optimized instructions : \n" << *this <<"\n";
// 	}

// 	///runs the instructions and places the results in @param[out] x
// 	void run(t_token & x, variable_set const & set) const {
// 		t_tokens stack;
// 		assert(!ops_.empty());

// 		//DEBUG_NG << "Running these instructions : \n" << *this <<"\n";

// 		t_operations::const_iterator i (ops_.begin());
// 		for(; i!= ops_.end(); ++i){
// 			(**i)(stack, set); }
// 		assert(stack.size() == 1);
// 		x = stack.back();
// 	}
// 	t_operations & ops() { return ops_; }
// 	t_operations const & ops() const { return ops_; }

// 	friend std::ostream & operator<<(std::ostream &out, t_instructions const & a){
// 		t_operations::const_iterator i (a.ops_.begin());
// 		for(; i!= a.ops_.end(); ++i){
// 			out <<(**i) <<"\n";
// 		}
// 		return out; }
	
// };



// 	///Stores the incremental parse information
// 	struct t_parse {
// 		//		enum t_state {DONE, INTERP, INTERP_DOLLAR, FORMULA, CONTINUE, ERROR};
		
// 		n_token::t_token token_; ///The input token
// 		t_tokens tokens_; ///Its tokens

// 		// t_tokens::iterator itoken_;
// 		// n_token::t_token var_name_; ///The string for interpolation
// 		// std::string::const_iterator var_begin_, var_end_; ///Start and end of the string for interpolation
// 		// t_state state_; 
// 		// int rfind_dollars_sign_from_; ///Current position to start searching for $ for interpolation

// 		t_instructions complete_parse_;

// 		friend std::ostream & operator<<(std::ostream &out, t_parse const & a){
// 			out<<"TOKENS ";
// 			foreach(t_token const &tok, a.tokens_){out <<" \""<<tok<<"\"";}
// 			out<<" /TOKENS \n OPS ";
// 			t_operations const & ops( a.complete_parse_.ops() );
// 			t_operations::const_iterator i(ops.begin());
// 			for(; i != ops.end(); ++i){ out << *i ;}
// 			return out<<" /OPS \n";
// 		}
// 		// friend std::ostream & operator<<(std::ostream &out, t_parse const & a){
// 		// 	return out << "state:tk:var ["<<a.state_<<" : "<<a.token_<<" : "<<a.var_name_<<" : "
// 		// 			   <<" ] "; }
		
// 		t_parse(n_token::t_token const & unparsed) 
// 			: token_(unparsed), complete_parse_() {}

// 		t_parse(t_parse const & a )
// 			: token_(a.token_), complete_parse_(a.complete_parse_) {}

// 		t_parse & operator=(t_parse const & a) {
// 			token_ = (a.token_); 
// 			complete_parse_ = a.complete_parse_;
// 			return *this; }

// 		///Calculate a parse
// 		t_instructions & parse(){
// 			t_tokenizer tokenizer(token_);
// 			tokens_ = tokenizer.tokenize();
// 			complete_parse_.ops().clear();

// 			// t_operation_ptr op(new t_operation_replace(z_empty));
// 			// complete_parse_.ops_.push_back(op);

// 			do_parse_plain(tokens_.begin());

// 			return complete_parse_;
// 		}

// 		t_token const & peek_next(t_tokens::iterator const & curr_pos ){
// 			t_tokens::iterator next(curr_pos + 1);
// 			return (next != tokens_.end() ) ? (*next) : z_empty; }

// 		//Keep concatenating components until you see a $ which indicates some kind of interpolation
// 		t_tokens::iterator do_parse_plain(t_tokens::iterator const & start_pos ){
// 			t_tokens::iterator curr_pos = start_pos;

// 			while(curr_pos != tokens_.end()) {

// 				if(*curr_pos == z_dollar) {
// 					//$| inserts just a $
// 					if( peek_next(curr_pos) == z_bar){
// 						static const t_operation_ptr op(new t_operation_append(z_dollar));
// 						complete_parse_.ops().push_back(op); 
// 						curr_pos+=2; }
				
// 					//A interpolation / formula
// 					else { 
// 						size_t good_size = complete_parse_.ops().size();
// 						try {
// 							curr_pos = do_parse_interp(++curr_pos); } 
// 						catch (wml_syntax_error & e) {
// 							ERR_NG << e.what()<<"\n";
// 							if(complete_parse_.ops().size() > good_size){ 
// 								complete_parse_.ops().resize(good_size); } } } }
					
// 				//text variable name component
// 				else {
// 					t_operation_ptr op(new t_operation_append(*curr_pos));
// 					complete_parse_.ops().push_back(op); 
// 					++curr_pos; } 
// 			}			
// 			return curr_pos;
// 		}

// 		t_tokens::iterator do_parse_interp(t_tokens::iterator const & start_pos ){			
// 			bool found_part_of_name(false);
// 			t_tokens::iterator curr_pos = start_pos + 1;

// 			//A $ at the end of the line is just a $
// 			if(start_pos == tokens_.end()){
// 				static const t_operation_ptr op(new t_operation_append(z_dollar));
// 				complete_parse_.ops().push_back(op);
// 				return start_pos; }

// 			//Start of the variable name
// 			if (is_good_for_varname(*start_pos)){
// 				found_part_of_name = true;
// 				t_operation_ptr op(new t_operation_push(*start_pos));
// 				complete_parse_.ops().push_back(op); }			

// 			else if(*start_pos == z_dollar){
// 				// $| Creates a $ to allow for $ in the string
// 				if( peek_next(start_pos) == z_bar){
// 					throw wml_syntax_error(tokens_, start_pos - tokens_.begin(), " $$| creates an illegal variable name"); }

// 				//A Nested interpolation / formula
// 				else {
// 					curr_pos = do_parse_interp(start_pos + 1); 
// 					//todo this is a small lie if the interpolated result contains a control character
// 					found_part_of_name = true; } }

// 			// $( ... ) Creates a WML formula 
// 			else if (*start_pos == z_lparen){  
// 				return do_parse_formula(start_pos); }

// 			// Everything else is an error
// 			 else { throw wml_syntax_error(tokens_, start_pos - tokens_.begin(), "missing variable name right after $"); }
			
// 			// Grab and append tokens until we reach the end of the variable name
// 			int bracket_nesting_level = 0;

// 			while(curr_pos != tokens_.end()){ 

// 				//text variable name component
// 				if (is_good_for_varname(*curr_pos)){
// 					found_part_of_name = true;
// 					t_operation_ptr op(new t_operation_append(*curr_pos));
// 					complete_parse_.ops().push_back(op); 
// 					++curr_pos; }

// 				else if(*curr_pos == z_dollar){
// 					// $| Creates a $ to allow for $ in the string
// 					if( peek_next(curr_pos) == z_bar){
// 						break; }

// 					//A Nested interpolation / formula
// 					else{
// 						found_part_of_name = true;
// 						curr_pos = do_parse_interp(++curr_pos); } }

// 				//[] Nesting brackets
// 				else if(*curr_pos == z_lbracket){
// 					static const t_operation_ptr op(new t_operation_append(z_lbracket));
// 					complete_parse_.ops().push_back(op);
// 					++bracket_nesting_level; 
// 					++curr_pos; }
// 				else if(*curr_pos == z_rbracket){
// 					if((--bracket_nesting_level) < 0) { break; } 
// 					static const t_operation_ptr op(new t_operation_append(z_rbracket));
// 					complete_parse_.ops().push_back(op); 
// 					++curr_pos; }

// 				//Dots
// 				//two dots terminates variable expansion
// 				// That matters for random=, e.g. $x..$y
// 				else if(*curr_pos == z_dot){
// 					if(peek_next(curr_pos) == z_dot){ break; } 
// 					static const t_operation_ptr op(new t_operation_append(z_dot));
// 					complete_parse_.ops().push_back(op); 
// 					++curr_pos; }

// 				//A | terminates the variable name
// 				// It's been used to end this variable name; now it has no more effect.
// 				// This can allow use of things like "$$composite_var_name|.x"
// 				// (Yes, that's a WML 'pointer' of sorts. They are sometimes useful.)
// 				else if(*curr_pos == z_bar) {
// 					++curr_pos;
// 					break; }

// 				//Invalid variable name component
// 				else if (!is_good_for_varname(*curr_pos)){
// 					break; }
				
// 				//Never happen
// 				else { assert(false); }
// 			}
			
// 			// If the last character is '.', then it can't be a sub-variable.
// 			// It's probably meant to be a period instead. Don't include it.
// 			// Would need to do it repetitively if there are multiple '.'s at the end,
// 			// but don't actually need to do so because the previous check for adjacent '.'s would catch that.
// 			// For example, "My score is $score." or "My score is $score..."
// 			if(*(curr_pos - 1) == z_dot && (curr_pos-1) != start_pos ){
// 			   // However, "$array[$i]" by itself does not name a variable,
// 			   // so if "$array[$i]." is encountered, then best to include the '.',
// 			   // so that it more closely follows the syntax of a variable (if only to get rid of all of it).
// 			   // (If it's the script writer's error, they'll have to fix it in either case.)
// 			   // For example in "$array[$i].$field_name", if field_name does not exist as a variable,
// 			   // then the result of the expansion should be "", not "." (which it would be if this exception did not exist).
// 				if( *(curr_pos-2) != z_rbracket) {
// 					complete_parse_.ops().pop_back();
// 					--curr_pos; }
// 				else {
// 					throw wml_syntax_error(tokens_, curr_pos - tokens_.begin()
// 										   , "missing field name after ]. (correct usage: $array[$i].fieldname)"); } }

// 			if(! found_part_of_name) {
// 				 throw wml_syntax_error(tokens_, start_pos - tokens_.begin()
// 										, "missing variable name after $ (valid characters are a-zA-Z0-9_)"); }
			
// 			//FInally an interpolation
// 			t_operation_ptr op(new t_operation_interp());
// 			complete_parse_.ops().push_back(op);
// 			return curr_pos;
// 		}

// 		/** do_parse forumal parses a formula invoked by $ (...) syntax
// 		 */
// 		t_tokens::iterator do_parse_formula(t_tokens::iterator const & start_pos ){
			
// 			//Deal with the first token in the iterpolated variable
// 			if(start_pos == tokens_.end()){
// 				 throw wml_syntax_error(tokens_, start_pos - tokens_.begin(), "formula hitting end of line"); }

// 			if(*start_pos != z_lparen){
// 				 throw wml_syntax_error(tokens_, start_pos - tokens_.begin(), "missing paren in formula should be $( ...)"); }

// 			// Grab and append tokens until we reach the end of the variable name
// 			t_tokens::iterator curr_pos = start_pos + 1; // +1 is skipping the left paren
// 			int paren_nesting_level = 0;
// 			bool in_string = false, in_comment = false;

// 			for(;curr_pos != tokens_.end(); ++curr_pos){

// 				//() Nesting
// 				if(*curr_pos == z_lparen){
// 					static const t_operation_ptr op(new t_operation_append(z_lparen));
// 					complete_parse_.ops().push_back(op);
// 					if(!in_string && !in_comment) {
// 						++paren_nesting_level; } }
// 				else if(*curr_pos == z_rparen){
// 					if(!in_string && !in_comment) {
// 						--paren_nesting_level; }
// 					if( paren_nesting_level  <= 0) { break; } 
// 					static const t_operation_ptr op(new t_operation_append(z_rparen));
// 					complete_parse_.ops().push_back(op); }

// 				else if(*curr_pos == z_sharp){
// 					static const t_operation_ptr op(new t_operation_append(z_sharp));
// 					complete_parse_.ops().push_back(op);
// 					if(!in_string ) {
// 						in_comment = !in_comment; }}

// 				else if(*curr_pos == z_single_quote){
// 					static const t_operation_ptr op(new t_operation_append(z_single_quote));
// 					complete_parse_.ops().push_back(op);
// 					if(!in_comment ) {
// 						in_string = !in_string; }}

// 				else {
// 					t_operation_ptr op(new t_operation_append(*curr_pos));
// 					complete_parse_.ops().push_back(op); }
// 			}
			
// 			if(paren_nesting_level > 0) {
// 				throw wml_syntax_error(tokens_, curr_pos - tokens_.begin(), "formula in WML string cannot be evaluated due to "
// 									   "missing closing paren"); }

// 			t_operation_ptr op(new t_operation_formula());
// 			complete_parse_.ops().push_back(op);
// 			return curr_pos; //skip the closing right paren
		
// 		}
// 	};

// 		/*		void do_parse_step2(){
// 			//			std::cerr<<"( in parse step : "<<token_<<" )";
// 			std::string const & res(token_);
// 			int start_point = rfind_dollars_sign_from_;
// 			// (This is only false when the previous '$' was at index 0)
// 			if(rfind_dollars_sign_from_ >= 0) {
// 				// Going in a backwards order allows nested variable-retrieval, e.g. in arrays.
// 				// For example, "I am $creatures[$i].user_description!"
// 				const std::string::size_type var_begin_loc = res.rfind('$', rfind_dollars_sign_from_);

// 				// If there are no '$' left then we're done.
// 				if(var_begin_loc == std::string::npos) {
// 					t_token first_piece( res.substr(0, rfind_dollars_sign_from_+1 ));
// 					std::cerr<<"first piece"<<first_piece<<" of "<<res<<"rfd"<<rfind_dollars_sign_from_<<"\n";
// 					t_operation_ptr op(new t_operation_prepend(first_piece));
// 					complete_parse_.ops_.push_back(op);
// 					state_ = DONE;
// 					return;
// 				}

// 				// For the next iteration of the loop, search for more '$'
// 				// (not from the same place because sometimes the '$' is not replaced)
// 				rfind_dollars_sign_from_ = int(var_begin_loc) - 1;

// 				var_begin_ = res.begin() + var_begin_loc;

// 				// The '$' is not part of the variable name.
// 				const std::string::const_iterator var_name_begin = var_begin_ + 1;
// 				var_end_ = var_name_begin;

// 				if(var_name_begin == res.end()) {
// 					// Any '$' at the end of a string is just a '$'
// 					state_ = CONTINUE;
// 					t_operation_ptr op(new t_operation_replace(z_dollar));
// 					complete_parse_.ops_.push_back(op);
// 					return;
// 				}
// 				if(*var_name_begin == '(') {
// 					// The $( ... ) syntax invokes a formula
// 					int paren_nesting_level = 0;
// 					bool in_string = false,
// 						in_comment = false;
// 					do {
// 						switch(*var_end_) {
// 						case '(':
// 							if(!in_string && !in_comment) {
// 								++paren_nesting_level;
// 							}
// 							break;
// 						case ')':
// 							if(!in_string && !in_comment) {
// 								--paren_nesting_level;
// 							}
// 							break;
// 						case '#':
// 							if(!in_string) {
// 								in_comment = !in_comment;
// 							}
// 							break;
// 						case '\'':
// 							if(!in_comment) {
// 								in_string = !in_string;
// 							}
// 							break;
// 							// TODO: support escape sequences when/if they are allowed in FormulaAI strings
// 						}
// 					} while(++var_end_ != res.end() && paren_nesting_level > 0);
// 					if(paren_nesting_level > 0) {
// 						ERR_NG << "Formula in WML string cannot be evaluated due to "
// 							   << "missing closing paren:\n\t--> \""
// 							   << std::string(var_begin_, var_end_) << "\"\n";
// 						state_ = ERROR;
// 						return;
// 					}
// 					state_ = FORMULA;

// 					t_token formula( std::string(static_cast<std::string const &>(var_name_).begin() + 2
// 												 , static_cast<std::string const &>(var_name_).end() - 1)  );
// 					t_operation_ptr op(new t_operation_formula( formula));
// 					complete_parse_.ops_.push_back(op);
// 					return;;
// 				}

// 				// Find the maximum extent of the variable name (it may be shortened later).
// 				for(int bracket_nesting_level = 0; var_end_ != res.end(); ++var_end_) {
// 					const char c = *var_end_;
// 					if(c == '[') {
// 						++bracket_nesting_level;
// 					}
// 					else if(c == ']') {
// 						if(--bracket_nesting_level < 0) {
// 							break;
// 						}
// 					}
// 					// isascii() breaks on mingw with -std=c++0x
// 					else if (!(((c) & ~0x7f) == 0)/.*isascii(c)*./ || (!isalnum(c) && c != '.' && c != '_')) {
// 						break;
// 					}
// 				}

// 				// Two dots in a row cannot be part of a valid variable name.
// 				// That matters for random=, e.g. $x..$y
// 				var_end_ = std::adjacent_find(var_name_begin, var_end_, two_dots);

// 				// If the last character is '.', then it can't be a sub-variable.
// 				// It's probably meant to be a period instead. Don't include it.
// 				// Would need to do it repetitively if there are multiple '.'s at the end,
// 				// but don't actually need to do so because the previous check for adjacent '.'s would catch that.
// 				// For example, "My score is $score." or "My score is $score..."
// 				if(*(var_end_-1) == '.'
// 				   // However, "$array[$i]" by itself does not name a variable,
// 				   // so if "$array[$i]." is encountered, then best to include the '.',
// 				   // so that it more closely follows the syntax of a variable (if only to get rid of all of it).
// 				   // (If it's the script writer's error, they'll have to fix it in either case.)
// 				   // For example in "$array[$i].$field_name", if field_name does not exist as a variable,
// 				   // then the result of the expansion should be "", not "." (which it would be if this exception did not exist).
// 				   && *(var_end_-2) != ']') {
// 					--var_end_;
// 				}

// 				const std::string var_name(var_name_begin, var_end_);

// 				if(var_end_ != res.end() && *var_end_ == '|') {
// 					// It's been used to end this variable name; now it has no more effect.
// 					// This can allow use of things like "$$composite_var_name|.x"
// 					// (Yes, that's a WML 'pointer' of sorts. They are sometimes useful.)
// 					// If there should still be a '|' there afterwards to affect other variable names (unlikely),
// 					// just put another '|' there, one matching each '$', e.g. "$$var_containing_var_name||blah"
// 					++var_end_;
// 				}


// 				if (var_name == "") {
// 					// Allow for a way to have $s in a string.
// 					// $| will be replaced by $.
// 					// std::string res2(res);
// 					// res2.replace(var_begin_-res.begin(), var_end_-var_begin_, "$");
// 					// token_ = n_token::t_token(res2);
// 					state_ = INTERP_DOLLAR;
// 					//++var_begin_;
// 					std::cerr<<"\nFFFFF "<<token_<<" doll "<< rfind_dollars_sign_from_ << " begin "
// 							 << *var_begin_ << " end " << *var_end_<<"\n";
// 					 //state_ = CONTINUE;
// 					 //--rfind_dollars_sign_from_;
// 					// --rfind_dollars_sign_from_;
// 					// ++var_end_;
// 					//var_name_ = n_token::t_token::z_empty();					
// 					t_operation_ptr op(new t_operation_prepend(z_dollar));
// 					complete_parse_.ops_.push_back(op);
// 				} else {
// 					// The variable is replaced with its value.
// 					state_ = INTERP;
// 					var_name_ = n_token::t_token(var_name);
// 					t_operation_ptr op(new t_operation_interp(var_name_));
// 					complete_parse_.ops_.push_back(op);
// 					std::cerr<<"\nGGG "<<token_<<" doll "<< rfind_dollars_sign_from_ << " begin "
// 							 << *var_begin_ << " end " << *var_end_<<"\n";
// 				}
// 				return;
// 			} else {
// 				state_ = DONE;
// 				return ;
// 			}
// 		}
// 	};
// 		*/

// 	// typedef n_token::t_token t_interp;

// 	// ///Default interpolation cache result producer
// 	// struct t_empty_interp {
// 	// 	t_interp operator()(config::attribute_value const & ){
// 	// 		return t_interp();}		
// 	// };

// 	// typedef  n_lru_cache::t_lru_cache<config::attribute_value, t_interp, t_empty_interp> t_interp_cache;

// 	// ///Do the interpolation, formula insertion or error output
// 	// struct t_parse_and_friends {
// 	// 	t_parse parse_; ///The input parse
// 	// 	t_interp_cache interp_cache_;  ///A cache of interpolated results for this parse input
// 	// 	t_parse_and_friends(t_parse const & a, t_interp_cache const & b=t_interp_cache(t_empty_interp(), CACHE_SIZE)) 
// 	// 		: parse_(a), interp_cache_(b){}
// 	// 	t_parse_and_friends(t_parse_and_friends const & a) : parse_(a.parse_), interp_cache_(a.interp_cache_){}

// 	// 	///interpolates the attribute_value value into the string
// 	// 	struct do_interp_visitor  :  public boost::static_visitor<void> {
// 	// 	///todo put back thonsew 		
// 	// 	// : public config::attribute_value::default_visitor {
// 	// 	// using default_visitor::operator();
// 	// 		t_parse const & self_;
// 	// 		std::string & result_;
// 	// 		std::string const & stoken_;
// 	// 		do_interp_visitor(t_parse const & s, std::string &r):self_(s), result_(r), stoken_(s.token_){}
// 	// 		void operator()() {
// 	// 			result_.replace(self_.var_begin_ - stoken_.begin(), self_.var_end_ - self_.var_begin_, ""); }
// 	// 		void operator()(bool const b) {	
// 	// 			result_.replace(self_.var_begin_ - stoken_.begin(), self_.var_end_ - self_.var_begin_, str_cast(b)); }
// 	// 		void operator()(int const i) {
// 	// 			result_.replace(self_.var_begin_ - stoken_.begin(), self_.var_end_ - self_.var_begin_, str_cast(i)); }
// 	// 		void operator()(double const d) {
// 	// 			result_.replace(self_.var_begin_ - stoken_.begin(), self_.var_end_ - self_.var_begin_, str_cast(d)); }
// 	// 		void operator()(n_token::t_token const & tok){
// 	// 			result_.replace(self_.var_begin_ - stoken_.begin(), self_.var_end_ - self_.var_begin_, tok); }
// 	// 		void operator()(const t_string &s){
// 	// 			result_.replace(self_.var_begin_ - stoken_.begin(), self_.var_end_ - self_.var_begin_, s); }
// 	// 		///todo remove 3 lines
// 	// 		void operator()(boost::blank const &){
// 	// 			result_.replace(self_.var_begin_ - stoken_.begin(), self_.var_end_ - self_.var_begin_, ""); }
// 	// 		void operator()(std::string const & tok){
// 	// 			result_.replace(self_.var_begin_ - stoken_.begin(), self_.var_end_ - self_.var_begin_, tok); }
// 	// 	};


// 	// 	///Create a cache value for a given input parse and stuffing
// 	// 	struct t_make_interp {
// 	// 		t_parse const * parse_;
// 	// 		t_make_interp(t_parse const & p) : parse_(&p){}
// 	// 		t_interp operator()(config::attribute_value const & stuffing){
// 	// 			//	std::cerr<<"Cache Miss "<<*parse_<<" stuffed \""<<stuffing<<"\"     ";
// 	// 			std::string const & res(parse_->token_);
// 	// 			std::string res2(res);
// 	// 			do_interp_visitor visitor(*parse_, res2);
// 	// 			///todo swap next 2
// 	// 			//stuffing.apply_visitor(visitor);
// 	// 			boost::apply_visitor(visitor, stuffing.to_value());
// 	// 			return t_interp(res2); }
// 	// 	};

// 	// 	///todo add formulae to the cache

// 	// 	///Process the parse
// 	// 	///@param[in] this the parse
// 	// 	///@param[in] set is the set of variables to search for the interpolated variable given by var_name_
// 	// 	n_token::t_token do_process_parse(const variable_set& set){
// 	// 		assert(parse_.state_ != t_parse::CONTINUE );
// 	// 		assert(parse_.state_ !=  t_parse::DONE);

// 	// 		switch(parse_.state_){
// 	// 		case t_parse::INTERP: {
// 	// 				std::cerr<<"varname in interp \""<<parse_.var_name_<<"\"->repval \""<<set.get_variable_const(parse_.var_name_)<<"\""; 
// 	// 			config::attribute_value stuffing(set.get_variable_const(parse_.var_name_));
// 	// 			t_make_interp calc(parse_);
// 	// 			n_token::t_token & result(interp_cache_.check(stuffing, calc));
// 	// 			//				std::cerr<<"->result \""<<result<<"\" ";
// 	// 			return result;
// 	// 		}
// 	// 			break;
// 	// 		case t_parse::INTERP_DOLLAR: {
// 	// 			std::cerr<<"varname in interp_dollar \""<<parse_.var_name_<<"\"->repval \""<<set.get_variable_const(parse_.var_name_)<<"\""; 
// 	// 			static const n_token::t_token z_dollar("$", false);
// 	// 			///todo remove the cast when attribute_value supports t_token directly
// 	// 			config::attribute_value stuffing;
// 	// 			stuffing = (static_cast<std::string const &>(z_dollar));
// 	// 			t_make_interp calc(parse_);
// 	// 			n_token::t_token & result(interp_cache_.check(stuffing, calc));
// 	// 							std::cerr<<"->result \""<<result<<"\" ";
// 	// 			return result;
// 	// 		}
// 	// 			break;
// 	// 		case t_parse::FORMULA: {
// 	// 			std::string const & res(parse_.token_);
// 	// 			std::string res2(res);
// 	// 			try {
// 	// 				const game_logic::formula form(std::string(parse_.var_begin_ + 2, parse_.var_end_ - 1));
// 	// 				res2.replace(parse_.var_begin_ - res.begin(), parse_.var_end_ - parse_.var_begin_ , form.evaluate().string_cast());
// 	// 			} catch(game_logic::formula_error& e) {
// 	// 				ERR_NG << "Formula in WML string cannot be evaluated due to "
// 	// 					   << e.type << "\n\t--> \""
// 	// 					   << e.formula << "\"\n";
// 	// 				res2.replace(parse_.var_begin_ - res.begin(), parse_.var_end_ - parse_.var_begin_, "");
// 	// 			}
// 	// 			return n_token::t_token(res2);
// 	// 			break;
// 	// 		}
// 	// 		case t_parse::ERROR: {
// 	// 			std::string const & res(parse_.token_);
// 	// 			std::string res2(res);
// 	// 			res2.replace(parse_.var_begin_ - res.begin(), parse_.var_end_ - parse_.var_begin_, "");
// 	// 			return n_token::t_token(res2);
// 	// 			break;
// 	// 		}
// 	// 		default : assert(false);
// 	// 		}
// 	// 		return n_token::t_token();
// 	// 	}

// 	// };


// /**  Parses and possibly stores variables for subsequent interpolation */
// class t_parse_and_interpolator {

// 	struct t_parser {
// 		/// Generate a parse for an  uncached token	
// 		t_instructions operator()(n_token::t_token const & unparsed){
// 			t_parse aparse(unparsed);
// 			t_instructions  instructions(aparse.parse());
// 			instructions.optimize();
// 			return instructions;
			
// 		}
// 	};

// 	typedef n_lru_cache::t_lru_cache<n_token::t_token, t_instructions, t_parser> t_all_parsed;
// 	/// A Least Recently Used (LRU) cache of previous parse results
// 	///These are independent of the interpolated data
// 	static t_all_parsed all_parsed_;

// 	n_token::t_token maybe_parsed_;
// 	t_instructions const * complete_parse_;
// 	bool is_done_;
// 	//	t_parse_and_friends * parse_interp_;
	
// public:
// 	t_parse_and_interpolator (n_token::t_token const & unparsed) 
// 		: maybe_parsed_(unparsed), complete_parse_(NULL), is_done_(false){}

// 	t_parse_and_interpolator (t_parse_and_interpolator const & a) 
// 		: maybe_parsed_(a.maybe_parsed_), complete_parse_(a.complete_parse_), is_done_(a.is_done_){}

// 	t_parse_and_interpolator & operator=(t_parse_and_interpolator const & a) {
// 		maybe_parsed_ = (a.maybe_parsed_); complete_parse_ = a.complete_parse_; is_done_ = a.is_done_; return *this;}

// 	friend std::ostream & operator<<(std::ostream &out, t_parse_and_interpolator const & a){
// 		out << ((a.is_done_) ? "parsed=" : "unparsed=") << a.maybe_parsed_;
// 		return out; }

// 	bool valid() const {return complete_parse_ != NULL;}
// 	bool is_done() const {return valid() && is_done_;}

// 	/// The core loop.  Interatively parse and interpolate until done
// 	/// The parse may be fetched from the cache and the same also with the interpolated result (after variable lookup)
// 	n_token::t_token const & parse_and_interpolate(const variable_set& set) {
// 		parse();
// 		return interpolate(set);
// 	}
	
// 	t_parse_and_interpolator & parse(){
// 		complete_parse_ = & all_parsed_.check(maybe_parsed_);
// 		is_done_ = false;
// 		return *this;
// 	}

// 	t_token const & interpolate(const variable_set& set){
// 		assert(complete_parse_);
// 		if(!is_done_){
// 			complete_parse_->run(maybe_parsed_, set); }
// 		return maybe_parsed_;
// 	}

// };

// //static member
// t_parse_and_interpolator::t_all_parsed t_parse_and_interpolator::all_parsed_(t_parse_and_interpolator::t_parser(), CACHE_SIZE);
	
// }

namespace utils {

std::string interpolate_variables_into_string(const std::string &str, const string_map * const symbols) {
	string_map_variable_set set(*symbols);
	return interpolate_variables_into_string(str, set);
}

std::string interpolate_variables_into_string(const std::string &str, const variable_set& variables) {
	n_token::t_token token(str);
	wml_interpolation::t_parse_and_interpolator interpolator(token);
	token = interpolator.parse_and_interpolate(variables);
	return static_cast<std::string const &>(token);
}

void interpolate_variables_into_token(n_token::t_token &token, const variable_set& variables) {
	wml_interpolation::t_parse_and_interpolator interpolator(token);
	token = interpolator.parse_and_interpolate(variables);
}

t_string interpolate_variables_into_tstring(const t_string &tstr, const variable_set& variables) {
	if(!tstr.str().empty()) {
		n_token::t_token token(tstr);
		wml_interpolation::t_parse_and_interpolator interpolator(token);
		token = interpolator.parse_and_interpolate(variables);
		if(tstr.str() != token) {
			return t_string( static_cast<std::string const &>(token) );
		}
	}
	return tstr;
}

}

std::string vgettext(const char *msgid, const utils::string_map& symbols)
{
	const std::string orig(_(msgid));
	const std::string msg = utils::interpolate_variables_into_string(orig, &symbols);
	return msg;
}

std::string vgettext(const char *domain
		, const char *msgid
		, const utils::string_map& symbols)
{
	const std::string orig(dgettext(domain, msgid));
	const std::string msg = utils::interpolate_variables_into_string(orig, &symbols);
	return msg;
}
std::string vngettext(const char* sing, const char* plur, int n, const utils::string_map& symbols)
{
	const std::string orig(_n(sing, plur, n));
	const std::string msg = utils::interpolate_variables_into_string(orig, &symbols);
	return msg;
}
