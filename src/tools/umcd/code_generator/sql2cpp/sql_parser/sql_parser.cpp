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

//  This example shows how to create a simple lexer recognizing a couple of 
//  different tokens aimed at a simple language and how to use this lexer with 
//  a grammar. It shows how to associate attributes to tokens and how to access the 
//  token attributes from inside the grammar.
//
//  Additionally, this example demonstrates, how to define a token set usable 
//  as the skip parser during parsing, allowing to define several tokens to be 
//  ignored.
//
//  The main purpose of this example is to show how inheritance can be used to 
//  overload parts of a base grammar and add token definitions to a base lexer.
//
//  Further, it shows how you can use the 'omit' attribute type specifier 
//  for token definitions to force the token to have no attribute (expose an 
//  unused attribute).
//
//  This example recognizes a very simple programming language having 
//  assignment statements and if and while control structures. Look at the file
//  example5.input for an example.

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/lex_lexertl.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>

#include <iostream>
#include <fstream>
#include <string>

using namespace boost::spirit;
using boost::phoenix::val;

// Token definition base, defines all tokens for the base grammar below
template <typename Lexer>
struct sql_tokens : lex::lexer<Lexer>
{
public:
    // Tokens with no attributes.
    lex::token_def<lex::omit> type_smallint, type_int, type_varchar, type_text, type_date;
    
    // Attributed tokens.
    lex::token_def<unsigned int> data_length;
    lex::token_def<std::string> identifier;

    sql_tokens()
    {
        // Column data types.
        type_smallint = "smallint";
        type_int = "int";
        type_varchar = "varchar";
        type_text = "text";
        type_date = "date";
        data_length = "[0-9]+";

        // Identifier.
        identifier = "[a-zA-Z][a-zA-Z0-9_]*";

        // associate the tokens and the token set with the lexer
        this->self += lex::token_def<>('(') | ')';
        this->self += type_smallint | type_int | type_varchar | type_text |
                      type_data | data_length;
        this->self += identifier;

        // define the whitespace to ignore.
        this->self("WS")
            =   lex::token_def<>("[ \\t\\n]+") 
            |   "--[^\\n]*\\n"  // Single line comments with --
            |   "\\/\\*[^*]*\\*+([^/*][^*]*\\*+)*\\/" // C-style comments
            ;
    }
};

// Grammar definition, define a little part of the SQL language.
template <typename Iterator, typename Lexer>
struct sql_grammar 
  : qi::grammar<Iterator, qi::in_state_skipper<Lexer> >
{
    template <typename TokenDef>
    sql_grammar(TokenDef const& tok)
      : sql_grammar::base_type(program)
    {
        using boost::spirit::_val;

        program 
            =  +statement
            ;

        statement 
            =   create_statement
            ;

        create_statement
            =   create_table
            ;

        create_table
            =   create_table_definition
            ;

        create_table_definition
            =   (tok.identifier >> column_definition)
            ;

        column_definition
            =   data_type
            ;

        data_type
            =   type_smallint
            |   type_int
            |   (type_varchar >> data_length)
            |   type_text
            |   type_date
            ;
    }

    typedef qi::in_state_skipper<Lexer> skipper_type;

    qi::rule<Iterator, skipper_type> program, statement;
    qi::rule<Iterator, skipper_type> create_statement, create_table, create_table_definition;
    qi::rule<Iterator, skipper_type> column_definition, data_type;
};

///////////////////////////////////////////////////////////////////////////////
int main()
{
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

    std::string str("value smallint");

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

    std::cout << "Bye... :-) \n\n";
    return 0;
}
