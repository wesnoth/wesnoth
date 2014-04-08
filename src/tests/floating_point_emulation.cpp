/*
   Copyright (C) 2011 - 2014 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "floating_point_emulation.hpp"
#include <boost/test/unit_test.hpp>

namespace floating_point_emulation {

template<class T, unsigned S>
inline std::ostream&
operator<<(std::ostream& stream, const tfloat<T, S>& rhs)
{
	stream << "\nfloat value_type »" << typeid(T).name()
			<< "« shift »" << S
			<< "« internal value »" << rhs.get_value()
			<< "« as double »" << rhs.to_double()
			<< "« as int »" << rhs.to_int()
			<< "«.\n";

	return stream;
}

} // namespace floating_point_emulation

template<class T, unsigned S>
static void
test_create()
{
	{
		floating_point_emulation::tfloat<T, S> f;
		BOOST_CHECK_EQUAL(f, 0);
		BOOST_CHECK_EQUAL(f, 0.);
	}

	{
		floating_point_emulation::tfloat<T, S> f(42);
		BOOST_CHECK_EQUAL(f, 42);
		BOOST_CHECK_EQUAL(f, 42.);
	}

	{
		floating_point_emulation::tfloat<T, S> f(42.);
		BOOST_CHECK_EQUAL(f, 42);
		BOOST_CHECK_EQUAL(f, 42.);
	}

	{
		floating_point_emulation::tfloat<T, S> f(42.42);
		BOOST_CHECK_NE(f, 42);
		BOOST_CHECK_EQUAL(f, 42.42);
	}
}

template<class T, unsigned S>
static void
test_assign()
{
	{
		floating_point_emulation::tfloat<T, S> f;
		f = 42;
		BOOST_CHECK_EQUAL(f, 42);
		BOOST_CHECK_EQUAL(f, 42.);
	}

	{
		floating_point_emulation::tfloat<T, S> f;
		f = 42.;
		BOOST_CHECK_EQUAL(f, 42);
		BOOST_CHECK_EQUAL(f, 42.);
	}

	{
		floating_point_emulation::tfloat<T, S> f;
		f = 42.42;
		BOOST_CHECK_NE(f, 42);
		BOOST_CHECK_EQUAL(f, 42.42);
	}

	{
		floating_point_emulation::tfloat<T, S> f, g;
		f = 42;
		g = f;
		BOOST_CHECK_EQUAL(f, g);
	}
}


template<class T, unsigned S>
static void
test_unary()
{
	{
		floating_point_emulation::tfloat<T, S> f(42), g(0);
		BOOST_CHECK_EQUAL(!f, false);
		BOOST_CHECK_EQUAL(!!f, true);

		BOOST_CHECK_EQUAL(!g, true);
		BOOST_CHECK_EQUAL(!!g, false);
	}

	{
		floating_point_emulation::tfloat<T, S> f(42), g(-42);
		BOOST_CHECK_NE(f, g);
		BOOST_CHECK_EQUAL(f, -g);

		BOOST_CHECK_EQUAL(-f, -42);
		BOOST_CHECK_EQUAL(-f, -42.);

		BOOST_CHECK_EQUAL(-42, -f);
		BOOST_CHECK_EQUAL(-42., -f);

		BOOST_CHECK_EQUAL(-f == g, true);
		BOOST_CHECK_EQUAL(f == -g, true);
		BOOST_CHECK_EQUAL(-g == f, true);
		BOOST_CHECK_EQUAL(g == -f, true);

		BOOST_CHECK_EQUAL(-f == -42, true);
		BOOST_CHECK_EQUAL(-f == -42., true);

		BOOST_CHECK_EQUAL(-42 == -f, true);
		BOOST_CHECK_EQUAL(-42. == -f, true);
	}

	{
		floating_point_emulation::tfloat<T, S> f(42);

		BOOST_CHECK_EQUAL(+f == f, true);
		BOOST_CHECK_EQUAL(f == +f, true);

		BOOST_CHECK_EQUAL(+f == 42, true);
		BOOST_CHECK_EQUAL(+f == 42., true);

		BOOST_CHECK_EQUAL(42 == +f, true);
		BOOST_CHECK_EQUAL(42. == +f, true);
	}
}

template<class T, unsigned S>
static void
test_multiply()
{
	{
		floating_point_emulation::tfloat<T, S> f(42), g(0);
		f *= g;
		BOOST_CHECK_EQUAL(f, 0);
		BOOST_CHECK_EQUAL(f, 0.);
	}

	{
		floating_point_emulation::tfloat<T, S> f(42);
		f *= 0;
		BOOST_CHECK_EQUAL(f, 0);
		BOOST_CHECK_EQUAL(f, 0.);
	}

	{
		floating_point_emulation::tfloat<T, S> f(42);
		f *= 0.;
		BOOST_CHECK_EQUAL(f, 0);
		BOOST_CHECK_EQUAL(f, 0.);
	}



	{
		floating_point_emulation::tfloat<T, S> f(42), g(42);
		f *= g;
		BOOST_CHECK_EQUAL(f, 1764);
		BOOST_CHECK_EQUAL(f, 1764.);
	}

	{
		floating_point_emulation::tfloat<T, S> f(42);
		f *= 42;
		BOOST_CHECK_EQUAL(f, 1764);
		BOOST_CHECK_EQUAL(f, 1764.);
	}

	{
		floating_point_emulation::tfloat<T, S> f(42);
		f *= 42.;
		BOOST_CHECK_EQUAL(f, 1764);
		BOOST_CHECK_EQUAL(f, 1764.);
	}



	{
		floating_point_emulation::tfloat<T, S> f(-42), g(42);
		f *= g;
		BOOST_CHECK_EQUAL(f, -1764);
		BOOST_CHECK_EQUAL(f, -1764.);
	}

	{
		floating_point_emulation::tfloat<T, S> f(-42);
		f *= 42;
		BOOST_CHECK_EQUAL(f, -1764);
		BOOST_CHECK_EQUAL(f, -1764.);
	}

	{
		floating_point_emulation::tfloat<T, S> f(-42);
		f *= 42.;
		BOOST_CHECK_EQUAL(f, -1764);
		BOOST_CHECK_EQUAL(f, -1764.);
	}



	{
		floating_point_emulation::tfloat<T, S> f(-42), g(-42);
		f *= g;
		BOOST_CHECK_EQUAL(f, 1764);
		BOOST_CHECK_EQUAL(f, 1764.);
	}

	{
		floating_point_emulation::tfloat<T, S> f(-42);
		f *= -42;
		BOOST_CHECK_EQUAL(f, 1764);
		BOOST_CHECK_EQUAL(f, 1764.);
	}

	{
		floating_point_emulation::tfloat<T, S> f(-42);
		f *= -42.;
		BOOST_CHECK_EQUAL(f, 1764);
		BOOST_CHECK_EQUAL(f, 1764.);
	}



	{
		floating_point_emulation::tfloat<T, S> f(42), g(-42);
		f *= g;
		BOOST_CHECK_EQUAL(f, -1764);
		BOOST_CHECK_EQUAL(f, -1764.);
	}

	{
		floating_point_emulation::tfloat<T, S> f(42);
		f *= -42;
		BOOST_CHECK_EQUAL(f, -1764);
		BOOST_CHECK_EQUAL(f, -1764.);
	}

	{
		floating_point_emulation::tfloat<T, S> f(42);
		f *= -42.;
		BOOST_CHECK_EQUAL(f, -1764);
		BOOST_CHECK_EQUAL(f, -1764.);
	}



	{
		floating_point_emulation::tfloat<T, S> f(1.25), g(2.5);
		f *= g;
		BOOST_CHECK_EQUAL(f, 3.125);
	}

	{
		floating_point_emulation::tfloat<T, S> f(1.25);
		f *= 2;
		BOOST_CHECK_EQUAL(f, 2.5);
	}

	{
		floating_point_emulation::tfloat<T, S> f(1.25);
		f *= 2.5;
		BOOST_CHECK_EQUAL(f, 3.125);
	}

	{
		floating_point_emulation::tfloat<T, S> f(-1.25), g(2.5);
		f *= g;
		BOOST_CHECK_EQUAL(f, -3.125);
	}

	{
		floating_point_emulation::tfloat<T, S> f(1.25);
		f *= -2;
		BOOST_CHECK_EQUAL(f, -2.5);
	}

	{
		floating_point_emulation::tfloat<T, S> f(-1.25);
		f *= -2.5;
		BOOST_CHECK_EQUAL(f, 3.125);
	}
}

template<class T, unsigned S>
static void
test_divide()
{
	{
		floating_point_emulation::tfloat<T, S> f(0), g(42);
		f /= g;
		BOOST_CHECK_EQUAL(f, 0);
		BOOST_CHECK_EQUAL(f, 0.);
	}

	{
		floating_point_emulation::tfloat<T, S> f(1764), g(42);
		f /= g;
		BOOST_CHECK_EQUAL(f, 42);
		BOOST_CHECK_EQUAL(f, 42.);
	}

	{
		floating_point_emulation::tfloat<T, S> f(1764);
		f /= 42;
		BOOST_CHECK_EQUAL(f, 42);
		BOOST_CHECK_EQUAL(f, 42.);
	}

	{
		floating_point_emulation::tfloat<T, S> f(1764);
		f /= 42.;
		BOOST_CHECK_EQUAL(f, 42);
		BOOST_CHECK_EQUAL(f, 42.);
	}



	{
		floating_point_emulation::tfloat<T, S> f(-1764), g(42);
		f /= g;
		BOOST_CHECK_EQUAL(f, -42);
		BOOST_CHECK_EQUAL(f, -42.);
	}

	{
		floating_point_emulation::tfloat<T, S> f(-1764);
		f /= 42;
		BOOST_CHECK_EQUAL(f, -42);
		BOOST_CHECK_EQUAL(f, -42.);
	}

	{
		floating_point_emulation::tfloat<T, S> f(-1764);
		f /= 42.;
		BOOST_CHECK_EQUAL(f, -42);
		BOOST_CHECK_EQUAL(f, -42.);
	}



	{
		floating_point_emulation::tfloat<T, S> f(-1764), g(-42);
		f /= g;
		BOOST_CHECK_EQUAL(f, 42);
		BOOST_CHECK_EQUAL(f, 42.);
	}

	{
		floating_point_emulation::tfloat<T, S> f(-1764);
		f /= -42;
		BOOST_CHECK_EQUAL(f, 42);
		BOOST_CHECK_EQUAL(f, 42.);
	}

	{
		floating_point_emulation::tfloat<T, S> f(-1764);
		f /= -42.;
		BOOST_CHECK_EQUAL(f, 42);
		BOOST_CHECK_EQUAL(f, 42.);
	}



	{
		floating_point_emulation::tfloat<T, S> f(1764), g(-42);
		f /= g;
		BOOST_CHECK_EQUAL(f, -42);
		BOOST_CHECK_EQUAL(f, -42.);
	}

	{
		floating_point_emulation::tfloat<T, S> f(1764);
		f /= -42;
		BOOST_CHECK_EQUAL(f, -42);
		BOOST_CHECK_EQUAL(f, -42.);
	}

	{
		floating_point_emulation::tfloat<T, S> f(1764);
		f /= -42.;
		BOOST_CHECK_EQUAL(f, -42);
		BOOST_CHECK_EQUAL(f, -42.);
	}



	{
		floating_point_emulation::tfloat<T, S> f(3.125), g(2.5);
		f /= g;
		BOOST_CHECK_EQUAL(f, 1.25);
	}

	{
		floating_point_emulation::tfloat<T, S> f(2.5);
		f /= 2;
		BOOST_CHECK_EQUAL(f, 1.25);
	}

	{
		floating_point_emulation::tfloat<T, S> f(3.125);
		f /= 2.5;
		BOOST_CHECK_EQUAL(f, 1.25);
	}

	{
		floating_point_emulation::tfloat<T, S> f(-3.125), g(2.5);
		f /= g;
		BOOST_CHECK_EQUAL(f, -1.25);
	}

	{
		floating_point_emulation::tfloat<T, S> f(2.5);
		f /= -2;
		BOOST_CHECK_EQUAL(f, -1.25);
	}

	{
		floating_point_emulation::tfloat<T, S> f(-3.125);
		f /= -2.5;
		BOOST_CHECK_EQUAL(f, 1.25);
	}
}

template<class T, unsigned S>
static void
test_add()
{
	{
		floating_point_emulation::tfloat<T, S> f;
		f += 42;
		BOOST_CHECK_EQUAL(f, 42);
		BOOST_CHECK_EQUAL(f, 42.);
	}

	{
		floating_point_emulation::tfloat<T, S> f;
		f += 42.;
		BOOST_CHECK_EQUAL(f, 42);
		BOOST_CHECK_EQUAL(f, 42.);
	}

	{
		floating_point_emulation::tfloat<T, S> f;
		f += 42.4;
		BOOST_CHECK_EQUAL(f, 42.4);
	}

	{
		floating_point_emulation::tfloat<T, S> f, g(42);
		f += g;
		BOOST_CHECK_EQUAL(f, 42);
		BOOST_CHECK_EQUAL(f, 42.);
	}

	{
		floating_point_emulation::tfloat<T, S> f, g(42.42);
		f += g;
		BOOST_CHECK_NE(f, 42);
		BOOST_CHECK_EQUAL(f, 42.42);
	}



	{
		floating_point_emulation::tfloat<T, S> f(-142);
		f += 42;
		BOOST_CHECK_EQUAL(f, -100);
		BOOST_CHECK_EQUAL(f, -100.);
	}

	{
		floating_point_emulation::tfloat<T, S> f(-142);
		f += 42.;
		BOOST_CHECK_EQUAL(f, -100);
		BOOST_CHECK_EQUAL(f, -100.);
	}

	{
		floating_point_emulation::tfloat<T, S> f(-142), g(42);
		f += g;
		BOOST_CHECK_EQUAL(f, -100);
		BOOST_CHECK_EQUAL(f, -100.);
	}

	{
		floating_point_emulation::tfloat<T, S> f(-100), g(-42.4);
		f += g;
		BOOST_CHECK_NE(f, -142);
		BOOST_CHECK_EQUAL(f, -142.4);
	}
}

template<class T, unsigned S>
static void
test_subtract()
{
	{
		floating_point_emulation::tfloat<T, S> f;
		f -= 42;
		BOOST_CHECK_EQUAL(f, -42);
		BOOST_CHECK_EQUAL(f, -42.);
	}

	{
		floating_point_emulation::tfloat<T, S> f;
		f += -42.;
		BOOST_CHECK_EQUAL(f, -42);
		BOOST_CHECK_EQUAL(f, -42.);
	}

	{
		floating_point_emulation::tfloat<T, S> f;
		f -= 42.4;
		BOOST_CHECK_EQUAL(f, -42.4);
	}

	{
		floating_point_emulation::tfloat<T, S> f, g(42);
		f -= g;
		BOOST_CHECK_EQUAL(f, -42);
		BOOST_CHECK_EQUAL(f, -42.);
	}

	{
		floating_point_emulation::tfloat<T, S> f, g(42.42);
		f -= g;
		BOOST_CHECK_NE(f, -42);
		BOOST_CHECK_EQUAL(f, -42.42);
	}



	{
		floating_point_emulation::tfloat<T, S> f(-142);
		f -= -42;
		BOOST_CHECK_EQUAL(f, -100);
		BOOST_CHECK_EQUAL(f, -100.);
	}

	{
		floating_point_emulation::tfloat<T, S> f(-142);
		f -= -42.;
		BOOST_CHECK_EQUAL(f, -100);
		BOOST_CHECK_EQUAL(f, -100.);
	}

	{
		floating_point_emulation::tfloat<T, S> f(-142), g(42);
		f -= -g;
		BOOST_CHECK_EQUAL(f, -100);
		BOOST_CHECK_EQUAL(f, -100.);
	}

	{
		floating_point_emulation::tfloat<T, S> f(-100), g(-42.4);
		f -= -g;
		BOOST_CHECK_NE(f, -142);
		BOOST_CHECK_EQUAL(f, -142.4);
	}
}

template<class T, unsigned S>
static void
test_less_than()
{
	{
		floating_point_emulation::tfloat<T, S> f(42), g(43);
		BOOST_CHECK_LT(f, g);
	}



	{
		floating_point_emulation::tfloat<T, S> f(42), g(43);
		BOOST_CHECK_EQUAL(f < g, true);
		BOOST_CHECK_EQUAL(g < f, false);
	}

	{
		floating_point_emulation::tfloat<T, S> f(42), g(43.);
		BOOST_CHECK_EQUAL(f < g, true);
		BOOST_CHECK_EQUAL(g < f, false);
	}

	{
		floating_point_emulation::tfloat<T, S> f(42.), g(43);
		BOOST_CHECK_EQUAL(f < g, true);
		BOOST_CHECK_EQUAL(g < f, false);
	}

	{
		floating_point_emulation::tfloat<T, S> f(42.), g(43.);
		BOOST_CHECK_EQUAL(f < g, true);
		BOOST_CHECK_EQUAL(g < f, false);
	}

	{
		floating_point_emulation::tfloat<T, S> f(42), g(42);
		BOOST_CHECK_EQUAL(f < g, false);
		BOOST_CHECK_EQUAL(g < f, false);
	}



	{
		floating_point_emulation::tfloat<T, S> f(42), g(42.);
		BOOST_CHECK_EQUAL(f < g, false);
		BOOST_CHECK_EQUAL(g < f, false);
	}

	{
		floating_point_emulation::tfloat<T, S> f(42.), g(42);
		BOOST_CHECK_EQUAL(f < g, false);
		BOOST_CHECK_EQUAL(g < f, false);
	}

	{
		floating_point_emulation::tfloat<T, S> f(42.), g(42.);
		BOOST_CHECK_EQUAL(f < g, false);
		BOOST_CHECK_EQUAL(g < f, false);
	}
}

template<class T, unsigned S>
static void
test_less_than_equal_to()
{
	{
		floating_point_emulation::tfloat<T, S> f(42), g(43);
		BOOST_CHECK_LE(f, g);
	}



	{
		floating_point_emulation::tfloat<T, S> f(42), g(43);
		BOOST_CHECK_EQUAL(f <= g, true);
		BOOST_CHECK_EQUAL(g <= f, false);
	}

	{
		floating_point_emulation::tfloat<T, S> f(42), g(43.);
		BOOST_CHECK_EQUAL(f <= g, true);
		BOOST_CHECK_EQUAL(g <= f, false);
	}

	{
		floating_point_emulation::tfloat<T, S> f(42.), g(43);
		BOOST_CHECK_EQUAL(f <= g, true);
		BOOST_CHECK_EQUAL(g <= f, false);
	}

	{
		floating_point_emulation::tfloat<T, S> f(42.), g(43.);
		BOOST_CHECK_EQUAL(f <= g, true);
		BOOST_CHECK_EQUAL(g <= f, false);
	}



	{
		floating_point_emulation::tfloat<T, S> f(42), g(42);
		BOOST_CHECK_EQUAL(f <= g, true);
		BOOST_CHECK_EQUAL(g <= f, true);
	}

	{
		floating_point_emulation::tfloat<T, S> f(42), g(42.);
		BOOST_CHECK_EQUAL(f <= g, true);
		BOOST_CHECK_EQUAL(g <= f, true);
	}

	{
		floating_point_emulation::tfloat<T, S> f(42.), g(42);
		BOOST_CHECK_EQUAL(f <= g, true);
		BOOST_CHECK_EQUAL(g <= f, true);
	}

	{
		floating_point_emulation::tfloat<T, S> f(42.), g(42.);
		BOOST_CHECK_EQUAL(f <= g, true);
		BOOST_CHECK_EQUAL(g <= f, true);
	}
}

template<class T, unsigned S>
static void
test_greater_than()
{
	{
		floating_point_emulation::tfloat<T, S> f(43), g(42);
		BOOST_CHECK_GT(f, g);
	}



	{
		floating_point_emulation::tfloat<T, S> f(43), g(42);
		BOOST_CHECK_EQUAL(f > g, true);
		BOOST_CHECK_EQUAL(g > f, false);
	}

	{
		floating_point_emulation::tfloat<T, S> f(43), g(42.);
		BOOST_CHECK_EQUAL(f > g, true);
		BOOST_CHECK_EQUAL(g > f, false);
	}

	{
		floating_point_emulation::tfloat<T, S> f(43.), g(42);
		BOOST_CHECK_EQUAL(f > g, true);
		BOOST_CHECK_EQUAL(g > f, false);
	}

	{
		floating_point_emulation::tfloat<T, S> f(43.), g(42.);
		BOOST_CHECK_EQUAL(f > g, true);
		BOOST_CHECK_EQUAL(g > f, false);
	}



	{
		floating_point_emulation::tfloat<T, S> f(42), g(42);
		BOOST_CHECK_EQUAL(f > g, false);
		BOOST_CHECK_EQUAL(g > f, false);
	}

	{
		floating_point_emulation::tfloat<T, S> f(42), g(42.);
		BOOST_CHECK_EQUAL(f > g, false);
		BOOST_CHECK_EQUAL(g > f, false);
	}

	{
		floating_point_emulation::tfloat<T, S> f(42.), g(42);
		BOOST_CHECK_EQUAL(f > g, false);
		BOOST_CHECK_EQUAL(g > f, false);
	}

	{
		floating_point_emulation::tfloat<T, S> f(42.), g(42.);
		BOOST_CHECK_EQUAL(f > g, false);
		BOOST_CHECK_EQUAL(g > f, false);
	}
}

template<class T, unsigned S>
static void
test_greater_than_equal_to()
{
	{
		floating_point_emulation::tfloat<T, S> f(43), g(42);
		BOOST_CHECK_GE(f, g);
	}



	{
		floating_point_emulation::tfloat<T, S> f(43), g(42);
		BOOST_CHECK_EQUAL(f >= g, true);
		BOOST_CHECK_EQUAL(g >= f, false);
	}

	{
		floating_point_emulation::tfloat<T, S> f(43), g(42.);
		BOOST_CHECK_EQUAL(f >= g, true);
		BOOST_CHECK_EQUAL(g >= f, false);
	}

	{
		floating_point_emulation::tfloat<T, S> f(43.), g(42);
		BOOST_CHECK_EQUAL(f >= g, true);
		BOOST_CHECK_EQUAL(g >= f, false);
	}

	{
		floating_point_emulation::tfloat<T, S> f(43.), g(42.);
		BOOST_CHECK_EQUAL(f >= g, true);
		BOOST_CHECK_EQUAL(g >= f, false);
	}



	{
		floating_point_emulation::tfloat<T, S> f(42), g(42);
		BOOST_CHECK_EQUAL(f >= g, true);
		BOOST_CHECK_EQUAL(g >= f, true);
	}

	{
		floating_point_emulation::tfloat<T, S> f(42), g(42.);
		BOOST_CHECK_EQUAL(f >= g, true);
		BOOST_CHECK_EQUAL(g >= f, true);
	}

	{
		floating_point_emulation::tfloat<T, S> f(42.), g(42);
		BOOST_CHECK_EQUAL(f >= g, true);
		BOOST_CHECK_EQUAL(g >= f, true);
	}

	{
		floating_point_emulation::tfloat<T, S> f(42.), g(42.);
		BOOST_CHECK_EQUAL(f >= g, true);
		BOOST_CHECK_EQUAL(g >= f, true);
	}
}

template<class T, unsigned S>
static void
test_equal_to()
{
	{
		floating_point_emulation::tfloat<T, S> f(42), g(42);
		BOOST_CHECK_EQUAL(f, g);
	}



	{
		floating_point_emulation::tfloat<T, S> f(42), g(42);
		BOOST_CHECK_EQUAL(f == g, true);

		BOOST_CHECK_EQUAL(f == 42, true);
		BOOST_CHECK_EQUAL(f == 42., true);

		BOOST_CHECK_EQUAL(f == 43, false);
		BOOST_CHECK_EQUAL(f == 43., false);

		BOOST_CHECK_EQUAL(42 == f, true);
		BOOST_CHECK_EQUAL(42. == f, true);

		BOOST_CHECK_EQUAL(43 == f, false);
		BOOST_CHECK_EQUAL(43. == f, false);
	}
}

template<class T, unsigned S>
static void
test_not_equal_to()
{
	{
		floating_point_emulation::tfloat<T, S> f(42), g(43);
		BOOST_CHECK_NE(f, g);
	}



	{
		floating_point_emulation::tfloat<T, S> f(43), g(42);
		BOOST_CHECK_EQUAL(f != g, true);

		BOOST_CHECK_EQUAL(f != 42, true);
		BOOST_CHECK_EQUAL(f != 42., true);

		BOOST_CHECK_EQUAL(f != 43, false);
		BOOST_CHECK_EQUAL(f != 43., false);

		BOOST_CHECK_EQUAL(42 != f, true);
		BOOST_CHECK_EQUAL(42. != f, true);

		BOOST_CHECK_EQUAL(43 != f, false);
		BOOST_CHECK_EQUAL(43. != f, false);
	}
}

template<class T, unsigned S>
static void
test_to_int()
{
	{
		floating_point_emulation::tfloat<T, S> f(42);
		BOOST_CHECK_EQUAL(f.to_int(), 42);
		BOOST_CHECK_EQUAL(f.to_int(), 42.);
	}

	{
		floating_point_emulation::tfloat<T, S> f(-42);
		BOOST_CHECK_EQUAL(f.to_int(), -42);
		BOOST_CHECK_EQUAL(f.to_int(), -42.);
	}

	{
		floating_point_emulation::tfloat<T, S> f(42);
		BOOST_CHECK_EQUAL(f.to_int(), 42);
		BOOST_CHECK_EQUAL(f.to_int(), 42.);
	}

	{
		floating_point_emulation::tfloat<T, S> f(42.42);
		BOOST_CHECK_EQUAL(f.to_int(), 42);
		BOOST_CHECK_EQUAL(f.to_int(), 42.);
	}

	{
		floating_point_emulation::tfloat<T, S> f(-42.42);
		BOOST_CHECK_EQUAL(f.to_int(), -42);
		BOOST_CHECK_EQUAL(f.to_int(), -42.);
	}
}

template<class T, unsigned S>
static void
test_to_double()
{
	{
		floating_point_emulation::tfloat<T, S> f(42);
		BOOST_CHECK_EQUAL(f.to_double(), 42);
		BOOST_CHECK_EQUAL(f.to_double(), 42.);
	}

	{
		floating_point_emulation::tfloat<T, S> f(-42);
		BOOST_CHECK_EQUAL(f.to_double(), -42);
		BOOST_CHECK_EQUAL(f.to_double(), -42.);
	}

	{
		floating_point_emulation::tfloat<T, S> f(42);
		BOOST_CHECK_EQUAL(f.to_double(), 42);
		BOOST_CHECK_EQUAL(f.to_double(), 42.);
	}

	{
		floating_point_emulation::tfloat<T, S> f(42.25), g(f.to_double());
		BOOST_CHECK_EQUAL(static_cast<int>(f.to_double()), 42);
		BOOST_CHECK_EQUAL(f.to_double(), 42.25);
		BOOST_CHECK_EQUAL(g, 42.25);
	}

	{
		floating_point_emulation::tfloat<T, S> f(-42.25), g(f.to_double());
		BOOST_CHECK_EQUAL(static_cast<int>(f.to_double()), -42);
		BOOST_CHECK_EQUAL(f.to_double(), -42.25);
		BOOST_CHECK_EQUAL(g, -42.25);
	}
}

template<class T, unsigned S>
static void
test_floor()
{
	{
		floating_point_emulation::tfloat<T, S> f(42.42);
		BOOST_CHECK_EQUAL(f.floor(), 42);
	}

	{
		floating_point_emulation::tfloat<T, S> f(-42.42);
		BOOST_CHECK_EQUAL(f.floor(), -43);
	}
}

template<class T, unsigned S>
static void
test_divide_range_optimizations()
{

	/* Basic test for division. */
	{
		floating_point_emulation::tfloat<T, S> f(0), g(2);
		f /= g;
		BOOST_CHECK_EQUAL(f, 0);
	}

	/*
	 * Test the entire positive range with the second last bit set.
	 *
	 * Dividing by an int so the divisor_low_bit will always be 8.
	 */

	for(int i = 1; i < 23; ++i) {
		floating_point_emulation::tfloat<T, S> f((1 << i) + 2. / 256.), g(2);
		f /= g;
		BOOST_CHECK_EQUAL(f, (1 << (i - 1)) + 1. / 256.);
	}

	/*
	 * Test the entire negative range with the second last bit set.
	 *
	 * Dividing by an int so the divisor_low_bit will always be 8.
	 */

	for(int i = 1; i < 23; ++i) {
		floating_point_emulation::tfloat<T, S> f(-((1 << i) + 2. / 256.)), g(2);
		f /= g;
		BOOST_CHECK_EQUAL(f, -((1 << (i - 1)) + 1. / 256.));
	}

	/* Test the maximum negative value. */

	{
		floating_point_emulation::tfloat<T, S> f(-(1 << 23)), g(2);
		f /= g;
		BOOST_CHECK_EQUAL(f, -(1 << 22));
	}

	/* Test the the positive range for all cases of divisor_low_bit. */

	for(int i = 1; i <= 8; ++i) {
		floating_point_emulation::tfloat<T, S> f((1 << 22) + (1 << (22 - i)));
		floating_point_emulation::tfloat<T, S> g(1. + 1. / (1 << i));
		f /= g;
		BOOST_CHECK_EQUAL(
				  f
				, ((1 << 22) + (1 << (22 - i))) / (1. + 1. / (1 << i)));
	}

	/* Test the the negative range for all cases of divisor_low_bit. */

	for(int i = 1; i <= 8; ++i) {
		floating_point_emulation::tfloat<T, S> f((1 << 22) + (1 << (22 - i)));
		floating_point_emulation::tfloat<T, S> g(- 1. - 1. / (1 << i));
		f /= g;
		BOOST_CHECK_EQUAL(
				  f
				, ((1 << 22) + (1 << (22 - i))) / (- 1. - 1. / (1 << i)));
	}
}

template<class T, unsigned S>
static void
test()
{
	test_create<T, S>();
	test_assign<T, S>();
	test_unary<T, S>();
	test_multiply<T, S>();
	test_divide<T, S>();
	test_add<T, S>();
	test_subtract<T, S>();
	test_less_than<T, S>();
	test_less_than_equal_to<T, S>();
	test_greater_than<T, S>();
	test_greater_than_equal_to<T, S>();
	test_equal_to<T, S>();
	test_not_equal_to<T, S>();
	test_to_int<T, S>();
	test_to_double<T, S>();
	test_floor<T, S>();

	test_divide_range_optimizations<T, S>();
}

BOOST_AUTO_TEST_CASE(test_floating_point_emulation)
{
//	asm("int3");
	test<double, 0>();
	test<double, 8>();
	test<Sint32, 8>();
	test<tfloat::value_type, tfloat::shift>();
}

/*
 * Instanciate some functions.
 *
 * This allows to check the assembly output of the function, which can be
 * useful to see what really happens.
 */

void
instanciate_idiv(
		  floating_point_emulation::tfloat<Sint32, 8>& f
		, const floating_point_emulation::tfloat<Sint32, 8> g);
void
instanciate_idiv(
		  floating_point_emulation::tfloat<Sint32, 8>& f
		, const floating_point_emulation::tfloat<Sint32, 8> g)
{
	f /= g;
}

