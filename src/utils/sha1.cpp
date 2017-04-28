/*
   Copyright (C) 2007 - 2017 by Benoit Timbert <benoit.timbert@free.fr>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 *  @file
 *  Secure Hash Algorithm 1 (SHA-1).
 *  Used to checksum the game-config / cache.
 */

/* This is supposed to be an implementation of the
   Secure Hash Algorithm 1 (SHA-1)

   Check RFC 3174 for details about the algorithm.

   Currently this implementation might produce different results on little-
   and big-endian machines, but for our current usage, we don't care :)
*/

#include "utils/sha1.hpp"

#include <iomanip>
#include <sstream>

#define sha_rotl(n,x)		( ((x) << (n)) | ((x) >> (32-(n))) )
#define sha_ch(x,y,z)		( ((x) & (y)) | ((~(x)) & (z)) )
#define sha_parity(x,y,z)	( (x) ^ (y) ^ (z) )
#define sha_maj(x,y,z)		( ((x) & (y)) | ((x) & (z)) | ((y) & (z)) )

std::string sha1_hash::display() {
	std::stringstream s;
	s << std::hex << std::setfill('0') << std::setw(8) << H0;
	s << std::hex << std::setfill('0') << std::setw(8) << H1;
	s << std::hex << std::setfill('0') << std::setw(8) << H2;
	s << std::hex << std::setfill('0') << std::setw(8) << H3;
	s << std::hex << std::setfill('0') << std::setw(8) << H4;
	return s.str();
}

sha1_hash::sha1_hash(const std::string& str)
: H0(0x67452301), H1(0xefcdab89), H2(0x98badcfe), H3(0x10325476), H4(0xc3d2e1f0)
{
	uint8_t block[64];

	int bytes_left = str.size();
	uint32_t ssz = bytes_left * 8; // string length in bits

	std::stringstream iss (str, std::stringstream::in);
	// cut our string in 64 bytes blocks then process it
	while (bytes_left > 0) {
		iss.read(reinterpret_cast<char*>(block), 64);
		if (bytes_left <= 64) { // if it's the last block, pad it
			if (bytes_left < 64) {
				block[bytes_left]= 0x80; // add a 1 bit right after the end of the string
			}
			int i;
			for (i = 63; i > bytes_left; i--) {
				block[i]=0; // pad our block with zeros
			}
			if (bytes_left < 56) { // enough space to store the length
				// put the length at the end of the block
				block[60] = ssz >> 24;
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4244)
#endif
				block[61] = ssz >> 16;
				block[62] = ssz >> 8;
				block[63] = ssz;
#ifdef _MSC_VER
#pragma warning (pop)
#endif
			} else { // not enough space for the zeros => we need a new block
				next(block);
				// new block
				for (i = 0; i < 60 ; i++) {
					block[i]=0; // pad our block with zeros
				}
				if (bytes_left == 64) {
					block[0]= 0x80; // add a 1 bit right after the end of the string = beginning of our new block
				}
				// put the length at the end of the block
				block[60] = ssz >> 24;
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4244)
#endif
				block[61] = ssz >> 16;
				block[62] = ssz >> 8;
				block[63] = ssz;
#ifdef _MSC_VER
#pragma warning (pop)
#endif
			}
		}
		next(block);
		bytes_left -= 64;
	}
}

void sha1_hash::next(uint8_t block[64]) {
	uint32_t W[80];
	uint32_t A, B, C, D, E, T;
	int i;

	A = H0;
	B = H1;
	C = H2;
	D = H3;
	E = H4;
	for (i = 0; i < 16; i++) {
		W[i]= (block[4 * i] << 24) | (block[4 * i + 1] << 16) | (block[4 * i + 2] << 8) | block[4 * i + 3];
	}
	for (; i < 80; i++) {
		W[i]=sha_rotl(1, W[i-3] ^ W[i-8] ^ W[i-14] ^ W[i-16]);
	}
	for (i = 0; i < 20; i++) {
		T = sha_rotl(5,A) + sha_ch(B,C,D) + E + W[i] + 0x5a827999;
		E = D;
		D = C;
		C = sha_rotl(30,B);
		B = A;
		A = T;
	}
	for (; i < 40; i++) {
		T = sha_rotl(5,A) + sha_parity(B,C,D) + E + W[i] + 0x6ed9eba1;
		E = D;
		D = C;
		C = sha_rotl(30,B);
		B = A;
		A = T;
	}
	for (; i < 60; i++) {
		T = sha_rotl(5,A) + sha_maj(B,C,D) + E + W[i] + 0x8f1bbcdc;
		E = D;
		D = C;
		C = sha_rotl(30,B);
		B = A;
		A = T;
	}
	for (; i < 80; i++) {
		T = sha_rotl(5,A) + sha_parity(B,C,D) + E + W[i] + 0xca62c1d6;
		E = D;
		D = C;
		C = sha_rotl(30,B);
		B = A;
		A = T;
	}
	H0 += A;
	H1 += B;
	H2 += C;
	H3 += D;
	H4 += E;
}
