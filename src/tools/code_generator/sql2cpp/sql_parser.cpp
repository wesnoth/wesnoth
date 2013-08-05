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
#include <boost/spirit/include/karma.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

#include <boost/algorithm/string.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/static_assert.hpp>
#include <boost/lexical_cast.hpp>

#include "tools/code_generator/sql2cpp/sql_type.hpp"
#include "tools/code_generator/sql2cpp/sql_type_constraint.hpp"

#include <iostream>
#include <fstream>
#include <string>

namespace bs = boost::spirit;
namespace lex = boost::spirit::lex;
namespace qi = boost::spirit::qi;
namespace karma = boost::spirit::karma;
namespace phx = boost::phoenix;

// Why not using read_file from filesystem.cpp?
// Because it adds too many dependencies for a single function...
std::string file2string(const std::string& filename)
{
	std::ifstream s(filename.c_str(), std::ios_base::binary);
	std::stringstream ss;
	ss << s.rdbuf();
	return ss.str();
}

std::string get_license_header_file()
{
	return "data/umcd/license_header.txt";
}

// Token definition base, defines all tokens for the base grammar below
template <typename Lexer>
struct sql_tokens : lex::lexer<Lexer>
{
public:
	// Tokens with no attributes.
	lex::token_def<lex::omit> type_smallint, type_int, type_varchar, type_text, type_date;
	lex::token_def<lex::omit> kw_not_null, kw_auto_increment, kw_unique, kw_default, kw_create,
		kw_table;

	// Attributed tokens. (If you add a new type, don't forget to add it to the lex::lexertl::token definition too).
	lex::token_def<int> signed_digit;
	lex::token_def<std::size_t> unsigned_digit;
	lex::token_def<std::string> identifier;
	lex::token_def<std::string> quoted_string;

	sql_tokens()
	{
		// Column data types.
		type_smallint = "(?i:smallint)";
		type_int = "(?i:int)";
		type_varchar = "(?i:varchar)";
		type_text = "(?i:text)";
		type_date = "(?i:date)";

		// Keywords.
		kw_not_null = "(?i:not null)";
		kw_auto_increment = "(?i:auto_increment)";
		kw_unique = "(?i:unique)";
		kw_default = "(?i:default)";
		kw_create = "(?i:create)";
		kw_table = "(?i:table)";

		// Values.
		signed_digit = "[+-]?[0-9]+";
		unsigned_digit = "[0-9]+";
		quoted_string = "\\\"(\\\\.|[^\\\"])*\\\""; // \"(\\.|[^\"])*\"

		// Identifier.
		identifier = "[a-zA-Z][a-zA-Z0-9_]*";

		// The token must be added in priority order.
		this->self += lex::token_def<>('(') | ')' | ',' | ';';
		this->self += type_smallint | type_int | type_varchar | type_text |
									type_date;
		this->self += kw_not_null | kw_auto_increment | kw_unique | kw_default |
									kw_create | kw_table;
		this->self += identifier | unsigned_digit | signed_digit | quoted_string;

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

BOOST_FUSION_ADAPT_STRUCT(
	sql_column,
	(std::string, column_identifier)
	(boost::shared_ptr<sql::type::base_type>, sql_type)
	(std::vector<boost::shared_ptr<sql::base_type_constraint> >, constraints)
)

struct sql_table
{
	std::string table_identifier;
	std::vector<sql_column> columns;
};

BOOST_FUSION_ADAPT_STRUCT(
	sql_table,
	(std::string, table_identifier)
	(std::vector<sql_column>, columns)
)

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
	typedef attribute<boost::shared_ptr<sql::type::base_type> > column_type_attribute;
	typedef attribute<std::string> default_value_attribute;
	typedef attribute<boost::shared_ptr<sql::base_type_constraint> > type_constraint_attribute;
	typedef attribute<sql_column> column_attribute;
	typedef attribute<std::vector<sql_column> > create_table_columns_attribute;
	typedef attribute<sql_table> create_table_attribute;
	typedef attribute<sql_table> create_statement_attribute;
	typedef attribute<sql_table> statement_attribute;
	typedef attribute<std::vector<sql_table> > program_attribute;

	template<class T>
	void make_column_type(typename column_type_attribute::s_type& res) const
	{
		res = boost::make_shared<T>();
	}

	void make_varchar_type(typename column_type_attribute::s_type& res, std::size_t length) const
	{
		res = boost::make_shared<sql::type::varchar>(length);
	}

	template <class T>
	void make_type_constraint(typename type_constraint_attribute::s_type& res) const
	{
		res = boost::make_shared<T>();
	}

	void make_default_value_constraint(typename type_constraint_attribute::s_type& res, const std::string& default_value) const
	{
		res = boost::make_shared<sql::default_value>(default_value);
	}
};

// Grammar definition, define a little part of the SQL language.
template <typename Iterator, typename Lexer>
struct sql_grammar 
	: qi::grammar<Iterator, qi::in_state_skipper<Lexer>, typename semantic_actions::program_attribute::type>
{
	template <typename TokenDef>
	sql_grammar(TokenDef const& tok)
		: sql_grammar::base_type(program, "program")
	{
		program 
			%=  (statement % ';') >> *qi::lit(';')
			;

		statement 
			%=   create_statement
			;

		create_statement
			%=   tok.kw_create >> create_table
			;

		create_table
			%=	tok.kw_table >> tok.identifier >> '(' >> create_table_columns >> ')'
			;

		create_table_columns
			%=   column_definition % ','     // comma separated list of column_definition.
			;

		column_definition
			%=   tok.identifier >> column_type >> *type_constraint
			;

		type_constraint
			=   tok.kw_not_null		[phx::bind(&semantic_actions::make_type_constraint<sql::not_null>, &sa_, qi::_val)]
			|   tok.kw_auto_increment	[phx::bind(&semantic_actions::make_type_constraint<sql::auto_increment>, &sa_, qi::_val)]
			|   tok.kw_unique			[phx::bind(&semantic_actions::make_type_constraint<sql::unique>, &sa_, qi::_val)]
			|   default_value 		[phx::bind(&semantic_actions::make_default_value_constraint, &sa_, qi::_val, qi::_1)]
			;

		default_value
			%=   tok.kw_default > tok.quoted_string
			;

		column_type
			=   tok.type_smallint		[phx::bind(&semantic_actions::make_column_type<sql::type::smallint>, &sa_, qi::_val)]
			|   tok.type_int 				[phx::bind(&semantic_actions::make_column_type<sql::type::integer>, &sa_, qi::_val)]
			|   (tok.type_varchar > '(' > tok.unsigned_digit > ')') [phx::bind(&semantic_actions::make_varchar_type, &sa_, qi::_val, qi::_1)]
			|   tok.type_text 			[phx::bind(&semantic_actions::make_column_type<sql::type::text>, &sa_, qi::_val)]
			|   tok.type_date			  [phx::bind(&semantic_actions::make_column_type<sql::type::date>, &sa_, qi::_val)]
			;

		program.name("program");
		statement.name("statement");
		create_statement.name("create statement");
		create_table.name("create table");
		create_table_columns.name("create table columns");
		column_definition.name("column definition");
		column_type.name("column type");
		default_value.name("default value");
		type_constraint.name("type constraint");

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

private:
	typedef qi::in_state_skipper<Lexer> skipper_type;
	template <class Attribute>
	struct rule
	{
		typedef qi::rule<Iterator, skipper_type, Attribute> type;
	};
	typedef qi::rule<Iterator, skipper_type> simple_rule;

	semantic_actions sa_;

	typename rule<typename semantic_actions::program_attribute::type>::type program;
	typename rule<typename semantic_actions::statement_attribute::type>::type statement;
	typename rule<typename semantic_actions::create_statement_attribute::type>::type create_statement;
	typename rule<typename semantic_actions::create_table_attribute::type>::type create_table;
	typename rule<typename semantic_actions::create_table_columns_attribute::type>::type create_table_columns;
	typename rule<typename semantic_actions::column_attribute::type>::type column_definition;
	typename rule<typename semantic_actions::type_constraint_attribute::type>::type type_constraint;
	typename rule<typename semantic_actions::default_value_attribute::type>::type default_value;
	typename rule<typename semantic_actions::column_type_attribute::type>::type column_type;
};

struct sql2cpp_type_visitor : sql::type::type_visitor
{
	sql2cpp_type_visitor(std::string& res)
	: res_(res)
	{}

	virtual void visit(const sql::type::smallint&)
	{
		res_ = "short";
	}

	virtual void visit(const sql::type::integer&)
	{
		res_ = "int";
	}

	virtual void visit(const sql::type::text&)
	{
		res_ = "std::string";
	}

	virtual void visit(const sql::type::date&)
	{
		res_ = "boost::posix_time::ptime";
	}

	virtual void visit(const sql::type::varchar& v)
	{
		res_ = "boost::array<char, " + boost::lexical_cast<std::string>(v.length) + ">";
	}

private:
	std::string& res_;
};

struct cpp_semantic_action
{
	cpp_semantic_action(const std::string& wesnoth_path)
	: license_header_(file2string(wesnoth_path + get_license_header_file())) 
	{}

	void type2string(std::string& res, typename semantic_actions::column_type_attribute::s_type const& type)
	{
		boost::shared_ptr<sql::type::type_visitor> visitor = boost::make_shared<sql2cpp_type_visitor>(boost::ref(res));
		type->accept(visitor);
	}

	void license_header(std::string& res)
	{
		res = license_header_;
	}

	void define_name(std::string& res, const std::string& class_name)
	{
		res = "UMCD_POD_" + boost::to_upper_copy(class_name) + "_HPP";
	}

private:
	std::string license_header_;
};

template <typename OutputIterator>
struct cpp_grammar 
: karma::grammar<OutputIterator, typename semantic_actions::program_attribute::type>
{
	cpp_grammar(const std::string& wesnoth_path)
	: cpp_grammar::base_type(program)
	, cpp_sa_(wesnoth_path)
	{
		using karma::eol;

		program 
			= create_file % eol
			;

		create_file 
			= header [karma::_1 = karma::_val] 
			<< create_class [karma::_1 = karma::_val] 
			<< footer
			;

		header 
			= license_header
			<< define_header [karma::_1 = phx::at_c<0>(karma::_val)]
			//<< include
			;

		footer
			= define_footer.alias()
			;

		define_footer
			= "#endif\n"
			;

		define_header
			= "#ifndef "
			<< karma::string [phx::bind(&cpp_semantic_action::define_name, &cpp_sa_, karma::_1, karma::_val)]
			<< "\n#define "
			<< karma::string [phx::bind(&cpp_semantic_action::define_name, &cpp_sa_, karma::_1, karma::_val)]
			<< "\n\n"
			;

		license_header 
			= "/*\n" 
			<< karma::string [phx::bind(&cpp_semantic_action::license_header, &cpp_sa_, karma::_1)]
			<< "\n*/\n\n"
			;

		create_class 
			= "struct " 
			<< karma::string 
			<< "\n{\n"
			<< create_members 
			<< "};\n\n"
			;

		create_members 
			= *('\t' << create_member << ";\n")
			;

		create_member 
			= create_member_type [karma::_1 = phx::at_c<1>(karma::_val)] 
			<< ' ' 
			<< karma::string [karma::_1 = phx::at_c<0>(karma::_val)]
			;

		create_member_type 
			= karma::string [phx::bind(&cpp_semantic_action::type2string, &cpp_sa_, karma::_1, karma::_val)]
			;
	}

private:
	cpp_semantic_action cpp_sa_;

	template <class Attribute>
	struct rule
	{
		typedef karma::rule<OutputIterator, Attribute> type;
	};
	typedef karma::rule<OutputIterator> simple_rule;

	typename rule<typename semantic_actions::program_attribute::type>::type program;
	typename rule<typename semantic_actions::create_table_attribute::type>::type create_file;
	typename rule<typename semantic_actions::create_table_attribute::type>::type create_class;
	typename rule<typename semantic_actions::create_table_attribute::type>::type header;
	typename rule<std::string()>::type define_header;
	simple_rule footer;
	simple_rule define_footer;
	simple_rule license_header;
	typename rule<typename semantic_actions::create_table_columns_attribute::type>::type create_members;
	typename rule<typename semantic_actions::column_attribute::type>::type create_member;
	typename rule<typename semantic_actions::column_type_attribute::type>::type create_member_type;
};

template <typename OutputIterator>
bool generate_cpp(OutputIterator& sink, typename semantic_actions::program_attribute::s_type const& sql_ast)
{
	cpp_grammar<OutputIterator> cpp_grammar("../");
	return karma::generate(sink, cpp_grammar, sql_ast);
}

int main(int argc, char* argv[])
{
	if(argc != 2)
	{
		std::cerr << "usage: " << argv[0] << " schema_filename\n";
		return 1;
	}

	// iterator type used to expose the underlying input stream
	typedef std::string::iterator base_iterator_type;

	// This is the lexer token type to use. The second template parameter lists 
	// all attribute types used for token_def's during token definition (see 
	// sql_tokens<> above). Here we use the predefined lexertl token 
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
		base_iterator_type, boost::mpl::vector<int, std::size_t, std::string> 
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

	std::string str(file2string(argv[1]));

	// At this point we generate the iterator pair used to expose the
	// tokenized input stream.
	base_iterator_type it = str.begin();
	iterator_type iter = tokens.begin(it, str.end());
	iterator_type end = tokens.end();

	// Parsing is done based on the the token stream, not the character 
	// stream read from the input.
	// Note how we use the lexer defined above as the skip parser. It must
	// be explicitly wrapped inside a state directive, switching the lexer 
	// state for the duration of skipping whitespace.
	std::string ws("WS");
	typename semantic_actions::program_attribute::s_type sql_ast;
	bool r = qi::phrase_parse(iter, end, sql, qi::in_state(ws)[tokens.self], sql_ast);

	if (r && iter == end)
	{
		std::cout << "-------------------------\n";
		std::cout << "Parsing succeeded\n";
		std::cout << "-------------------------\n";

		std::string generated;
		std::back_insert_iterator<std::string> sink(generated);
		if(generate_cpp(sink, sql_ast))
		{
			std::cout << "Generation succeeded\n";
			std::cout << generated << std::endl;
		}
		else
		{
			std::cout << "Generation failed\n";
		}
	}
	else
	{
		std::cout << "-------------------------\n";
		std::cout << "Parsing failed\n";
		std::cout << "-------------------------\n";
	}
	return 0;
}
