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

// This file contains optimisations for functions in the utils.cpp file.
// These optimisations are mostly done by parallelizing operations using SIMD and unrolling loops.
// This file is divided into sections for: platform detection, scalar helpers, SIMD traits, SIMD implementations and dispatchers.

#include "utils_simd.hpp"

// ============================================================================
// PLATFORM DETECTION,INTRINSICS & SIMD TRAITS SELECTION
// ============================================================================

// Ensure __SSE2__ is defined. Microsoft Visual C++ does not define this macro by default,
// 64-bit x86 builds (GCC/Clang/MSVC) mandate SSE2 support. Or MSVC 32-bit builds: _M_IX86_FP >= 2 guarantees SSE2 support.
#if !defined(__SSE2__) && (defined(__x86_64__) || defined(_M_X64) || (defined(_MSC_VER) && defined(_M_IX86_FP) && _M_IX86_FP >= 2))
#define __SSE2__
#endif

#if defined(__SSE2__)
#include <emmintrin.h>
#define SIMD_IMPLEMENTED true

#elif defined(__ARM_NEON)
#include <arm_neon.h>
#define SIMD_IMPLEMENTED true

#else
#define SIMD_IMPLEMENTED false
#endif

namespace {

	// ============================================================================
	// SCALAR HELPERS
	// ============================================================================

	void mask_surface_scalar_remainder(uint32_t* surf_ptr, const uint32_t* mask_ptr, std::size_t remaining_pixels, bool& empty)
	{
		for (std::size_t i = 0; i < remaining_pixels; ++i) {
			const uint32_t surf_pixel = surf_ptr[i];
			const uint32_t mask_alpha = mask_ptr[i] >> 24;
			const uint32_t surf_alpha = surf_pixel >> 24;
			const uint32_t alpha = std::min(surf_alpha, mask_alpha);
			if (alpha > 0) empty = false;
			surf_ptr[i] = (alpha << 24) | (surf_pixel & 0x00FFFFFF);
		}
	}

	void in_mask_surface_scalar_remainder(const uint32_t* surf_ptr, const uint32_t* mask_ptr, std::size_t remaining_pixels, bool& fits)
	{
		for (std::size_t i = 0; i < remaining_pixels; ++i) {
			const uint32_t mask_alpha = mask_ptr[i] >> 24;
			if (mask_alpha == 0) {
				const uint32_t surf_alpha = surf_ptr[i] >> 24;
				if (surf_alpha > 0) {
					fits = false;
					return;
				}
			}
		}
	}

	void apply_surface_opacity_scalar_remainder(uint32_t* surf_ptr, std::size_t remaining_pixels, uint8_t alpha_mod_scalar)
	{
		for (std::size_t i = 0; i < remaining_pixels; ++i) {
			uint32_t pixel = surf_ptr[i];
			uint8_t a = pixel >> 24;
			if (a) {
				uint32_t prod = a * alpha_mod_scalar;
				uint32_t new_a = (prod + 128 + (prod >> 8)) >> 8;
				surf_ptr[i] = (new_a << 24) | (pixel & 0x00FFFFFF);
			}
		}
	}

	void adjust_surface_color_scalar_remainder(uint32_t* surf_ptr, std::size_t remaining_pixels, int red, int green, int blue)
	{
		for (std::size_t i = 0; i < remaining_pixels; ++i) {
			uint32_t pixel = surf_ptr[i];
			uint8_t a = pixel >> 24;
			uint8_t r = (pixel >> 16) & 0xFF;
			uint8_t g = (pixel >> 8) & 0xFF;
			uint8_t b = pixel & 0xFF;
			r = std::clamp(static_cast<int>(r) + red, 0, 255);
			g = std::clamp(static_cast<int>(g) + green, 0, 255);
			b = std::clamp(static_cast<int>(b) + blue, 0, 255);
			surf_ptr[i] = (a << 24) | (r << 16) | (g << 8) | b;
		}
	}

	void flip_row_scalar_remainder(uint32_t* row_ptr, std::size_t remaining_pixels)
	{
		if (remaining_pixels <= 1) {
			return;
		}
		for (std::size_t x = 0; x < remaining_pixels / 2; ++x) {
			const std::size_t index1 = x;
			const std::size_t index2 = remaining_pixels - x - 1;
			std::swap(row_ptr[index1], row_ptr[index2]);
		}
	}

	// ============================================================================
	// SIMD TRAITS (Abstraction Layer)
	// ============================================================================

#if defined(__SSE2__)
	struct SSE2Traits {
		using type = __m128i;
		static constexpr int width = 4;

		// -- Lifecycle --
		static inline type setzero() { return _mm_setzero_si128(); }
		static inline type set1(uint32_t v) { return _mm_set1_epi32(v); }
		static inline type load(const uint32_t* p) { return _mm_loadu_si128(reinterpret_cast<const type*>(p)); }
		static inline void store(uint32_t* p, type v) { _mm_storeu_si128(reinterpret_cast<type*>(p), v); }

		// -- Bitwise Logic --
		static inline type bitwise_and(type a, type b) { return _mm_and_si128(a, b); }
		static inline type bitwise_or(type a, type b) { return _mm_or_si128(a, b); }
		static inline type bitwise_xor(type a, type b) { return _mm_xor_si128(a, b); }

		// -- Comparison & Checks --
		// Returns 0xFFFFFFFF where equal, 0 otherwise
		static inline type cmpeq_32(type a, type b) { return _mm_cmpeq_epi32(a, b); }

		// Returns true if any bit in the vector is set to 1
		static inline bool check_any_nonzero(type v) {
			// _mm_movemask_epi8 creates a 16-bit mask from the MSB of each byte.
			// If v is all zeros, cmpeq(v, zero) is all ones, movemask is 0xFFFF.
			return _mm_movemask_epi8(_mm_cmpeq_epi8(v, _mm_setzero_si128())) != 0xFFFF;
		}

		// -- Arithmetic --
		static inline type min_u8(type a, type b) { return _mm_min_epu8(a, b); }
		static inline type add_saturated_u8(type a, type b) { return _mm_adds_epu8(a, b); }
		static inline type sub_saturated_u8(type a, type b) { return _mm_subs_epu8(a, b); }
		static inline type add_16(type a, type b) { return _mm_add_epi16(a, b); }
		static inline type mullo_16(type a, type b) { return _mm_mullo_epi16(a, b); }

		// -- Shifts --
		static inline type srl_32(type a, int i) { return _mm_srli_epi32(a, i); }
		static inline type sll_32(type a, int i) { return _mm_slli_epi32(a, i); }

		// -- Specialized Composites --

		// Reverses the order of 32-bit lanes: [A, B, C, D] -> [D, C, B, A]
		static inline type reverse_lanes_32(type v) {
			return _mm_shuffle_epi32(v, _MM_SHUFFLE(0, 1, 2, 3));
		}

		// Calculates (v / 255) for 16-bit integers using the fast approximation:
		// (x + 128 + (x >> 8)) >> 8
		static inline type div_255_u16(type v) {
			const type c128 = _mm_set1_epi16(128);
			type tmp = _mm_add_epi16(v, c128);
			type tmp_div8 = _mm_srli_epi16(v, 8);
			return _mm_srli_epi16(_mm_add_epi16(tmp, tmp_div8), 8);
		}
	};
#endif

#if defined(__ARM_NEON)
	struct NeonTraits {
		using type = uint32x4_t;
		static constexpr int width = 4;

		// Helper casts to reduce visual noise
		static inline uint32x4_t to_u32(uint8x16_t v) { return vreinterpretq_u32_u8(v); }
		static inline uint32x4_t to_u32(uint16x8_t v) { return vreinterpretq_u32_u16(v); }
		static inline uint8x16_t to_u8(uint32x4_t v) { return vreinterpretq_u8_u32(v); }
		static inline uint16x8_t to_u16(uint32x4_t v) { return vreinterpretq_u16_u32(v); }

		// -- Lifecycle --
		static inline type setzero() { return vdupq_n_u32(0); }
		static inline type set1(uint32_t v) { return vdupq_n_u32(v); }
		static inline type load(const uint32_t* p) { return vld1q_u32(p); }
		static inline void store(uint32_t* p, type v) { vst1q_u32(p, v); }

		// -- Bitwise Logic --
		static inline type bitwise_and(type a, type b) { return vandq_u32(a, b); }
		static inline type bitwise_or(type a, type b) { return vorrq_u32(a, b); }
		static inline type bitwise_xor(type a, type b) { return veorq_u32(a, b); }

		// -- Comparison & Checks --
		static inline type cmpeq_32(type a, type b) { return vceqq_u32(a, b); }

		static inline bool check_any_nonzero(type v) {
			// Fold 128-bit vector down to a single 32-bit value
			uint32x2_t tmp = vorr_u32(vget_low_u32(v), vget_high_u32(v));
			return vget_lane_u32(vpmax_u32(tmp, tmp), 0) != 0;
		}

		// -- Arithmetic --
		static inline type min_u8(type a, type b) {return to_u32(vminq_u8(to_u8(a), to_u8(b)));}
		static inline type add_saturated_u8(type a, type b) {return to_u32(vqaddq_u8(to_u8(a), to_u8(b)));}
		static inline type sub_saturated_u8(type a, type b) {return to_u32(vqsubq_u8(to_u8(a), to_u8(b)));}
		static inline type add_16(type a, type b) {return to_u32(vaddq_u16(to_u16(a), to_u16(b)));}
		static inline type mullo_16(type a, type b) {return to_u32(vmulq_u16(to_u16(a), to_u16(b)));}

		// -- Shifts --
		static inline type srl_32(type a, int i) {return vshlq_u32(a, vdupq_n_s32(-i));}
		static inline type sll_32(type a, int i) {return vshlq_u32(a, vdupq_n_s32(i));}

		// -- Specialized Composites --

		static inline type reverse_lanes_32(type v) {
			// [0,1,2,3] -> [1,0,3,2]
			uint32x4_t rev32 = vrev64q_u32(v);
			// [1,0,3,2] -> [3,2,1,0] (Swap 64-bit halves)
			return vextq_u32(rev32, rev32, 2);
		}

		static inline type div_255_u16(type v) {
			// Formula: (x + 128 + (x >> 8)) >> 8
			const uint16x8_t c128 = vdupq_n_u16(128);
			uint16x8_t v16 = to_u16(v);

			uint16x8_t tmp = vaddq_u16(v16, c128);
			// Use immediate shift for constant 8
			uint16x8_t tmp_div8 = vshrq_n_u16(v16, 8);
			uint16x8_t result = vshrq_n_u16(vaddq_u16(tmp, tmp_div8), 8);

			return to_u32(result);
		}
	};
#endif

	// ============================================================================
	// SIMD TRAITS SELECTION
	// ============================================================================

#if defined(__SSE2__)
	using SimdImplTraits = SSE2Traits;
#elif defined(__ARM_NEON)
	using SimdImplTraits = NeonTraits;
#else
	struct NoSimdTraits {};
	using SimdImplTraits = NoSimdTraits;
#endif

	// ============================================================================
	// ALGORITHMS (Templated Logic)
	// ============================================================================

	template <typename Traits>
	void mask_surface_impl(uint32_t* surf_ptr, const uint32_t* mask_ptr, std::size_t total_pixels, bool& empty)
	{
		using Vec = typename Traits::type;

		const Vec rgb_mask = Traits::set1(0x00FFFFFF);
		const Vec alpha_mask = Traits::set1(0xFF000000);
		Vec has_alpha_acc = Traits::setzero();

		std::size_t offset = 0;
		const std::size_t vec_width = Traits::width; // 4 pixels

		// 1. UNROLLED LOOP (4 Vectors / 16 Pixels per iteration)
		const std::size_t block_size = vec_width * 4;
		if (total_pixels >= block_size) {
			const std::size_t limit = total_pixels - block_size;

			for (; offset <= limit; offset += block_size) {
				// Load
				Vec s0 = Traits::load(surf_ptr + offset);
				Vec m0 = Traits::load(mask_ptr + offset);
				Vec s1 = Traits::load(surf_ptr + offset + 4);
				Vec m1 = Traits::load(mask_ptr + offset + 4);
				Vec s2 = Traits::load(surf_ptr + offset + 8);
				Vec m2 = Traits::load(mask_ptr + offset + 8);
				Vec s3 = Traits::load(surf_ptr + offset + 12);
				Vec m3 = Traits::load(mask_ptr + offset + 12);

				// Calculate Min Alpha (masked to keep only alpha bits)
				Vec min0 = Traits::bitwise_and(Traits::min_u8(s0, m0), alpha_mask);
				Vec min1 = Traits::bitwise_and(Traits::min_u8(s1, m1), alpha_mask);
				Vec min2 = Traits::bitwise_and(Traits::min_u8(s2, m2), alpha_mask);
				Vec min3 = Traits::bitwise_and(Traits::min_u8(s3, m3), alpha_mask);

				// Reconstruct Pixel: (SourceRGB) | (MinAlpha)
				Traits::store(surf_ptr + offset, Traits::bitwise_or(Traits::bitwise_and(s0, rgb_mask), min0));
				Traits::store(surf_ptr + offset + 4, Traits::bitwise_or(Traits::bitwise_and(s1, rgb_mask), min1));
				Traits::store(surf_ptr + offset + 8, Traits::bitwise_or(Traits::bitwise_and(s2, rgb_mask), min2));
				Traits::store(surf_ptr + offset + 12, Traits::bitwise_or(Traits::bitwise_and(s3, rgb_mask), min3));

				// Accumulate alpha check
				has_alpha_acc = Traits::bitwise_or(has_alpha_acc,
					Traits::bitwise_or(Traits::bitwise_or(min0, min1), Traits::bitwise_or(min2, min3)));
			}
		}

		// 2. VECTOR REMAINDER LOOP (Process remaining chunks of 4 pixels)
		for (; offset <= total_pixels - vec_width; offset += vec_width) {
			Vec s = Traits::load(surf_ptr + offset);
			Vec m = Traits::load(mask_ptr + offset);
			Vec min_alpha = Traits::bitwise_and(Traits::min_u8(s, m), alpha_mask);
			Traits::store(surf_ptr + offset, Traits::bitwise_or(Traits::bitwise_and(s, rgb_mask), min_alpha));
			has_alpha_acc = Traits::bitwise_or(has_alpha_acc, min_alpha);
		}

		// Check accumulator
		if (Traits::check_any_nonzero(has_alpha_acc)) {
			empty = false;
		}

		// 3. SCALAR REMAINDER (Process final 1-3 pixels)
		if (offset < total_pixels) {
			mask_surface_scalar_remainder(surf_ptr + offset, mask_ptr + offset, total_pixels - offset, empty);
		}
	}

	template <typename Traits>
	void in_mask_surface_impl(const uint32_t* surf_ptr, const uint32_t* mask_ptr, std::size_t total_pixels, bool& fits)
	{
		using Vec = typename Traits::type;

		const Vec alpha_mask = Traits::set1(0xFF000000);
		const Vec zero = Traits::setzero();
		Vec bad_pixels_acc = Traits::setzero();

		std::size_t offset = 0;
		const std::size_t vec_width = Traits::width;

		// 1. UNROLLED LOOP (4 Vectors / 16 Pixels per iteration)
		const std::size_t block_size = vec_width * 4;
		if (total_pixels >= block_size) {
			const std::size_t limit = total_pixels - block_size;

			for (; offset <= limit; offset += block_size) {
				Vec s0 = Traits::load(surf_ptr + offset);
				Vec m0 = Traits::load(mask_ptr + offset);
				Vec s1 = Traits::load(surf_ptr + offset + 4);
				Vec m1 = Traits::load(mask_ptr + offset + 4);
				Vec s2 = Traits::load(surf_ptr + offset + 8);
				Vec m2 = Traits::load(mask_ptr + offset + 8);
				Vec s3 = Traits::load(surf_ptr + offset + 12);
				Vec m3 = Traits::load(mask_ptr + offset + 12);

				// Logic: Bad if (MaskAlpha == 0) AND (SurfAlpha != 0)
				Vec bad0 = Traits::bitwise_and(Traits::cmpeq_32(Traits::bitwise_and(m0, alpha_mask), zero), Traits::bitwise_and(s0, alpha_mask));
				Vec bad1 = Traits::bitwise_and(Traits::cmpeq_32(Traits::bitwise_and(m1, alpha_mask), zero), Traits::bitwise_and(s1, alpha_mask));
				Vec bad2 = Traits::bitwise_and(Traits::cmpeq_32(Traits::bitwise_and(m2, alpha_mask), zero), Traits::bitwise_and(s2, alpha_mask));
				Vec bad3 = Traits::bitwise_and(Traits::cmpeq_32(Traits::bitwise_and(m3, alpha_mask), zero), Traits::bitwise_and(s3, alpha_mask));

				// Accumulate bad pixels
				bad_pixels_acc = Traits::bitwise_or(bad_pixels_acc,
					Traits::bitwise_or(Traits::bitwise_or(bad0, bad1), Traits::bitwise_or(bad2, bad3)));
			}
		}

		// Early exit check after main bulk
		if (Traits::check_any_nonzero(bad_pixels_acc)) {
			fits = false;
			return;
		}

		// 2. VECTOR REMAINDER LOOP
		for (; offset <= total_pixels - vec_width; offset += vec_width) {
			Vec s = Traits::load(surf_ptr + offset);
			Vec m = Traits::load(mask_ptr + offset);
			Vec bad = Traits::bitwise_and(Traits::cmpeq_32(Traits::bitwise_and(m, alpha_mask), zero), Traits::bitwise_and(s, alpha_mask));
			if (Traits::check_any_nonzero(bad)) {
				fits = false;
				return;
			}
		}

		// 3. SCALAR REMAINDER
		if (offset < total_pixels) {
			in_mask_surface_scalar_remainder(surf_ptr + offset, mask_ptr + offset, total_pixels - offset, fits);
		}
	}

	template <typename Traits>
	void apply_surface_opacity_impl(uint32_t* surf_ptr, std::size_t total_pixels, uint8_t alpha_mod_scalar)
	{
		using Vec = typename Traits::type;
		const Vec rgb_mask = Traits::set1(0x00FFFFFF);
		const Vec mod_vec = Traits::set1(alpha_mod_scalar);

		auto process_vec = [&](Vec pixel) {
			Vec alpha = Traits::srl_32(pixel, 24); // 1. Isolate Alpha (Shift to LSB)
			Vec prod = Traits::mullo_16(alpha, mod_vec); // 2. Multiply by Modifier
			Vec new_alpha = Traits::div_255_u16(prod); // 3. Divide by 255 (Specialized Composite)
			new_alpha = Traits::sll_32(new_alpha, 24); // 4. Shift back and Combine
			return Traits::bitwise_or(Traits::bitwise_and(pixel, rgb_mask), new_alpha); // Use bitwise_or and bitwise_and
			};

		// Standard loop boilerplate
		std::size_t offset = 0;
		const std::size_t vec_width = Traits::width;
		const std::size_t block_size = vec_width * 4;

		// 1. UNROLLED LOOP (4 Vectors / 16 Pixels per iteration)
		if (total_pixels >= block_size) {
			const std::size_t limit = total_pixels - block_size;
			for (; offset <= limit; offset += block_size) {
				Vec p0 = Traits::load(surf_ptr + offset);
				Vec p1 = Traits::load(surf_ptr + offset + 4);
				Vec p2 = Traits::load(surf_ptr + offset + 8);
				Vec p3 = Traits::load(surf_ptr + offset + 12);

				Traits::store(surf_ptr + offset, process_vec(p0));
				Traits::store(surf_ptr + offset + 4, process_vec(p1));
				Traits::store(surf_ptr + offset + 8, process_vec(p2));
				Traits::store(surf_ptr + offset + 12, process_vec(p3));
			}
		}

		// 2. VECTOR REMAINDER LOOP (Process remaining chunks of 4 pixels)
		for (; offset <= total_pixels - vec_width; offset += vec_width) {
			Vec p = Traits::load(surf_ptr + offset);
			Traits::store(surf_ptr + offset, process_vec(p));
		}

		// 3. SCALAR REMAINDER (Process final 1-3 pixels)
		if (offset < total_pixels) {
			apply_surface_opacity_scalar_remainder(surf_ptr + offset, total_pixels - offset, alpha_mod_scalar);
		}
	}

	template <typename Traits>
	void adjust_surface_color_impl(uint32_t* surf_ptr, std::size_t total_pixels, int r, int g, int b)
	{
		using Vec = typename Traits::type;

		// 1. Prepare Add/Sub Vectors based on sign
		// We use 32-bit integers to construct the mask so it aligns with pixel format 0xAARRGGBB
		// Alpha delta is always 0.
		uint32_t add_mask_val = 0;
		uint32_t sub_mask_val = 0;

		if (r > 0) add_mask_val |= (r << 16);
		else       sub_mask_val |= (static_cast<uint32_t>(std::abs(r)) << 16);
		if (g > 0) add_mask_val |= (g << 8);
		else       sub_mask_val |= (static_cast<uint32_t>(std::abs(g)) << 8);
		if (b > 0) add_mask_val |= static_cast<uint32_t>(b);
		else       sub_mask_val |= static_cast<uint32_t>(std::abs(b));

		const Vec add_vec = Traits::set1(add_mask_val);
		const Vec sub_vec = Traits::set1(sub_mask_val);

		std::size_t offset = 0;
		const std::size_t vec_width = Traits::width;

		// 1. UNROLLED LOOP
		const std::size_t block_size = vec_width * 4;
		if (total_pixels >= block_size) {
			const std::size_t limit = total_pixels - block_size;
			for (; offset <= limit; offset += block_size) {
				Vec p0 = Traits::load(surf_ptr + offset);
				Vec p1 = Traits::load(surf_ptr + offset + 4);
				Vec p2 = Traits::load(surf_ptr + offset + 8);
				Vec p3 = Traits::load(surf_ptr + offset + 12);

				// Adjust colors
				p0 = Traits::add_saturated_u8(Traits::sub_saturated_u8(p0, sub_vec), add_vec);
				p1 = Traits::add_saturated_u8(Traits::sub_saturated_u8(p1, sub_vec), add_vec);
				p2 = Traits::add_saturated_u8(Traits::sub_saturated_u8(p2, sub_vec), add_vec);
				p3 = Traits::add_saturated_u8(Traits::sub_saturated_u8(p3, sub_vec), add_vec);

				Traits::store(surf_ptr + offset, p0);
				Traits::store(surf_ptr + offset + 4, p1);
				Traits::store(surf_ptr + offset + 8, p2);
				Traits::store(surf_ptr + offset + 12, p3);
			}
		}

		// 2. VECTOR REMAINDER LOOP
		for (; offset <= total_pixels - vec_width; offset += vec_width) {
			Vec p = Traits::load(surf_ptr + offset);
			p = Traits::add_saturated_u8(Traits::sub_saturated_u8(p, sub_vec), add_vec);
			Traits::store(surf_ptr + offset, p);
		}

		// 3. SCALAR REMAINDER
		if (offset < total_pixels) {
			adjust_surface_color_scalar_remainder(surf_ptr + offset, total_pixels - offset, r, g, b);
		}
	}

	template<typename Traits>
	void flip_row_impl(uint32_t* row_ptr, std::size_t width_total_pixels)
	{
		using Vec = typename Traits::type;
		constexpr std::size_t VEC_PPC = Traits::width;

		// 1. Calculate how many PAIRS of vectors we can swap.
		const std::size_t pairs_to_process = (width_total_pixels / VEC_PPC) / 2;

		uint32_t* left_ptr = row_ptr;
		uint32_t* right_ptr = row_ptr + width_total_pixels - VEC_PPC;
		std::size_t i = 0;

		// 2. UNROLLED LOOP (Process 2 Pairs / 4 Vectors / 16 Pixels per iteration)
		if (pairs_to_process >= 2) {
			const std::size_t limit = pairs_to_process - 2;

			for (; i <= limit; i += 2) {
				// LOAD
				Vec l1 = Traits::load(left_ptr);
				Vec r1 = Traits::load(right_ptr);
				Vec l2 = Traits::load(left_ptr + VEC_PPC);
				Vec r2 = Traits::load(right_ptr - VEC_PPC);

				// REVERSE
				Vec l1_rev = Traits::reverse_lanes_32(l1);
				Vec r1_rev = Traits::reverse_lanes_32(r1);
				Vec l2_rev = Traits::reverse_lanes_32(l2);
				Vec r2_rev = Traits::reverse_lanes_32(r2);

				// STORE (Swap sides)
				Traits::store(left_ptr, r1_rev);
				Traits::store(right_ptr, l1_rev);
				Traits::store(left_ptr + VEC_PPC, r2_rev);
				Traits::store(right_ptr - VEC_PPC, l2_rev);

				// ADVANCE POINTERS
				left_ptr += (VEC_PPC * 2);
				right_ptr -= (VEC_PPC * 2);
			}
		}

		// 3. VECTOR REMAINDER LOOP (Process remaining single pairs)
		for (; i < pairs_to_process; ++i) {
			Vec v_left = Traits::load(left_ptr);
			Vec v_right = Traits::load(right_ptr);
			Vec v_left_rev = Traits::reverse_lanes_32(v_left);
			Vec v_right_rev = Traits::reverse_lanes_32(v_right);
			Traits::store(left_ptr, v_right_rev);
			Traits::store(right_ptr, v_left_rev);
			left_ptr += VEC_PPC;
			right_ptr -= VEC_PPC;
		}

		// 4. SCALAR REMAINDER (Middle pixels)
		std::size_t pixels_processed = pairs_to_process * VEC_PPC * 2;
		std::size_t remaining_pixels = width_total_pixels - pixels_processed;

		if (remaining_pixels > 0) {
			flip_row_scalar_remainder(left_ptr, remaining_pixels);
		}
	}

} // namespace

// ============================================================================
// PUBLIC DISPATCHERS
// ============================================================================

bool mask_surface_simd(uint32_t* surf_ptr, const uint32_t* mask_ptr, std::size_t total_pixels, bool& empty)
{
	// Minimum 4 vectors (16 pixels) check is good practice to amortize the setup cost
	if (total_pixels < 16 || !SIMD_IMPLEMENTED) return false;
	mask_surface_impl<SimdImplTraits>(surf_ptr, mask_ptr, total_pixels, empty);
	return true;
}

bool in_mask_surface_simd(const uint32_t* surf_ptr, const uint32_t* mask_ptr, std::size_t total_pixels, bool& fits)
{
	if (total_pixels < 16 || !SIMD_IMPLEMENTED) return false;
	in_mask_surface_impl<SimdImplTraits>(surf_ptr, mask_ptr, total_pixels, fits);
	return true;
}

bool apply_surface_opacity_simd(uint32_t* surf_ptr, std::size_t total_pixels, uint8_t alpha_mod)
{
	if (total_pixels < 16 || !SIMD_IMPLEMENTED) return false;
	apply_surface_opacity_impl<SimdImplTraits>(surf_ptr, total_pixels, alpha_mod);
	return true;
}

bool adjust_surface_color_simd(uint32_t* surf_ptr, std::size_t width_total_pixels, int r, int g, int b)
{
	if (width_total_pixels < 16 || !SIMD_IMPLEMENTED) return false;
	adjust_surface_color_impl<SimdImplTraits>(surf_ptr, width_total_pixels, r, g, b);
	return true;
}

bool flip_row_simd(uint32_t* row_ptr, std::size_t width_total_pixels)
{
	if (width_total_pixels < 16 || !SIMD_IMPLEMENTED) return false;
	flip_row_impl<SimdImplTraits>(row_ptr, width_total_pixels);
	return true;
}
