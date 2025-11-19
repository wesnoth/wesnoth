/*
	Copyright (C) 2003 - 2025
	by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

#include "utils_simd.hpp"
#include <algorithm> // For std::min

// ============================================================================
// PLATFORM DETECTION & INTRINSICS
// ============================================================================

// Ensure __SSE2__ is defined
// 64-bit x86 builds (GCC/Clang/MSVC) mandate SSE2 support. Or MSVC 32-bit builds: _M_IX86_FP >= 2 guarantees SSE2 support.
#if !defined(__SSE2__) && (defined(__x86_64__) || defined(_M_X64) || (defined(_MSC_VER) && defined(_M_IX86_FP) && _M_IX86_FP >= 2))
#define __SSE2__
#endif

#ifdef __SSE2__
#include <emmintrin.h>
#endif

#if defined(__ARM_NEON)
#include <arm_neon.h>
#endif

namespace {

	// ----------------------------------------------------------------------------
	// SCALAR HELPERS (For handling loop remainders)
	// ----------------------------------------------------------------------------

	/**
	 * Processes the final 0-3 pixels that didn't fit into the SIMD vector loop for mask_surface.
	 */
	void mask_surface_scalar_remainder(uint32_t* surf_ptr, const uint32_t* mask_ptr, int remainder, bool& empty)
	{
		for (int i = 0; i < remainder; ++i) {
			const uint32_t surf_pixel = surf_ptr[i];
			const uint32_t mask_alpha = mask_ptr[i] >> 24;
			const uint32_t surf_alpha = surf_pixel >> 24;

			const uint32_t alpha = std::min(surf_alpha, mask_alpha);
			if (alpha) empty = false;

			// Unconditional write is generally faster than branching here
			surf_ptr[i] = (alpha << 24) | (surf_pixel & 0x00FFFFFF);
		}
	}

	/**
	 * Processes the final 0-3 pixels that didn't fit into the SIMD vector loop for in_mask_surface.
	 */
	void in_mask_surface_scalar_remainder(const uint32_t* surf_ptr, const uint32_t* mask_ptr, int remainder, bool& fits)
	{
		for (int i = 0; i < remainder; ++i) {
			const uint32_t mask_alpha = mask_ptr[i] >> 24;
			// If mask is transparent, surface must also be transparent
			if (mask_alpha == 0) {
				const uint32_t surf_alpha = surf_ptr[i] >> 24;
				if (surf_alpha > 0) {
					fits = false;
					return;
				}
			}
		}
	}

} // namespace

// ============================================================================
// SSE2 IMPLEMENTATIONS (x86 / Intel / AMD)
// ============================================================================
#ifdef __SSE2__

static void mask_surface_sse2(uint32_t* surf_ptr, const uint32_t* mask_ptr, int total_pixels, bool& empty)
{
	const int sse_count = total_pixels / 4;
	const int remainder = total_pixels % 4;

	const __m128i rgb_mask = _mm_set1_epi32(0x00FFFFFF);
	const __m128i alpha_mask = _mm_set1_epi32(0xFF000000);
	__m128i has_alpha = _mm_setzero_si128();

	for (int i = 0; i < sse_count; ++i) {
		__m128i surf = _mm_loadu_si128(reinterpret_cast<const __m128i*>(surf_ptr + i * 4));
		__m128i mask = _mm_loadu_si128(reinterpret_cast<const __m128i*>(mask_ptr + i * 4));

		// Calculate new alpha: min(surf_alpha, mask_alpha)
		__m128i surf_alpha = _mm_and_si128(surf, alpha_mask);
		__m128i mask_alpha = _mm_and_si128(mask, alpha_mask);
		__m128i min_alpha = _mm_min_epu8(surf_alpha, mask_alpha);

		// Combine old RGB with new Alpha
		__m128i rgb = _mm_and_si128(surf, rgb_mask);
		__m128i result = _mm_or_si128(rgb, min_alpha);

		_mm_storeu_si128(reinterpret_cast<__m128i*>(surf_ptr + i * 4), result);

		// Accumulate alpha presence
		has_alpha = _mm_or_si128(has_alpha, min_alpha);
	}

	// Check if any pixel in the processed blocks had alpha > 0
	if (_mm_movemask_epi8(_mm_cmpeq_epi32(has_alpha, _mm_setzero_si128())) != 0xFFFF) {
		empty = false;
	}

	if (remainder > 0) {
		mask_surface_scalar_remainder(surf_ptr + sse_count * 4, mask_ptr + sse_count * 4, remainder, empty);
	}
}

static void in_mask_surface_sse2(const uint32_t* surf_ptr, const uint32_t* mask_ptr, int total_pixels, bool& fits)
{
	const int sse_count = total_pixels / 4;
	const int remainder = total_pixels % 4;

	const __m128i alpha_mask = _mm_set1_epi32(0xFF000000);
	const __m128i zero = _mm_setzero_si128();

	for (int i = 0; i < sse_count; ++i) {
		__m128i surf = _mm_loadu_si128(reinterpret_cast<const __m128i*>(surf_ptr + i * 4));
		__m128i mask = _mm_loadu_si128(reinterpret_cast<const __m128i*>(mask_ptr + i * 4));

		// Check where Mask Alpha is Zero
		__m128i mask_alpha = _mm_and_si128(mask, alpha_mask);
		__m128i mask_is_transp = _mm_cmpeq_epi32(mask_alpha, zero); // FFFFFFFF if transparent

		// Check Surface Alpha
		__m128i surf_alpha = _mm_and_si128(surf, alpha_mask);

		// Identify Violations: Mask is Transparent AND Surface has Alpha
		__m128i bad_pixels = _mm_and_si128(mask_is_transp, surf_alpha);

		// If bad_pixels is NOT zero, we have a violation
		__m128i is_clean = _mm_cmpeq_epi32(bad_pixels, zero);
		if (_mm_movemask_epi8(is_clean) != 0xFFFF) {
			fits = false;
			return;
		}
	}

	if (remainder > 0) {
		in_mask_surface_scalar_remainder(surf_ptr + sse_count * 4, mask_ptr + sse_count * 4, remainder, fits);
	}
}

#endif // __SSE2__

// ============================================================================
// NEON IMPLEMENTATIONS (ARM / Apple Silicon)
// ============================================================================
#if defined(__ARM_NEON)

static void mask_surface_neon(uint32_t* surf_ptr, const uint32_t* mask_ptr, int total_pixels, bool& empty)
{
	const int neon_count = total_pixels / 4;
	const int remainder = total_pixels % 4;

	const uint32x4_t rgb_mask = vmovq_n_u32(0x00FFFFFF);
	const uint32x4_t alpha_mask = vmovq_n_u32(0xFF000000);
	uint32x4_t has_alpha = vmovq_n_u32(0);

	for (int i = 0; i < neon_count; ++i) {
		uint32x4_t surf = vld1q_u32(surf_ptr + i * 4);
		uint32x4_t mask = vld1q_u32(mask_ptr + i * 4);

		// Calculate new alpha (byte-wise min)
		uint32x4_t surf_alpha = vandq_u32(surf, alpha_mask);
		uint32x4_t mask_alpha = vandq_u32(mask, alpha_mask);

		// vminq_u8 operates on 8-bit lanes, essentially finding the min alpha byte
		uint8x16_t min_alpha_u8 = vminq_u8(vreinterpretq_u8_u32(surf_alpha), vreinterpretq_u8_u32(mask_alpha));
		uint32x4_t min_alpha = vreinterpretq_u32_u8(min_alpha_u8);

		// Combine
		uint32x4_t rgb = vandq_u32(surf, rgb_mask);
		uint32x4_t result = vorrq_u32(rgb, min_alpha);

		vst1q_u32(surf_ptr + i * 4, result);

		// Accumulate
		has_alpha = vorrq_u32(has_alpha, min_alpha);
	}

	// Horizontal reduction to check if any alpha bit is set
	uint32x2_t p = vorr_u32(vget_low_u32(has_alpha), vget_high_u32(has_alpha));
	p = vorr_u32(p, vrev64_u32(p));

	if (vget_lane_u32(p, 0) != 0) {
		empty = false;
	}

	if (remainder > 0) {
		mask_surface_scalar_remainder(surf_ptr + neon_count * 4, mask_ptr + neon_count * 4, remainder, empty);
	}
}

static void in_mask_surface_neon(const uint32_t* surf_ptr, const uint32_t* mask_ptr, int total_pixels, bool& fits)
{
	const int neon_count = total_pixels / 4;
	const int remainder = total_pixels % 4;

	const uint32x4_t alpha_mask = vmovq_n_u32(0xFF000000);
	const uint32x4_t zero = vmovq_n_u32(0);

	for (int i = 0; i < neon_count; ++i) {
		uint32x4_t surf = vld1q_u32(surf_ptr + i * 4);
		uint32x4_t mask = vld1q_u32(mask_ptr + i * 4);

		// Check where Mask Alpha is Zero
		uint32x4_t mask_alpha = vandq_u32(mask, alpha_mask);
		uint32x4_t mask_is_transp = vceqq_u32(mask_alpha, zero); // All 1s if true

		// Check Surface Alpha
		uint32x4_t surf_alpha = vandq_u32(surf, alpha_mask);

		// Identify Violations: Mask Transparent & Surface Has Alpha
		uint32x4_t bad_pixels = vandq_u32(mask_is_transp, surf_alpha);

		// Check if bad_pixels is non-zero (Horizontal reduction)
		uint32x2_t p = vorr_u32(vget_low_u32(bad_pixels), vget_high_u32(bad_pixels));
		p = vorr_u32(p, vrev64_u32(p));

		if (vget_lane_u32(p, 0) != 0) {
			fits = false;
			return;
		}
	}

	if (remainder > 0) {
		in_mask_surface_scalar_remainder(surf_ptr + neon_count * 4, mask_ptr + neon_count * 4, remainder, fits);
	}
}

#endif // __ARM_NEON

// ============================================================================
// PUBLIC DISPATCHERS
// ============================================================================

bool mask_surface_simd(uint32_t* surf_ptr, const uint32_t* mask_ptr, int total_pixels, bool& empty)
{
	// Require at least 4 pixels to justify SIMD overhead
	if (total_pixels < 4) {
		return false;
	}

#if defined(__SSE2__)
	mask_surface_sse2(surf_ptr, mask_ptr, total_pixels, empty);
	return true;
#elif defined(__ARM_NEON)
	mask_surface_neon(surf_ptr, mask_ptr, total_pixels, empty);
	return true;
#else
	return false;
#endif
}

bool in_mask_surface_simd(const uint32_t* surf_ptr, const uint32_t* mask_ptr, int total_pixels, bool& fits)
{
	if (total_pixels < 4) {
		return false;
	}

#if defined(__SSE2__)
	in_mask_surface_sse2(surf_ptr, mask_ptr, total_pixels, fits);
	return true;
#elif defined(__ARM_NEON)
	in_mask_surface_neon(surf_ptr, mask_ptr, total_pixels, fits);
	return true;
#else
	return false;
#endif
}
