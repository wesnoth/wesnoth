/* boost random/uniform_int.hpp header file
 *
 * Copyright Jens Maurer 2000-2001
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org for most recent version including documentation.
 *
 * $Id: uniform_int.hpp 47233 2008-07-08 16:22:46Z steven_watanabe $
 *
 * Revision history
 *  2001-04-08  added min<max assertion (N. Becker)
 *  2001-02-18  moved to individual header files
 */

#ifndef BOOST_RANDOM_UNIFORM_INT_HPP
#define BOOST_RANDOM_UNIFORM_INT_HPP

#include <cassert>
#include <iostream>
#include <boost/config.hpp>
#include <boost/limits.hpp>
#include <boost/static_assert.hpp>
#include <boost/detail/workaround.hpp>
#include <boost/random/uniform_smallint.hpp>
#include <boost/random/detail/signed_unsigned_tools.hpp>
#include <boost/type_traits/make_unsigned.hpp>
#ifdef BOOST_NO_LIMITS_COMPILE_TIME_CONSTANTS
#include <boost/type_traits/is_float.hpp>
#endif

namespace boost {

// uniform integer distribution on [min, max]
template<class IntType = int>
class uniform_int
{
public:
  typedef IntType input_type;
  typedef IntType result_type;
  typedef typename make_unsigned<result_type>::type range_type;

  explicit uniform_int(IntType min_arg = 0, IntType max_arg = 9)
    : _min(min_arg), _max(max_arg)
  {
#ifndef BOOST_NO_LIMITS_COMPILE_TIME_CONSTANTS
    // MSVC fails BOOST_STATIC_ASSERT with std::numeric_limits at class scope
    BOOST_STATIC_ASSERT(std::numeric_limits<IntType>::is_integer);
#endif
    assert(min_arg <= max_arg);
    init();
  }

  result_type min BOOST_PREVENT_MACRO_SUBSTITUTION () const { return _min; }
  result_type max BOOST_PREVENT_MACRO_SUBSTITUTION () const { return _max; }
  void reset() { }
  
  // can't have member function templates out-of-line due to MSVC bugs
  template<class Engine>
  result_type operator()(Engine& eng)
  {
    typedef typename Engine::result_type base_result;
    // ranges are always unsigned
    typedef typename make_unsigned<base_result>::type base_unsigned;
    const base_result bmin = (eng.min)();
    const base_unsigned brange =
      random::detail::subtract<base_result>()((eng.max)(), (eng.min)());

    if(_range == 0) {
      return _min;    
    } else if(brange == _range) {
      // this will probably never happen in real life
      // basically nothing to do; just take care we don't overflow / underflow
      base_unsigned v = random::detail::subtract<base_result>()(eng(), bmin);
      return random::detail::add<base_unsigned, result_type>()(v, _min);
    } else if(brange < _range) {
      // use rejection method to handle things like 0..3 --> 0..4
      for(;;) {
        // concatenate several invocations of the base RNG
        // take extra care to avoid overflows
        range_type limit;
        if(_range == (std::numeric_limits<range_type>::max)()) {
          limit = _range/(range_type(brange)+1);
          if(_range % range_type(brange)+1 == range_type(brange))
            ++limit;
        } else {
          limit = (_range+1)/(range_type(brange)+1);
        }
        // We consider "result" as expressed to base (brange+1):
        // For every power of (brange+1), we determine a random factor
        range_type result = range_type(0);
        range_type mult = range_type(1);
        while(mult <= limit) {
          result += random::detail::subtract<base_result>()(eng(), bmin) * mult;
          mult *= range_type(brange)+range_type(1);
        }
        if(mult == limit)
          // _range+1 is an integer power of brange+1: no rejections required
          return result;
        // _range/mult < brange+1  -> no endless loop
        result += uniform_int<range_type>(0, _range/mult)(eng) * mult;
        if(result <= _range)
          return random::detail::add<range_type, result_type>()(result, _min);
      }
    } else {                   // brange > range
      if(brange / _range > 4 /* quantization_cutoff */ ) {
        // the new range is vastly smaller than the source range,
        // so quantization effects are not relevant
        return boost::uniform_smallint<result_type>(_min, _max)(eng);
      } else {
        // use rejection method to handle cases like 0..5 -> 0..4
        for(;;) {
          base_unsigned result =
	    random::detail::subtract<base_result>()(eng(), bmin);
          // result and range are non-negative, and result is possibly larger
          // than range, so the cast is safe
          if(result <= static_cast<base_unsigned>(_range))
            return random::detail::add<base_unsigned, result_type>()(result, _min);
        }
      }
    }
  }

#if !defined(BOOST_NO_OPERATORS_IN_NAMESPACE) && !defined(BOOST_NO_MEMBER_TEMPLATE_FRIENDS)
  template<class CharT, class Traits>
  friend std::basic_ostream<CharT,Traits>&
  operator<<(std::basic_ostream<CharT,Traits>& os, const uniform_int& ud)
  {
    os << ud._min << " " << ud._max;
    return os;
  }

  template<class CharT, class Traits>
  friend std::basic_istream<CharT,Traits>&
  operator>>(std::basic_istream<CharT,Traits>& is, uniform_int& ud)
  {
# if BOOST_WORKAROUND(_MSC_FULL_VER, BOOST_TESTED_AT(13102292)) && BOOST_MSVC == 1400
      return detail::extract_uniform_int(is, ud, ud.impl);
# else
   is >> std::ws >> ud._min >> std::ws >> ud._max;
    ud.init();
    return is;
# endif
  }
#endif

private:
  void init()
  {
    _range = random::detail::subtract<result_type>()(_max, _min);
  }

  // The result_type may be signed or unsigned, but the _range is always
  // unsigned.
  result_type _min, _max;
  range_type _range;
};

} // namespace boost

#endif // BOOST_RANDOM_UNIFORM_INT_HPP
