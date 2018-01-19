/*
   Copyright (C) 2012 - 2018 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
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

#include "addon/validation.hpp"

BOOST_AUTO_TEST_SUITE( addons )

BOOST_AUTO_TEST_CASE( validation )
{
	BOOST_CHECK( !addon_filename_legal("") );
	BOOST_CHECK( !addon_filename_legal(".") );
	BOOST_CHECK( !addon_filename_legal("..") );
	BOOST_CHECK( !addon_filename_legal("invalid/slash") );
	BOOST_CHECK( !addon_filename_legal("invalid\\backslash") );
	BOOST_CHECK( !addon_filename_legal("invalid:colon") );
	BOOST_CHECK( !addon_filename_legal("invalid~tilde") );
	BOOST_CHECK( !addon_filename_legal("invalid/../parent") );

	BOOST_CHECK( addon_name_legal("-0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz") );

	BOOST_CHECK( !addon_name_legal("invalid\nnewline") );
	BOOST_CHECK( !addon_name_legal("invalid\x0A""explicitLF") );
	BOOST_CHECK( !addon_name_legal("invalid\x0D\x0A""explicitCRLF") );
	BOOST_CHECK( !addon_name_legal("invalid\x0D""explicitCR") );
	BOOST_CHECK( !addon_name_legal("invalid`grave accent`") );
	BOOST_CHECK( !addon_name_legal("invalid$dollarsign$") );
}

BOOST_AUTO_TEST_CASE( encoding )
{
	BOOST_CHECK( encode_binary("").empty() );
	BOOST_CHECK( unencode_binary("").empty() );

	//
	// Plain string.
	//

	const std::string plain = "ABC";

	BOOST_CHECK( encode_binary(plain) == plain );
	BOOST_CHECK( unencode_binary(plain) == plain );

	//
	// Binary escaping (direct).
	//

	const char bin_escape = '\x01';
	const std::string bin_special = "\x0D\xFE";
	const std::string raw = "ABC \x01 DEF \x0D\x0A JKL \xFE MNO";

	std::string encoded;

	//
	// The encoding algorithm as of 1.11.15 is as follows:
	//
	//   * let c be the char to encode
	//   * let e be the escaping char (\x01)
	//   * if (c in \x00\x0D\xFE or c == e) then return e followed by the
	//     character with value c+1.
	//
	// There is no test for \x00 here because \x00 really shouldn't occur in
	// a string -- otherwise things get weird.
	//
	for(const char c : raw)
	{
		if(c == bin_escape || bin_special.find(c) != std::string::npos) {
			encoded += bin_escape;
			encoded += (c + 1);
		} else {
			encoded += c;
		}
	}

	BOOST_CHECK( encode_binary(raw) == encoded );
	BOOST_CHECK( unencode_binary(encoded) == raw );
	// Identity.
	BOOST_CHECK( unencode_binary(encode_binary(raw)) == raw );
	BOOST_CHECK( unencode_binary(encode_binary(encoded)) == encoded );

	//
	// Binary escaping (recursive).
	//

	const unsigned recursive_steps = 16;
	std::string recursive_encoded = raw;

	for(unsigned n = 0; n < recursive_steps; ++n) {
		recursive_encoded = encode_binary(recursive_encoded);
	}

	BOOST_CHECK( recursive_encoded != raw );

	for(unsigned n = 0; n < recursive_steps; ++n) {
		recursive_encoded = unencode_binary(recursive_encoded);
	}

	BOOST_CHECK( recursive_encoded == raw );
}

BOOST_AUTO_TEST_SUITE_END()
