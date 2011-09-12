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

#include "formula_string_utils_backend.hpp"
#include "game_display.cpp" //for add_chat_message for the error message

namespace wml_interpolation {

typedef n_token::t_token t_token;
typedef std::vector<t_token> t_tokens;

//static const t_token z_empty("", false);
static const t_token z_dollar("$", false);
static const t_token z_bar("|", false);
static const t_token z_dot(".", false);
static const t_token z_lbracket("[", false);
static const t_token z_rbracket("]", false);
static const t_token z_lparen("(", false);
static const t_token z_rparen(")", false);
static const t_token z_single_quote("'", false);
static const t_token z_sharp("#", false);


template <typename T>
t_tokenizer::t_tokenizer(T const & in)
	: tokens_(), is_done_(false), is_valid_varname_(false), input_string_((*in))
	, curr_pos_(0), end_(input_string_.size())
	, varname_start_(input_string_.npos){}

template <typename T>
t_tokens const & t_tokenizer::tokenize(T const & in) {
	input_string_ = ((*in));
	is_done_=false;
	return tokenize(); }

t_tokens const & t_tokenizer::tokenize() {
	static const t_token z_empty("", false);

	if(!is_done_){
		tokens_.clear();
		curr_pos_=0;
		end_=input_string_.size();
		if(end_ == 0) {
			tokens_.push_back(z_empty);
			is_done_ = true; }
		else {
			varname_start_=input_string_.npos;
			int guard(0);
			while (!is_done_ && (++guard < 1e6)){
				process_char(); } }
		assert(is_done_);
	}

	if(! lg::debug.dont_log(log_interpolate)){
		std::stringstream ss;
		ss << "tokenized [";
		foreach(t_token const &tok, tokens_){ ss << " \""<<tok<<"\"";}
		DEBUG_INTERP << ss.str() << "]\n";
	}
	return tokens_;
}

void t_tokenizer::grab_name(){
	if(varname_start_ != input_string_.npos){
		t_token next_token(input_string_.substr(varname_start_, curr_pos_- varname_start_));
		tokens_.push_back(next_token);
		varname_start_=input_string_.npos;
	}
}

void t_tokenizer::process_char(){
	if(curr_pos_ == end_){ grab_name(); is_done_=true; return; }

	//if the char at curr_pos is a control character then it is its own token
	//otherwise it is the start of a multicharacter token

	char curr_char = input_string_[curr_pos_];
	switch(curr_char){
	case '$' : grab_name(); tokens_.push_back(z_dollar); ++curr_pos_; return ;
	case '|' : grab_name(); tokens_.push_back(z_bar); ++curr_pos_; return ;
	case '[' : grab_name(); tokens_.push_back(z_lbracket); ++curr_pos_; return;
	case ']': grab_name(); tokens_.push_back(z_rbracket); ++curr_pos_; return;
	case '(': grab_name(); tokens_.push_back(z_lparen); ++curr_pos_; return;
	case ')': grab_name(); tokens_.push_back(z_rparen); ++curr_pos_; return;
	case '.': grab_name(); tokens_.push_back(z_dot); ++curr_pos_; return;
	case '#': grab_name(); tokens_.push_back(z_sharp); ++curr_pos_; return;
	case '\'': grab_name(); tokens_.push_back(z_single_quote); ++curr_pos_; return;
	default:
		//Multi-character tokens are either all valid variable name characters or all not valid variable name characters
		bool is_good_char = is_alnum_or_underscore(curr_char);
		if(is_good_char != is_valid_varname_){
			grab_name();
		}
		if(varname_start_ == input_string_.npos){
			varname_start_ = curr_pos_;
			is_valid_varname_ = is_good_char ;
		}
		++curr_pos_;
	}
}



/**
   t_operation is a functor that does 2 things
   1. When provided with a token stack and a variable set it performs an operation like interpolation.
   2. When given a token stack an t_operation stack it performs its operation if it can do so without the
   variable set, otherwise it puts an operation on the instruction stack.  This function can be called when the expression
   is parsed in order to reduce the operations to their smallest set.
 */

//static member
t_operation_ptr const t_operation::void_op_;


std::string const t_operation_noop::string() const {return "[noop] "; }
std::string const t_operation_push::string() const {std::stringstream ss; ss << "[push \"" <<x_<<"\"] "; return ss.str(); }
std::string const t_operation_append::string() const {std::stringstream ss; ss << "[append \"" <<x_<<"\"] "; return ss.str(); }
std::string const t_operation_append::string(std::string const & res) const {
	std::stringstream ss; ss << "[append \"" <<x_<<"\"->\""<< res <<"\"] "; return ss.str(); }
std::string const t_operation_interp::string() const {return "[interpolate] "; }
std::string const t_operation_interp::string(std::string const & var, std::string const & res) const {
		std::stringstream ss; ss << "[interpolate \"" << var <<"\"->\""<< res <<"\"] "; return ss.str(); }
std::string const t_operation_formula::string() const {return "[formula] "; }
std::string const t_operation_formula::string(std::string const & var, std::string const & res) const {
	std::stringstream ss; ss << "[formula \"" << var <<"\"->\""<< res <<"\"] "; return ss.str(); }


//Static Member
t_operation_append::t_append_cache t_operation_append::cache_(t_operation_append::t_make_append_result(), CACHE_SIZE);



void t_operation_formula::operator()(t_tokens & stack,  variable_set const & variable_set) const {
	t_token var_name(stack.back());
	stack.pop_back();
	try {
		const game_logic::formula form(*var_name);
		t_token rhs(form.evaluate().string_cast());
		t_operation_append append(rhs);
		append(stack, variable_set);
		DEBUG_INTERP << string(var_name, (*rhs) ) << "\n";
	} catch(game_logic::formula_error& e) {
		ERR_INTERP << _("Formula in WML string cannot be evaluated because, ")
			   << e.type << "\n\t--> \""
			   << e.formula << "\"\n";
		t_operation_append append(var_name);
		append(stack, variable_set);
	}
}




void t_instructions::optimize(t_token const & unparsed) {
	t_operations optimized;
	DEBUG_INTERP << "Optimizing these instructions : \n" << *this <<"\n";
	if(ops_.size() > 1){
		bool last_instruction_optimized(false);
		t_operations::iterator i_not (ops_.begin() + 1);

		while(i_not!=ops_.end()){
			//reduce either the last 2 instructions or the last instruction and the last optimized instruction
			t_operation_ptr const & first_op = (last_instruction_optimized == true) ? optimized.back() : *(i_not-1);

			t_operation::t_reduce_result reduced =first_op->reduce(*i_not);

			if(reduced.second){
				DEBUG_INTERP <<"Reducing "<< *first_op << " + " << (**i_not) << " to "<< *reduced.first<<"\n";
				if(last_instruction_optimized){ optimized.pop_back(); }
				last_instruction_optimized = true;
				optimized.push_back(reduced.first); }
			else{
				DEBUG_INTERP <<"Not reducing "<< *first_op << " + " << (**i_not) << " \n";
				if(last_instruction_optimized){ }
				else { optimized.push_back(first_op); }
				last_instruction_optimized = false; }

			++i_not;
		}

		if(! last_instruction_optimized){ optimized.push_back( ops_.back() ); }

		ops_.swap(optimized);
	}
	//Check to see if the optimized result is to just push the original token and clears the ops_.
	if(ops_.size() == 1){
		t_operation_push op_push_unparsed(unparsed);
		if(op_push_unparsed == ops_[0]){ ops_.clear(); }

	}
	DEBUG_INTERP << "Optimized instructions : \n" << *this <<"\n";
}

///runs the instructions and places the results in @param[out] x
void t_instructions::run_core(t_token & x, variable_set const & set) const {
	t_tokens stack;
	assert(!ops_.empty());

	//DEBUG_INTERP << "Running these instructions : \n" << *this <<"\n";

	t_operations::const_iterator i (ops_.begin());
	for(; i!= ops_.end(); ++i){
		(**i)(stack, set); }
	assert(stack.size() == 1);
	x = stack.back();
}

std::ostream & operator<<(std::ostream &out, t_instructions const & a){
	t_operations::const_iterator i (a.ops_.begin());
	for(; i!= a.ops_.end(); ++i){
		out <<(**i) <<"\n";
	}
	return out; }



std::ostream & operator<<(std::ostream &out, t_parse const & a){
	out<<"TOKENS ";
	foreach(t_token const &tok, a.tokens_){out <<" \""<<tok<<"\"";}
	out<<" /TOKENS \n OPS ";
	t_operations const & ops( a.complete_parse_.ops() );
	t_operations::const_iterator i(ops.begin());
	for(; i != ops.end(); ++i){ out << *i ;}
	return out<<" /OPS \n";
}

t_parse::t_parse(n_token::t_token const & unparsed)
	: token_(unparsed)
	, tokens_()
	, complete_parse_() {}

t_parse::t_parse(t_parse const & a )
	: token_(a.token_)
	, tokens_(a.tokens_)
	, complete_parse_(a.complete_parse_)
{
}

t_parse & t_parse::operator=(t_parse const & a) {
	token_ = (a.token_);
	complete_parse_ = a.complete_parse_;
	return *this; }

///Calculate a parse
t_instructions & t_parse::parse(){
	t_tokenizer tokenizer(token_);
	tokens_ = tokenizer.tokenize();
	complete_parse_.ops().clear();

	do_parse_plain(tokens_.begin());

	return complete_parse_;
}

t_token const & t_parse::peek_next(t_tokens::iterator const & curr_pos ){
	static const t_token z_empty("", false);
	t_tokens::iterator next(curr_pos + 1);
	return (next != tokens_.end() ) ? (*next) : z_empty; }

//Keep concatenating components until you see a $ which indicates some kind of interpolation
t_tokens::iterator t_parse::do_parse_plain(t_tokens::iterator const & start_pos ){
	t_tokens::iterator curr_pos = start_pos;

	while(curr_pos != tokens_.end()) {

		if(*curr_pos == z_dollar) {
			//$| inserts just a $
			if( peek_next(curr_pos) == z_bar){
				static const t_operation_ptr op(new t_operation_append(z_dollar));
				complete_parse_.ops().push_back(op);
				curr_pos+=2; }

			//A interpolation / formula
			else {
				size_t good_size = complete_parse_.ops().size();
				try {
					curr_pos = do_parse_interp(++curr_pos); }
				catch (game::wml_syntax_error & e) {
					ERR_INTERP << e.what()<<"\n";
					static const config::t_token z_caption(_("Invalid WML found"), false);
					if(resources::screen) { 
						resources::screen->add_chat_message(time(NULL), z_caption, 0, e.what(),
															events::chat_handler::MESSAGE_PUBLIC, false); }
					if(complete_parse_.ops().size() > good_size){
						complete_parse_.ops().resize(good_size); } } } }

		//text variable name component
		else {
			t_operation_ptr op(new t_operation_append(*curr_pos));
			complete_parse_.ops().push_back(op);
			++curr_pos; }
	}
	return curr_pos;
}

t_tokens::iterator t_parse::do_parse_interp(t_tokens::iterator const & start_pos ){
	bool found_part_of_name(false);
	t_tokens::iterator curr_pos = start_pos + 1;

	//A $ at the end of the line is just a $
	if(start_pos == tokens_.end()){
		static const t_operation_ptr op(new t_operation_append(z_dollar));
		complete_parse_.ops().push_back(op);
		return start_pos; }

	//Start of the variable name
	if (is_good_for_varname(*start_pos)){
		found_part_of_name = true;
		t_operation_ptr op(new t_operation_push(*start_pos));
		complete_parse_.ops().push_back(op); }

	else if(*start_pos == z_dollar){
		// $| Creates a $ to allow for $ in the string
		if( peek_next(start_pos) == z_bar){
			throw game::wml_syntax_error(tokens_, start_pos - tokens_.begin(), _(" $$| creates an illegal variable name")); }

		//A Nested interpolation / formula
		else {
			curr_pos = do_parse_interp(start_pos + 1);
			//todo this is a small lie if the interpolated result contains a control character
			found_part_of_name = true; } }

	// $( ... ) Creates a WML formula
	else if (*start_pos == z_lparen){
		return do_parse_formula(start_pos); }

	// Everything else is an error
	else { throw game::wml_syntax_error(tokens_, start_pos - tokens_.begin(), _("missing variable name immediately after $")); }

	// Grab and append tokens until we reach the end of the variable name
	int bracket_nesting_level = 0;

	while(curr_pos != tokens_.end()){

		//text variable name component
		if (is_good_for_varname(*curr_pos)){
			found_part_of_name = true;
			t_operation_ptr op(new t_operation_append(*curr_pos));
			complete_parse_.ops().push_back(op);
			++curr_pos; }

		else if(*curr_pos == z_dollar){
			// $| Creates a $ to allow for $ in the string
			if( peek_next(curr_pos) == z_bar){
				break; }

			//A Nested interpolation / formula
			else{
				found_part_of_name = true;
				curr_pos = do_parse_interp(++curr_pos); } }

		//[] Nesting brackets
		else if(*curr_pos == z_lbracket){
			static const t_operation_ptr op(new t_operation_append(z_lbracket));
			complete_parse_.ops().push_back(op);
			++bracket_nesting_level;
			++curr_pos; }
		else if(*curr_pos == z_rbracket){
			if((--bracket_nesting_level) < 0) { break; }
			static const t_operation_ptr op(new t_operation_append(z_rbracket));
			complete_parse_.ops().push_back(op);
			++curr_pos; }

		//Dots
		//two dots terminates variable expansion
		// That matters for random=, e.g. $x..$y
		else if(*curr_pos == z_dot){
			if(peek_next(curr_pos) == z_dot){ break; }
			static const t_operation_ptr op(new t_operation_append(z_dot));
			complete_parse_.ops().push_back(op);
			++curr_pos; }

		//A | terminates the variable name
		// It's been used to end this variable name; now it has no more effect.
		// This can allow use of things like "$$composite_var_name|.x"
		// (Yes, that's a WML 'pointer' of sorts. They are sometimes useful.)
		else if(*curr_pos == z_bar) {
			++curr_pos;
			break; }

		//Invalid variable name component
		else if (!is_good_for_varname(*curr_pos)){
			break; }

		//Never happen
		else { assert(false); }
	}

	// If the last character is '.', then it can't be a sub-variable.
	// It's probably meant to be a period instead. Don't include it.
	// Would need to do it repetitively if there are multiple '.'s at the end,
	// but don't actually need to do so because the previous check for adjacent '.'s would catch that.
	// For example, "My score is $score." or "My score is $score..."
	if(*(curr_pos - 1) == z_dot && (curr_pos-1) != start_pos ){
		// However, "$array[$i]" by itself does not name a variable,
		// so if "$array[$i]." is encountered, then best to include the '.',
		// so that it more closely follows the syntax of a variable (if only to get rid of all of it).
		// (If it's the script writer's error, they'll have to fix it in either case.)
		// For example in "$array[$i].$field_name", if field_name does not exist as a variable,
		// then the result of the expansion should be "", not "." (which it would be if this exception did not exist).
		if( *(curr_pos-2) != z_rbracket) {
			complete_parse_.ops().pop_back();
			--curr_pos; }
		else {
			throw game::wml_syntax_error(tokens_, curr_pos - tokens_.begin()
										 , _("missing field name after ]. (correct usage: $array[$i].fieldname)")); } }

	if(! found_part_of_name) {
		throw game::wml_syntax_error(tokens_, start_pos - tokens_.begin()
									 , _("missing variable name after $ (valid characters are a-zA-Z0-9_)")); }

	//FInally an interpolation
	t_operation_ptr op(new t_operation_interp());
	complete_parse_.ops().push_back(op);
	return curr_pos;
}

/** do_parse forumal parses a formula invoked by $ (...) syntax
 */
t_tokens::iterator t_parse::do_parse_formula(t_tokens::iterator const & start_pos ){

	//Deal with the first token in the iterpolated variable
	if(start_pos == tokens_.end()){
		throw game::wml_syntax_error(tokens_, start_pos - tokens_.begin(), _("formula is incomplete at the end of the line")); }

	if(*start_pos != z_lparen){
		throw game::wml_syntax_error(tokens_, start_pos - tokens_.begin(), _("of missing paren in the formula should be $( ...)")); }

	// Grab and append tokens until we reach the end of the variable name
	t_tokens::iterator curr_pos = start_pos + 1; // +1 is skipping the left paren
	int paren_nesting_level = 1;
	bool in_string = false, in_comment = false;

	static const t_token z_empty("", false);
	static const t_operation_ptr op_empty(new t_operation_push(z_empty));
	complete_parse_.ops().push_back(op_empty);

	for(;curr_pos != tokens_.end(); ++curr_pos){

		//() Nesting
		if(*curr_pos == z_lparen){
			static const t_operation_ptr op(new t_operation_append(z_lparen));
			complete_parse_.ops().push_back(op);
			if(!in_string && !in_comment) {
				++paren_nesting_level; } }
		else if(*curr_pos == z_rparen){
			if(!in_string && !in_comment) {
				--paren_nesting_level; }
			if( paren_nesting_level  <= 0) { break; }
			static const t_operation_ptr op(new t_operation_append(z_rparen));
			complete_parse_.ops().push_back(op); }

		else if(*curr_pos == z_sharp){
			static const t_operation_ptr op(new t_operation_append(z_sharp));
			complete_parse_.ops().push_back(op);
			if(!in_string ) {
				in_comment = !in_comment; }}

		else if(*curr_pos == z_single_quote){
			static const t_operation_ptr op(new t_operation_append(z_single_quote));
			complete_parse_.ops().push_back(op);
			if(!in_comment ) {
				in_string = !in_string; }}

		else if(*curr_pos == z_dollar){
			// $| Creates a $ to allow for $ in the string
			if( peek_next(curr_pos) == z_bar){
				static const t_operation_ptr op(new t_operation_append(z_dollar));
				complete_parse_.ops().push_back(op); }

			//A Nested interpolation / formula
			else{
				curr_pos = do_parse_interp(++curr_pos) - 1; } }

		else {
			t_operation_ptr op(new t_operation_append(*curr_pos));
			complete_parse_.ops().push_back(op); }
	}

	if(paren_nesting_level > 0) {
		throw game::wml_syntax_error(tokens_, curr_pos - tokens_.begin(), _(" of missing closing paren")); }

	t_operation_ptr op(new t_operation_formula());
	complete_parse_.ops().push_back(op);
	return ++curr_pos; //skip the closing right paren

}


t_parse_and_interpolator::t_parse_and_interpolator (t_parse_and_interpolator const & a)
	: maybe_parsed_(a.maybe_parsed_), complete_parse_(a.complete_parse_), is_done_(a.is_done_){}

t_parse_and_interpolator & t_parse_and_interpolator::operator=(t_parse_and_interpolator const & a) {
	maybe_parsed_ = (a.maybe_parsed_); complete_parse_ = a.complete_parse_; is_done_ = a.is_done_; return *this;}

std::ostream & operator<<(std::ostream &out, t_parse_and_interpolator const & a){
	out << ((a.is_done_) ? "parsed=" : "unparsed=") << a.maybe_parsed_;
	return out; }

//static member
t_parse_and_interpolator::t_all_parsed t_parse_and_interpolator::all_parsed_(t_parse_and_interpolator::t_parser(), CACHE_SIZE);

}

