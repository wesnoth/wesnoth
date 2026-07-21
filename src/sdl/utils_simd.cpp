/*
	Copyright (C) 2025
	by Durzi/mentos987
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

// This file contains optimisations for functions in the sdl/utils.cpp file.
// Optimisations use SIMD (SSE2/NEON) processing 4 pixels at a time.

#include "utils_simd.hpp"

// ============================================================================
// PLATFORM DETECTION
// ============================================================================

// x86/x64 SSE2
#if defined(__SSE2__) || \
	defined(_M_X64) || \
	(defined(_MSC_VER) && defined(_M_IX86_FP) && _M_IX86_FP >= 2)

#include <emmintrin.h>
#define SIMD_SSE2 1
#define SIMD_NEON 0

// ARM NEON
#elif defined(__ARM_NEON) || defined(__aarch64__)

#include <arm_neon.h>
#define SIMD_NEON 1
#define SIMD_SSE2 0

// No SIMD support
#else
#define SIMD_SSE2 0
#define SIMD_NEON 0
#endif

namespace {

// ============================================================================
// SSE2 IMPLEMENTATIONS
// ============================================================================

#if (SIMD_SSE2)

void mask_surface_simd_sse2(uint32_t* surf_ptr, const uint32_t* mask_ptr, const std::size_t pixel_count, bool& empty)
{
	// 1. Setup bit-mask for 128-bit registers (4 pixels each)
	// Each pixel is 0xAARRGGBB.
	// ALPHA_MASK isolates the top byte of each 32-bit lane.
	const __m128i ALPHA_MASK = _mm_set1_epi32(0xFF000000);

	// This accumulator tracks if we've seen any non-zero alpha.
	// If it remains all zeros, the entire surface is transparent.
	__m128i alpha_acc = _mm_setzero_si128();

	// 2. Main Vector Loop
	for(std::size_t offset = 0; offset < pixel_count; offset += 4) { // Process in blocks of 4
		// Load 4 pixels from surface and 4 pixels from mask (unaligned load)
		const __m128i s = _mm_loadu_si128(reinterpret_cast<const __m128i*>(surf_ptr + offset));
		const __m128i m = _mm_loadu_si128(reinterpret_cast<const __m128i*>(mask_ptr + offset));

		// Compute the minimum of every byte in the registers.
		// This effectively finds min(s.Alpha, m.Alpha), min(s.Red, m.Red), etc.
		const __m128i minv = _mm_min_epu8(s, m);

		// We only want the calculated Alpha. Mask out the RGB components of the 'min' result.
		const __m128i newa = _mm_and_si128(minv, ALPHA_MASK);

		// Construct final pixel: (Original Surface RGB) OR (New Masked Alpha)
		// We use _mm_andnot_si128(ALPHA_MASK, s) to flip the mask and isolate RGB
		const __m128i res = _mm_or_si128(_mm_andnot_si128(ALPHA_MASK, s), newa);

		// Store the 4 resulting pixels back to memory
		_mm_storeu_si128(reinterpret_cast<__m128i*>(surf_ptr + offset), res);

		// Keep track of the alpha values. OR-ing ensures that if any bit
		// in any alpha channel is 1, alpha_acc will eventually be non-zero.
		alpha_acc = _mm_or_si128(alpha_acc, newa);
	}

	// 3. Is the surface empty?
	// Compare alpha_acc to a zero register. If equal, it returns 0xFF per byte.
	// _mm_movemask_epi8 collects the most significant bit of each byte into a 16-bit int.
	if(_mm_movemask_epi8(_mm_cmpeq_epi8(alpha_acc, _mm_setzero_si128())) != 0xFFFF) {
		empty = false;
	}
}

void apply_surface_opacity_simd_sse2(uint32_t* surf_ptr, const std::size_t pixel_count, const uint8_t alpha_mod_scalar)
{
	const __m128i ALPHA_MASK = _mm_set1_epi32(0xFF000000);

	// Prepare constants for fixed-point math: (a * mod * 0x10100 + 0x800000) >> 24
	const __m128i MOD_VEC = _mm_set1_epi32(0x10100 * alpha_mod_scalar);
	const __m128i C128 = _mm_set1_epi32(0x800000);

	for(std::size_t offset = 0; offset < pixel_count; offset += 4) {
		const __m128i p = _mm_loadu_si128(reinterpret_cast<const __m128i*>(surf_ptr + offset));

		// Quick skip: if all 4 pixels have Alpha == 0, skip calculations.
		if (_mm_movemask_epi8(_mm_cmpeq_epi8(_mm_and_si128(p, ALPHA_MASK), _mm_setzero_si128())) == 0xFFFF) continue;

		// Shift right by 24 bits so Alpha is in the lowest 8 bits of each 32-bit lane.
		const __m128i a = _mm_srli_epi32(p, 24);

		// 32-bit Multiplication: m0 handles pixels 0 & 2, m1 handles pixels 1 & 3.
		const __m128i m0 = _mm_mul_epu32(a, MOD_VEC);
		const __m128i m1 = _mm_mul_epu32(_mm_srli_si128(a, 4), MOD_VEC);

		// Recombine results back into 32-bit lanes to perform rounding once.
		const __m128i merged = _mm_or_si128(m0, _mm_slli_si128(m1, 4));

		// Round and apply the new alpha values to the alpha channel.
		const __m128i new_alpha = _mm_and_si128(_mm_add_epi32(merged, C128), ALPHA_MASK);

		// Merge results back into the original surface pixels (preserving RGB channels).
		_mm_storeu_si128(reinterpret_cast<__m128i*>(surf_ptr + offset),
			_mm_or_si128(_mm_andnot_si128(ALPHA_MASK, p), new_alpha));
	}
}

void adjust_surface_color_simd_sse2(uint32_t* surf_ptr, const std::size_t pixel_count, int r, int g, int b)
{
	r = std::clamp(r, -255, 255);
	g = std::clamp(g, -255, 255);
	b = std::clamp(b, -255, 255);

	// Prepare Addition and Subtraction masks for unsigned saturating arithmetic.
	uint32_t add_mask = 0, sub_mask = 0;

	if(r > 0)  add_mask |= (static_cast<uint8_t>(r) << 16);
	else       sub_mask |= (static_cast<uint8_t>(std::abs(r)) << 16);
	if(g > 0)  add_mask |= (static_cast<uint8_t>(g) << 8);
	else       sub_mask |= (static_cast<uint8_t>(std::abs(g)) << 8);
	if(b > 0)  add_mask |= static_cast<uint8_t>(b);
	else       sub_mask |= static_cast<uint8_t>(std::abs(b));

	const __m128i ADD_VEC = _mm_set1_epi32(add_mask);
	const __m128i SUB_VEC = _mm_set1_epi32(sub_mask);

	for(std::size_t offset = 0; offset < pixel_count; offset += 4) {
		__m128i p = _mm_loadu_si128(reinterpret_cast<const __m128i*>(surf_ptr + offset));

		// Use _mm_subs_epu8 and _mm_adds_epu8 to automatically handle 0-255 clamping.
		p = _mm_subs_epu8(p, SUB_VEC);
		p = _mm_adds_epu8(p, ADD_VEC);

		_mm_storeu_si128(reinterpret_cast<__m128i*>(surf_ptr + offset), p);
	}
}

void flip_image_simd_sse2(uint32_t* pixels, const std::size_t width, const std::size_t height)
{
	const std::size_t half_width = width / 2;

	// Reorder lanes [0, 1, 2, 3] to [3, 2, 1, 0] to mirror the 4-pixel block.
	auto reverse_sse2 = [](__m128i v) -> __m128i {
		return _mm_shuffle_epi32(v, _MM_SHUFFLE(0, 1, 2, 3));
	};

	for(std::size_t y = 0; y < height; ++y) {
		uint32_t* row = pixels + y * width;
		std::size_t x = 0;

		for(; x + 4 <= half_width; x += 4) { // 4 pixels from the left and right. May leave 0-3 pixels unprocessed in the middle.
			const std::size_t r_idx = width - x - 4;

			__m128i l = _mm_loadu_si128(reinterpret_cast<const __m128i*>(row + x));
			__m128i r = _mm_loadu_si128(reinterpret_cast<const __m128i*>(row + r_idx));

			_mm_storeu_si128(reinterpret_cast<__m128i*>(row + x), reverse_sse2(r));
			_mm_storeu_si128(reinterpret_cast<__m128i*>(row + r_idx), reverse_sse2(l));
		}
	}
}

#endif // (SIMD_SSE2)

// ============================================================================
// NEON IMPLEMENTATIONS
// ============================================================================

#if (SIMD_NEON)

void mask_surface_simd_neon(uint32_t* __restrict surf_ptr, const uint32_t* __restrict mask_ptr, const std::size_t pixel_count, bool& empty)
{
	// NEON uses 128-bit registers (uint32x4_t) to process 4 pixels.
	const uint32x4_t ALPHA_MASK = vdupq_n_u32(0xFF000000);
	uint32x4_t alpha_acc = vdupq_n_u32(0);

	for(std::size_t offset = 0; offset < pixel_count; offset += 4) {
		// vld1q_u32 loads 4 contiguous 32-bit values.
		const uint32x4_t s = vld1q_u32(surf_ptr + offset);
		const uint32x4_t m = vld1q_u32(mask_ptr + offset);

		// Find minimum values across all channels.
		const uint32x4_t min_val = vminq_u32(s, m);

		// Accumulate alpha bits to check for non-empty surface later.
		alpha_acc = vorrq_u32(alpha_acc, min_val);

		// vbslq_u32 (Bitwise Select) acts as a ternary: ALPHA_MASK ? min_val : s
		// This keeps original RGB while applying the new masked Alpha.
		const uint32x4_t res = vbslq_u32(ALPHA_MASK, min_val, s);
		vst1q_u32(surf_ptr + offset, res);
	}

	// Isolate alpha bits and reduce the 128-bit vector to a single scalar check
	// by OR-ing the register halves and finding the maximum lane value.
	alpha_acc = vandq_u32(alpha_acc, ALPHA_MASK);
	uint32x2_t combined_halves = vorr_u32(vget_low_u32(alpha_acc), vget_high_u32(alpha_acc));
	combined_halves = vpmax_u32(combined_halves, combined_halves);
	if(vget_lane_u32(combined_halves, 0) > 0) {
		empty = false;
	}
}

void apply_surface_opacity_simd_neon(uint32_t* __restrict surf_ptr, const std::size_t pixel_count, const uint8_t alpha_mod_scalar)
{
	const uint32x4_t ALPHA_MASK = vdupq_n_u32(0xFF000000);
	const uint32x4_t MOD_VEC = vdupq_n_u32(0x10100 * alpha_mod_scalar);
	const uint32x4_t C128 = vdupq_n_u32(0x800000);

	for(std::size_t offset = 0; offset < pixel_count; offset += 4) {
		uint32x4_t p = vld1q_u32(surf_ptr + offset);

		uint32x4_t alpha_check = vandq_u32(p, ALPHA_MASK);
		if (vgetq_lane_u64(vreinterpretq_u64_u32(alpha_check), 0) == 0 &&
			vgetq_lane_u64(vreinterpretq_u64_u32(alpha_check), 1) == 0) continue;

		const uint32x4_t alpha_32 = vshrq_n_u32(p, 24);
		const uint32x4_t new_alpha_shifted = vaddq_u32(vmulq_u32(alpha_32, MOD_VEC), C128);
		const uint32x4_t res = vbslq_u32(ALPHA_MASK, new_alpha_shifted, p);

		vst1q_u32(surf_ptr + offset, res);
	}
}

void adjust_surface_color_simd_neon(uint32_t* __restrict surf_ptr, const std::size_t pixel_count, int r, int g, int b)
{
	r = std::clamp(r, -255, 255);
	g = std::clamp(g, -255, 255);
	b = std::clamp(b, -255, 255);

	uint32_t add_pkd = ((r > 0 ? r : 0) << 16) | ((g > 0 ? g : 0) << 8) | (b > 0 ? b : 0);
	uint32_t sub_pkd = ((r < 0 ? -r : 0) << 16) | ((g < 0 ? -g : 0) << 8) | (b < 0 ? -b : 0);

	const uint8x16_t ADD_VEC = vreinterpretq_u8_u32(vdupq_n_u32(add_pkd));
	const uint8x16_t SUB_VEC = vreinterpretq_u8_u32(vdupq_n_u32(sub_pkd));

	for(std::size_t offset = 0; offset < pixel_count; offset += 4) {
		// vqsubq/vqaddq handle saturating byte arithmetic (clamping to 0-255).
		uint8x16_t p = vld1q_u8(reinterpret_cast<uint8_t*>(surf_ptr + offset));
		p = vqaddq_u8(vqsubq_u8(p, SUB_VEC), ADD_VEC);
		vst1q_u8(reinterpret_cast<uint8_t*>(surf_ptr + offset), p);
	}
}

void flip_image_simd_neon(uint32_t* __restrict pixels, const std::size_t width, const std::size_t height)
{
	const std::size_t half_width = width / 2;

	// vrev64q_u32 reverses elements within 64-bit halves; vcombine/vget swaps the halves.
	auto reverse_vector_u32 = [](uint32x4_t v) -> uint32x4_t {
		v = vrev64q_u32(v);
		return vcombine_u32(vget_high_u32(v), vget_low_u32(v));
	};

	for(std::size_t y = 0; y < height; ++y) {
		uint32_t* row = pixels + y * width;
		std::size_t x = 0;

		for(; x + 4 <= half_width; x += 4) {
			const std::size_t r_idx = width - x - 4;
			const uint32x4_t L = vld1q_u32(row + x);
			const uint32x4_t R = vld1q_u32(row + r_idx);

			vst1q_u32(row + x, reverse_vector_u32(R));
			vst1q_u32(row + r_idx, reverse_vector_u32(L));
		}
	}
}

#endif // (SIMD_NEON)

} // End anonymous namespace

// ============================================================================
// PUBLIC DISPATCHERS
// ============================================================================

std::size_t mask_surface_simd(surface& surf, const surface& mask, bool& empty)
{
	surface_lock lock(surf);
	const_surface_lock mlock(mask);
	const std::size_t simd_count = (surf.area() / 4) * 4;
	if(simd_count <= simd::SIMD_THRESHOLD) return 0;

#if (SIMD_SSE2)
	mask_surface_simd_sse2(lock.pixels(), mlock.pixels(), simd_count, empty);
	return simd_count;
#elif (SIMD_NEON)
	mask_surface_simd_neon(lock.pixels(), mlock.pixels(), simd_count, empty);
	return simd_count;
#else
	return 0;
#endif
}

std::size_t apply_surface_opacity_simd(surface& surf, const uint8_t alpha_mod)
{
	surface_lock lock(surf);
	const std::size_t simd_count = (surf.area() / 4) * 4;
	if(simd_count <= simd::SIMD_THRESHOLD) return 0;

#if (SIMD_SSE2)
	apply_surface_opacity_simd_sse2(lock.pixels(), simd_count, alpha_mod);
	return simd_count;
#elif (SIMD_NEON)
	apply_surface_opacity_simd_neon(lock.pixels(), simd_count, alpha_mod);
	return simd_count;
#else
	return 0;
#endif
}

std::size_t adjust_surface_color_simd(surface& surf, const int r, const int g, const int b)
{
	surface_lock lock(surf);
	const std::size_t simd_count = (surf.area() / 4) * 4;
	if(simd_count <= simd::SIMD_THRESHOLD) return 0;

#if (SIMD_SSE2)
	adjust_surface_color_simd_sse2(lock.pixels(), simd_count, r, g, b);
	return simd_count;
#elif (SIMD_NEON)
	adjust_surface_color_simd_neon(lock.pixels(), simd_count, r, g, b);
	return simd_count;
#else
	return 0;
#endif
}

std::size_t flip_image_simd(surface& surf)
{
	surface_lock lock(surf);
	const std::size_t simd_cols = (surf->w / 2) / 4 * 4;
	if (surf.area() <= simd::SIMD_THRESHOLD) return 0;

#if (SIMD_SSE2)
	flip_image_simd_sse2(lock.pixels(), surf->w, surf->h);
	return simd_cols;
#elif (SIMD_NEON)
	flip_image_simd_neon(lock.pixels(), surf->w, surf->h);
	return simd_cols;
#else
	return 0;
#endif
}
