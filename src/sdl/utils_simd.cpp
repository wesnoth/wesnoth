/*
	Copyright (C) 2003 - 2025
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

// This file contains optimisations for functions in the utils.cpp file.
// These optimisations are mostly done by parallelizing operations using SIMD and unrolling loops.
// This file is divided into sections for: platform detection, AVX2 implementations, NEON implementations, and dispatcher functions.

#include "utils_simd.hpp"

// ============================================================================
// PLATFORM DETECTION
// ============================================================================

// x86/x64 SSE2
#if defined(__SSE2__) || \
    defined(_M_X64) || \
    (defined(_MSC_VER) && defined(_M_IX86_FP) && _M_IX86_FP >= 2) // MSVC 86 does not define __SSE2__ by default, even if supported

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

// ============================================================================
// SSE2 IMPLEMENTATIONS
// ============================================================================

#if (SIMD_SSE2)

	void mask_surface_simd_sse2(uint32_t* surf_ptr, const uint32_t* mask_ptr, std::size_t total_pixels, bool& empty)
	{
		const __m128i ALPHA_MASK = _mm_set1_epi32(0xFF000000);
		__m128i alpha_acc = _mm_setzero_si128();
		std::size_t offset = 0;
		const std::size_t BLOCK = 16;
		const std::size_t limit = total_pixels & ~(BLOCK - 1);

		// ------------------------------------------------------------
		// Main loop (unrolled) Process 16 pixels per iteration (4 vectors)
		// ------------------------------------------------------------
		for (; offset < limit; offset += BLOCK) {
			//Loads and arithmetics are woven to hide memory latency and keep execution ports saturated.

			// Stage 1: Load V0/V1 Data
			__m128i s0 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(surf_ptr + offset + 0));
			__m128i m0 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(mask_ptr + offset + 0));
			__m128i s1 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(surf_ptr + offset + 4));
			__m128i m1 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(mask_ptr + offset + 4));

			// Stage 2: Calculate New Alpha (min(Surface.Alpha, Mask.Alpha)) for V0/V1
			__m128i minv0 = _mm_min_epu8(s0, m0);
			__m128i minv1 = _mm_min_epu8(s1, m1);

			// Stage 3: Load V2/V3 Data
			__m128i s2 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(surf_ptr + offset + 8));
			__m128i m2 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(mask_ptr + offset + 8));
			__m128i s3 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(surf_ptr + offset + 12));
			__m128i m3 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(mask_ptr + offset + 12));

			// Stage 4: Process V0, Initiate V2, Process V1, Initiate V3
			__m128i newa0 = _mm_and_si128(minv0, ALPHA_MASK);       // V0: Isolate new alpha channel
			__m128i res0 = _mm_or_si128(_mm_and_si128(s0, _mm_set1_epi32(0x00FFFFFF)), newa0); // V0: Result = S0.RGB + newa0.A
			_mm_storeu_si128(reinterpret_cast<__m128i*>(surf_ptr + offset + 0), res0);
			__m128i minv2 = _mm_min_epu8(s2, m2); // V2: Calculate new alpha
			__m128i newa1 = _mm_and_si128(minv1, ALPHA_MASK);       // V1: Isolate new alpha channel
			__m128i res1 = _mm_or_si128(_mm_and_si128(s1, _mm_set1_epi32(0x00FFFFFF)), newa1); // V1: Result = S1.RGB + newa1.A
			_mm_storeu_si128(reinterpret_cast<__m128i*>(surf_ptr + offset + 4), res1);
			__m128i minv3 = _mm_min_epu8(s3, m3); // V3: Calculate new alpha

			// Stage 5: Finalize V2/V3 (Combine new alpha with original RGB, then store)
			__m128i newa2 = _mm_and_si128(minv2, ALPHA_MASK);       // V2: Isolate new alpha channel
			__m128i res2 = _mm_or_si128(_mm_and_si128(s2, _mm_set1_epi32(0x00FFFFFF)), newa2); // V2: Result = S2.RGB + newa2.A
			_mm_storeu_si128(reinterpret_cast<__m128i*>(surf_ptr + offset + 8), res2);
			__m128i newa3 = _mm_and_si128(minv3, ALPHA_MASK);       // V3: Isolate new alpha channel
			__m128i res3 = _mm_or_si128(_mm_and_si128(s3, _mm_set1_epi32(0x00FFFFFF)), newa3); // V3: Result = S3.RGB + newa3.A
			_mm_storeu_si128(reinterpret_cast<__m128i*>(surf_ptr + offset + 12), res3);

			// Stage 6: Alpha Reduction (Accumulate all 16 new alpha channels for 'empty' check)
			__m128i local_alpha = _mm_or_si128(_mm_or_si128(newa0, newa1),
				_mm_or_si128(newa2, newa3));
			alpha_acc = _mm_or_si128(alpha_acc, local_alpha);
		}

		// ------------------------------------------------------------
		// Vector remainder (4 pixels at a time)
		// ------------------------------------------------------------
		for (; offset + 4 <= total_pixels; offset += 4) {
			__m128i s = _mm_loadu_si128(reinterpret_cast<const __m128i*>(surf_ptr + offset));
			__m128i m = _mm_loadu_si128(reinterpret_cast<const __m128i*>(mask_ptr + offset));
			__m128i minv = _mm_min_epu8(s, m);
			__m128i newa = _mm_and_si128(minv, _mm_set1_epi32(0xFF000000));
			__m128i res = _mm_or_si128(_mm_and_si128(s, _mm_set1_epi32(0x00FFFFFF)), newa);
			_mm_storeu_si128(reinterpret_cast<__m128i*>(surf_ptr + offset), res);
			alpha_acc = _mm_or_si128(alpha_acc, newa);
		}

		// Alpha accumulator check
		if (_mm_movemask_epi8(_mm_cmpeq_epi8(alpha_acc, _mm_setzero_si128())) != 0xFFFF)
			empty = false;

		// ------------------------------------------------------------
		// Scalar remainder (Handles last 1-3 pixels)
		// ------------------------------------------------------------
		for (; offset < total_pixels; ++offset) {
			uint32_t s = surf_ptr[offset];
			uint32_t m = mask_ptr[offset];
			uint32_t sa = s >> 24;
			uint32_t ma = m >> 24;
			uint32_t a = (sa < ma) ? sa : ma;
			if (a > 0) empty = false;
			surf_ptr[offset] = (a << 24) | (s & 0x00FFFFFF);
		}
	}

	void apply_surface_opacity_simd_sse2(uint32_t* surf_ptr, std::size_t total_pixels, uint8_t alpha_mod_scalar)
	{
		const __m128i ALPHA_MASK = _mm_set1_epi32(0xFF000000);
		const __m128i MOD_VEC = _mm_set1_epi16(alpha_mod_scalar);
		const __m128i C128 = _mm_set1_epi16(128);
		std::size_t offset = 0;
		const std::size_t BLOCK = 16;
		const std::size_t limit = total_pixels - BLOCK;

		// ------------------------------------------------------------
		// Main loop (unrolled) Process 16 pixels per iteration (4 vectors)
		// ------------------------------------------------------------
		for (; offset <= limit; offset += BLOCK) {
			//Loads and arithmetics are woven to hide memory latency and keep execution ports saturated.

			// 1. Initial Load: Fetch first 8 pixels (V0, V1)
			__m128i p0 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(surf_ptr + offset + 0));
			__m128i p1 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(surf_ptr + offset + 4));

			// 2. Begin Alpha Isolation: Shift V0/V1 while fetching remaining 8 pixels (V2, V3)
			__m128i a0 = _mm_srli_epi32(p0, 24);
			__m128i p2 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(surf_ptr + offset + 8));
			__m128i a1 = _mm_srli_epi32(p1, 24);
			__m128i p3 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(surf_ptr + offset + 12));

			// 3. Transparency Check: Skip the entire 16-pixel block if all alpha channels are zero
			__m128i combined = _mm_or_si128(_mm_or_si128(p0, p1), _mm_or_si128(p2, p3));
			if (_mm_movemask_epi8(_mm_cmpeq_epi8(_mm_and_si128(combined, ALPHA_MASK), _mm_setzero_si128())) == 0xFFFF) continue;

			// 4. Apply Modifier: Multiply isolated Alpha (V0/V1) and isolate remaining Alpha (V2/V3)
			__m128i m0 = _mm_mullo_epi16(a0, MOD_VEC);
			__m128i a2 = _mm_srli_epi32(p2, 24);
			__m128i m1 = _mm_mullo_epi16(a1, MOD_VEC);
			__m128i a3 = _mm_srli_epi32(p3, 24);

			// 5. Normalization: Begin Div-255 for V0/V1 and multiply Alpha for V2/V3
			__m128i m2 = _mm_mullo_epi16(a2, MOD_VEC);
			__m128i t0 = _mm_add_epi16(m0, C128);
			__m128i m3 = _mm_mullo_epi16(a3, MOD_VEC);
			__m128i t1 = _mm_add_epi16(m1, C128);

			// 6. Complete Normalization: Finalize V0/V1 Alpha results and begin Div-255 for V2/V3
			__m128i n0 = _mm_srli_epi16(_mm_add_epi16(t0, _mm_srli_epi16(m0, 8)), 8);
			__m128i t2 = _mm_add_epi16(m2, C128);
			__m128i n1 = _mm_srli_epi16(_mm_add_epi16(t1, _mm_srli_epi16(m1, 8)), 8);
			__m128i t3 = _mm_add_epi16(m3, C128);

			// 7. Re-insertion: Merge new Alpha with original RGB for V0/V1 and finalize V2/V3 Alpha
			__m128i n2 = _mm_srli_epi16(_mm_add_epi16(t2, _mm_srli_epi16(m2, 8)), 8);
			__m128i new_alpha0 = _mm_slli_epi32(n0, 24);
			_mm_storeu_si128(reinterpret_cast<__m128i*>(surf_ptr + offset + 0),
				_mm_or_si128(_mm_and_si128(p0, _mm_set1_epi32(0x00FFFFFF)), new_alpha0));
			__m128i n3 = _mm_srli_epi16(_mm_add_epi16(t3, _mm_srli_epi16(m3, 8)), 8);
			__m128i new_alpha1 = _mm_slli_epi32(n1, 24);
			_mm_storeu_si128(reinterpret_cast<__m128i*>(surf_ptr + offset + 4),
				_mm_or_si128(_mm_and_si128(p1, _mm_set1_epi32(0x00FFFFFF)), new_alpha1));

			// 8. Final Store: Merge and write-back results for V2/V3
			__m128i new_alpha2 = _mm_slli_epi32(n2, 24);
			_mm_storeu_si128(reinterpret_cast<__m128i*>(surf_ptr + offset + 8),
				_mm_or_si128(_mm_and_si128(p2, _mm_set1_epi32(0x00FFFFFF)), new_alpha2));
			__m128i new_alpha3 = _mm_slli_epi32(n3, 24);
			_mm_storeu_si128(reinterpret_cast<__m128i*>(surf_ptr + offset + 12),
				_mm_or_si128(_mm_and_si128(p3, _mm_set1_epi32(0x00FFFFFF)), new_alpha3));
		}

		// ------------------------------------------------------------
		// Single Vector Remainder (4 pixels at a time)
		// ------------------------------------------------------------
		for (; offset + 4 <= total_pixels; offset += 4) {
			__m128i p = _mm_loadu_si128(reinterpret_cast<const __m128i*>(surf_ptr + offset));
			if (_mm_movemask_epi8(_mm_cmpeq_epi8(_mm_and_si128(p, ALPHA_MASK), _mm_setzero_si128())) == 0xFFFF) continue;
			__m128i a = _mm_srli_epi32(p, 24);
			__m128i m = _mm_mullo_epi16(a, MOD_VEC);
			__m128i newa = _mm_srli_epi16(_mm_add_epi16(_mm_add_epi16(m, C128), _mm_srli_epi16(m, 8)), 8);
			__m128i new_alpha = _mm_slli_epi32(newa, 24);
			_mm_storeu_si128(reinterpret_cast<__m128i*>(surf_ptr + offset),
				_mm_or_si128(_mm_and_si128(p, _mm_set1_epi32(0x00FFFFFF)), new_alpha));
		}

		// ------------------------------------------------------------
		// Scalar Remainder (Handles last 1-3 pixels)
		// ------------------------------------------------------------
		for (; offset < total_pixels; ++offset) {
			uint32_t pixel = surf_ptr[offset];
			uint32_t a = (pixel >> 24);
			if (a != 0) {
				uint32_t prod = a * alpha_mod_scalar;
				uint32_t new_a = (prod + 128 + (prod >> 8)) >> 8;
				surf_ptr[offset] = (new_a << 24) | (pixel & 0x00FFFFFF);
			}
		}
	}

	void adjust_surface_color_simd_sse2(uint32_t* surf_ptr, std::size_t total_pixels, int r, int g, int b)
	{
		// Prepare Add/Sub Vectors. Construct masks as 0x00RRGGBB. Alpha (high byte) is 0 to remain untouched.
		uint32_t add_mask = 0, sub_mask = 0;

		if (r > 0) add_mask |= (static_cast<uint8_t>(r) << 16);
		else       sub_mask |= (static_cast<uint8_t>(std::abs(r)) << 16);

		if (g > 0) add_mask |= (static_cast<uint8_t>(g) << 8);
		else       sub_mask |= (static_cast<uint8_t>(std::abs(g)) << 8);

		if (b > 0) add_mask |= static_cast<uint8_t>(b);
		else       sub_mask |= static_cast<uint8_t>(std::abs(b));

		const __m128i ADD_VEC = _mm_set1_epi32(add_mask);
		const __m128i SUB_VEC = _mm_set1_epi32(sub_mask);

		std::size_t offset = 0;
		const std::size_t BLOCK = 16;
		const std::size_t limit = total_pixels - BLOCK;

		// -------------------------------------------------------------------------
		// Main loop: 16 pixels per iteration (4 vectors).
		// -------------------------------------------------------------------------
		for (; offset <= limit; offset += BLOCK) {
			//Loads and arithmetic are woven to hide memory latency and keep execution ports saturated.

			// 1. Initial Load: Fetch first 8 pixels
			__m128i p0 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(surf_ptr + offset + 0));
			__m128i p1 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(surf_ptr + offset + 4));

			// 2. Begin Subtraction: Process V0/V1 while fetching V2/V3 from memory
			__m128i res0 = _mm_subs_epu8(p0, SUB_VEC);
			__m128i p2 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(surf_ptr + offset + 8));
			__m128i res1 = _mm_subs_epu8(p1, SUB_VEC);
			__m128i p3 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(surf_ptr + offset + 12));

			// 3. Apply Addition: Perform saturating add for V0/V1 and subtract for V2/V3
			res0 = _mm_adds_epu8(res0, ADD_VEC);
			__m128i res2 = _mm_subs_epu8(p2, SUB_VEC);
			res1 = _mm_adds_epu8(res1, ADD_VEC);
			__m128i res3 = _mm_subs_epu8(p3, SUB_VEC);

			// 4. Final Addition & Store: Complete V2/V3 and write all results back
			res2 = _mm_adds_epu8(res2, ADD_VEC);
			_mm_storeu_si128(reinterpret_cast<__m128i*>(surf_ptr + offset + 0), res0);

			res3 = _mm_adds_epu8(res3, ADD_VEC);
			_mm_storeu_si128(reinterpret_cast<__m128i*>(surf_ptr + offset + 4), res1);

			_mm_storeu_si128(reinterpret_cast<__m128i*>(surf_ptr + offset + 8), res2);
			_mm_storeu_si128(reinterpret_cast<__m128i*>(surf_ptr + offset + 12), res3);
		}

		// -------------------------------------------------------------------------
		// Single Vector Remainder (4 pixels at a time)
		// -------------------------------------------------------------------------
		for (; offset + 4 <= total_pixels; offset += 4) {
			__m128i p = _mm_loadu_si128(reinterpret_cast<const __m128i*>(surf_ptr + offset));
			p = _mm_subs_epu8(p, SUB_VEC);
			p = _mm_adds_epu8(p, ADD_VEC);
			_mm_storeu_si128(reinterpret_cast<__m128i*>(surf_ptr + offset), p);
		}

		// -------------------------------------------------------------------------
		// Scalar Remainder (1-3 pixels)
		// -------------------------------------------------------------------------
		for (; offset < total_pixels; ++offset) {
			uint32_t pixel = surf_ptr[offset];
			int pr = std::clamp(static_cast<int>((pixel >> 16) & 0xFF) + r, 0, 255);
			int pg = std::clamp(static_cast<int>((pixel >> 8) & 0xFF) + g, 0, 255);
			int pb = std::clamp(static_cast<int>(pixel & 0xFF) + b, 0, 255);
			surf_ptr[offset] = (pixel & 0xFF000000) | (pr << 16) | (pg << 8) | pb;
		}
	}

	void flip_image_simd_sse2(uint32_t* pixels, std::size_t width, std::size_t height)
	{
		// Prepare constants and 4-row pointers for unrolled vertical processing.
		const std::size_t half_width = width / 2;                           // Only swap up to the middle pixel
		std::size_t y = 0;                                                  // Row counter
		uint32_t* r0 = pixels;
		uint32_t* r1 = r0 + width;
		uint32_t* r2 = r1 + width;
		uint32_t* r3 = r2 + width;
		const std::size_t stride_x4 = width * 4;

		// Reverses the order of 4 uint32 elements in a 128-bit register.
		auto reverse_sse2 = [](__m128i v) -> __m128i {
			return _mm_shuffle_epi32(v, _MM_SHUFFLE(0, 1, 2, 3));           // Reverse 32-bit elements
		};

		// --- SECTION: 4-ROW STRIPED LOOP ---
		// Processes blocks of 4 rows at once to maximize cache throughput.
		for (; y + 4 <= height; y += 4) {
			std::size_t x = 0;

			// Vectorized horizontal swap: 4 pixels at a time from both ends.
			for (; x + 4 <= half_width; x += 4) {
				//Loads and arithmetics are woven to hide memory latency and keep execution ports saturated.

				std::size_t r_idx = width - x - 4;

				// Load Row 0 and Row 1
				__m128i l0 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(r0 + x));
				__m128i R0 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(r0 + r_idx));
				__m128i l1 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(r1 + x));
				__m128i R1 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(r1 + r_idx));

				// Flip elements so they face the correct way
				l0 = reverse_sse2(l0);
				R0 = reverse_sse2(R0);
				l1 = reverse_sse2(l1);
				R1 = reverse_sse2(R1);

				// Store swapped results back to memory
				_mm_storeu_si128(reinterpret_cast<__m128i*>(r0 + x), R0);
				_mm_storeu_si128(reinterpret_cast<__m128i*>(r0 + r_idx), l0);
				_mm_storeu_si128(reinterpret_cast<__m128i*>(r1 + x), R1);
				_mm_storeu_si128(reinterpret_cast<__m128i*>(r1 + r_idx), l1);

				// Repeat for Row 2 and Row 3
				__m128i l2 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(r2 + x));
				__m128i R2 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(r2 + r_idx));
				__m128i l3 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(r3 + x));
				__m128i R3 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(r3 + r_idx));
				l2 = reverse_sse2(l2);
				R2 = reverse_sse2(R2);
				l3 = reverse_sse2(l3);
				R3 = reverse_sse2(R3);
				_mm_storeu_si128(reinterpret_cast<__m128i*>(r2 + x), R2);
				_mm_storeu_si128(reinterpret_cast<__m128i*>(r2 + r_idx), l2);
				_mm_storeu_si128(reinterpret_cast<__m128i*>(r3 + x), R3);
				_mm_storeu_si128(reinterpret_cast<__m128i*>(r3 + r_idx), l3);
			}

			// Scalar Cleanup: Handle remaining pixels in the middle of these 4 rows.
			for (; x < half_width; ++x) {
				std::size_t r_idx = width - 1 - x;
				std::swap(r0[x], r0[r_idx]);
				std::swap(r1[x], r1[r_idx]);
				std::swap(r2[x], r2[r_idx]);
				std::swap(r3[x], r3[r_idx]);
			}

			// Advance pointers by 4 full rows to start the next stripe.
			r0 += stride_x4;
			r1 += stride_x4;
			r2 += stride_x4;
			r3 += stride_x4;
		}

		// --- SECTION: HEIGHT REMAINDER ---
		// Handle remaining 1-3 rows if height is not a multiple of 4.
		for (; y < height; ++y) {
			std::size_t x = 0;
			// Vector loop for a single row.
			for (; x + 4 <= half_width; x += 4) {
				std::size_t r_idx = width - x - 4;
				__m128i l = _mm_loadu_si128(reinterpret_cast<__m128i*>(r0 + x));
				__m128i r = _mm_loadu_si128(reinterpret_cast<__m128i*>(r0 + r_idx));

				l = reverse_sse2(l);
				r = reverse_sse2(r);

				_mm_storeu_si128(reinterpret_cast<__m128i*>(r0 + x), r);
				_mm_storeu_si128(reinterpret_cast<__m128i*>(r0 + r_idx), l);
			}
			// Scalar loop for the very last few pixels in the middle of the row.
			for (; x < half_width; ++x) {
				std::swap(r0[x], r0[width - 1 - x]);
			}
			r0 += width;
		}
	}

#endif // (SIMD_SSE2)

// ============================================================================
// NEON IMPLEMENTATIONS
// ============================================================================

#if (SIMD_NEON)

	// Helper: Horizontal OR across uint32x4_t (ARMv7 compatible)
	static inline uint32_t horizontal_or_u32(uint32x4_t v) {
		uint32x2_t tmp = vorr_u32(vget_low_u32(v), vget_high_u32(v));
		tmp = vpmax_u32(tmp, tmp);
		return vget_lane_u32(tmp, 0);
	}

	// Helper: Horizontal max across uint8x16_t (ARMv7 compatible)
	static inline uint8_t horizontal_max_u8(uint8x16_t v) {
		uint8x8_t max1 = vpmax_u8(vget_low_u8(v), vget_high_u8(v));
		max1 = vpmax_u8(max1, max1);
		max1 = vpmax_u8(max1, max1);
		max1 = vpmax_u8(max1, max1);
		return vget_lane_u8(max1, 0);
	}

	void mask_surface_simd_neon(uint32_t* __restrict surf_ptr, const uint32_t* __restrict mask_ptr, std::size_t total_pixels, bool& empty)
	{
		const uint32x4_t ALPHA_MASK = vdupq_n_u32(0xFF000000);
		uint32x4_t alpha_acc = vdupq_n_u32(0);

		std::size_t n_blocks = total_pixels / 16;
		std::size_t remainder = total_pixels % 16;

		// ------------------------------------------------------------
		// Main loop: 16 pixels per iteration (4 vectors)
		// ------------------------------------------------------------
		while (n_blocks--) {
			// Load Surface and Mask
			uint32x4_t s0 = vld1q_u32(surf_ptr);
			uint32x4_t m0 = vld1q_u32(mask_ptr);
			uint32x4_t s1 = vld1q_u32(surf_ptr + 4);
			uint32x4_t m1 = vld1q_u32(mask_ptr + 4);
			uint32x4_t s2 = vld1q_u32(surf_ptr + 8);
			uint32x4_t m2 = vld1q_u32(mask_ptr + 8);
			uint32x4_t s3 = vld1q_u32(surf_ptr + 12);
			uint32x4_t m3 = vld1q_u32(mask_ptr + 12);

			// Calculate byte-wise minimums
			uint32x4_t min0 = vreinterpretq_u32_u8(vminq_u8(vreinterpretq_u8_u32(s0), vreinterpretq_u8_u32(m0)));
			uint32x4_t min1 = vreinterpretq_u32_u8(vminq_u8(vreinterpretq_u8_u32(s1), vreinterpretq_u8_u32(m1)));
			uint32x4_t min2 = vreinterpretq_u32_u8(vminq_u8(vreinterpretq_u8_u32(s2), vreinterpretq_u8_u32(m2)));
			uint32x4_t min3 = vreinterpretq_u32_u8(vminq_u8(vreinterpretq_u8_u32(s3), vreinterpretq_u8_u32(m3)));

			// Accumulate alpha bits (dirty RGB included, will mask later)
			uint32x4_t acc_tmp0 = vorrq_u32(min0, min1);
			uint32x4_t acc_tmp1 = vorrq_u32(min2, min3);
			alpha_acc = vorrq_u32(alpha_acc, vorrq_u32(acc_tmp0, acc_tmp1));

			// Select: Keep original RGB, replace Alpha with minimum
			vst1q_u32(surf_ptr, vbslq_u32(ALPHA_MASK, min0, s0));
			vst1q_u32(surf_ptr + 4, vbslq_u32(ALPHA_MASK, min1, s1));
			vst1q_u32(surf_ptr + 8, vbslq_u32(ALPHA_MASK, min2, s2));
			vst1q_u32(surf_ptr + 12, vbslq_u32(ALPHA_MASK, min3, s3));

			surf_ptr += 16;
			mask_ptr += 16;
		}

		// ------------------------------------------------------------
		// Vector remainder: Handle blocks of 4 pixels
		// ------------------------------------------------------------
		while (remainder >= 4) {
			uint32x4_t s = vld1q_u32(surf_ptr);
			uint32x4_t m = vld1q_u32(mask_ptr);
			uint32x4_t min_val = vreinterpretq_u32_u8(vminq_u8(vreinterpretq_u8_u32(s), vreinterpretq_u8_u32(m)));
			alpha_acc = vorrq_u32(alpha_acc, min_val);
			vst1q_u32(surf_ptr, vbslq_u32(ALPHA_MASK, min_val, s));

			surf_ptr += 4;
			mask_ptr += 4;
			remainder -= 4;
		}

		// ------------------------------------------------------------
		// Final Accumulator Check (ARMv7 compatible)
		// ------------------------------------------------------------
		alpha_acc = vandq_u32(alpha_acc, ALPHA_MASK);
		if (horizontal_or_u32(alpha_acc) > 0) {
			empty = false;
		}

		// ------------------------------------------------------------
		// Scalar remainder: Handle last 1-3 pixels
		// ------------------------------------------------------------
		while (remainder > 0) {
			uint32_t s = *surf_ptr;
			uint32_t m = *mask_ptr;
			uint32_t sa = s >> 24;
			uint32_t ma = m >> 24;
			uint32_t a = (sa < ma) ? sa : ma;
			if (a > 0) empty = false;
			*surf_ptr = (a << 24) | (s & 0x00FFFFFF);

			surf_ptr++;
			mask_ptr++;
			remainder--;
		}
	}

	void apply_surface_opacity_simd_neon(uint32_t* __restrict surf_ptr, std::size_t total_pixels, uint8_t alpha_mod_scalar)
	{
		// Prepare constants
		const uint8x8_t MOD_VEC_U8 = vdup_n_u8(alpha_mod_scalar);
		const uint16x8_t C128 = vdupq_n_u16(128);

		std::size_t offset = 0;
		std::size_t limit = total_pixels & ~15;

		// ------------------------------------------------------------
		// Main loop: 16 pixels per iteration
		// ------------------------------------------------------------
		uint32_t* __restrict ptr = surf_ptr;

		for (; offset < limit; offset += 16) {
			// Load de-interleaved: val[0]=R, val[1]=G, val[2]=B, val[3]=A
			uint8x16x4_t pixels = vld4q_u8(reinterpret_cast<uint8_t*>(ptr));

			// Skip block if all alphas are zero (ARMv7 compatible)
			if (horizontal_max_u8(pixels.val[3]) == 0) {
				ptr += 16;
				continue;
			}

			// Process Alpha (Low 8 bytes)
			uint16x8_t m_low = vmull_u8(vget_low_u8(pixels.val[3]), MOD_VEC_U8);
			uint16x8_t n_low = vaddq_u16(m_low, C128);
			n_low = vaddq_u16(n_low, vshrq_n_u16(m_low, 8));
			n_low = vshrq_n_u16(n_low, 8);

			// Process Alpha (High 8 bytes)
			uint16x8_t m_high = vmull_u8(vget_high_u8(pixels.val[3]), MOD_VEC_U8);
			uint16x8_t n_high = vaddq_u16(m_high, C128);
			n_high = vaddq_u16(n_high, vshrq_n_u16(m_high, 8));
			n_high = vshrq_n_u16(n_high, 8);

			// Pack back and update alpha channel
			pixels.val[3] = vcombine_u8(vqmovn_u16(n_low), vqmovn_u16(n_high));

			// Store interleaved
			vst4q_u8(reinterpret_cast<uint8_t*>(ptr), pixels);

			ptr += 16;
		}

		// ------------------------------------------------------------
		// Vector remainder (4 pixels at a time)
		// ------------------------------------------------------------
		for (; offset + 4 <= total_pixels; offset += 4) {
			uint32x4_t p = vld1q_u32(ptr);

			// Extract alpha channel
			uint32x4_t alpha_32 = vshrq_n_u32(p, 24);
			uint16x4_t alpha_16 = vmovn_u32(alpha_32);
			uint8x8_t alpha_8 = vmovn_u16(vcombine_u16(alpha_16, alpha_16));

			// Quick zero check
			if (vget_lane_u32(vreinterpret_u32_u8(alpha_8), 0) == 0) {
				ptr += 4;
				continue;
			}

			// Multiply alpha (returns uint16x8_t)
			uint16x8_t m = vmull_u8(alpha_8, MOD_VEC_U8);

			// Normalize (take low half since we only need 4 values)
			uint16x4_t m_low = vget_low_u16(m);
			uint16x4_t newa = vadd_u16(m_low, vget_low_u16(C128));
			newa = vadd_u16(newa, vshr_n_u16(m_low, 8));
			newa = vshr_n_u16(newa, 8);

			// Expand and shift to alpha position
			uint32x4_t new_alpha = vshlq_n_u32(vmovl_u16(newa), 24);
			uint32x4_t res = vorrq_u32(vandq_u32(p, vdupq_n_u32(0x00FFFFFF)), new_alpha);

			vst1q_u32(ptr, res);
			ptr += 4;
		}

		// ------------------------------------------------------------
		// Scalar Remainder
		// ------------------------------------------------------------
		for (; offset < total_pixels; ++offset) {
			uint32_t pixel = *ptr;
			uint32_t a = (pixel >> 24);
			if (a != 0) {
				uint32_t prod = a * alpha_mod_scalar;
				uint32_t new_a = (prod + 128 + (prod >> 8)) >> 8;
				*ptr = (new_a << 24) | (pixel & 0x00FFFFFF);
			}
			ptr++;
		}
	}

	void adjust_surface_color_simd_neon(uint32_t* __restrict surf_ptr, std::size_t total_pixels, int r, int g, int b)
	{
		// Pre-calculate positive and negative adjustments
		uint8_t r_pos = (r > 0) ? static_cast<uint8_t>(std::min(r, 255)) : 0;
		uint8_t r_neg = (r < 0) ? static_cast<uint8_t>(std::min(-r, 255)) : 0;
		uint8_t g_pos = (g > 0) ? static_cast<uint8_t>(std::min(g, 255)) : 0;
		uint8_t g_neg = (g < 0) ? static_cast<uint8_t>(std::min(-g, 255)) : 0;
		uint8_t b_pos = (b > 0) ? static_cast<uint8_t>(std::min(b, 255)) : 0;
		uint8_t b_neg = (b < 0) ? static_cast<uint8_t>(std::min(-b, 255)) : 0;

		// Broadcast to vectors
		const uint8x16_t R_ADD = vdupq_n_u8(r_pos);
		const uint8x16_t R_SUB = vdupq_n_u8(r_neg);
		const uint8x16_t G_ADD = vdupq_n_u8(g_pos);
		const uint8x16_t G_SUB = vdupq_n_u8(g_neg);
		const uint8x16_t B_ADD = vdupq_n_u8(b_pos);
		const uint8x16_t B_SUB = vdupq_n_u8(b_neg);

		std::size_t offset = 0;
		std::size_t limit = total_pixels & ~15;

		// ------------------------------------------------------------
		// Main loop: 16 pixels per iteration
		// ------------------------------------------------------------
		uint32_t* ptr = surf_ptr;

		for (; offset < limit; offset += 16) {
			// De-interleave: val[0]=Blue, val[1]=Green, val[2]=Red, val[3]=Alpha
			uint8x16x4_t pixels = vld4q_u8(reinterpret_cast<uint8_t*>(ptr));

			// Apply saturating add/subtract to each channel
			pixels.val[0] = vqsubq_u8(vqaddq_u8(pixels.val[0], B_ADD), B_SUB);
			pixels.val[1] = vqsubq_u8(vqaddq_u8(pixels.val[1], G_ADD), G_SUB);
			pixels.val[2] = vqsubq_u8(vqaddq_u8(pixels.val[2], R_ADD), R_SUB);
			// Alpha (val[3]) unchanged

			// Interleave and store
			vst4q_u8(reinterpret_cast<uint8_t*>(ptr), pixels);

			ptr += 16;
		}

		// ------------------------------------------------------------
		// Scalar Remainder
		// ------------------------------------------------------------
		for (; offset < total_pixels; ++offset) {
			uint32_t pixel = *ptr;
			int pr = std::clamp(static_cast<int>((pixel >> 16) & 0xFF) + r, 0, 255);
			int pg = std::clamp(static_cast<int>((pixel >> 8) & 0xFF) + g, 0, 255);
			int pb = std::clamp(static_cast<int>(pixel & 0xFF) + b, 0, 255);
			*ptr = (pixel & 0xFF000000) | (pr << 16) | (pg << 8) | pb;
			ptr++;
		}
	}

	void flip_image_simd_neon(uint32_t* __restrict pixels, std::size_t width, std::size_t height)
	{
		const std::size_t half_width = width / 2;
		std::size_t y = 0;

		// ARMv7 compatible reverse using vrev32 and vext
		auto reverse_vector_u32 = [](uint32x4_t v) -> uint32x4_t {
			// Reverse pairs: [0,1,2,3] -> [1,0,3,2]
			v = vrev64q_u32(v);
			// Swap halves: [1,0,3,2] -> [3,2,1,0]
			return vcombine_u32(vget_high_u32(v), vget_low_u32(v));
			};

		// Process 4 rows at a time
		for (; y + 4 <= height; y += 4) {
			uint32_t* r0 = pixels + (y + 0) * width;
			uint32_t* r1 = pixels + (y + 1) * width;
			uint32_t* r2 = pixels + (y + 2) * width;
			uint32_t* r3 = pixels + (y + 3) * width;

			std::size_t x = 0;

			for (; x + 4 <= half_width; x += 4) {
				std::size_t r_idx = width - x - 4;

				// Batch loads
				uint32x4_t L0 = vld1q_u32(r0 + x);
				uint32x4_t R0 = vld1q_u32(r0 + r_idx);
				uint32x4_t L1 = vld1q_u32(r1 + x);
				uint32x4_t R1 = vld1q_u32(r1 + r_idx);
				uint32x4_t L2 = vld1q_u32(r2 + x);
				uint32x4_t R2 = vld1q_u32(r2 + r_idx);
				uint32x4_t L3 = vld1q_u32(r3 + x);
				uint32x4_t R3 = vld1q_u32(r3 + r_idx);

				// Reverse all vectors
				uint32x4_t revL0 = reverse_vector_u32(L0);
				uint32x4_t revR0 = reverse_vector_u32(R0);
				uint32x4_t revL1 = reverse_vector_u32(L1);
				uint32x4_t revR1 = reverse_vector_u32(R1);
				uint32x4_t revL2 = reverse_vector_u32(L2);
				uint32x4_t revR2 = reverse_vector_u32(R2);
				uint32x4_t revL3 = reverse_vector_u32(L3);
				uint32x4_t revR3 = reverse_vector_u32(R3);

				// Batch stores (swapped)
				vst1q_u32(r0 + x, revR0);
				vst1q_u32(r0 + r_idx, revL0);
				vst1q_u32(r1 + x, revR1);
				vst1q_u32(r1 + r_idx, revL1);
				vst1q_u32(r2 + x, revR2);
				vst1q_u32(r2 + r_idx, revL2);
				vst1q_u32(r3 + x, revR3);
				vst1q_u32(r3 + r_idx, revL3);
			}

			// Scalar cleanup
			for (; x < half_width; ++x) {
				std::size_t r_idx = width - 1 - x;
				std::swap(r0[x], r0[r_idx]);
				std::swap(r1[x], r1[r_idx]);
				std::swap(r2[x], r2[r_idx]);
				std::swap(r3[x], r3[r_idx]);
			}
		}

		// Height remainder
		for (; y < height; ++y) {
			uint32_t* r = pixels + y * width;
			std::size_t x = 0;

			for (; x + 4 <= half_width; x += 4) {
				std::size_t r_idx = width - x - 4;
				uint32x4_t L = vld1q_u32(r + x);
				uint32x4_t R = vld1q_u32(r + r_idx);
				vst1q_u32(r + x, reverse_vector_u32(R));
				vst1q_u32(r + r_idx, reverse_vector_u32(L));
			}

			for (; x < half_width; ++x) {
				std::swap(r[x], r[width - 1 - x]);
			}
		}
	}

#endif // (SIMD_NEON)

// ============================================================================
// PUBLIC DISPATCHERS
// ============================================================================

bool mask_surface_simd(uint32_t* surf_ptr, const uint32_t* mask_ptr, std::size_t total_pixels, bool& empty)
{
	if (total_pixels > 63) {
#if (SIMD_SSE2)
		mask_surface_simd_sse2(surf_ptr, mask_ptr, total_pixels, empty);
		return true;
#elif (SIMD_NEON)
		mask_surface_simd_neon(surf_ptr, mask_ptr, total_pixels, empty);
		return true;
#else
		return false;
#endif
	}
	return false;
}

bool apply_surface_opacity_simd(uint32_t* surf_ptr, std::size_t total_pixels, uint8_t alpha_mod)
{
	if (total_pixels > 63) {
#if (SIMD_SSE2)
		apply_surface_opacity_simd_sse2(surf_ptr, total_pixels, alpha_mod);
		return true;
#elif (SIMD_NEON)
		apply_surface_opacity_simd_neon(surf_ptr, total_pixels, alpha_mod);
		return true;
#else
		return false;
#endif
	}
	return false;
}

bool adjust_surface_color_simd(uint32_t* surf_ptr, std::size_t total_pixels, int r, int g, int b)
{
	if (total_pixels > 63) {
#if (SIMD_SSE2)
		adjust_surface_color_simd_sse2(surf_ptr, total_pixels, r, g, b);
		return true;
#elif (SIMD_NEON)
		adjust_surface_color_simd_neon(surf_ptr, total_pixels, r, g, b);
		return true;
#else
		return false;
#endif
	}
	return false;
}

bool flip_image_simd(uint32_t* pixel_pointer, std::size_t width, std::size_t height)
{
	if ((width*height) > 63) {
#if (SIMD_SSE2)
		flip_image_simd_sse2(pixel_pointer, width, height);
		return true;
#elif (SIMD_NEON)
		flip_image_simd_neon(pixel_pointer, width, height);
		return true;
#else
		return false;
#endif
	}
	return false;
}
