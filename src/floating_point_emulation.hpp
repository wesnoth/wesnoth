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
 * Contains code for a floating point emulation.
 *
 * Depending on some compiler switches the code can use floating point code or
 * fixed point code. The code is still work-in-progress.
 *
 * At the moment three emulation modi are supported:
 * * A 32-bit signed integer, shifted 8 bits.
 * * A @c double, not shifted.
 * * A @c double, shifted 8 bits (for debugging).
 *
 * In the comments some conventions are used:
 * - lowercase variables are unscaled values
 * - UPPERCASE VARIABLES are scaled values
 * - the variable `sf' is the scale factor
 * - the variable `result' and `RESULT' are the result of a calculation
 *
 * There are several define's to control the compilation.
 *
 * FLOATING_POINT_EMULATION_USE_SCALED_INT
 * When this macro is defined the @ref tfloat is defined as the 32-bit scaled
 * integer. If not the @ref tfloat is defined as a @c double, whether or not the
 * value is shifted depends on @c FLOATING_POINT_EMULATION_ENABLE_RANGE_CHECK.
 *
 * FLOATING_POINT_EMULATION_ENABLE_RANGE_CHECK
 * This macro can only be defined if @c FLOATING_POINT_EMULATION_USE_SCALED_INT
 * is not defined. This macro shifts the @c doubles so it can be checked
 * against the range of the scaled int. (It would have been possible to scale
 * the validation range as well, but this feels easier to check.)
 *
 * FLOATING_POINT_EMULATION_ENABLE_RANGE_CHECK_THROW
 * This macro can only be defined if
 * @c FLOATING_POINT_EMULATION_ENABLE_RANGE_CHECK is defined. Instead of
 * printing a warning and then go on it throws an exception if the range check
 * fails. This is intended to aid debugging and should not be enabled in
 * production code.
 *
 * FLOATING_POINT_EMULATION_TRACER_ENABLE
 * This macro allows to place trace markers in the code. When using the markers
 * it's possible to gather statistics regarding the code paths being executed.
 * This can be used to analyze execution and add branch prediction markers.
 */

#ifndef FLOATING_POINT_EMULATION_HPP_INCLUDED
#define FLOATING_POINT_EMULATION_HPP_INCLUDED

#include "global.hpp"
#include "util.hpp"

#include <SDL_types.h>

#include <boost/utility/enable_if.hpp>

#include <cassert>
#include <cmath>

//#define FLOATING_POINT_EMULATION_USE_SCALED_INT

//#define FLOATING_POINT_EMULATION_ENABLE_RANGE_CHECK
//#define FLOATING_POINT_EMULATION_ENABLE_RANGE_CHECK_THROW

//#define FLOATING_POINT_EMULATION_TRACER_ENABLE

#ifdef FLOATING_POINT_EMULATION_ENABLE_RANGE_CHECK_THROW
#ifndef FLOATING_POINT_EMULATION_ENABLE_RANGE_CHECK
#error FLOATING_POINT_EMULATION_ENABLE_RANGE_CHECK_THROW requires            \
	FLOATING_POINT_EMULATION_ENABLE_RANGE_CHECK
#endif
#include <stdexcept>
#define FLOATING_POINT_EMULATION_RANGE_CHECK_THROW                           \
	do {                                                                     \
		throw std::range_error("");                                          \
	} while(0)
#else
#define FLOATING_POINT_EMULATION_RANGE_CHECK_THROW                           \
	do {                                                                     \
	} while(0)
#endif

#ifdef FLOATING_POINT_EMULATION_ENABLE_RANGE_CHECK
#ifdef FLOATING_POINT_EMULATION_USE_SCALED_INT
#error FLOATING_POINT_EMULATION_USE_SCALED_INT and                           \
	FLOATING_POINT_EMULATION_ENABLE_RANGE_CHECK are mutually exclusive.
#endif
#include <iostream>
#define FLOATING_POINT_EMULATION_RANGE_CHECK                                 \
	FLOATING_POINT_EMULATION_RANGE_CHECK_OBJECT((*this))

#define FLOATING_POINT_EMULATION_RANGE_CHECK_OBJECT(object)                  \
	do {                                                                     \
		if(object.value_ >= 2147483648.0) {                                  \
			std::cerr << "Positive overflow »" << object.value_              \
					<< "« as double »" << object.to_double() <<"« .\n";      \
			FLOATING_POINT_EMULATION_RANGE_CHECK_THROW;                      \
		}                                                                    \
		if(object.value_ < -2147483648.0) {                                  \
			std::cerr << "Negative overflow »" << object.value_              \
					<< "« as double »" << object.to_double() <<"« .\n";      \
			FLOATING_POINT_EMULATION_RANGE_CHECK_THROW;                      \
		}                                                                    \
	} while(0)
#else
#define FLOATING_POINT_EMULATION_RANGE_CHECK                                 \
	do {                                                                     \
	} while(0)
#define FLOATING_POINT_EMULATION_RANGE_CHECK_OBJECT(object)                  \
	do {                                                                     \
	} while(0)
#endif

#ifdef FLOATING_POINT_EMULATION_TRACER_ENABLE
#include "tracer.hpp"
#define	FLOATING_POINT_EMULATION_TRACER_ENTRY(interval) TRACER_ENTRY(interval)
#define FLOATING_POINT_EMULATION_TRACER_COUNT(marker) TRACER_COUNT(marker)
#else
#define	FLOATING_POINT_EMULATION_TRACER_ENTRY(interval)                      \
	do {                                                                     \
	} while(0)

#define FLOATING_POINT_EMULATION_TRACER_COUNT(marker)                        \
	do {                                                                     \
	} while(0)
#endif

namespace floating_point_emulation {

template<class T, unsigned S>
class tfloat;

namespace detail {

/***** ***** Internal conversion functions. ***** *****/

template<class T, unsigned S, class E = void>
struct tscale
{
};

template<class T>
struct tscale<T, 0>
{
	/**
	 * Scales the internal value up.
	 *
	 * Upscaling is used for:
	 * - Returning the value to external code; the internal representation
	 *   needs to be adjusted for the external representation.
	 * - After multiplying a tfloat, then both values were scaled so the value
	 *   is a factor @p tfloat::scale_factor too big.
	 */
	static T
	down(T& value)
	{
		return value;
	}

	/**
	 * Scales the value up.
	 *
	 * Upscaling is used for:
	 * - Loading the value it needs to be scaled to the internal value.
	 * - After dividing a tfloat, then both values were scaled so the value
	 *   is a factor @p tfloat::scale_factor too small.
	 */
	static T
	up(T& value)
	{
		return value;
	}
};

template<unsigned S>
struct tscale<double, S, typename boost::enable_if_c<S != 0>::type>
{
	static double
	down(double& value)
	{
		value /= (1 << S);
		return value;
	}

	static double
	up(double& value)
	{
		value *= (1 << S);
		return value;
	}
};

template<unsigned S>
struct tscale<Sint32, S, typename boost::enable_if_c<S != 0>::type>
{
	static Sint32
	down(Sint32& value)
	{
		if(value > 0) {
			value >>= S;
		} else {
			value = -((-value) >> S);
		}
		return value;
	}

	static Sint32
	up(Sint32& value)
	{
		value <<= S;
		return value;
	}
};

template<class T, unsigned S>
inline T
load(int value)
{
	T result(value);
	return tscale<T, S>::up(result);
}

template<class T, unsigned S>
inline T
load(double value)
{
	return tscale<double, S>::up(value);
}

template<class R, class T, unsigned S>
inline R
store(T value)
{
	R result(value);
	return tscale<R, S>::down(result);
}

template<class T, unsigned S>
struct tfloor
{
};

template<unsigned S>
struct tfloor<double, S>
{
	static int
	floor(double value)
	{
		return std::floor(tscale<double, S>::down(value));
	}
};

template<unsigned S>
struct tfloor<Sint32, S>
{
	static int
	floor(Sint32 value)
	{
		value >>= S;
		return value;
	}
};

/**
 * Shift arithmetically left.
 *
 * Shifting of floating point values doesn't exist so it's emulated by a
 * multiplication. This function allows @ref tidiv::idiv() to contain generic
 * code.
 *
 * @param lhs                     The value to `shift'.
 * @param rhs                     The number of bits to `shift'.
 */
inline void
sal(double& lhs, unsigned rhs)
{
	lhs *= (1 << rhs);
}

/**
 * Shift arithmetically left.
 *
 * @param lhs                     The value to shift.
 * @param rhs                     The number of bits to shift.
 */
inline void
sal(Sint32& lhs, unsigned rhs)
{
	lhs <<= rhs;
}

/**
 * Shift arithmetically right.
 *
 * Shifting of floating point values doesn't exist so it's emulated by a
 * division. This function allows @ref tidiv::idiv() to contain generic
 * code.
 *
 * @param lhs                     The value to `shift'.
 * @param rhs                     The number of bits to `shift'.
 */
inline void
sar(double& lhs, unsigned rhs)
{
	lhs /= (1 << rhs);
}

/**
 * Shift arithmetically right.
 *
 * @param lhs                     The value to shift.
 * @param rhs                     The number of bits to shift.
 */
inline void
sar(Sint32& lhs, unsigned rhs)
{
	lhs >>= rhs;
}

template<class T, unsigned S>
struct tidiv
{
	static tfloat<T, S>&
	idiv(tfloat<T, S>& lhs, tfloat<T, S> rhs)
	{
		detail::tscale<T, S>::up(lhs.value_);
		FLOATING_POINT_EMULATION_RANGE_CHECK_OBJECT(lhs);
		lhs.value_ /= rhs.value_;
		FLOATING_POINT_EMULATION_RANGE_CHECK_OBJECT(lhs);
		return lhs;
	}
};

template<class T>
struct tidiv<T, 8>
{
	static tfloat<T, 8>&
	idiv(tfloat<T, 8>& lhs, tfloat<T, 8> rhs);
};

/**
 * An optimized version of the division operator.
 *
 * This version is optimized to maintain the highest numeric stability when
 * dividing.
 *
 * As documented at operator/():
 * THIS * RHS = this * sf * rhs * sf = result * sf * sf = RESULT * sf
 *
 * THIS   this * sf   this
 * ---- = --------- = ---- = result
 *  RHS    rhs * sf    rhs
 *
 * Thus in order to get RESULT there needs to be multiplied by sf:
 *
 *          THIS
 * RESULT = ---- * sf
 *           RHS
 *
 * Since multiplication is associative this can also be written as:
 *
 *           sf   THIS
 * RESULT =  -- * ---- * 2
 *            2    RHS
 *
 * And as
 *
 *           (sf / 2) * THIS
 * RESULT =  --------------- * 2
 *                       RHS
 *
 * Thus it is possible to try to get the dividend as large as possible before
 * the division and thus loose less precision.
 *
 * Another option to get the proper result is:
 *
 *                THIS
 * RESULT = ----------
 *          (RHS / sf)
 *
 * Obviously dumping the last bits of the divisor will loose a lot of
 * precision, except if these bits contain zeros.
 *
 * It's also possible to do a combination of these ways and thus generating an
 * algorithm like:
 * * Try to shift the dividend up by as many bits as possible without overflow.
 * * Try to shift the divisor down by as many bits as possible without loosing
 *   resolution.
 * * divide.
 * * shift the result up by the required number of bits.
 *
 * The code has some other optimizations as well. On a 2-complement system
 * there are additional tests required for negative and positive values, to
 * remove these branches, the code uses a temporary value which contains the
 * positive value.
 *
 * @note The shifted double also uses this version in order to test overflows.
 *
 * @todo The function needs more branch-prediction markers.
 *
 * @todo The function is too large to inline and needs to be separated in more
 * functions, where the unlikely cases are not inlined but a function call.
 */
template<class T>
inline tfloat<T, 8>&
tidiv<T, 8>::idiv(tfloat<T, 8>& lhs, tfloat<T, 8> rhs)
{
	/* The interval has been determined empirically. */
	FLOATING_POINT_EMULATION_TRACER_ENTRY(100000);

	/* Division of zero is a NOP, so bail out directly. */
	if(lhs.value_ == 0) {
		FLOATING_POINT_EMULATION_TRACER_COUNT("lhs.value_ == 0");
		return lhs;
	}

	/*
	 * Test whether any high bit of the dividend is set.
	 *
	 * If the original value is smaller than 1 << 15 then the shifted
	 * value is smaller than 1 << 31, which can be done without any risk. This
	 * value is also quite likely to happen, so the case is optimized.
	 */

	Uint32 lhs_value = std::abs(lhs.value_);
	if(LIKELY(lhs_value <= 0x007FFFFFu)) {
		FLOATING_POINT_EMULATION_TRACER_COUNT("lhs_value <= 0x007FFFFFu");
		sal(lhs.value_, 8);
		FLOATING_POINT_EMULATION_RANGE_CHECK_OBJECT(lhs);

		lhs.value_ /= rhs.value_;
		FLOATING_POINT_EMULATION_RANGE_CHECK_OBJECT(lhs);
		return lhs;
	}

	/***********************************************************************/

	/*
	 * Shift the dividend up.
	 *
	 * The code is based upon the Linux kernel
	 * http://lxr.linux.no/#linux+v3.2.6/include/asm-generic/bitops/fls.h
	 */

	/*
	 * Will contain the offset from the MSB to the MSB set.
	 *
	 * The range returned should be [0, 8), the values outside this range
	 * should have been handled before. We don't look at the MSB since it
	 * `contains' the sign, except for the maximum negative value, where we
	 * can't shift at all).
	 */
	int dividend_high_bit = 0;

	if(UNLIKELY(lhs.value_ == static_cast<int>(0x80000000))) {
		/*
		 * Do nothing dividend_high_bit is already 0.
		 *
		 * The reason for this way of the if is to allow for a trace marker.
		 */
		FLOATING_POINT_EMULATION_TRACER_COUNT("lhs.value_ == 0x80000000");
	} else {
		if(!(lhs_value & 0x78000000u)) {
			FLOATING_POINT_EMULATION_TRACER_COUNT("!(lhs_value & 0x78000000u)");
			lhs_value <<= 4;
			dividend_high_bit += 4;
		}
		if(!(lhs_value & 0x60000000u)) {
			FLOATING_POINT_EMULATION_TRACER_COUNT("!(lhs_value & 0x60000000u)");
			lhs_value <<= 2;
			dividend_high_bit += 2;
		}
		if(!(lhs_value & 0x40000000u)) {
			FLOATING_POINT_EMULATION_TRACER_COUNT("!(lhs_value & 0x40000000u)");
			dividend_high_bit += 1;
		}
	}

	assert(dividend_high_bit >= 0);
	assert(dividend_high_bit < 8);

	sal(lhs.value_, dividend_high_bit);
	FLOATING_POINT_EMULATION_RANGE_CHECK_OBJECT(lhs);

	int shifted = dividend_high_bit;

	/***********************************************************************/

	if(shifted < 8) {
		/*
		 * Shift the divisor down.
		 *
		 * Code adapted from the algorithm used above.
		 *
		 * This test is only required if we didn't already shift the wanted
		 * number of positions.
		 */

		Uint32 rhs_value = std::abs(rhs.value_);

		/*
		 * Will contain the offset from bit 0 of the LSB set. Since we're only
		 * interested in values in the range [0-8] the value is saturated at 8.
		 */
		int divisor_low_bit = 0;

		if(!(rhs_value & 0xffu)) {
			FLOATING_POINT_EMULATION_TRACER_COUNT("!(rhs_value & 0xffu)");
			divisor_low_bit = 8;
		} else {
			if(!(rhs_value & 0xfu)) {
				FLOATING_POINT_EMULATION_TRACER_COUNT("!(rhs_value & 0xfu)");
				divisor_low_bit += 4;
				rhs_value >>= 4;
			}
			if(!(rhs_value & 0x3u)) {
				FLOATING_POINT_EMULATION_TRACER_COUNT("!(rhs_value & 0x3u)");
				divisor_low_bit += 2;
				rhs_value >>= 2;
			}
			if(!(rhs_value & 0x1u)) {
				FLOATING_POINT_EMULATION_TRACER_COUNT("!(rhs_value & 0x1u)");
				divisor_low_bit += 1;
			}
		}
		assert(divisor_low_bit >= 0);
		assert(divisor_low_bit <= 8);

		divisor_low_bit = std::min(8 - shifted, divisor_low_bit);

		sar(rhs.value_, divisor_low_bit);
		FLOATING_POINT_EMULATION_RANGE_CHECK_OBJECT(rhs);

		shifted += divisor_low_bit;
	}

	/***********************************************************************/

	lhs.value_ /= rhs.value_;
	FLOATING_POINT_EMULATION_RANGE_CHECK_OBJECT(lhs);

	/* Make the final shift adjustment. */
	sal(lhs.value_, 8 - shifted);
	FLOATING_POINT_EMULATION_RANGE_CHECK_OBJECT(lhs);

	return lhs;
}

} // namespace detail

/**
 * Template class for the emulation.
 *
 * @note Operators that are not documented do the expected thing.
 *
 * @note For conversions it would be nice to use the `operator int()' instead
 * of @ref to_int(). Unfortunately C++98 doesn't support the @c explicit for
 * conversion operators and the implicit conversion is evil.
 *
 * @tparam T                      The type used for the emulation. At the
 *                                moment the following types are supported:
 *                                * @c double
 */
template<class T, unsigned S>
class tfloat
{
	template<class TT, unsigned SS> friend struct detail::tidiv;
public:

	/***** ***** Types. ***** *****/

	typedef T value_type;

	enum { shift = S };
	enum { factor = 1 << S };

	/***** ***** Constructor, destructor, assignment. ***** *****/

	tfloat()
		: value_(0)
	{
	}

	explicit tfloat(const int value)
		: value_(detail::load<T, S>(value))
	{
		FLOATING_POINT_EMULATION_RANGE_CHECK;
	}

	explicit tfloat(const double value)
		: value_(detail::load<T, S>(value))
	{
		FLOATING_POINT_EMULATION_RANGE_CHECK;
	}


	tfloat&
	operator=(const tfloat rhs)
	{
		value_ = rhs.value_;
		FLOATING_POINT_EMULATION_RANGE_CHECK;
		return *this;
	}

	tfloat&
	operator=(const int rhs)
	{
		return operator=(tfloat(rhs));
	}

	tfloat&
	operator=(const double rhs)
	{
		return operator=(tfloat(rhs));
	}


	/***** ***** Relational operators. ***** *****/

	bool operator<(const tfloat rhs) const
	{
		return value_ < rhs.value_;
	}


	bool operator<=(const tfloat rhs) const
	{
		return !(rhs.value_ < value_);
	}


	bool operator>(const tfloat rhs) const
	{
		return rhs.value_ < value_;
	}


	bool operator>=(const tfloat rhs) const
	{
		return !(value_ < rhs.value_);
	}


	bool
	operator==(const tfloat rhs) const
	{
		return value_ == rhs.value_;
	}

	bool
	operator==(const int rhs) const
	{
		return operator==(tfloat(rhs));
	}

	bool
	operator==(const double rhs) const
	{
		return operator==(tfloat(rhs));
	}


	bool
	operator!=(const tfloat rhs) const
	{
		return !operator==(rhs);
	}

	bool
	operator!=(const int rhs) const
	{
		return !operator==(rhs);
	}

	bool
	operator!=(const double rhs) const
	{
		return !operator==(rhs);
	}


	/***** ***** Unary operators. ***** *****/

	/*
	 * This set are normally non-members, but this is easier to code.
	 */

	bool
	operator!() const
	{
		return value_ == 0;
	}

	tfloat
	operator-() const
	{
		return tfloat(*this).negative();
	}

	tfloat
	operator+() const
	{
		return *this;
	}


	/***** ***** Assignment operators. ***** *****/

	/***** Mul *****/

	/**
	 * Multiply
	 *
	 * Keep in mind that:
	 *
	 * THIS * RHS = this * sf * rhs * sf = result * sf * sf = RESULT * sf
	 *
	 * Thus in order to get RESULT there needs to be divided by sf:
	 *
	 * RESULT = THIS * RHS / sf
	 */
	tfloat&
	operator*=(const tfloat rhs)
	{
		value_ *= rhs.value_;

		/*
		 * There is no need to check the range at this point. The specialized
		 * version makes a short trip to 64-bit value, so overflowing is not
		 * possible.
		 */

		detail::tscale<T, S>::down(value_);
		FLOATING_POINT_EMULATION_RANGE_CHECK;
		return *this;
	}

	/**
	 * Multiply
	 *
	 * No extra adjustment needed since:
	 *
	 * RESULT = THIS * rhs
	 */
	tfloat&
	operator*=(const int rhs)
	{
		value_ *= rhs;
		FLOATING_POINT_EMULATION_RANGE_CHECK;
		return *this;
	}

	/**
	 * Multiply
	 *
	 * No extra adjustment needed since:
	 *
	 * RESULT = THIS * rhs
	 */
	tfloat&
	operator*=(const double rhs)
	{
		value_ *= rhs;
		FLOATING_POINT_EMULATION_RANGE_CHECK;
		return *this;
	}

	/***** Div *****/

	/**
	 * Divide
	 *
	 * Keep in mind that:
	 *
	 * THIS * RHS = this * sf * rhs * sf = result * sf * sf = RESULT * sf
	 *
	 * THIS   this * sf   this
	 * ---- = --------- = ---- = result
	 *  RHS    rhs * sf    rhs
	 *
	 * Thus in order to get RESULT there needs to be multiplied by sf:
	 *
	 *          THIS
	 * RESULT = ---- * sf
	 *           RHS
	 */
	tfloat&
	operator/=(const tfloat rhs)
	{
		return detail::tidiv<T, S>::idiv(*this, rhs);
	}

	/**
	 * Divide
	 *
	 * No extra adjustment needed since:
	 *
	 * RESULT = THIS / rhs
	 */
	tfloat&
	operator/=(const int rhs)
	{
		value_ /= rhs;
		FLOATING_POINT_EMULATION_RANGE_CHECK;
		return *this;
	}

	/**
	 * Divide
	 *
	 * No extra adjustment needed since:
	 *
	 * RESULT = THIS / rhs
	 */
	tfloat&
	operator/=(const double rhs)
	{
		value_ /= rhs;
		FLOATING_POINT_EMULATION_RANGE_CHECK;
		return *this;
	}


	/***** Add *****/

	tfloat&
	operator+=(const tfloat rhs)
	{
		value_ += rhs.value_;
		FLOATING_POINT_EMULATION_RANGE_CHECK;
		return *this;
	}

	tfloat&
	operator+=(const int rhs)
	{
		return operator+=(tfloat(rhs));
	}

	tfloat&
	operator+=(const double rhs)
	{
		return operator+=(tfloat(rhs));
	}

	/***** Sub *****/

	tfloat&
	operator-=(const tfloat rhs)
	{
		value_ -= rhs.value_;
		FLOATING_POINT_EMULATION_RANGE_CHECK;
		return *this;
	}

	tfloat&
	operator-=(const int rhs)
	{
		return operator-=(tfloat(rhs));
	}

	tfloat&
	operator-=(const double rhs)
	{
		return operator-=(tfloat(rhs));
	}

	/***** ***** Conversion functions. ***** *****/

	int
	to_int() const
	{
		return detail::store<int, T, S>(value_);
	}

	double
	to_double() const
	{
		return detail::store<double, T, S>(value_);
	}

	/***** ***** Math functions. ***** *****/

	int
	floor()
	{
		return detail::tfloor<T, S>::floor(value_);
	}

	/***** ***** Setters, getters. ***** *****/

	T
	get_value() const
	{
		return value_;
	}

private:

	/***** ***** Members. ***** *****/

	/**
	 * The value of the emulation.
	 *
	 * What the value represents is not really defined for the user.
	 */
	T value_;

	/**
	 * Changes the object to its negative value.
	 *
	 * There is an issue with @ref operator-(). There is an constructor that
	 * takes @ref value_type as parameter. When @ref shift != 0 the issue
	 * occurs. Implementing @ref operator-() as return tfloat(-value_) returns
	 * a scaled value. Scaling twice might loose resolution and cause some
	 * overhead. So just add this function to solve the issue.
	 *
	 * @returns                   @c *this, but it's value is negated.
	 */
	tfloat&
	negative()
	{
		value_ = -value_;
		return *this;
	}
};

/***** ***** Relational operators. ***** *****/

template<class T, unsigned S>
inline bool
operator==(const int lhs, const tfloat<T, S> rhs)
{
	return rhs.operator==(lhs);
}

template<class T, unsigned S>
inline bool
operator==(const double lhs, const tfloat<T, S> rhs)
{
	return rhs.operator==(lhs);
}

template<class T, unsigned S>
inline bool
operator!=(const int lhs, const tfloat<T, S> rhs)
{
	return rhs.operator!=(lhs);
}

template<class T, unsigned S>
inline bool
operator!=(const double lhs, const tfloat<T, S> rhs)
{
	return rhs.operator!=(lhs);
}


/***** ***** Math operators. ***** *****/

/***** Mul *****/

/**
 * Multiply
 *
 * Specialized for the Sint32 with a shift of 8.
 *
 * Instead of figuring out the optimal shift before multiplying simply multiply
 * as a 64-bit value and then perform the shift. This is rather cheap on the
 * Pandora and also keeps the code short on that platform (only two extra
 * instructions on the ARM v7; a `logical shift right' instruction followed by
 * a `logical left shifted or' instruction.)
 */
template<>
inline tfloat<Sint32, 8>&
tfloat<Sint32, 8>::operator*=(const tfloat<Sint32, 8> rhs)
{
	value_ = (static_cast<Sint64>(value_) * rhs.value_) >> 8;
	return *this;
}

template<class T, unsigned S>
inline tfloat<T, S>
operator*(tfloat<T, S> lhs, const tfloat<T, S> rhs)
{
	return lhs.operator*=(rhs);
}

template<class T, unsigned S>
inline tfloat<T, S>
operator*(tfloat<T, S> lhs, const int rhs)
{
	return lhs.operator*=(rhs);
}

template<class T, unsigned S>
inline tfloat<T, S>
operator*(tfloat<T, S> lhs, const double rhs)
{
	return lhs.operator*=(rhs);
}

template<class T, unsigned S>
inline tfloat<T, S>
operator*(const int lhs, tfloat<T, S> rhs)
{
	return rhs.operator*=(lhs);
}

template<class T, unsigned S>
inline tfloat<T, S>
operator*(const double lhs, tfloat<T, S> rhs)
{
	return rhs.operator*=(lhs);
}

/***** Div *****/

template<class T, unsigned S>
inline tfloat<T, S>
operator/(tfloat<T, S> lhs, const tfloat<T, S> rhs)
{
	return lhs.operator/=(rhs);
}

template<class T, unsigned S>
inline tfloat<T, S>
operator/(tfloat<T, S> lhs, const int rhs)
{
	return lhs.operator/=(rhs);
}

template<class T, unsigned S>
inline tfloat<T, S>
operator/(tfloat<T, S> lhs, const double rhs)
{
	return lhs.operator/=(rhs);
}

template<class T, unsigned S>
inline tfloat<T, S>
operator/(const int lhs, tfloat<T, S> rhs)
{
	return tfloat<T, S>(lhs).operator/=(rhs);
}

template<class T, unsigned S>
inline tfloat<T, S>
operator/(const double lhs, tfloat<T, S> rhs)
{
	return tfloat<T, S>(lhs).operator/=(rhs);
}

/***** Add *****/

template<class T, unsigned S>
inline tfloat<T, S>
operator+(tfloat<T, S> lhs, const tfloat<T, S> rhs)
{
	return lhs.operator+=(rhs);
}

template<class T, unsigned S>
inline tfloat<T, S>
operator+(tfloat<T, S> lhs, const int rhs)
{
	return lhs.operator+=(rhs);
}

template<class T, unsigned S>
inline tfloat<T, S>
operator+(tfloat<T, S> lhs, const double rhs)
{
	return lhs.operator+=(rhs);
}

template<class T, unsigned S>
inline tfloat<T, S>
operator+(const int lhs, const tfloat<T, S> rhs)
{
	return rhs.operator+=(lhs);
}

template<class T, unsigned S>
inline tfloat<T, S>
operator+(const double lhs, const tfloat<T, S> rhs)
{
	return rhs.operator+=(lhs);
}

/***** Sub *****/

template<class T, unsigned S>
inline tfloat<T, S>
operator-(tfloat<T, S> lhs, const tfloat<T, S> rhs)
{
	return lhs.operator-=(rhs);
}

template<class T, unsigned S>
inline tfloat<T, S>
operator-(tfloat<T, S> lhs, const int rhs)
{
	return lhs.operator-=(rhs);
}

template<class T, unsigned S>
inline tfloat<T, S>
operator-(tfloat<T, S> lhs, const double rhs)
{
	return lhs.operator-=(rhs);
}

template<class T, unsigned S>
inline tfloat<T, S>
operator-(const int lhs, tfloat<T, S> rhs)
{
	return tfloat<T, S>(lhs).operator-=(rhs);
}

template<class T, unsigned S>
inline tfloat<T, S>
operator-(const double lhs, tfloat<T, S> rhs)
{
	return tfloat<T, S>(lhs).operator-=(rhs);
}


/***** ***** Math functions. ***** *****/

template<class T, unsigned S>
inline int
floor(tfloat<T, S> lhs)
{
	return lhs.floor();
}

} // namespace floating_point_emulation

#ifdef FLOATING_POINT_EMULATION_USE_SCALED_INT
typedef floating_point_emulation::tfloat<Sint32, 8> tfloat;
#else
#ifdef FLOATING_POINT_EMULATION_ENABLE_RANGE_CHECK
	typedef floating_point_emulation::tfloat<double, 8> tfloat;
#else
	typedef floating_point_emulation::tfloat<double, 0> tfloat;
#endif
#endif

#endif
