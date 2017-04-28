/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef SERIALIZATION_UCS4_CONVERT_IMPL_HPP_INCLUDED
#define SERIALIZATION_UCS4_CONVERT_IMPL_HPP_INCLUDED

#include "unicode_types.hpp"
#include "utils/math.hpp"
#include <cassert>

namespace ucs4_convert_impl
{
	struct utf8_impl
	{
		static const char* get_name()  { return "utf8"; }
		static size_t byte_size_from_ucs4_codepoint(ucs4::char_t ch)
		{
			if(ch < (1u << 7))
				return 1;
			else if(ch < (1u << 11))
				return 2;
			else if(ch < (1u << 16))
				return 3;
			else if(ch < (1u << 21))
				return 4;
			else if(ch < (1u << 26))
				return 5;
			else if(ch < (1u << 31))
				return 6;
			else
				throw utf8::invalid_utf8_exception(); // Invalid UCS-4
		}

		static int byte_size_from_utf8_first(utf8::char_t ch)
		{
			if (!(ch & 0x80)) {
				return 1;  // US-ASCII character, 1 byte
			}
			/* first bit set: character not in US-ASCII, multiple bytes
			 * number of set bits at the beginning = bytes per character
			 * e.g. 11110xxx indicates a 4-byte character */
			int count = count_leading_ones(ch);
			if (count == 1 || count > 6) {		// count > 4 after RFC 3629
				throw utf8::invalid_utf8_exception(); // Stop on invalid characters
			}
			return count;
		}

		/**
		 * Writes a UCS-4 character to a UTF-8 stream.
		 *
		 * @param out  An object to write utf8::char_t. Required operations:
		 *             1) push(utf8::char_t) to write a single character
		 *             2) can_push(size_t n) to check whether there is still
		 *                enough space for n characters.
		 * @param ch   The UCS-4 character to write to the stream.
		 */
		template<typename writer>
		static inline void write(writer out, ucs4::char_t ch)
		{
			size_t count = byte_size_from_ucs4_codepoint(ch);
			assert(out.can_push(count));
			if(count == 1) {
				out.push(static_cast<utf8::char_t>(ch));
			} else {
				for(int j = static_cast<int>(count) - 1; j >= 0; --j) {
					unsigned char c = (ch >> (6 * j)) & 0x3f;
					c |= 0x80;
					if(j == static_cast<int>(count) - 1) {
						c |= 0xff << (8 - count);
					}
					out.push(c);
				}
			}
		}
		/**
		 * Reads a UCS-4 character from a UTF-8 stream
		 *
		 * @param input  An iterator pointing to the first character of a UTF-8
		 *               sequence to read.
		 * @param end    An iterator pointing to the end of the UTF-8 sequence
		 *               to read.
		 */
		template<typename iitor_t>
		static inline ucs4::char_t read(iitor_t& input, const iitor_t& end)
		{
			assert(input != end);
			size_t size = byte_size_from_utf8_first(*input);

			ucs4::char_t current_char = static_cast<unsigned char>(*input);

			// Convert the first character
			if(size != 1) {
				current_char &= 0xFF >> (size + 1);
			}

			// Convert the continuation bytes
			// i == number of '++input'
			++input;
			for(size_t i = 1; i < size; ++i, ++input) {
				// If the string ends occurs within an UTF8-sequence, this is bad.
				if (input == end)
					throw utf8::invalid_utf8_exception();

				if ((*input & 0xC0) != 0x80)
					throw utf8::invalid_utf8_exception();

				current_char = (current_char << 6) | (static_cast<unsigned char>(*input) & 0x3F);
			}
			//i == size => input was increased size times.

			// Check for non-shortest-form encoding
			// This has been forbidden in Unicode 3.1 for security reasons
			if (size > byte_size_from_ucs4_codepoint(current_char))
				throw utf8::invalid_utf8_exception();
			return current_char;
		}
	};

	struct utf16_impl
	{
		static const char* get_name()  { return "utf16"; }
		template<typename writer>
		static inline void write(writer out, ucs4::char_t ch)
		{
			const ucs4::char_t bit17 = 0x10000;

			if(ch < bit17)
			{
				assert(out.can_push(1));
				out.push(static_cast<utf16::char_t>(ch));
			}
			else
			{
				assert(out.can_push(2));
				const ucs4::char_t char20 = ch - bit17;
				assert(char20 < (1 << 20));
				const ucs4::char_t lead = 0xD800 + (char20 >> 10);
				const ucs4::char_t trail = 0xDC00 + (char20 & 0x3FF);
				assert(lead < bit17);
				assert(trail < bit17);
				out.push(static_cast<utf16::char_t>(lead));
				out.push(static_cast<utf16::char_t>(trail));
			}
		}

		template<typename iitor_t>
		static inline ucs4::char_t read(iitor_t& input, const iitor_t& end)
		{
			const ucs4::char_t last10 = 0x3FF;
			const ucs4::char_t type_filter = 0xFC00;
			const ucs4::char_t type_lead = 0xD800;
			const ucs4::char_t type_trail = 0xDC00;

			assert(input != end);
			ucs4::char_t current_char = static_cast<utf16::char_t>(*input);
			++input;
			ucs4::char_t type = current_char & type_filter;
			if(type == type_trail)
			{
				//found trail without head
				throw utf8::invalid_utf8_exception();
			}
			else if(type == type_lead)
			{
				if(input == end)
				{
					//If the string ends occurs within an UTF16-sequence, this is bad.
					throw utf8::invalid_utf8_exception();
				}
				if((*input & type_filter) != type_trail)
				{
					throw utf8::invalid_utf8_exception();
				}
				current_char &= last10;
				current_char <<= 10;
				current_char += (*input & last10);
				current_char += 0x10000;
				++input;
			}
			return current_char;
		}
	};

	struct utf32_impl
	{
		static const char* get_name()  { return "UCS4"; }
		template<typename writer>
		static inline void write(writer out, ucs4::char_t ch)
		{
			assert(out.can_push(1));
			out.push(ch);
		}

		template<typename iitor_t>
		static inline ucs4::char_t read(iitor_t& input, const iitor_t& end)
		{
			assert(input != end);
			ucs4::char_t current_char = *input;
			++input;
			return current_char;
		}
	};

	template<typename T_CHAR>
	struct convert_impl {};

	template<>
	struct convert_impl<utf8::char_t>
	{
		typedef utf8_impl type;
	};

	template<>
	struct convert_impl<utf16::char_t>
	{
		typedef utf16_impl type;
	};

	template<>
	struct convert_impl<ucs4::char_t>
	{
		typedef utf32_impl type;
	};
}

#endif
