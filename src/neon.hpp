/*
   Copyright (C) 2012 - 2014 by Mark de Wever <koraq@xs4all.nl>
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
 * @file
 * Helper class for ARM NEON support.
 *
 * When using g++ on an ARM that support the NEON it uses the gcc intrinsics
 * [1], for all other platforms an emulation is used. The emulation is based on
 * the RealView Compilation Tool Assembler Guide (ARM DUI 0204J (ID101213)) [2].
 * The emulation follows the latter convensions instead of the former. The
 * numbers in the sections refer to the section numers in [2].
 *
 * Not everything is implemented, only functions used are implemented.
 *
 * Common template parameters are:
 * * Td type of the destination.
 * * Tn type of the first operand.
 * * Tm type of the second operand.
 * * Ts type of the source (Tm and Tn).
 * * S number of vector elements.
 * * D number of matrix vectors.
 *
 * [1]
 * http://gcc.gnu.org/onlinedocs/gcc-4.7.0/gcc/ARM-NEON-Intrinsics.html
 * [2]
 * http://infocenter.arm.com/help/topic/com.arm.doc.dui0204i/DUI0204I_rvct_assembler_guide.pdf
 */

#ifndef NEON_HPP_INCLUDED
#define NEON_HPP_INCLUDED

#if defined __GNUC__ && defined __ARM_NEON__

#include <arm_neon.h>

#else

#include <boost/cstdint.hpp>

/***** ***** ***** ***** types ***** ***** ***** *****/

/**
 * Emulates a vector.
 *
 * Gcc also supports  __attribute__ ((__vector_size__ (8))) but that only works
 * with gcc, _not_ with g++. It also is not portable.
 *
 * @tparam T                      The base type of the vector.
 * @tparam S                      The size of the vector.
 */
template<class T, unsigned S>
struct tvector
{
	const T&
	operator[](unsigned i) const
	{
		return data[i];
	}

	T&
	operator[](unsigned i)
	{
		return data[i];
	}

	T data[S];
};

typedef tvector<uint8_t, 8> uint8x8_t;
typedef tvector<uint16_t, 8> uint16x8_t;

/**
 * Emulates a matrix.
 *
 * The guide [2] doesn't use the term matrix, but uses various terms for it;
 * e.g. table in the VTBL instructions (5.8.9) and lanes in the VLDn
 * instructions (5.12).
 */
template<class T, unsigned S, unsigned D>
struct tmatrix
{
	tvector<T, S> val[D];
};

typedef tmatrix<uint8_t, 8, 4> uint8x8x4_t;


/***** ***** ***** ***** 5.8.3 VDUP ***** ***** ***** *****/

/* The imm is actually the Rm. */
template<class Td, unsigned S>
inline tvector<Td, S>
vdup_n(Td imm)
{
	tvector<Td, S> d;
	for(unsigned i = 0; i < S; ++i) {
		d[i] = imm;
	}
	return d;
}

inline uint16x8_t
vdupq_n_u16(uint16_t imm)
{
	return vdup_n<uint16_t, 8>(imm);
}

inline uint8x8_t
vdup_n_u8(uint8_t imm)
{
	return vdup_n<uint8_t, 8>(imm);
}

/***** ***** ***** ***** 5.9.3 VSHR ***** ***** ***** *****/

template<class Td, class Tm, unsigned S>
inline tvector<Td, S>
vshr(tvector<Tm, S> m, const unsigned imm)
{
	 tvector<Td, S> d;
	 for(unsigned i = 0; i < S; ++i) {
		d[i] = m[i] >> imm;
	 }
	 return d;
}

inline uint8x8_t
vshrn_n_u16(uint16x8_t m, const unsigned imm)
{
	return vshr<uint8_t, uint16_t, 8>(m, imm);
}

/***** ***** ***** ***** 5.10.3 VADD ***** ***** ***** *****/

template<class Td, class Tn, class Tm, unsigned S>
inline tvector<Td, S>
vadd(tvector<Tn, S> n, tvector<Tm, S> m)
{
	 tvector<Td, S> d;
	 for(unsigned i = 0; i < S; ++i) {
		d[i] = n[i] + m[i];
	 }
	 return d;
}

inline uint16x8_t
vaddq_u16(uint16x8_t n, uint16x8_t m)
{
	return vadd<uint16_t, uint16_t, uint16_t, 8>(n, m);
}

/***** ***** ***** ***** 5.11.1 VMUL ***** ***** ***** *****/

template<class Td, class Ts, unsigned S>
inline tvector<Td, S>
vmul(tvector<Ts, S> n, tvector<Ts, S> m)
{
	 tvector<Td, S> d;
	 for(unsigned i = 0; i < S; ++i) {
		d[i] = n[i] * m[i];
	 }
	 return d;
}


inline uint16x8_t
vmull_u8(uint8x8_t n, uint8x8_t m)
{
	return vmul<uint16_t, uint8_t, 8>(n, m);
}

/***** ***** ***** ***** 5.12.3 VLDn and VSTn ***** ***** ***** *****/

template<class Td, unsigned S, unsigned D>
inline tmatrix<Td, S, D>
vld(Td* base)
{
	tmatrix<Td, S, D> d;
	for(unsigned i = 0; i < S; ++i) {
		for(unsigned j = 0; j < D; ++j) {
			d.val[j][i] = static_cast<Td>(base[i * D + j]);
		}
	}
	return d;
}

inline uint8x8x4_t
vld4_u8(uint8_t* base)
{
	return vld<uint8_t, 8, 4>(base);
}

template<class Td, unsigned S, unsigned D>
inline void
vst(Td* base, tmatrix<Td, S, D> list)
{
	for(unsigned i = 0; i < S; ++i) {
		for(unsigned j = 0; j < D; ++j) {
			base[i * D + j] = list.val[j][i];
		}
	}
}

inline void
vst4_u8(uint8_t* base, uint8x8x4_t list)
{
	vst<uint8_t, 8, 4>(base, list);
}

#endif

#endif
