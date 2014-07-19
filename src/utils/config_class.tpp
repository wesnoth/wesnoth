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
 * These template functions act as an interface to the functions defined by
 * the CONFIG_CLASS macro, if you don't want to read the macros below.
 *
 * CONFIG_CLASS defines an all public class (not technically a struct though) from 
 * a list of member variables, identifiers, and attribute_value_converter objects.
 * Converter objects are function objects which might be constructed with default 
 * values or special policies, and convert between some type and config::attribute_value
 * and back. The variable name is enforced to be the corresponding config attribute 
 * value, plus an underscore. 
 *
 * TODO: A fallback attribute value to read from may be specified in case the intended
 * attribute is missing.
 *
 * TODO: Extend this to include config converter objects to parse children.
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

// Generate code from a ( type, identifier, convertor) triple
// Gets a variable defn corresponding to a triple
#define GET_VAR_DEFN3( a, b, c ) a GET_VAR_NAME( b ) ;
#define EXPAND_VAR_DEFS3( r, data, elem ) GET_VAR_DEFN3 elem

// Gets a read statement corresponding to a triple
#define GET_READ3( a, b, c ) GET_VAR_NAME( b ) = c ( cfg[ QQ( b ) ] ) ;
#define EXPAND_READS3( r, data, elem ) GET_READ3 elem

// Gets a write statement corresponding to a triple
#define GET_WRITE3( a, b, c ) cfg[ QQ( b ) ] = c ( GET_VAR_NAME( b ) ) ;
#define EXPAND_WRITES3( r, data, elem ) GET_WRITE3 elem

// Formats args as triples so that we can use BOOST_PP_SEQ_FOREACH
#define ADD_PARENS_1( A, B, C ) ((A, B, C)) ADD_PARENS_2
#define ADD_PARENS_2( A, B, C ) ((A, B, C)) ADD_PARENS_1
#define ADD_PARENS_1_END
#define ADD_PARENS_2_END
#define MAKE_TRIPLES( INPUT ) BOOST_PP_CAT(ADD_PARENS_1 INPUT,_END)

// Generate code from a ( type, identifier, convertor, fallback config attribute ) quad
// Get a variable definition from a quad the same as from a triple
#define GET_VAR_DEFN4( a, b, c, d ) GET_VAR_DEFN3( a, b, c )
#define EXPAND_VAR_DEFS4( r, data, elem ) GET_VAR_DEFN4 elem

// Get a write statement from a quad the same as from a triple
#define GET_WRITE4( a, b, c, d ) GET_WRITE3( a, b, c )
#define EXPAND_WRITES4( r, data, elem ) GET_WRITE4 elem

// Get a read statement corresponding to a quad
#define GET_READ4( a, b, c, d ) GET_VAR_NAME( b ) = c ( cfg.has_attribute( QQ( b ) ) ? cfg[ QQ( b ) ] : cfg[ d ] )

// Format args as quads so that we can use BOOST_PP_SEQ_FOREACH
#define ADD_PARENS_3( A, B, C, D ) ((A, B, C, D)) ADD_PARENS_4
#define ADD_PARENS_4( A, B, C, D ) ((A, B, C, D)) ADD_PARENS_3
#define ADD_PARENS_3_END
#define ADD_PARENS_4_END
#define MAKE_QUADS( INPUT ) BOOST_PP_CAT(ADD_PARENS_3 INPUT,_END)

// Define the config struct, with variable defns, read and write functions, and a name.
// This can now be used with the templates defined above.
#define CONFIG_CLASS( NAME, CONTENTS3 ) \
class NAME { \
public: \
	BOOST_PP_SEQ_FOR_EACH( EXPAND_VAR_DEFS3 , , MAKE_TRIPLES( CONTENTS3 ) ) \
	void read(const config & cfg) { \
		BOOST_PP_SEQ_FOR_EACH( EXPAND_READS3, , MAKE_TRIPLES( CONTENTS3 ) ) \
	} \
	void write(config & cfg) const { \
		BOOST_PP_SEQ_FOR_EACH( EXPAND_WRITES3, , MAKE_TRIPLES( CONTENTS3 ) ) \
	} \
	static const std::string name() { return QQ(NAME) ; } \
};

/*
#define CONFIG_CLASS( NAME, CONTENTS3, CONTENTS4 ) \
class NAME { \
public: \
	BOOST_PP_SEQ_FOREACH( EXPAND_VAR_DEFS3 , , MAKE_TRIPLES( CONTENTS3 ) ) \
	BOOST_PP_SEQ_FOREACH( EXPAND_VAR_DEFS4 , , MAKE_QUADS( CONTENTS4 ) ) \
	void read(const config & cfg) { \
		BOOST_PP_SEQ_FOREACH( EXPAND_READS3, , MAKE_TRIPLES( CONTENTS3 ) ) \
		BOOST_PP_SEQ_FOREACH( EXPAND_READS4, , MAKE_QUADS( CONTENTS4 ) ) \
	} \
	void write(config & cfg) { \
		BOOST_PP_SEQ_FOREACH( EXPAND_WRITES3, , MAKE_TRIPLES( CONTENTS3 ) ) \
		BOOST_PP_SEQ_FOREACH( EXPAND_WRITES4, , MAKE_QUADS( CONTENTS4 ) ) \
	} \
	static const std::string name( QQ(NAME) ); \
};
*/

#endif
