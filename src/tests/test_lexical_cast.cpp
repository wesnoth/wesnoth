/* $Id$ */
/*
   Copyright (C) 2009 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "utils/test_support.hpp"

#define GETTEXT_DOMAIN "wesnoth-test"

#include "lexical_cast.hpp"





namespace test_throw {

#define LEXICAL_CAST_DEBUG
#include "lexical_cast.hpp"

#define TEST_CASE(type_send, type_used, initializer)                           \
	do {                                                                       \
		typedef type_send TS;                                                  \
		TS val = initializer value;                                            \
                                                                               \
		typedef type_used TU;                                                  \
		typedef implementation::tlexical_cast<std::string, TU> tclass;         \
		try {                                                                  \
			lexical_cast<std::string>(val);                                    \
		} catch(const std::type_info* type) {                                  \
			static const std::type_info& expected_type =                       \
					typeid((std::string (tclass::*)                            \
						(TU, const boost::true_type&)) &tclass::cast);         \
                                                                               \
			BOOST_REQUIRE_MESSAGE((*type == expected_type) == match,           \
					"Test failed : Excpected result "                          \
					<< (match ? "equal " : "not equal") << '\n'                \
					<< "type:     " << typeid(TS).name() << '\n'               \
					<< "caught:   " <<type->name() << '\n'                     \
					<< "expected: " << expected_type.name() << '\n');          \
			break;                                                             \
		}                                                                      \
                                                                               \
		BOOST_CHECK(false);                                                    \
	} while(0)


template<class T>
void test_intergral(const bool match = true)
{
	T value = T();

	TEST_CASE(T, T, );
	TEST_CASE(const T, T, );

	TEST_CASE(T&, T, );
	TEST_CASE(const T&, T, );

	TEST_CASE(T*, T*, &);
	TEST_CASE(const T*, const T*, &);

	TEST_CASE(T* const, T*, &);
	TEST_CASE(const T* const, const T*, &);
}
#undef TEST_CASE

BOOST_AUTO_TEST_CASE(test_lexical_cast_throw)
{
	/* note Wesnoth's coding style doesn't allow w_char so ignore them. */

	test_intergral<bool>();

	/*
	 * We don't want chars to match since a string cast of a char is
	 * ambiguous; does the user want it interpreted as a char or as a number?
	 * But as long as that hasn't been fixed, leave the char.
	 */
	test_intergral<char>();
	test_intergral<signed char>();
	test_intergral<unsigned char>();

	test_intergral<short>();
	test_intergral<int>();
	test_intergral<long>();
	test_intergral<long long>();

	test_intergral<unsigned short>();
	test_intergral<unsigned int>();
	test_intergral<unsigned long>();
	test_intergral<unsigned long long>();

	test_intergral<float>(false);
	test_intergral<double>(false);
	test_intergral<long double>(false);
}

} //  namespace test_throw

BOOST_AUTO_TEST_CASE(test_lexical_cast_result)
{
	BOOST_CHECK(lexical_cast<std::string>(true) == "1");
	BOOST_CHECK(lexical_cast<std::string>(false) == "0");

	BOOST_CHECK(lexical_cast<std::string>(1) == "1");
	BOOST_CHECK(lexical_cast<std::string>(1u) == "1");

	BOOST_CHECK(lexical_cast<std::string>(1.2f) == "1.2");
	BOOST_CHECK(lexical_cast<std::string>(1.2) == "1.2");
}
