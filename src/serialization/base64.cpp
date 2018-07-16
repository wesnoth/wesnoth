/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project https://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "serialization/base64.hpp"

#include <vector>
#include <string>

namespace {
const std::string base64_itoa_map = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
const std::string crypt64_itoa_map = "./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

void fill_atoi_map(std::vector<int>& atoi, const std::string& itoa)
{
	for(int i=0; i<64; ++i) {
		atoi[itoa[i]] = i;
	}
}

const std::vector<int>& base64_atoi_map()
{
	static std::vector<int> atoi64(256, -1);
	if(atoi64['A'] == -1) {
		fill_atoi_map(atoi64, base64_itoa_map);
	}
	return atoi64;
}
const std::vector<int>& crypt64_atoi_map()
{
	static std::vector<int> atoi64(256, -1);
	if(atoi64['A'] == -1) {
		fill_atoi_map(atoi64, crypt64_itoa_map);
	}
	return atoi64;
}
char itoa(unsigned value, const std::string& map)
{
	return map[value & 0x3f];
}

std::vector<uint8_t> generic_decode_be(utils::string_view in, const std::vector<int>& atoi_map)
{
	const int last_char = in.find_last_not_of("=");
	const int num_chars = last_char + 1;
	const int length = num_chars * 6 / 8;

	std::vector<uint8_t> out;
	out.reserve(length);

	int val = 0, bits = -8;
	for(unsigned char c: in) {
		if(atoi_map[c] == -1) {
			// Non-base64 character encountered. Should be =
			if(c != '='){
				// If it's not a valid char, return an empty result
				return {};
			}
			break;
		}
		val = (val<<6) + atoi_map[c];
		bits += 6;
		if(bits >= 0) {
			out.push_back(static_cast<char>((val >> bits) & 0xFF));
			bits -= 8;
			val &= 0xFFFF; // Prevent shifting bits off the left end, which is UB
		}
	}
	if(static_cast<int>(out.size()) != length) {
		return {};
	}

	return out;
}

std::vector<uint8_t> generic_decode_le(utils::string_view in, const std::vector<int>& atoi_map)
{
	const int last_char = in.find_last_not_of("=");
	const int length = last_char * 6 / 8;

	std::vector<uint8_t> out;
	out.reserve(length);

	for(int i = 0; i <= last_char; i += 4) {
		//add first char (always)
		unsigned value = atoi_map[in[i]];

		const bool second_char = i + 1 <= last_char;
		if(!second_char) {
			break;
		}
		//add second char (if present)
		value |= atoi_map[in[i+1]] << 6;

		//output first byte (if second char)
		out.push_back(value & 0xFF);

		const bool third_char = i + 2 <= last_char;
		if(!third_char) {
			break;
		}
		//add third char (if present)
		value |= atoi_map[in[i+2]] << 12;

		//output second byte (if third char)
		out.push_back((value >> 8) & 0xFF);

		const bool fourth_char = i + 3 <= last_char;
		if(!fourth_char) {
			break;
		}
		//add fourth char (if present)
		value |= atoi_map[in[i+3]] << 18;

		//output third byte (if fourth char)
		out.push_back((value >> 16) & 0xFF);
	}

	return out;
}

std::string generic_encode_be(utils::byte_string_view in, const std::string& itoa_map, bool pad)
{
	const int in_len = in.length();
	const int groups = (in_len + 2) / 3;
	const int out_len = groups * 4;

	std::string out;

	int i = 0;
	out.reserve(out_len);
	unsigned value = 0;
	unsigned bits = 0;
	while(i < in_len) {
		value <<= 8;
		value |= in[i++];
		bits += 8;
		do {
			bits -= 6;
			out.push_back(itoa(value >> bits, itoa_map));
		} while(bits >= 6);
	}
	if(bits > 0) {
		out.push_back(itoa(value << (6 - bits), itoa_map));
	}

	if(pad) {
		// If not round, append = chars
		out.resize(out_len, '=');
	}

	return out;

}
std::string generic_encode_le(utils::byte_string_view in, const std::string& itoa_map, bool pad)
{
	const int in_len = in.length();
	const int groups = (in_len + 2) / 3;
	const int out_len = groups * 4;

	std::string out;

	int i = 0;
	out.reserve(out_len);
	while(i < in_len) {
		//add first byte (always)
		unsigned value = in[i];
		//output first char (always)
		out.push_back(itoa(value, itoa_map));
		//add second byte (if present)
		const bool second_byte = ++i < in_len;
		if(second_byte) {
			value |= static_cast<int>(in[i]) << 8;
		}
		//output second char (always, contains 2 bits from first byte)
		out.push_back(itoa(value >> 6, itoa_map));
		if(!second_byte) {
			break;
		}
		//add third byte (if present)
		const bool third_byte = ++i < in_len;
		if(third_byte) {
			value |= static_cast<int>(in[i]) << 16;
		}
		//output third char (if second byte)
		out.push_back(itoa(value >> 12, itoa_map));
		//output fourth char (if third byte)
		if(third_byte) {
			out.push_back(itoa(value >> 18, itoa_map));
			++i;
		}
	}

	if(pad) {
		// If not round, append = chars
		out.resize(out_len, '=');
	}

	return out;

}
}

namespace base64 {
std::vector<uint8_t> decode(utils::string_view in)
{
	return generic_decode_be(in, base64_atoi_map());
}
std::string encode(utils::byte_string_view bytes)
{
	return generic_encode_be(bytes, base64_itoa_map, true);
}
}
namespace crypt64{
std::vector<uint8_t> decode(utils::string_view in)
{
	return generic_decode_le(in, crypt64_atoi_map());
}
std::string encode(utils::byte_string_view bytes)
{
	return generic_encode_le(bytes, crypt64_itoa_map, false);
}
int decode(char encoded_char)
{
	std::size_t pos = crypt64_itoa_map.find(encoded_char);
	return pos == std::string::npos ? -1 : pos;
}
char encode(int value)
{
	return itoa(value, crypt64_itoa_map);
}
}
