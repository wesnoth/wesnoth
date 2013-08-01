/*
	Copyright (C) 2013 by Pierre Talbot <ptalbot@mopong.net>
	Part of the Battle for Wesnoth Project http://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/lex_lexertl.hpp>
#include <boost/spirit/include/phoenix.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/static_assert.hpp>

#include "tools/code_generator/sql2cpp/sql_type.hpp"
#include "tools/code_generator/sql2cpp/sql_type_constraint.hpp"

#include <iostream>
#include <fstream>
#include <string>

namespace bs = boost::spirit;
namespace lex = boost::spirit::lex;
namespace qi = boost::spirit::qi;
namespace phx = boost::phoenix;

// Token definition base, defines all tokens for the base grammar below
template <typename Lexer>
struct sql_tokens : lex::lexer<Lexer>
{
public:
	// Tokens with no attributes.
	lex::token_def<lex::omit> type_smallint, type_int, type_varchar, type_text, type_date;
	lex::token_def<lex::omit> kw_not_null, kw_auto_increment, kw_unique, kw_default;

	// Attributed tokens. (If you add a new type, don't forget to add it to the lex::lexertl::token definition too).
	lex::token_def<std::size_t> signed_digit;
	lex::token_def<std::string> identifier;

	sql_tokens()
	{
		// Column data types.
		type_smallint = "smallint";
		type_int = "int";
		type_varchar = "varchar";
		type_text = "text";
		type_date = "date";

		// Keywords.
		kw_not_null = "not null";
		kw_auto_increment = "auto_increment";
		kw_unique = "unique";
		kw_default = "default";

		// Values.
		signed_digit = "[+-]?[0-9]+";

		// Identifier.
		identifier = "[a-zA-Z][a-zA-Z0-9_]*";

		// The token must be added in priority order.
		this->self += lex::token_def<>('(') | ')' | ',';
		this->self += type_smallint | type_int | type_varchar | type_text |
									type_date;
		this->self += kw_not_null | kw_auto_increment | kw_unique | kw_default;
		this->self += identifier;
		this->self += signed_digit;

		// define the whitespace to ignore.
		this->self("WS")
				=		lex::token_def<>("[ \\t\\n]+") 
				|		"--[^\\n]*\\n"  // Single line comments with --
				|		"\\/\\*[^*]*\\*+([^/*][^*]*\\*+)*\\/" // C-style comments
				;
	}
};

struct sql_column
{
	std::string column_identifier;
	boost::shared_ptr<sql::type::base_type> sql_type;
	std::vector<boost::shared_ptr<sql::base_type_constraint> > constraints;
};

template <class synthesized, class inherited = void>
struct attribute
{
	typedef synthesized s_type;
	typedef inherited i_type;
	typedef s_type type(i_type);
};

template <class synthesized>
struct attribute <synthesized, void>
{
	typedef synthesized s_type;
	typedef s_type type();
};

template <class inherited>
struct attribute <void, inherited>
{
	typedef inherited i_type;
	typedef void type(i_type);
};

class semantic_actions
{
public:
	typedef attribute<boost::shared_ptr<sql::type::base_type> > data_attribute;

	template<class T>
	void make_data_type(typename data_attribute::s_type& res) const
	{
		res = boost::make_shared<T>();
	}

	void make_varchar_type(typename data_attribute::s_type& res, std::size_t length) const
	{
		res = boost::make_shared<sql::type::varchar>(length);
	}
};

// Grammar definition, define a little part of the SQL language.
template <typename Iterator, typename Lexer>
struct sql_grammar 
	: qi::grammar<Iterator, qi::in_state_skipper<Lexer> >
{
	template <typename TokenDef>
	sql_grammar(TokenDef const& tok)
		: sql_grammar::base_type(program, "sql")
	{
		program 
			=  +statement
			;

		statement 
			=   create_statement.alias()
			;

		create_statement
			=   create_table.alias()
			;

		create_table
			=   create_table_definition.alias()
			;

		create_table_definition
			=   column_definition % ','     // comma separated list of column_definition.
			;

		column_definition
			=   tok.identifier >> data_type >> *constraint_definition
			;

		constraint_definition
			=   tok.kw_not_null
			|   tok.kw_auto_increment
			|   tok.kw_unique
			|   default_value
			;

		default_value
			=   tok.kw_default >> tok.signed_digit
			;

		data_type
			=   tok.type_smallint		[phx::bind(&semantic_actions::make_data_type<sql::type::smallint>, &sa_, qi::_val)]
			|   tok.type_int 				[phx::bind(&semantic_actions::make_data_type<sql::type::integer>, &sa_, qi::_val)]
			|   (tok.type_varchar > '(' > tok.signed_digit > ')') [phx::bind(&semantic_actions::make_varchar_type, &sa_, qi::_val, qi::_1)]
			|   tok.type_text 			[phx::bind(&semantic_actions::make_data_type<sql::type::text>, &sa_, qi::_val)]
			|   tok.type_date			  [phx::bind(&semantic_actions::make_data_type<sql::type::date>, &sa_, qi::_val)]
			;

		program.name("program");
		statement.name("statement");
		create_statement.name("create statement");
		create_table.name("create table");
		create_table_definition.name("create table definition");
		column_definition.name("column definition");
		data_type.name("data type");
		default_value.name("default value");
		constraint_definition.name("constraint definition");

		using namespace qi::labels;
		qi::on_error<qi::fail>
		(
			program,
			std::cout
				<< phx::val("Error! Expecting ")
				<< bs::_4                               // what failed?
				<< phx::val(" here: \"")
				<< phx::construct<std::string>(bs::_3, bs::_2)   // iterators to error-pos, end
				<< phx::val("\"")
				<< std::endl
		);
	}

	typedef qi::in_state_skipper<Lexer> skipper_type;
	template <class Attribute>
	struct rule
	{
		typedef qi::rule<Iterator, skipper_type, Attribute> type;
	};
	typedef qi::rule<Iterator, skipper_type> simple_rule;

	semantic_actions sa_;

	simple_rule program, statement;
	simple_rule create_statement, create_table, create_table_definition;
	simple_rule column_definition, default_value, constraint_definition;
	typename rule<typename semantic_actions::data_attribute::type>::type data_type;
};


int main(int argc, char* argv[])
{
	if(argc != 2)
		exit(1);

	// iterator type used to expose the underlying input stream
	typedef std::string::iterator base_iterator_type;

	// This is the lexer token type to use. The second template parameter lists 
	// all attribute types used for token_def's during token definition (see 
	// example5_base_tokens<> above). Here we use the predefined lexertl token 
	// type, but any compatible token type may be used instead.
	//
	// If you don't list any token attribute types in the following declaration 
	// (or just use the default token type: lexertl_token<base_iterator_type>)  
	// it will compile and work just fine, just a bit less efficient. This is  
	// because the token attribute will be generated from the matched input  
	// sequence every time it is requested. But as soon as you specify at 
	// least one token attribute type you'll have to list all attribute types 
	// used for token_def<> declarations in the token definition class above,  
	// otherwise compilation errors will occur.
	typedef lex::lexertl::token<
		base_iterator_type, boost::mpl::vector<unsigned int, std::string> 
	> token_type;

	// Here we use the lexertl based lexer engine.
	typedef lex::lexertl::lexer<token_type> lexer_type;

	// This is the token definition type (derived from the given lexer type).
	typedef sql_tokens<lexer_type> sql_tokens;

	// this is the iterator type exposed by the lexer 
	typedef sql_tokens::iterator_type iterator_type;

	// this is the type of the grammar to parse
	typedef sql_grammar<iterator_type, sql_tokens::lexer_def> sql_grammar;

	// now we use the types defined above to create the lexer and grammar
	// object instances needed to invoke the parsing process
	sql_tokens tokens;                         // Our lexer
	sql_grammar sql(tokens);                  // Our parser

	std::string str(argv[1]);

	// At this point we generate the iterator pair used to expose the
	// tokenized input stream.
	std::string::iterator it = str.begin();
	iterator_type iter = tokens.begin(it, str.end());
	iterator_type end = tokens.end();

	// Parsing is done based on the the token stream, not the character 
	// stream read from the input.
	// Note how we use the lexer defined above as the skip parser. It must
	// be explicitly wrapped inside a state directive, switching the lexer 
	// state for the duration of skipping whitespace.
	std::string ws("WS");
	bool r = qi::phrase_parse(iter, end, sql, qi::in_state(ws)[tokens.self]);


	if (r && iter == end)
	{
		std::cout << "-------------------------\n";
		std::cout << "Parsing succeeded\n";
		std::cout << "-------------------------\n";
	}
	else
	{
		std::cout << "-------------------------\n";
		std::cout << "Parsing failed\n";
		std::cout << "-------------------------\n";
	}
	return 0;
}
