//  Copyright (c) 2001-2008 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_KARMA_NUMERIC_UTILS_FEB_23_2007_0841PM)
#define BOOST_SPIRIT_KARMA_NUMERIC_UTILS_FEB_23_2007_0841PM

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once      // MS compatible compilers support #pragma once
#endif

#include <boost/config/no_tr1/cmath.hpp>
#include <limits>

#include <boost/type_traits/is_integral.hpp>
#include <boost/spirit/home/support/char_class.hpp>
#include <boost/spirit/home/support/iso8859_1.hpp>
#include <boost/spirit/home/support/ascii.hpp>
#include <boost/spirit/home/support/unused.hpp>
#include <boost/spirit/home/karma/detail/generate_to.hpp>
#include <boost/spirit/home/karma/detail/string_generate.hpp>
#include <boost/spirit/home/support/detail/math/fpclassify.hpp>
#include <boost/spirit/home/support/detail/math/signbit.hpp>

///////////////////////////////////////////////////////////////////////////////
//
//  The value BOOST_KARMA_NUMERICS_LOOP_UNROLL specifies, how to unroll the 
//  integer string generation loop (see below).
//
//      Set the value to some integer in between 0 (no unrolling) and the 
//      largest expected generated integer string length (complete unrolling). 
//      If not specified, this value defaults to 6.
//
///////////////////////////////////////////////////////////////////////////////
#if !defined(BOOST_KARMA_NUMERICS_LOOP_UNROLL)
#define BOOST_KARMA_NUMERICS_LOOP_UNROLL 6
#endif

#if BOOST_KARMA_NUMERICS_LOOP_UNROLL < 0 
#error "Please set the BOOST_KARMA_NUMERICS_LOOP_UNROLL to a positive value!"
#endif

namespace boost { namespace spirit { namespace karma { 

    namespace detail 
    {
        ///////////////////////////////////////////////////////////////////////
        //
        //  return the absolute value from a given number, avoiding over- and 
        //  underflow
        //
        ///////////////////////////////////////////////////////////////////////
        inline unsigned short absolute_value (short n)
        {
            return (n >= 0) ? n : (unsigned short)(-n);
        }
        
        inline unsigned int absolute_value (int n)
        {
            return (n >= 0) ? n : (unsigned int)(-n);
        }

        inline unsigned long absolute_value (long n)
        {
            return (n >= 0) ? n : (unsigned long)(-n);
        }

#ifdef BOOST_HAS_LONG_LONG
        inline boost::ulong_long_type absolute_value (boost::long_long_type n)
        {
            return (n >= 0) ? n : (boost::ulong_long_type)(-n);
        }
#endif
        
        inline float absolute_value (float n)
        {
            return boost::math::signbit(n) ? boost::math::changesign(n) : n;
        }
        
        inline double absolute_value (double n)
        {
            return boost::math::signbit(n) ? boost::math::changesign(n) : n;
        }
        
        inline long double absolute_value (long double n)
        {
            return boost::math::signbit(n) ? boost::math::changesign(n) : n;
        }
        
        template <typename T>
        inline T absolute_value (T n)
        {
            using namespace std;
            return fabs(n);
        }

        ///////////////////////////////////////////////////////////////////////
        inline bool is_negative(float n) 
        { 
            return boost::math::signbit(n); 
        }
        
        inline bool is_negative(double n) 
        { 
            return boost::math::signbit(n); 
        }
        
        inline bool is_negative(long double n) 
        { 
            return boost::math::signbit(n); 
        }
        
        template <typename T>
        inline bool is_negative(T n)
        {
            return (n < 0) ? true : false;
        }
        
        ///////////////////////////////////////////////////////////////////////
        inline bool is_zero(float n) 
        { 
            return boost::math::fpclassify(n) == FP_ZERO; 
        }
        
        inline bool is_zero(double n) 
        { 
            return boost::math::fpclassify(n) == FP_ZERO; 
        }
        
        inline bool is_zero(long double n) 
        { 
            return boost::math::fpclassify(n) == FP_ZERO; 
        }
        
        template <typename T>
        inline bool is_zero(T n)
        {
            return (n == 0) ? true : false;
        }
        
        ///////////////////////////////////////////////////////////////////////
        struct cast_to_long
        {
            static long call(float n, mpl::false_)
            {
                return static_cast<long>(std::floor(n));
            }
            
            static long call(double n, mpl::false_)
            {
                return static_cast<long>(std::floor(n));
            }
            
            static long call(long double n, mpl::false_)
            {
                return static_cast<long>(std::floor(n));
            }
            
            template <typename T>
            static long call(T n, mpl::false_)
            {
                // allow for ADL to find the correct overload for floor and 
                // lround
                using namespace std;
                return lround(floor(n));
            }

            template <typename T>
            static long call(T n, mpl::true_)
            {
                return static_cast<long>(n);
            }

            template <typename T>
            static long call(T n)
            {
                return call(n, mpl::bool_<is_integral<T>::value>());
            }
        };
        
        ///////////////////////////////////////////////////////////////////////
        struct round_to_long
        {
            static long call(float n, mpl::false_)
            {
                return static_cast<long>(std::floor(n + 0.5f));
            }
            
            static long call(double n, mpl::false_)
            {
                return static_cast<long>(std::floor(n + 0.5));
            }
            
            static long call(long double n, mpl::false_)
            {
                return static_cast<long>(std::floor(n + 0.5l));
            }
            
            template <typename T>
            static long call(T n, mpl::false_)
            {
                using namespace std;
                return lround(n);
            }

            template <typename T>
            static long call(T n, mpl::true_)
            {
                return static_cast<long>(n);
            }

            template <typename T>
            static long call(T n)
            {
                return call(n, mpl::bool_<is_integral<T>::value>());
            }
        };
        
        ///////////////////////////////////////////////////////////////////////
        //
        //  Traits class for radix specific number conversion
        //
        //      Convert a digit from binary representation to character 
        //      representation:
        //
        //          static int digit(unsigned n);
        //
        ///////////////////////////////////////////////////////////////////////
        template<unsigned Radix, typename Tag>
        struct radix_traits;

        // Binary
        template<typename Tag>
        struct radix_traits<2, Tag>
        {
            static int digit(unsigned n)
            {
                return n + '0';
            }
        };

        // Octal
        template<typename Tag>
        struct radix_traits<8, Tag>
        {
            static int digit(unsigned n)
            {
                return n + '0';
            }
        };

        // Decimal 
        template<typename Tag>
        struct radix_traits<10, Tag>
        {
            static int digit(unsigned n)
            {
                return n + '0';
            }
        };

        // Hexadecimal, lower case
        template<>
        struct radix_traits<16, unused_type>
        {
            static int digit(unsigned n)
            {
                if (n <= 9)
                    return n + '0';
                return n - 10 + 'a';
            }
        };

        // Hexadecimal, upper case
        template<typename Tag>
        struct radix_traits<16, Tag>
        {
            typedef typename Tag::char_set char_set;
            typedef typename Tag::char_class char_class_;

            static int digit(unsigned n)
            {
                if (n <= 9)
                    return n + '0';

                using spirit::char_class::convert;
                return convert<char_set>::to(char_class_(), n - 10 + 'a');
            }
        };

        ///////////////////////////////////////////////////////////////////////
        template <unsigned Radix>
        struct divide
        {
            template <typename T>
            static T call(T& n, mpl::true_)
            {
                return n / Radix;
            }
            
            template <typename T>
            static T call(T& n, mpl::false_)
            {
                // Allow ADL to find the correct overload for floor
                using namespace std; 
                return floor(n / Radix);
            }
            
            template <typename T>
            static T call(T& n)
            {
                return call(n, mpl::bool_<is_integral<T>::value>());
            }
        };
        
        ///////////////////////////////////////////////////////////////////////
        template <unsigned Radix>
        struct remainder
        {
            template <typename T>
            static long call(T n, mpl::true_)
            {
                // this cast is safe since we know the result is not larger 
                // than Radix
                return static_cast<long>(n % Radix);
            }
            
            template <typename T>
            static long call(T n, mpl::false_)
            {
                // Allow ADL to find the correct overload for fmod
                using namespace std; 
                return cast_to_long::call(fmod(n, T(Radix)));
            }
            
            template <typename T>
            static long call(T n)
            {
                return call(n, mpl::bool_<is_integral<T>::value>());
            }
        };
        
    }   // namespace detail
    
    ///////////////////////////////////////////////////////////////////////////
    //
    //  The int_inserter template takes care of the integer to string 
    //  conversion. If specified, the loop is unrolled for better performance.
    //
    //      Set the value BOOST_KARMA_NUMERICS_LOOP_UNROLL to some integer in 
    //      between 0 (no unrolling) and the largest expected generated integer 
    //      string length (complete unrolling). 
    //      If not specified, this value defaults to 6.
    //
    ///////////////////////////////////////////////////////////////////////////
#define BOOST_KARMA_NUMERICS_INNER_LOOP_PREFIX(z, x, data)                    \
        if (!detail::is_zero(n)) {                                            \
            int ch = radix_type::digit(remainder_type::call(n));              \
            n = divide_type::call(n);                                         \
    /**/

#define BOOST_KARMA_NUMERICS_INNER_LOOP_SUFFIX(z, x, data)                    \
            *sink = ch;                                                       \
            ++sink;                                                           \
        }                                                                     \
    /**/

    template <unsigned Radix, typename Tag = unused_type>
    struct int_inserter
    {
        typedef detail::radix_traits<Radix, Tag> radix_type;
        typedef detail::divide<Radix> divide_type;
        typedef detail::remainder<Radix> remainder_type;
        
        //  Common code for integer string representations
        template <typename OutputIterator, typename T>
        static bool
        call(OutputIterator& sink, T n)
        {
            // remainder_type::call returns n % Radix
            int ch = radix_type::digit(remainder_type::call(n));
            n = divide_type::call(n);

            BOOST_PP_REPEAT(
                BOOST_KARMA_NUMERICS_LOOP_UNROLL,
                BOOST_KARMA_NUMERICS_INNER_LOOP_PREFIX, _);

            if (!detail::is_zero(n)) 
                call(sink, n);

            BOOST_PP_REPEAT(
                BOOST_KARMA_NUMERICS_LOOP_UNROLL,
                BOOST_KARMA_NUMERICS_INNER_LOOP_SUFFIX, _);

            *sink = ch;
            ++sink;
            return true;
        }
    };

#undef BOOST_KARMA_NUMERICS_INNER_LOOP_PREFIX
#undef BOOST_KARMA_NUMERICS_INNER_LOOP_SUFFIX

    ///////////////////////////////////////////////////////////////////////////
    //
    //  The sign_inserter template generates a sign for a given numeric value.
    //
    //    The parameter ForceSign allows to generate a sign even for positive  
    //    numbers.
    //
    ///////////////////////////////////////////////////////////////////////////
    template <bool ForceSign>
    struct sign_inserter
    {
        template <typename OutputIterator>
        static bool
        call(OutputIterator& sink, bool /*is_zero*/, bool is_negative)
        {
            // generate a sign for negative numbers only
            if (is_negative) {
                *sink = '-';
                ++sink;
            }
            return true;
        }
    };
    
    template <>
    struct sign_inserter<true>
    {
        template <typename OutputIterator>
        static bool
        call(OutputIterator& sink, bool is_zero, bool is_negative)
        {
            // generate a sign for all numbers except zero
            if (!is_zero) 
                *sink = is_negative ? '-' : '+';
            else 
                *sink = ' ';
                
            ++sink;
            return true;
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    //  These are helper functions for the real policies allowing to generate
    //  a single character and a string
    ///////////////////////////////////////////////////////////////////////////
    template <typename Tag = unused_type>
    struct char_inserter
    {
        template <typename OutputIterator, typename Char>
        static bool
        call(OutputIterator& sink, Char c)
        {
            return detail::generate_to(sink, c, Tag());
        }
    };
    
    template <typename Tag = unused_type>
    struct string_inserter
    {
        template <typename OutputIterator, typename String>
        static bool
        call(OutputIterator& sink, String str)
        {
            return detail::string_generate(sink, str, Tag());
        }
    };
    
    ///////////////////////////////////////////////////////////////////////////
    //
    //  The real_inserter template takes care of the floating point number to 
    //  string conversion. The RealPolicies template parameter is used to allow
    //  customization of the formatting process
    //
    ///////////////////////////////////////////////////////////////////////////
    template <typename T, typename RealPolicies, typename Tag = unused_type>
    struct real_inserter    
    {
        enum { force_sign = RealPolicies::force_sign };
        
        template <typename OutputIterator>
        static bool
        call (OutputIterator& sink, float n, RealPolicies const& p)
        {
            int fpclass = boost::math::fpclassify(n);
            if (FP_NAN == fpclass)
                return RealPolicies::template nan<force_sign, Tag>(sink, n);
            else if (FP_INFINITE == fpclass)
                return RealPolicies::template inf<force_sign, Tag>(sink, n);
            return call_n(sink, n, p);
        }
        
        template <typename OutputIterator>
        static bool
        call (OutputIterator& sink, double n, RealPolicies const& p)
        {
            int fpclass = boost::math::fpclassify(n);
            if (FP_NAN == fpclass)
                return RealPolicies::template nan<force_sign, Tag>(sink, n);
            else if (FP_INFINITE == fpclass)
                return RealPolicies::template inf<force_sign, Tag>(sink, n);
            return call_n(sink, n, p);
        }
        
        template <typename OutputIterator>
        static bool
        call (OutputIterator& sink, long double n, RealPolicies const& p)
        {
            int fpclass = boost::math::fpclassify(n);
            if (FP_NAN == fpclass)
                return RealPolicies::template nan<force_sign, Tag>(sink, n);
            else if (FP_INFINITE == fpclass)
                return RealPolicies::template inf<force_sign, Tag>(sink, n);
            return call_n(sink, n, p);
        }
                        
        template <typename OutputIterator, typename U>
        static bool
        call (OutputIterator& sink, U n, RealPolicies const& p)
        {
            // we have no means of testing whether the number is normalized if
            // the type is not float, double or long double
            return call_n(sink, T(n), p);
        }
        
#if BOOST_WORKAROUND(BOOST_MSVC, >= 1400)  
# pragma warning(push)  
# pragma warning(disable: 4100)   // 'p': unreferenced formal parameter  
# pragma warning(disable: 4127)   // conditional expression is constant
#endif 

        ///////////////////////////////////////////////////////////////////////
        //  This is the workhorse behind the real generator
        ///////////////////////////////////////////////////////////////////////
        template <typename OutputIterator, typename U>
        static bool
        call_n (OutputIterator& sink, U n, RealPolicies const& p)
        {
        // prepare sign and get output format
            bool sign_val = false;
            int flags = p.floatfield(n);
            if (detail::is_negative(n)) 
            {
                n = -n;
                sign_val = true;
            }
            
        // The scientific representation requires the normalization of the 
        // value to convert.

            // allow for ADL to find the correct overloads for log10 et.al.
            using namespace std;
            
            U dim = 0;
            if (0 == (p.fixed & flags) && !detail::is_zero(n))
            {
                dim = log10(n);
                if (dim > 0)
                    n /= pow(U(10.0), (int)detail::round_to_long::call(dim));
                else if (n < 1.) 
                    n *= pow(U(10.0), (int)detail::round_to_long::call(-dim));
            }
            
        // prepare numbers (sign, integer and fraction part)
            unsigned precision = p.precision(n);
            U integer_part;
            U precexp = std::pow(10.0, (int)precision);
            U fractional_part = modf(n, &integer_part);
            
            fractional_part = floor(fractional_part * precexp + 0.5);
            if (fractional_part >= precexp) 
            {
                fractional_part -= precexp;
                integer_part += 1;    // handle rounding overflow
            }

        // if trailing zeros are to be omitted, normalize the precision and
        // fractional part
            U long_int_part = floor(integer_part);
            U long_frac_part = floor(fractional_part);
            if (!p.trailing_zeros)
            {
                if (0 != long_frac_part) {
                    // remove the trailing zeros
                    while (0 != precision && 
                           0 == detail::remainder<10>::call(long_frac_part)) 
                    {
                        long_frac_part = detail::divide<10>::call(long_frac_part);
                        --precision;
                    }
                }
                else {
                    // if the fractional part is zero, we don't need to output 
                    // any additional digits
                    precision = 0;
                }
            }
            
        // call the actual generating functions to output the different parts
            if (sign_val && detail::is_zero(long_int_part) && 
                detail::is_zero(long_frac_part))
            {
                sign_val = false;     // result is zero, no sign please
            }
            
        // generate integer part
            bool r = p.template integer_part<force_sign>(
                sink, long_int_part, sign_val);

        // generate decimal point
            r = r && p.dot(sink, long_frac_part);
            
        // generate fractional part with the desired precision
            r = r && p.fraction_part(sink, long_frac_part, precision);

            if (r && 0 == (p.fixed & flags)) {
                return p.template exponent<Tag>(sink, 
                    detail::round_to_long::call(dim));
            }
            return r;
        }

#if BOOST_WORKAROUND(BOOST_MSVC, >= 1400)  
# pragma warning(pop)  
#endif 

    };

}}}

#endif

