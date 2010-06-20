//  Copyright (c) 2001-2008 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_KARMA_NUMERIC_FWD_FEB_27_2007_1338PM)
#define BOOST_SPIRIT_KARMA_NUMERIC_FWD_FEB_27_2007_1338PM

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once      // MS compatible compilers support #pragma once
#endif

namespace boost { namespace spirit { namespace karma
{
    ///////////////////////////////////////////////////////////////////////////
    // forwards
    ///////////////////////////////////////////////////////////////////////////
    template <bool IsLiteral, typename T, unsigned Radix, bool ForceSign, 
        typename Tag = unused_type>
    struct int_generator;

    template <bool IsLiteral, typename T, unsigned Radix, bool ForceSign, 
        typename Tag = unused_type>
    struct uint_generator;

    ///////////////////////////////////////////////////////////////////////////
    template <typename T = int, unsigned Radix = 10, bool ForceSign = false>
    struct int_spec;
    
    template <typename T = unsigned int, unsigned Radix = 10, 
        bool ForceSign = false>
    struct uint_spec;
    
    ///////////////////////////////////////////////////////////////////////////
    template <bool IsLiteral, typename T, typename RealPolicies,
        typename Tag = unused_type>
    struct real_generator;
    
    template <typename T>
    struct real_generator_policies;

    ///////////////////////////////////////////////////////////////////////////
    template <typename T = double, 
        typename RealPolicies = real_generator_policies<T> >
    struct real_spec;
    
}}}   // namespace boost::spirit::karma

#endif


