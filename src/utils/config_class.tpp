/*
   Copyright (C) 2014 by Chris Beck <render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef CONFIG_CLASS_HPP_
#define CONFIG_CLASS_HPP_

#include "config.hpp"
#include "global.hpp"
#include "wml_exception.hpp"

#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <cassert>
#include <cstddef>
#include <string>

/**
 * This file defines the CONFIG_CLASS macro and interface.
 *
 * A config class is essentially a struct which specifies read and write methods
 * to convert it to and from a config. These conversions are specified in the definition,
 * by listing a type, an identifier, and (optionally) a conversion policy. Example usage:
 *
 * CONFIG_CLASS( demo,
 * 	(CONFIG_VAL(int,		i,	convert::to_int(3) ))
 * 	(CONFIG_VAL(std::string,	s,	convert::str() ))
 *	(CONFIG_VAL(bool,		b, 	convert::to_bool(true)))
 * )
 *
 * This defines a class with public member variables int i_, std::string s_, and bool b_,
 * which can be converted to and from a config using the template functions
 * `to_config<demo>` and `from_config<demo>`. The convert functions defined above refer
 * to member functions of config::attribute value. The third argument can be any
 * attribute_value_converter object. If it is ommitted, then the assignment operator of
 * config::attribute_value is invoked.
 *
 * The CONFIG_VAL_WITH_FALLBACK macro specifies the name of a secondary config attribute to
 * read if the first one is missing.
 *
 * The CONFIG_VAL_T macro is for use with templated converters, it adds the type (first arg)
 * as a template parameter to the converter implicitly.
 *
 * TODO: Extend this to include config converter objects to parse children.
 */

/**
 * These template functions act as an interface to the functions defined by
 * the CONFIG_CLASS macro, so that directly calling the read / write functions
 * defined by the macro is unnecessary.
 */
template <class T>
config to_config(const T & t)
{
	config c;
	t.write(c);
	return c;
}

template <class T>
T from_config(const config & c)
{
	T ret;
	ret.read(c);
	return ret;
}

template <class T>
T from_config_warn(const config & c)
{
	T ret = from_config<T>(c);
	config check = to_config<T>(ret);
	config diff = check.get_diff(c);
	VALIDATE_WITH_DEV_MESSAGE( !diff.child("delete") , std::string("Lost some information when parsing a config to class ") + T::name() , diff.child("delete").debug());
	return ret;
}


/**
 * This abstract base class defines an conversion pair for attribute value
 * and some other type. There may be many of these for any type, and the
 * converter may take arguments when it is constructed. Objects derived from
 * this type are expected to be used as building blocks to define a config
 * class.
 */
template <typename T>
class attribute_value_converter {
public:
	virtual T operator()(const config::attribute_value &) = 0;
	virtual config::attribute_value operator()(const T &) = 0;

	virtual ~attribute_value_converter() {}
};

// Wrappers for primitive token pasting ops and quote ops
#define CAT2( A, B ) A ## B
#define CAT3( A, B, C ) A ## B ## C
#define QQ( A ) #A

// Gets a variable name by appending _
#define GET_VAR_NAME( b ) CAT2( b, _ )

// Gets a config atttribute value from name
#define GET_CFG_NAME( b ) cfg [ QQ ( b ) ]

// Compose a var def, read statement, write statement, and identifier name, from an input tuple. We hold onto the identifier name for use when making the list of attributes.
#define CONFIG_VAL( a, b, c ) ((  a GET_VAR_NAME(b) ; \
				, GET_VAR_NAME(b) = c ( GET_CFG_NAME(b) ) ; \
				, GET_CFG_NAME(b) = c ( GET_VAR_NAME(b) ) ; \
				, b))

// Compose a var def, read statement, write statement, and identifier name using a fallback value when reading
#define CONFIG_VAL_WITH_FALLBACK( a, b, c, d ) (( a GET_VAR_NAME(b) ; \
				, if (cfg.has_attribute( QQ( b ) )) GET_VAR_NAME(b) = c ( GET_CFG_NAME(b) ) ; \
				  else GET_VAR_NAME(b) = c ( cfg[ d ] ); \
				, GET_CFG_NAME(b) = c ( GET_VAR_NAME(b) ) ; \
				, b))

// Pass the type a as a template argument to the converter c, and use default constructor
#define CONFIG_VAL_T( a, b, c ) CONFIG_VAL(a, b, c< a >())

// Generate code from a ( decl, read, write, identifier ) quad (the output of CONFIG_VAL-type macros)
// Gets a variable defn corresponding to a triple
#define GET_VAR_DEFN( a, b, c, d ) a
#define GET_VAR_HELPER( r, data, elem ) GET_VAR_DEFN elem
#define EXPAND_VAR_DEFS( CONTENTS ) BOOST_PP_SEQ_FOR_EACH( GET_VAR_HELPER, , CONTENTS )

// Gets a read statement corresponding to a triple
#define GET_READ( a, b, c, d ) b
#define GET_READ_HELPER( r, data, elem ) GET_READ elem
#define EXPAND_READS( CONTENTS ) BOOST_PP_SEQ_FOR_EACH( GET_READ_HELPER, , CONTENTS )

// Gets a write statement corresponding to a triple
#define GET_WRITE( a, b, c, d ) c
#define GET_WRITE_HELPER( r, data, elem ) GET_WRITE elem
#define EXPAND_WRITES( CONTENTS ) BOOST_PP_SEQ_FOR_EACH( GET_WRITE_HELPER, , CONTENTS )

// Gets a pushback statement used when making a list of attributes
#define GET_PUSHBACK_NAME( a, b, c, d ) attrs.push_back(QQ(d));
#define GET_NAME_HELPER( r, data, elem ) GET_PUSHBACK_NAME elem
#define EXPAND_ATTR_NAMES( CONTENTS ) BOOST_PP_SEQ_FOR_EACH( GET_NAME_HELPER, , CONTENTS )

// Define the config struct, with variable defns, read and write functions, and a name.
// This can now be used with the templates defined above.
#define CONFIG_CLASS( NAME, CONTENTS ) \
class NAME { \
public: \
	EXPAND_VAR_DEFS(CONTENTS)\
	void read(const config & cfg) { \
		EXPAND_READS(CONTENTS) \
	} \
	void write(config & cfg) const { \
		EXPAND_WRITES(CONTENTS) \
	} \
	static const std::string name() { return QQ(NAME) ; } \
	static const std::vector<std::string> attributes() { \
		std::vector<std::string> attrs; \
		EXPAND_ATTR_NAMES(CONTENTS) \
		return attrs; \
	} \
};

#endif
