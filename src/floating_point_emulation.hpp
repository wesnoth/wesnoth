/* $Id$ */
/*
   Copyright (C) 2012 by Mark de Wever <koraq@xs4all.nl>
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
 * integer. If not the @tfloat is defined as a @c double, whether or not the
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
 * This can be used to analyse execution and add branch prediction markers.
 */

#ifndef FLOATING_POINT_EMULATION_HPP_INCLUDED
#define FLOATING_POINT_EMULATION_HPP_INCLUDED

#include "global.hpp"

#include <SDL_types.h>

#include <boost/utility/enable_if.hpp>

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
	do {                                                                     \
		if(value_ >= 2147483648.0) {                                         \
			std::cerr << "Positive overflow »" << value_                     \
					<< "« as double »" << to_double() <<"« .\n";             \
			FLOATING_POINT_EMULATION_RANGE_CHECK_THROW;                      \
		}                                                                    \
		if(value_ < -2147483648.0) {                                         \
			std::cerr << "Negative overflow »" << value_                     \
					<< "« as double »" << to_double() <<"« .\n";             \
			FLOATING_POINT_EMULATION_RANGE_CHECK_THROW;                      \
		}                                                                    \
	} while(0)
#else
#define FLOATING_POINT_EMULATION_RANGE_CHECK                                 \
	do {                                                                     \
	} while(0)
#endif

#ifdef FLOATING_POINT_EMULATION_TRACER_ENABLE
#include <boost/foreach.hpp>
#include <map>
#include <iomanip>
#include <iostream>

/** Helper structure for gathering the tracing statistics. */
struct ttracer
{
	/**
	 * Helper structure to print the tracing statistics.
	 *
	 * When the constructor gets a valid @ref ttracer pointer it prints the
	 * tracing statistics in its destructor. This allows the structure to be
	 * initialised at the beginning of a scope and print the statistics once
	 * the scope is left. (This makes it easier to write the tracing macros.)
	 */
	struct tprint
	{
		explicit tprint(const ttracer* const tracer__)
			: tracer(tracer__)
		{
		}

		~tprint()
		{
			if(!tracer) {
				return;
			}

			std::cerr << "Run statistics:\n"
					<< "Runs:\t" << std::dec << tracer->run << "\n";

			typedef std::pair<std::pair<int, std::string>, int> thack;
			size_t maximum_length = 0;
			BOOST_FOREACH(const thack& counter, tracer->counters) {
				maximum_length = std::max(
						  maximum_length
						, counter.first.second.length());
			}

			std::ios_base::fmtflags original_flag = std::cerr.setf(
					  std::ios_base::left
					, std::ios_base::adjustfield);

			BOOST_FOREACH(const thack& counter, tracer->counters) {
				std::cerr << "Marker: "
						<< std::left
						<< std::setw(maximum_length) << counter.first.second
						<< std::right
						<< " [" << std::setw(5) << counter.first.first << ']'
						<< "    hits " << counter.second << "\n";
			}

			std::cerr.setf(original_flag, std::ios_base::adjustfield);
		}

		/** The tracer, whose statistics to print. */
		const ttracer* const tracer;
	};

	ttracer()
		: run(0)
		, counters()
	{
	}

	/** The total number of runs. */
	int run;

	/**
	 * The tracer counters.
	 *
	 * The first pair contains a line number and a name of the marker.
	 * This has two advantages:
	 * * A line number is always unique, thus if using markers with the same
	 *   name, they are not the same marker.
	 * * The markers are sorted in order of appearance and not in order of
	 * their names.
	 *
	 * The second pair contains the number of times a marker is hit.
	 */
	std::map<std::pair<int, std::string>, int> counters;
};

/**
 * The beginning of a traced scope.
 *
 * It is not intended that tracer scopes are nested, but it should work at the
 * moment.
 *
 * @param interval                The interval between printing the statistics.
 */
#define	FLOATING_POINT_EMULATION_TRACER_ENTRY(interval)                      \
	static ttracer tracer;                                                   \
	ttracer::tprint print((++tracer.run % interval) == 0 ? &tracer : NULL)

/**
 * A trace count point.
 *
 * When this macro is reached the counter for this marker is increased.
 *
 * @param marker                  A string with the name of the marker.
 */
#define FLOATING_POINT_EMULATION_TRACER_COUNT(marker)                        \
	do {                                                                     \
		++tracer.counters[std::make_pair(__LINE__, marker)];                 \
	} while(0)
#else
#define	FLOATING_POINT_EMULATION_TRACER_ENTRY(interval)                      \
	do {                                                                     \
	} while(0)

#define FLOATING_POINT_EMULATION_TRACER_COUNT(marker)                        \
	do {                                                                     \
	} while(0)
#endif

namespace floating_point_emulation {

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
		value /= (1 << S);
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
		FLOATING_POINT_EMULATION_RANGE_CHECK;
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
	 * Thus in order to get RESULT there needs to be mulitplied by sf:
	 *
	 *          THIS
	 * RESULT = ---- * sf
	 *           RHS
	 */
	tfloat&
	operator/=(tfloat rhs)
	{
		detail::tscale<T, S>::up(value_);
		FLOATING_POINT_EMULATION_RANGE_CHECK;
		value_ /= rhs.value_;
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
