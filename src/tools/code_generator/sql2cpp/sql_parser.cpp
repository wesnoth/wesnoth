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
#define BOOST_SPIRIT_QI_DEBUG

#include "tools/code_generator/sql2cpp/sql/lexer.hpp"
#include "tools/code_generator/sql2cpp/sql/parser.hpp"

#include <boost/spirit/include/karma.hpp>

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

struct sql2cpp_type_visitor : sql::type::type_visitor
{
private:
	std::string add_unsigned_qualifier(const sql::type::numeric_type& num_type)
	{
		return (num_type.is_unsigned) ? "u":"";
	}
public:

	sql2cpp_type_visitor(std::string& res)
	: res_(res)
	{}

	virtual void visit(const sql::type::smallint& s)
	{
		res_ = "boost::" + add_unsigned_qualifier(s) + "int16_t";
	}

	virtual void visit(const sql::type::integer& i)
	{
		res_ = "boost::" + add_unsigned_qualifier(i) + "int32_t";
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

struct sql2cpp_header_type_visitor : sql::type::type_visitor
{
	sql2cpp_header_type_visitor(std::string& res)
	: res_(res)
	{}

	virtual void visit(const sql::type::smallint&)
	{
		res_ = "#include <boost/cstdint.hpp>";
	}

	virtual void visit(const sql::type::integer&)
	{
		res_ = "#include <boost/cstdint.hpp>";
	}

	virtual void visit(const sql::type::text&)
	{
		res_ = "#include <string>";
	}

	virtual void visit(const sql::type::date&)
	{
		res_ = "#include <boost/date_time/posix_time/posix_time.hpp>";
	}

	virtual void visit(const sql::type::varchar&)
	{
		res_ = "#include <boost/array.hpp>";
	}

private:
	std::string& res_;
};

#include <boost/algorithm/string.hpp>

struct cpp_semantic_actions
{
	cpp_semantic_actions(const std::string& wesnoth_path, std::ofstream& generated, const std::string& output_dir)
	: license_header_(file2string(wesnoth_path + get_license_header_file())) 
	, generated_(generated)
	, output_dir_(output_dir)
	{}

	void type2string(std::string& res, sql::ast::column_type_ptr const& type)
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

	void includes(std::string& res, sql::ast::column_list const& class_members, std::set<std::string>& included)
	{
		for(std::size_t i = 0; i < class_members.size(); ++i)
		{
			std::string preproc_include;
			boost::shared_ptr<sql::type::type_visitor> visitor = boost::make_shared<sql2cpp_header_type_visitor>(boost::ref(preproc_include));
			class_members[i].sql_type->accept(visitor);
			if(included.insert(preproc_include).second && !preproc_include.empty())
			{
				res += preproc_include + "\n";
			}
		}
	}

	void open_sink(const std::string& class_name)
	{
		generated_.close();
		std::string filepath = output_dir_ + boost::to_lower_copy(class_name) + ".hpp";
		generated_.open(filepath.c_str());
		if(!generated_.is_open())
		{
			throw std::runtime_error("Could not open the file " + filepath); 
		}
	}

private:
	std::string license_header_;
	std::ofstream& generated_;
	std::string output_dir_;
};

template <typename OutputIterator>
struct cpp_grammar 
: karma::grammar<OutputIterator, sql::ast::schema()>
{
	typedef OutputIterator iterator_type;

	cpp_grammar(const std::string& wesnoth_path, std::ofstream& generated, const std::string& output_dir)
	: cpp_grammar::base_type(schema)
	, cpp_sa_(wesnoth_path, generated, output_dir)
	{
		using karma::eol;

		RULE_DEF(schema,
			= +create_file
			);

		RULE_DEF(create_file,
			= karma::eps [phx::bind(&cpp_semantic_actions::open_sink, &cpp_sa_, phx::at_c<0>(karma::_val))]
			<< header [karma::_1 = karma::_val] 
			<< create_class [karma::_1 = karma::_val] 
			<< footer
			);

		RULE_DEF(header,
			= license_header << eol
			<< do_not_modify << eol
			<< define_header
			<< includes << eol
			<< namespace_open << eol
			);

		RULE_DEF(namespace_open,
			= "namespace pod{\n"
			);

		RULE_DEF(namespace_close,
			= "} // namespace pod\n"
			);

		RULE_DEF(do_not_modify,
			=  "// WARNING: This file has been auto-generated with the tool sql2cpp. We keep in sync the SQL schema and the POD classes."
			<< eol
			<< "//          Please do not modify this file by hand. Modify the SQL schema and rebuild the project.\n"
			);

		RULE_NDEF(includes,
			= karma::string [phx::bind(&cpp_semantic_actions::includes, &cpp_sa_, karma::_1, karma::_val, karma::_a)]
			);

		RULE_DEF(footer,
			= namespace_close
			<< define_footer
			);

		RULE_DEF(define_footer,
			= "#endif\n"
			);

		RULE_DEF(define_header,
			= karma::eps [phx::bind(&cpp_semantic_actions::define_name, &cpp_sa_, karma::_a, karma::_val)]
			<< "#ifndef "
			<< karma::string [karma::_1 = karma::_a]
			<< "\n#define "
			<< karma::string [karma::_1 = karma::_a]
			<< "\n\n"
			);

		RULE_DEF(license_header,
			= "/*\n" 
			<< karma::string [phx::bind(&cpp_semantic_actions::license_header, &cpp_sa_, karma::_1)]
			<< "\n*/\n"
			);

		RULE_DEF(create_class,
			= "struct " 
			<< karma::string [karma::_1 = phx::at_c<0>(karma::_val)]
			<< "\n{\n"
			<< create_members [karma::_1 = phx::at_c<1>(karma::_val)]
			<< "};\n\n"
			);

		RULE_DEF(create_members,
			= *('\t' << create_member << ";\n")
			);

		RULE_DEF(create_member,
			= create_member_type [karma::_1 = phx::at_c<1>(karma::_val)] 
			<< ' ' 
			<< karma::string [karma::_1 = phx::at_c<0>(karma::_val)]
			);

		RULE_DEF(create_member_type,
			= karma::string [phx::bind(&cpp_semantic_actions::type2string, &cpp_sa_, karma::_1, karma::_val)]
			);
	}

private:
	cpp_semantic_actions cpp_sa_;

	KA_RULE(sql::ast::schema(), schema);
	KA_RULE(sql::ast::table(), create_file);
	KA_RULE(sql::ast::table(), create_class);
	KA_RULE(sql::ast::table(), header);
	KA_RULE_LOC(std::string(), std::string, define_header);
	KA_RULE_LOC(sql::ast::column_list(), std::set<std::string>, includes);

	KA_RULE0(footer);
	KA_RULE0(define_footer);
	KA_RULE0(license_header);
	KA_RULE0(namespace_open);
	KA_RULE0(namespace_close);
	KA_RULE0(do_not_modify);

	KA_RULE(sql::ast::column_list(), create_members);
	KA_RULE(sql::ast::column(), create_member);
	KA_RULE(sql::ast::column_type_ptr(), create_member_type);
};

template <typename OutputIterator>
bool generate_cpp(OutputIterator& sink, sql::ast::schema const& sql_ast, std::ofstream& generated, const std::string& output_dir)
{
	cpp_grammar<OutputIterator> cpp_grammar("../", generated, output_dir);
	return karma::generate(sink, cpp_grammar, sql_ast);
}

int main(int argc, char* argv[])
{
	if(argc != 3)
	{
		std::cerr << "usage: " << argv[0] << " schema_filename output_directory\n";
		return 1;
	}

	// iterator type used to expose the underlying input stream
	typedef std::string::iterator base_iterator_type;

	// We use the default lexer engine.
	typedef sql::lexer<base_iterator_type>::type lexer_type;

	// This is the token definition type (derived from the given lexer type).
	typedef sql::tokens<lexer_type> sql_tokens;

	// this is the iterator type exposed by the lexer 
	typedef sql_tokens::iterator_type iterator_type;

	// this is the type of the grammar to parse
	typedef sql::sql_grammar<iterator_type> sql_grammar;

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
	sql::ast::schema sql_ast;
	bool r = qi::parse(iter, end, sql, sql_ast);

	if (r && iter == end)
	{
		std::cout << "-------------------------\n";
		std::cout << "Parsing succeeded\n";
		std::cout << "-------------------------\n";

		//std::string generated;
		//std::back_insert_iterator<std::string> sink(generated);

		std::ofstream generated("dummy.txt");
		std::ostream_iterator<char> sink(generated);

		if(generate_cpp(sink, sql_ast, generated, argv[2]))
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
		std::cout << "-------------------------\n";
		std::cout << "Parsing failed\n";
		std::cout << "-------------------------\n";
	}
	return 0;
}
