//  Copyright (c) 2001-2008 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_KARMA_REAL_POLICIES_MAR_02_2007_0936AM)
#define BOOST_SPIRIT_KARMA_REAL_POLICIES_MAR_02_2007_0936AM

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once      // MS compatible compilers support #pragma once
#endif

#include <boost/spirit/home/support/char_class.hpp>
#include <boost/spirit/home/karma/generate.hpp>
#include <boost/spirit/home/karma/char.hpp>
#include <boost/spirit/home/karma/numeric/int.hpp>
#include <boost/config/no_tr1/cmath.hpp>
#include <boost/spirit/home/support/detail/math/fpclassify.hpp>

namespace boost { namespace spirit { namespace karma 
{
    ///////////////////////////////////////////////////////////////////////////
    //
    //  real_generator_policies, if you need special handling of your floating
    //  point numbers, just overload this policy class and use it as a template
    //  parameter to the karma::real_spec floating point specifier:
    //
    //      template <typename T>
    //      struct scientific_policy : karma::real_generator_policies<T>
    //      {
    //          //  we want the numbers always to be in scientific format
    //          static int floatfield(T n) { return scientific; }
    //      };
    //
    //      typedef 
    //          karma::real_spec<double, scientific_policy<double> > 
    //      science_type;
    //
    //      karma::generate(sink, science_type(), 1.0); // will output: 1.0e00
    //
    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    struct real_generator_policies
    {
        ///////////////////////////////////////////////////////////////////////
        //  Specifies, which representation type to use during output 
        //  generation.
        ///////////////////////////////////////////////////////////////////////
        enum fmtflags 
        {
            scientific = 0,   // Generate floating-point values in scientific 
                              // format (with an exponent field).
            fixed = 1         // Generate floating-point values in fixed-point 
                              // format (with no exponent field). 
        };
        
        ///////////////////////////////////////////////////////////////////////
        //  The default behavior is to not to require generating a sign. If 
        //  'force_sign' is specified as true, then all generated numbers will 
        //  have a sign ('+' or '-', zeros will have a space instead of a sign)
        ///////////////////////////////////////////////////////////////////////
        static bool const force_sign = false;
        
        ///////////////////////////////////////////////////////////////////////
        //  The 'trailing_zeros' flag instructs the floating point generator to 
        //  emit trailing zeros up to the required precision digits.
        ///////////////////////////////////////////////////////////////////////
        static bool const trailing_zeros = false;
        
        ///////////////////////////////////////////////////////////////////////
        //  Decide, which representation type to use in the generated output.
        //
        //  By default all numbers having an absolute value of zero or in 
        //  between 0.001 and 100000 will be generated using the fixed format, 
        //  all others will be generated using the scientific representation.
        //
        //  The trailing_zeros flag can be used to force the output of trailing 
        //  zeros in the fractional part up to the number of digits returned by 
        //  the precision() member function. The default is not to generate 
        //  the trailing zeros.
        //  
        //      n     The floating point number to output. This can be used to 
        //            adjust the formatting flags depending on the value of 
        //            this number.
        ///////////////////////////////////////////////////////////////////////
        static int 
        floatfield(T n)
        {
            if (detail::is_zero(n))
                return fixed;

            T abs_n = detail::absolute_value(n);
            return (abs_n >= 1e5 || abs_n < 1e-3) ? scientific : fixed;
        }
        
        ///////////////////////////////////////////////////////////////////////
        //  The 'fractional_precision' constant specifies the default number of 
        //  digits to generate for the fractional part of a floating point 
        //  number. This is used by this (default) policies implementation 
        //  only. If you need another fractional precision you'll have to 
        //  overload the precision function below.
        //  
        //  Note: The actual number of digits for a floating point number is 
        //        determined by the precision() function below. This allows to
        //        have different precisions depending on the value of the
        //        floating point number.
        ///////////////////////////////////////////////////////////////////////
        static unsigned int const fractional_precision = 3;
        
        ///////////////////////////////////////////////////////////////////////
        //  Return the maximum number of decimal digits to generate in the 
        //  fractional part of the output.
        //  
        //      n     The floating point number to output. This can be used to 
        //            adjust the required precision depending on the value of 
        //            this number. If the trailing zeros flag is specified the
        //            fractional part of the output will be 'filled' with 
        //            zeros, if appropriate
        //
        //  Note:     If the trailing_zeros flag is not in effect additional
        //            comments apply. See the comment for the fraction_part()
        //            function below.
        ///////////////////////////////////////////////////////////////////////
        static unsigned int
        precision(T)
        {
            // generate max. 'fractional_precision' fractional digits
            return fractional_precision;   
        }

        ///////////////////////////////////////////////////////////////////////
        //  Generate the integer part of the number.
        //
        //      sink  The output iterator to use for generation
        //      n     The absolute value of the integer part of the floating 
        //            point number to convert (always non-negative). 
        //      sign  The sign of the overall floating point number to convert.
        ///////////////////////////////////////////////////////////////////////
        template <bool ForceSign, typename OutputIterator>
        static bool
        integer_part (OutputIterator& sink, T n, bool sign)
        {
            return sign_inserter<ForceSign>::call(
                        sink, detail::is_zero(n), sign) &&
                   int_inserter<10>::call(sink, n);
        }
        
        ///////////////////////////////////////////////////////////////////////
        //  Generate the decimal point.
        //
        //      sink  The output iterator to use for generation
        //      n     The fractional part of the floating point number to 
        //            convert. Note that this number is scaled such, that 
        //            it represents the number of units which correspond
        //            to the value returned from the precision() function 
        //            earlier. I.e. a fractional part of 0.01234 is
        //            represented as 1234 when the 'Precision' is 5.
        //
        //            This is given to allow to decide, whether a decimal point
        //            has to be generated at all.
        //
        //  Note:     If the trailing_zeros flag is not in effect additional
        //            comments apply. See the comment for the fraction_part()
        //            function below.
        ///////////////////////////////////////////////////////////////////////
        template <typename OutputIterator>
        static bool
        dot (OutputIterator& sink, T)
        {
            return char_inserter<>::call(sink, '.');  // generate the dot by default 
        }
        
        ///////////////////////////////////////////////////////////////////////
        //  Generate the fractional part of the number.
        //
        //      sink  The output iterator to use for generation
        //      n     The fractional part of the floating point number to 
        //            convert. This number is scaled such, that it represents 
        //            the number of units which correspond to the 'Precision'. 
        //            I.e. a fractional part of 0.01234 is represented as 1234 
        //            when the 'precision_' parameter is 5.
        //
        //  Note: If the trailing_zeros flag is not returned from the 
        //        floatfield() function, the 'precision_' parameter will have 
        //        been corrected from the value the precision() function 
        //        returned earlier (defining the maximal number of fractional 
        //        digits) in the sense, that it takes into account trailing 
        //        zeros. I.e. a floating point number 0.0123 and a value of 5 
        //        returned from precision() will result in:
        //
        //        trailing_zeros is not specified:
        //            n           123
        //            precision_  4
        //
        //        trailing_zeros is specified:
        //            n           1230
        //            precision_  5
        //
        ///////////////////////////////////////////////////////////////////////
        template <typename OutputIterator>
        static bool
        fraction_part (OutputIterator& sink, T n, unsigned precision_)
        {
            // allow for ADL to find the correct overload for floor and log10
            using namespace std;

            // The following is equivalent to:
            //    generate(sink, right_align(precision, '0')[ulong], n);
            // but it's spelled out to avoid inter-modular dependencies.
            
            T digits = (detail::is_zero(n) ? 0 : floor(log10(n))) + 1;
            bool r = true;
            for (/**/; r && digits < precision_; digits = digits + 1)
                r = char_inserter<>::call(sink, '0');
            return r && int_inserter<10>::call(sink, n);
        }

        ///////////////////////////////////////////////////////////////////////
        //  Generate the exponential part of the number (this is called only 
        //  if the floatfield() function returned the 'scientific' flag).
        //
        //      sink  The output iterator to use for generation
        //      n     The (signed) exponential part of the floating point 
        //            number to convert. 
        //
        //  The Tag template parameter is either of the type unused_type or
        //  describes the character class and conversion to be applied to any 
        //  output possibly influenced by either the lower[...] or upper[...] 
        //  directives.
        ///////////////////////////////////////////////////////////////////////
        template <typename Tag, typename OutputIterator>
        static bool
        exponent (OutputIterator& sink, T n)
        {
            T abs_n = detail::absolute_value(n);
            bool r = char_inserter<Tag>::call(sink, 'e') &&
                     sign_inserter<false>::call(
                          sink, detail::is_zero(n), detail::is_negative(n));

            // the C99 Standard requires at least two digits in the exponent
            if (r && abs_n < 10)
                r = char_inserter<Tag>::call(sink, '0');
            return r && int_inserter<10>::call(sink, abs_n);
        }

        ///////////////////////////////////////////////////////////////////////
        //  Print the textual representations for non-normal floats (NaN and 
        //  Inf)
        //
        //      sink      The output iterator to use for generation
        //      n         The (signed) floating point number to convert. 
        //      
        //  The Tag template parameter is either of the type unused_type or
        //  describes the character class and conversion to be applied to any 
        //  output possibly influenced by either the lower[...] or upper[...] 
        //  directives.
        //
        //  Note: These functions get called only if fpclassify() returned 
        //        FP_INFINITY or FP_NAN.
        ///////////////////////////////////////////////////////////////////////
        template <bool ForceSign, typename Tag, typename OutputIterator>
        static bool 
        nan (OutputIterator& sink, T n)
        {
            return sign_inserter<ForceSign>::call(
                        sink, false, detail::is_negative(n)) &&
                   string_inserter<Tag>::call(sink, "nan");
        }

        template <bool ForceSign, typename Tag, typename OutputIterator>
        static bool 
        inf (OutputIterator& sink, T n)
        {
            return sign_inserter<ForceSign>::call(
                        sink, false, detail::is_negative(n)) &&
                   string_inserter<Tag>::call(sink, "inf");
        }
    };
    
}}}

#endif // defined(BOOST_SPIRIT_KARMA_REAL_POLICIES_MAR_02_2007_0936AM)
