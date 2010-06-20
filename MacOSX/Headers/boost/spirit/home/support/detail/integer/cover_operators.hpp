//  boost/integer/cover_operators.hpp ----------------------------------------//

//  (C) Copyright Darin Adler 2000

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//----------------------------------------------------------------------------//

#ifndef BOOST_INTEGER_COVER_OPERATORS_HPP
#define BOOST_INTEGER_COVER_OPERATORS_HPP

#include <boost/operators.hpp>
#include <iosfwd>

namespace boost
{
  namespace integer
  {

  // A class that adds integer operators to an integer cover class

    template <typename T, typename IntegerType>
    class cover_operators : boost::operators<T>
    {
      // The other operations take advantage of the type conversion that's
      // built into unary +.

      // Unary operations.
      friend IntegerType operator+(const T& x) { return x; }
      friend IntegerType operator-(const T& x) { return -+x; }
      friend IntegerType operator~(const T& x) { return ~+x; }
      friend IntegerType operator!(const T& x) { return !+x; }

      // The basic ordering operations.
      friend bool operator==(const T& x, IntegerType y) { return +x == y; }
      friend bool operator<(const T& x, IntegerType y) { return +x < y; }
      
      // The basic arithmetic operations.
      friend T& operator+=(T& x, IntegerType y) { return x = +x + y; }
      friend T& operator-=(T& x, IntegerType y) { return x = +x - y; }
      friend T& operator*=(T& x, IntegerType y) { return x = +x * y; }
      friend T& operator/=(T& x, IntegerType y) { return x = +x / y; }
      friend T& operator%=(T& x, IntegerType y) { return x = +x % y; }
      friend T& operator&=(T& x, IntegerType y) { return x = +x & y; }
      friend T& operator|=(T& x, IntegerType y) { return x = +x | y; }
      friend T& operator^=(T& x, IntegerType y) { return x = +x ^ y; }
      friend T& operator<<=(T& x, IntegerType y) { return x = +x << y; }
      friend T& operator>>=(T& x, IntegerType y) { return x = +x >> y; }
      
      // A few binary arithmetic operations not covered by operators base class.
      friend IntegerType operator<<(const T& x, IntegerType y) { return +x << y; }
      friend IntegerType operator>>(const T& x, IntegerType y) { return +x >> y; }
      
      // Auto-increment and auto-decrement can be defined in terms of the
      // arithmetic operations.
      friend T& operator++(T& x) { return x += 1; }
      friend T& operator--(T& x) { return x -= 1; }

  /// TODO: stream I/O needs to be templatized on the stream type, so will
  /// work with wide streams, etc.

      // Stream input and output.
      friend std::ostream& operator<<(std::ostream& s, const T& x)
        { return s << +x; }
      friend std::istream& operator>>(std::istream& s, T& x)
        {
          IntegerType i;
          if (s >> i)
            x = i;
          return s;
        }
    };
  } // namespace integer
} // namespace boost

#endif // BOOST_INTEGER_COVER_OPERATORS_HPP
