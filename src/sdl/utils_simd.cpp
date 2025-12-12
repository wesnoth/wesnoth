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
// This file is divided into sections for: platform detection, SIMD traits, SIMD implementations and dispatchers.

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
	// SIMD TRAITS (Abstraction Layer)
	// ============================================================================

#if defined(__SSE2__)
	struct SSE2Traits {
		using type = __m128i; // SSE2's 128-bit integer vector type
		static constexpr int width = 4; // Number of 32-bit elements (pixels) in the vector
		static constexpr int vectors_per_loop = 4; // Number of vectors processed in the main unrolled loop

		// -- Lifecycle --
		static inline type setzero() { return _mm_setzero_si128(); } // Sets all bits in the 128-bit vector to zero
		static inline type set1(uint32_t v) { return _mm_set1_epi32(v); } // Sets all four 32-bit lanes to the scalar value 'v'
		static inline type load(const uint32_t* p) { return _mm_loadu_si128(reinterpret_cast<const type*>(p)); } // Loads 16 bytes (4x32-bit) from unaligned memory
		static inline void store(uint32_t* p, type v) { _mm_storeu_si128(reinterpret_cast<type*>(p), v); } // Stores 16 bytes (4x32-bit) to unaligned memory

		// -- Bitwise Logic --
		static inline type bitwise_and(type a, type b) { return _mm_and_si128(a, b); } // Bitwise AND of 128-bit vectors
		static inline type bitwise_or(type a, type b) { return _mm_or_si128(a, b); } // Bitwise OR of 128-bit vectors
		static inline type bitwise_xor(type a, type b) { return _mm_xor_si128(a, b); } // Bitwise XOR of 128-bit vectors

		// -- Comparison & Checks --
		static inline type cmpeq_32(type a, type b) { return _mm_cmpeq_epi32(a, b); } // Compares packed 32-bit integers for equality, returns all 1s or all 0s for each lane

		static inline bool check_any_nonzero(type v) { // Returns true if any bit in the vector is set to 1
			return _mm_movemask_epi8(_mm_cmpeq_epi8(v, _mm_setzero_si128())) != 0xFFFF; // _mm_movemask_epi8 creates a 16-bit mask from the MSB of each byte. If v is all zeros, cmpeq(v, zero) is all ones, movemask is 0xFFFF.
		}

		// -- Arithmetic --
		static inline type min_u8(type a, type b) { return _mm_min_epu8(a, b); } // Minimum of packed unsigned 8-bit integers
		static inline type add_saturated_u8(type a, type b) { return _mm_adds_epu8(a, b); } // Saturated add of packed unsigned 8-bit integers
		static inline type sub_saturated_u8(type a, type b) { return _mm_subs_epu8(a, b); } // Saturated subtract of packed unsigned 8-bit integers
		static inline type add_16(type a, type b) { return _mm_add_epi16(a, b); } // Add packed 16-bit integers
		static inline type mullo_16(type a, type b) { return _mm_mullo_epi16(a, b); } // Multiply packed 16-bit integers (low 16 bits of result)

		// -- Shifts --
		static inline type srl_32(type a, int i) { return _mm_srli_epi32(a, i); } // Logical right shift packed 32-bit integers by immediate 'i'
		static inline type sll_32(type a, int i) { return _mm_slli_epi32(a, i); } // Logical left shift packed 32-bit integers by immediate 'i'

		// -- Specialized Composites --

		static inline type reverse_lanes_32(type v) { // Reverses the order of 32-bit lanes: [A, B, C, D] -> [D, C, B, A]
			return _mm_shuffle_epi32(v, _MM_SHUFFLE(0, 1, 2, 3)); // Shuffles 32-bit lanes based on a control mask
		}

		static inline type div_255_u16(type v) { // Calculates (v / 255) for 16-bit integers using the fast approximation: (x + 128 + (x >> 8)) >> 8
			const type c128 = _mm_set1_epi16(128);
			type tmp = _mm_add_epi16(v, c128);
			type tmp_div8 = _mm_srli_epi16(v, 8);
			return _mm_srli_epi16(_mm_add_epi16(tmp, tmp_div8), 8);
		}
	};
#endif

#if defined(__ARM_NEON)
	struct NeonTraits {
		using type = uint32x4_t; // NEON's 128-bit vector type containing four 32-bit unsigned integers
		static constexpr int width = 4; // Number of 32-bit elements (pixels) in the vector
		static constexpr int vectors_per_loop = 8; // Number of vectors processed in the main unrolled loop

		// Helper casts to reduce visual noise
		static inline uint32x4_t to_u32(uint8x16_t v) { return vreinterpretq_u32_u8(v); } // Reinterpret 16x8-bit vector as 4x32-bit vector
		static inline uint32x4_t to_u32(uint16x8_t v) { return vreinterpretq_u32_u16(v); } // Reinterpret 8x16-bit vector as 4x32-bit vector
		static inline uint8x16_t to_u8(uint32x4_t v) { return vreinterpretq_u8_u32(v); } // Reinterpret 4x32-bit vector as 16x8-bit vector
		static inline uint16x8_t to_u16(uint32x4_t v) { return vreinterpretq_u16_u32(v); } // Reinterpret 4x32-bit vector as 8x16-bit vector

		// -- Lifecycle --
		static inline type setzero() { return vdupq_n_u32(0); } // Sets all 32-bit lanes to 0
		static inline type set1(uint32_t v) { return vdupq_n_u32(v); } // Sets all four 32-bit lanes to the scalar value 'v'
		static inline type load(const uint32_t* p) { return vld1q_u32(p); } // Loads 16 bytes (4x32-bit) from memory (naturally aligned or unaligned)
		static inline void store(uint32_t* p, type v) { vst1q_u32(p, v); } // Stores 16 bytes (4x32-bit) to memory

		// -- Bitwise Logic --
		static inline type bitwise_and(type a, type b) { return vandq_u32(a, b); } // Bitwise AND of 128-bit vectors
		static inline type bitwise_or(type a, type b) { return vorrq_u32(a, b); } // Bitwise OR of 128-bit vectors
		static inline type bitwise_xor(type a, type b) { return veorq_u32(a, b); } // Bitwise XOR of 128-bit vectors

		// -- Comparison & Checks --
		static inline type cmpeq_32(type a, type b) { return vceqq_u32(a, b); } // Compares packed 32-bit integers for equality, returns all 1s or all 0s for each lane

		static inline bool check_any_nonzero(type v) { // Returns true if any bit in the vector is set to 1
			uint32x2_t tmp = vorr_u32(vget_low_u32(v), vget_high_u32(v)); // Fold 128-bit vector down to a single 32-bit value. Bitwise OR of the two 64-bit halves
			return vget_lane_u32(vpmax_u32(tmp, tmp), 0) != 0; // Get the max of the two elements and check if it's non-zero
		}

		// -- Arithmetic --
		static inline type min_u8(type a, type b) { return to_u32(vminq_u8(to_u8(a), to_u8(b))); } // Minimum of packed unsigned 8-bit integers
		static inline type add_saturated_u8(type a, type b) { return to_u32(vqaddq_u8(to_u8(a), to_u8(b))); } // Saturated add of packed unsigned 8-bit integers
		static inline type sub_saturated_u8(type a, type b) { return to_u32(vqsubq_u8(to_u8(a), to_u8(b))); } // Saturated subtract of packed unsigned 8-bit integers
		static inline type add_16(type a, type b) { return to_u32(vaddq_u16(to_u16(a), to_u16(b))); } // Add packed 16-bit integers
		static inline type mullo_16(type a, type b) { return to_u32(vmulq_u16(to_u16(a), to_u16(b))); } // Multiply packed 16-bit integers

		// -- Shifts --
		static inline type srl_32(type a, int i) { return vshlq_u32(a, vdupq_n_s32(-i)); } // Logical right shift packed 32-bit integers by 'i'
		static inline type sll_32(type a, int i) { return vshlq_u32(a, vdupq_n_s32(i)); } // Logical left shift packed 32-bit integers by 'i'

		// -- Specialized Composites --

		static inline type reverse_lanes_32(type v) {
			uint32x4_t rev32 = vrev64q_u32(v); // [0,1,2,3] -> [1,0,3,2] .Reverses the order of 32-bit elements in each 64-bit half
			return vextq_u32(rev32, rev32, 2); // [1,0,3,2] -> [3,2,1,0] (Swap 64-bit halves). Extract vector by shifting/wrapping the elements (moves lane 2 to lane 0, 3 to 1, etc.)
		}

		static inline type div_255_u16(type v) { // Calculates (v / 255) for 16-bit integers using the fast approximation: (x + 128 + (x >> 8)) >> 8
			const uint16x8_t c128 = vdupq_n_u16(128);
			uint16x8_t v16 = to_u16(v);
			uint16x8_t tmp = vaddq_u16(v16, c128); // Use immediate shift for constant 8
			uint16x8_t tmp_div8 = vshrq_n_u16(v16, 8); // Logical right shift 16-bit integers by 8
			uint16x8_t result = vshrq_n_u16(vaddq_u16(tmp, tmp_div8), 8); // Add and then logical right shift by 8

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
		constexpr int N = Traits::vectors_per_loop;

		const Vec rgb_mask = Traits::set1(0x00FFFFFF);
		const Vec alpha_mask = Traits::set1(0xFF000000);
		Vec has_alpha_acc = Traits::setzero();

		std::size_t offset = 0;
		const std::size_t vec_width = Traits::width;

		// 1. UNROLLED LOOP (N vectors per iteration: SSE2=4×4=16px, NEON=8×4=32px)
		const std::size_t block_size = vec_width * N;
		if (total_pixels >= block_size) {
			const std::size_t limit = total_pixels - block_size;

			for (; offset <= limit; offset += block_size) {
				Vec s[N];
				Vec m[N];
				Vec min_v[N];

				// Load
				for (int i = 0; i < N; ++i) {
					s[i] = Traits::load(surf_ptr + offset + (i * vec_width));
					m[i] = Traits::load(mask_ptr + offset + (i * vec_width));
				}

				// Process
				for (int i = 0; i < N; ++i) {
					// Calculate Min Alpha (masked to keep only alpha bits)
					min_v[i] = Traits::bitwise_and(Traits::min_u8(s[i], m[i]), alpha_mask);

					// Reconstruct Pixel: (SourceRGB) | (MinAlpha)
					Vec result = Traits::bitwise_or(Traits::bitwise_and(s[i], rgb_mask), min_v[i]);
					Traits::store(surf_ptr + offset + (i * vec_width), result);

					// Accumulate alpha check
					has_alpha_acc = Traits::bitwise_or(has_alpha_acc, min_v[i]);
				}
			}
		}

		// 2. VECTOR REMAINDER LOOP (Process remaining pixels in chunks of 4)
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
			std::size_t remaining_pixels = total_pixels - offset;
			uint32_t* current_surf_ptr = surf_ptr + offset;
			const uint32_t* current_mask_ptr = mask_ptr + offset;

			for (std::size_t i = 0; i < remaining_pixels; ++i) {
				const uint32_t surf_pixel = current_surf_ptr[i];
				const uint32_t mask_alpha = current_mask_ptr[i] >> 24;
				const uint32_t surf_alpha = surf_pixel >> 24;
				const uint32_t alpha = std::min(surf_alpha, mask_alpha);
				if (alpha > 0) empty = false;
				current_surf_ptr[i] = (alpha << 24) | (surf_pixel & 0x00FFFFFF);
			}
		}
	}

	template <typename Traits>
	void in_mask_surface_impl(const uint32_t* surf_ptr, const uint32_t* mask_ptr, std::size_t total_pixels, bool& fits)
	{
		using Vec = typename Traits::type;
		constexpr int N = Traits::vectors_per_loop;

		const Vec alpha_mask = Traits::set1(0xFF000000);
		const Vec zero = Traits::setzero();
		Vec bad_pixels_acc = Traits::setzero();

		std::size_t offset = 0;
		const std::size_t vec_width = Traits::width;

		// 1. UNROLLED LOOP (N vectors per iteration: SSE2=4×4=16px, NEON=8×4=32px)
		const std::size_t block_size = vec_width * N;
		if (total_pixels >= block_size) {
			const std::size_t limit = total_pixels - block_size;

			for (; offset <= limit; offset += block_size) {
				Vec s[N];
				Vec m[N];

				// Load
				for (int i = 0; i < N; ++i) {
					s[i] = Traits::load(surf_ptr + offset + (i * vec_width));
					m[i] = Traits::load(mask_ptr + offset + (i * vec_width));
				}

				// Process, look for "bad" pixels (visible pixels outside the allowed mask area)
				for (int i = 0; i < N; ++i) {
					// Logic: Bad if (MaskAlpha == 0) AND (SurfAlpha != 0)
					Vec mask_zero = Traits::cmpeq_32(Traits::bitwise_and(m[i], alpha_mask), zero);
					Vec surf_alpha = Traits::bitwise_and(s[i], alpha_mask);
					Vec bad = Traits::bitwise_and(mask_zero, surf_alpha);

					// Accumulate bad pixels
					bad_pixels_acc = Traits::bitwise_or(bad_pixels_acc, bad);
				}

				// Early exit check
				if (Traits::check_any_nonzero(bad_pixels_acc)) {
					fits = false;
					return;
				}
			}
		}

		// 2. VECTOR REMAINDER LOOP (Process remaining pixels in chunks of 4)
		for (; offset <= total_pixels - vec_width; offset += vec_width) {
			Vec s = Traits::load(surf_ptr + offset);
			Vec m = Traits::load(mask_ptr + offset);
			Vec bad = Traits::bitwise_and(Traits::cmpeq_32(Traits::bitwise_and(m, alpha_mask), zero), Traits::bitwise_and(s, alpha_mask));
			if (Traits::check_any_nonzero(bad)) {
				fits = false;
				return;
			}
		}

		// 3. SCALAR REMAINDER (Process final 1-3 pixels)
		if (offset < total_pixels) {
			std::size_t remaining_pixels = total_pixels - offset;
			const uint32_t* current_surf_ptr = surf_ptr + offset;
			const uint32_t* current_mask_ptr = mask_ptr + offset;

			for (std::size_t i = 0; i < remaining_pixels; ++i) {
				const uint32_t mask_alpha = current_mask_ptr[i] >> 24;
				if (mask_alpha == 0) {
					const uint32_t surf_alpha = current_surf_ptr[i] >> 24;
					if (surf_alpha > 0) {
						fits = false;
						return;
					}
				}
			}
		}
	}

	template <typename Traits>
	void apply_surface_opacity_impl(uint32_t* surf_ptr, std::size_t total_pixels, uint8_t alpha_mod_scalar)
	{
		using Vec = typename Traits::type;
		constexpr int N = Traits::vectors_per_loop;

		const Vec rgb_mask = Traits::set1(0x00FFFFFF);
		const Vec mod_vec = Traits::set1(alpha_mod_scalar);

		// Lambda to process a single vector of pixels
		auto process_vec = [&](Vec pixel) {
			Vec alpha = Traits::srl_32(pixel, 24); // 1. Isolate Alpha (Shift to LSB)
			Vec prod = Traits::mullo_16(alpha, mod_vec); // 2. Multiply by Modifier
			Vec new_alpha = Traits::div_255_u16(prod); // 3. Divide by 255 (Specialized Composite)
			new_alpha = Traits::sll_32(new_alpha, 24); // 4. Shift back and Combine
			return Traits::bitwise_or(Traits::bitwise_and(pixel, rgb_mask), new_alpha);
			};

		// Standard loop boilerplate
		std::size_t offset = 0;
		const std::size_t vec_width = Traits::width;
		const std::size_t block_size = vec_width * N;

		// 1. UNROLLED LOOP (N vectors per iteration: SSE2=4×4=16px, NEON=8×4=32px)
		if (total_pixels >= block_size) {
			const std::size_t limit = total_pixels - block_size;
			for (; offset <= limit; offset += block_size) {
				Vec p[N];

				//Load
				for (int i = 0; i < N; ++i) {
					p[i] = Traits::load(surf_ptr + offset + (i * vec_width));
				}

				// Lambda process & Store
				for (int i = 0; i < N; ++i) {
					Traits::store(surf_ptr + offset + (i * vec_width), process_vec(p[i]));
				}
			}
		}

		// 2. VECTOR REMAINDER LOOP (Process remaining chunks of 4 pixels)
		for (; offset <= total_pixels - vec_width; offset += vec_width) {
			Vec p = Traits::load(surf_ptr + offset);
			Traits::store(surf_ptr + offset, process_vec(p));
		}

		// 3. SCALAR REMAINDER (Process final 1-3 pixels)
		if (offset < total_pixels) {
			std::size_t remaining_pixels = total_pixels - offset;
			uint32_t* current_surf_ptr = surf_ptr + offset;

			for (std::size_t i = 0; i < remaining_pixels; ++i) {
				uint32_t pixel = current_surf_ptr[i];
				uint8_t a = pixel >> 24;
				if (a != 0) {
					uint32_t prod = a * alpha_mod_scalar;
					uint32_t new_a = (prod + 128 + (prod >> 8)) >> 8;
					current_surf_ptr[i] = (new_a << 24) | (pixel & 0x00FFFFFF);
				}
			}
		}
	}

	template <typename Traits>
	void adjust_surface_color_impl(uint32_t* surf_ptr, std::size_t total_pixels, int r, int g, int b)
	{
		using Vec = typename Traits::type;
		constexpr int N = Traits::vectors_per_loop;

		// 1. Prepare Add/Sub Vectors based on the sign of the color delta.
		// This splits the signed color adjustments (r, g, b) into two masks:
		// The masks are constructed as 0xAARRGGBB, aligned for byte-wise SIMD operations.
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

		// 1. UNROLLED LOOP (N vectors per iteration: SSE2=4×4=16px, NEON=8×4=32px)
		const std::size_t block_size = vec_width * N;
		if (total_pixels >= block_size) {
			const std::size_t limit = total_pixels - block_size;
			for (; offset <= limit; offset += block_size) {
				Vec p[N];

				// Load
				for (int i = 0; i < N; ++i) {
					p[i] = Traits::load(surf_ptr + offset + (i * vec_width));
				}

				// Process: subtract negative adjustments, then add positive adjustments (both saturating)
				for (int i = 0; i < N; ++i) {
					p[i] = Traits::add_saturated_u8(Traits::sub_saturated_u8(p[i], sub_vec), add_vec);
					Traits::store(surf_ptr + offset + (i * vec_width), p[i]);
				}
			}
		}

		// 2. VECTOR REMAINDER LOOP (Process remaining chunks of 4 pixels)
		for (; offset <= total_pixels - vec_width; offset += vec_width) {
			Vec p = Traits::load(surf_ptr + offset);
			p = Traits::add_saturated_u8(Traits::sub_saturated_u8(p, sub_vec), add_vec);
			Traits::store(surf_ptr + offset, p);
		}

		// 3. SCALAR REMAINDER (Process final 1-3 pixels)
		if (offset < total_pixels) {
			std::size_t remaining_pixels = total_pixels - offset;
			uint32_t* current_surf_ptr = surf_ptr + offset;

			for (std::size_t i = 0; i < remaining_pixels; ++i) {
				uint32_t pixel = current_surf_ptr[i];
				uint8_t a = pixel >> 24;
				uint8_t pr = (pixel >> 16) & 0xFF;
				uint8_t pg = (pixel >> 8) & 0xFF;
				uint8_t pb = pixel & 0xFF;
				pr = std::clamp(static_cast<int>(pr) + r, 0, 255);
				pg = std::clamp(static_cast<int>(pg) + g, 0, 255);
				pb = std::clamp(static_cast<int>(pb) + b, 0, 255);
				current_surf_ptr[i] = (a << 24) | (pr << 16) | (pg << 8) | pb;
			}
		}
	}

	template<typename Traits>
	void flip_row_impl(uint32_t* row_ptr, std::size_t width_total_pixels)
	{
		using Vec = typename Traits::type;
		constexpr std::size_t VEC_PPC = Traits::width;
		constexpr int PAIRS_PER_LOOP = Traits::vectors_per_loop / 2;
		const std::size_t pairs_to_process = (width_total_pixels / VEC_PPC) / 2;

		uint32_t* left_ptr = row_ptr;
		uint32_t* right_ptr = row_ptr + width_total_pixels - VEC_PPC;
		std::size_t i = 0;

		// 1. UNROLLED LOOP (PAIRS_PER_LOOP pairs of vectors per iteration: SSE2=2*2×4=16px, NEON=2*4×4=32px)
		if (pairs_to_process >= PAIRS_PER_LOOP) {
			const std::size_t limit = pairs_to_process - PAIRS_PER_LOOP;

			for (; i <= limit; i += PAIRS_PER_LOOP) {
				Vec l[PAIRS_PER_LOOP];
				Vec r[PAIRS_PER_LOOP];

				// LOAD
				for (int p = 0; p < PAIRS_PER_LOOP; ++p) {
					l[p] = Traits::load(left_ptr + (p * VEC_PPC));
					r[p] = Traits::load(right_ptr - (p * VEC_PPC));
				}

				// Process & Store Loop
				for (int p = 0; p < PAIRS_PER_LOOP; ++p) {

					// REVERSE
					Vec l_rev = Traits::reverse_lanes_32(l[p]);
					Vec r_rev = Traits::reverse_lanes_32(r[p]);

					// STORE (Swap sides)
					Traits::store(left_ptr + (p * VEC_PPC), r_rev);
					Traits::store(right_ptr - (p * VEC_PPC), l_rev);
				}

				// ADVANCE POINTERS
				left_ptr += (VEC_PPC * PAIRS_PER_LOOP);
				right_ptr -= (VEC_PPC * PAIRS_PER_LOOP);
			}
		}

		// 2. VECTOR REMAINDER LOOP (Process remaining single pairs)
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

		// 3. SCALAR REMAINDER (Handle middle pixels that don't form complete vector pairs)
		std::size_t pixels_processed = pairs_to_process * VEC_PPC * 2;
		std::size_t remaining_pixels = width_total_pixels - pixels_processed;

		if (remaining_pixels > 1) {
			uint32_t* current_row_ptr = left_ptr;

			// Swap the middle section
			for (std::size_t x = 0; x < remaining_pixels / 2; ++x) {
				std::swap(current_row_ptr[x], current_row_ptr[remaining_pixels - x - 1]);
			}
		}

	}

} // namespace

// ============================================================================
// PUBLIC DISPATCHERS
// ============================================================================

bool mask_surface_simd(uint32_t* surf_ptr, const uint32_t* mask_ptr, std::size_t total_pixels, bool& empty)
{
	if (!SIMD_IMPLEMENTED) return false;
	mask_surface_impl<SimdImplTraits>(surf_ptr, mask_ptr, total_pixels, empty);
	return true;
}

bool in_mask_surface_simd(const uint32_t* surf_ptr, const uint32_t* mask_ptr, std::size_t total_pixels, bool& fits)
{
	if (!SIMD_IMPLEMENTED) return false;
	in_mask_surface_impl<SimdImplTraits>(surf_ptr, mask_ptr, total_pixels, fits);
	return true;
}

bool apply_surface_opacity_simd(uint32_t* surf_ptr, std::size_t total_pixels, uint8_t alpha_mod)
{
	if (!SIMD_IMPLEMENTED) return false;
	apply_surface_opacity_impl<SimdImplTraits>(surf_ptr, total_pixels, alpha_mod);
	return true;
}

bool adjust_surface_color_simd(uint32_t* surf_ptr, std::size_t width_total_pixels, int r, int g, int b)
{
	if (!SIMD_IMPLEMENTED) return false;
	adjust_surface_color_impl<SimdImplTraits>(surf_ptr, width_total_pixels, r, g, b);
	return true;
}

bool flip_row_simd(uint32_t* row_ptr, std::size_t width_total_pixels)
{
	if (!SIMD_IMPLEMENTED) return false;
	flip_row_impl<SimdImplTraits>(row_ptr, width_total_pixels);
	return true;
}
