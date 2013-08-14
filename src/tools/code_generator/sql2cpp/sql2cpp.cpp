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
//#define BOOST_SPIRIT_QI_DEBUG

#include "tools/code_generator/sql2cpp/sql/lexer.hpp"
#include "tools/code_generator/sql2cpp/sql/parser.hpp"
#include "tools/code_generator/sql2cpp/cpp/generator.hpp"
#include "tools/code_generator/sql2cpp/utility.hpp"

#include <iostream>

int main(int argc, char* argv[])
{
	if(argc != 3)
	{
		std::cerr << "usage: " << argv[0] << " schema_filename output_directory\n";
		return 1;
	}

	// now we use the types defined above to create the lexer and grammar
	// object instances needed to invoke the parsing process
	sql::tokens_type tokens;                         // Our lexer
	sql::grammar_type sql(tokens);                  // Our parser

	std::string str(file2string(argv[1])) ;

	// At this point we generate the iterator pair used to expose the
	// tokenized input stream.
	std::string::iterator it = str.begin();
	sql::token_iterator_type iter = tokens.begin(it, str.end());
	sql::token_iterator_type end = tokens.end();

	// Parsing is done based on the the token stream, not the character 
	// stream read from the input.
	sql::ast::schema sql_ast;
	bool r = boost::spirit::qi::parse(iter, end, sql, sql_ast);

	if (r && iter == end)
	{
		std::cout << "Parsing succeeded\n";
	
		std::ofstream generated("dummy.txt");
		std::ostream_iterator<char> sink(generated);

		if(cpp::generate(sink, sql_ast, generated, argv[2]))
		{
			std::cout << "Generation succeeded\n";
		}
		else
		{
			std::cout << "Generation failed\n";
		}
	}
	else
	{
		std::cout << "Parsing failed\n";
	}
	return 0;
}
