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
 */

#ifndef FLOATING_POINT_EMULATION_HPP_INCLUDED
#define FLOATING_POINT_EMULATION_HPP_INCLUDED

#include <SDL_types.h>

#include <boost/utility/enable_if.hpp>

#include <cmath>

#define FLOATING_POINT_EMULATION_RANGE_CHECK do {} while(0)

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

	tfloat&
	operator*=(const tfloat rhs)
	{
		value_ *= rhs.value_;
		FLOATING_POINT_EMULATION_RANGE_CHECK;
		detail::tscale<T, S>::down(value_);
		FLOATING_POINT_EMULATION_RANGE_CHECK;
		return *this;
	}

	tfloat&
	operator*=(const int rhs)
	{
		value_ *= rhs;
		FLOATING_POINT_EMULATION_RANGE_CHECK;
		return *this;
	}

	tfloat&
	operator*=(const double rhs)
	{
		value_ *= rhs;
		FLOATING_POINT_EMULATION_RANGE_CHECK;
		return *this;
	}

	/***** Div *****/

	tfloat&
	operator/=(tfloat rhs)
	{
		detail::tscale<T, S>::up(value_);
		FLOATING_POINT_EMULATION_RANGE_CHECK;
		value_ /= rhs.value_;
		FLOATING_POINT_EMULATION_RANGE_CHECK;
		return *this;
	}

	tfloat&
	operator/=(const int rhs)
	{
		value_ /= rhs;
		FLOATING_POINT_EMULATION_RANGE_CHECK;
		return *this;
	}

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

typedef floating_point_emulation::tfloat<double, 0> tfloat;

#endif
