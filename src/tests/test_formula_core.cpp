/*
 Copyright (C) 2008 - 2017
 Part of the Battle for Wesnoth Project http://www.wesnoth.org/

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY.

 See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-test"

#include <boost/test/unit_test.hpp>

#include <ctime>

#include "formula/formula.hpp"
#include "formula/callable.hpp"
#include "formula/tokenizer.hpp"

using namespace wfl;

class mock_char : public formula_callable {
	variant get_value(const std::string& key) const {
		if(key == "strength") {
			return variant(15);
		} else if(key == "agility") {
			return variant(12);
		}

		return variant(10);
	}
};

class mock_party : public formula_callable {
	variant get_value(const std::string& key) const {
		if(key == "members") {
			i_[0].add("strength",variant(12));
			i_[1].add("strength",variant(16));
			i_[2].add("strength",variant(14));
			std::vector<variant> members;
			for(int n = 0; n != 3; ++n) {
				members.emplace_back(i_[n].fake_ptr());
			}

			return variant(members);
		} else if(key == "char") {
			return variant(c_.fake_ptr());
		} else {
			return variant(0);
		}
	}

	mock_char c_;
	mutable map_formula_callable i_[3];

public:
	mock_party() {}
};

BOOST_AUTO_TEST_SUITE(formula_core)

mock_char c;
mock_party p;

BOOST_AUTO_TEST_CASE(test_formula_basic_arithmetic)
{
	BOOST_CHECK_EQUAL(formula("strength").evaluate(c).as_int(), 15);
	BOOST_CHECK_EQUAL(formula("17").evaluate().as_int(), 17);

	BOOST_CHECK_EQUAL(formula("strength/2 + agility").evaluate(c).as_int(), 19);
	BOOST_CHECK_EQUAL(formula("(strength+agility)/2").evaluate(c).as_int(), 13);

	BOOST_CHECK_EQUAL(formula("20 % 3").evaluate().as_int(), 2);
	BOOST_CHECK_EQUAL(formula("19.5 % 3").evaluate().as_decimal(),
		static_cast<int>(1000.0 * 1.5));

	BOOST_CHECK_EQUAL(formula("-5").evaluate().as_int(), -5);

	BOOST_CHECK_EQUAL(formula("4^2").evaluate().as_int(), 16);
	BOOST_CHECK_EQUAL(formula("2+3^3").evaluate().as_int(), 29);
	BOOST_CHECK_EQUAL(formula("2*3^3+2").evaluate().as_int(), 56);
	BOOST_CHECK_EQUAL(formula("2^3^2").evaluate().as_int(), 512);
	BOOST_CHECK_EQUAL(formula("9^3").evaluate().as_int(), 729);
	BOOST_CHECK(formula("(-2)^0.5").evaluate().is_null());
}

BOOST_AUTO_TEST_CASE(test_formula_basic_logic)
{
	BOOST_CHECK_EQUAL(formula("strength > 12").evaluate(c).as_int(), 1);
	BOOST_CHECK_EQUAL(formula("strength > 18").evaluate(c).as_int(), 0);

	BOOST_CHECK_EQUAL(formula("if(strength > 12, 7, 2)").evaluate(c).as_int(), 7);
	BOOST_CHECK_EQUAL(formula("if(strength > 18, 7, 2)").evaluate(c).as_int(), 2);

	BOOST_CHECK_EQUAL(formula("2 and 1").evaluate().as_int(), 1);
	BOOST_CHECK_EQUAL(formula("2 and 0").evaluate().as_int(), 0);
	BOOST_CHECK_EQUAL(formula("2 or 0").evaluate().as_int(), 2);

	BOOST_CHECK_EQUAL(formula("not 5").evaluate().as_int(), 0);
	BOOST_CHECK_EQUAL(formula("not 0").evaluate().as_int(), 1);
}

BOOST_AUTO_TEST_CASE(test_formula_callable)
{
	// These are just misc tests that were in the original unit tests
	// I wasn't sure how to classify them.
	BOOST_CHECK_EQUAL(formula("char.strength").evaluate(p).as_int(), 15);
	BOOST_CHECK_EQUAL(formula("choose(members,strength).strength").evaluate(p).as_int(), 16);

	BOOST_CHECK_EQUAL(formula("char.sum([strength, agility, intelligence])").evaluate(p).as_int(), 37);
}

BOOST_AUTO_TEST_CASE(test_formula_where_clause)
{
	BOOST_CHECK_EQUAL(formula("x*5 where x=1").evaluate().as_int(), 5);
	BOOST_CHECK_EQUAL(formula("x*5 where x=2").evaluate().as_int(), 10);

	BOOST_CHECK_EQUAL(formula("x*(a*b where a=2,b=1) where x=5").evaluate().as_int(), 10);
	BOOST_CHECK_EQUAL(formula("char.strength * ability where ability=3").evaluate(p).as_int(), 45);
}

BOOST_AUTO_TEST_CASE(test_formula_strings)
{
	BOOST_CHECK_EQUAL(formula("'abcd' = 'abcd'").evaluate().as_bool(), true);
	BOOST_CHECK_EQUAL(formula("'abcd' = 'acd'").evaluate().as_bool(), false);

	BOOST_CHECK_EQUAL(formula("'ab' .. 'cd'").evaluate().as_string(), "abcd");

	BOOST_CHECK_EQUAL(formula("'strength, agility: [strength], [agility]'").evaluate(c).as_string(),
		   "strength, agility: 15, 12");

	BOOST_CHECK_EQUAL(formula("'String with [']quotes['] and [(]brackets[)]!'").evaluate().as_string(),
		"String with 'quotes' and [brackets]!");
	BOOST_CHECK_EQUAL(formula("'String with ['embedded ' .. 'string']!'").evaluate().as_string(),
		"String with embedded string!");
}

BOOST_AUTO_TEST_CASE(test_formula_dice) {
	const int dice_roll = formula("3d6").evaluate().as_int();
	assert(dice_roll >= 3 && dice_roll <= 18);
}

BOOST_AUTO_TEST_CASE(test_formula_containers) {
	variant myarray = formula("[1,2,3]").evaluate();
	BOOST_CHECK_EQUAL(myarray.num_elements(), 3);
	BOOST_CHECK_EQUAL(myarray[0].as_int(), 1);
	BOOST_CHECK_EQUAL(myarray[1].as_int(), 2);
	BOOST_CHECK_EQUAL(myarray[2].as_int(), 3);

	variant mydict = formula("['foo' -> 5, 'bar' ->7]").evaluate();
	BOOST_CHECK_EQUAL(mydict.num_elements(), 2);
	BOOST_CHECK_EQUAL(mydict[variant("foo")].as_int(), 5);
	BOOST_CHECK_EQUAL(mydict[variant("bar")].as_int(), 7);

	variant myrange = formula("-2~2").evaluate();
	BOOST_CHECK_EQUAL(myrange.num_elements(), 5);
	BOOST_CHECK_EQUAL(myrange[0].as_int(), -2);
	BOOST_CHECK_EQUAL(myrange[1].as_int(), -1);
	BOOST_CHECK_EQUAL(myrange[2].as_int(), 0);
	BOOST_CHECK_EQUAL(myrange[3].as_int(), 1);
	BOOST_CHECK_EQUAL(myrange[4].as_int(), 2);

	variant myslice = formula("(10~20)[[1,3,7,9]]").evaluate();
	BOOST_CHECK_EQUAL(myslice.num_elements(), 4);
	BOOST_CHECK_EQUAL(myslice[0].as_int(), 11);
	BOOST_CHECK_EQUAL(myslice[1].as_int(), 13);
	BOOST_CHECK_EQUAL(myslice[2].as_int(), 17);
	BOOST_CHECK_EQUAL(myslice[3].as_int(), 19);
}

BOOST_AUTO_TEST_CASE(test_formula_tokenizer) {
	using namespace wfl::tokenizer;
	std::string test = "[(abc + 4 * (5+3))^2.0, functions, '[']thing[']']";
	std::string::const_iterator i1 = test.begin();
	std::string::const_iterator i2 = test.end();
	std::pair<std::string, TOKEN_TYPE> tokens[] =  {
		{"[", TOKEN_LSQUARE}, {"(", TOKEN_LPARENS}, {"abc", TOKEN_IDENTIFIER},
		{" ", TOKEN_WHITESPACE}, {"+", TOKEN_OPERATOR}, {" ", TOKEN_WHITESPACE},
		{"4", TOKEN_INTEGER}, {" ", TOKEN_WHITESPACE}, {"*", TOKEN_OPERATOR},
		{" ", TOKEN_WHITESPACE}, {"(", TOKEN_LPARENS}, {"5", TOKEN_INTEGER},
		{"+", TOKEN_OPERATOR}, {"3", TOKEN_INTEGER}, {")", TOKEN_RPARENS},
		{")", TOKEN_RPARENS}, {"^", TOKEN_OPERATOR}, {"2.0", TOKEN_DECIMAL},
		{",", TOKEN_COMMA}, {" ", TOKEN_WHITESPACE}, {"functions", TOKEN_KEYWORD},
		{",", TOKEN_COMMA}, {" ", TOKEN_WHITESPACE}, {"'[']thing[']'", TOKEN_STRING_LITERAL},
		{"]", TOKEN_RSQUARE},
	};
	for(auto tok : tokens) {
		token t = get_token(i1, i2);
		assert(std::string(t.begin, t.end) == tok.first);
		assert(t.type == tok.second);
	}
}

BOOST_AUTO_TEST_SUITE_END()
