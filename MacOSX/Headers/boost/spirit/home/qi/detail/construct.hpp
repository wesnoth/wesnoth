//  Copyright (c) 2001-2008 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_CONSTRUCT_MAR_24_2007_0629PM)
#define BOOST_SPIRIT_CONSTRUCT_MAR_24_2007_0629PM

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once      // MS compatible compilers support #pragma once
#endif

#include <boost/spirit/home/qi/parse.hpp>
#include <boost/spirit/home/qi/numeric.hpp>

namespace boost { namespace spirit { namespace qi { namespace detail
{
    namespace construct_
    {
        ///////////////////////////////////////////////////////////////////////
        //  We provide overloads for the construct customization point for all
        //  built in types
        ///////////////////////////////////////////////////////////////////////
        template <typename Iterator>
        inline void
        construct(char& attr, Iterator const& first, Iterator const& last)
        {
            attr = *first;
        }

#if !defined(BOOST_NO_INTRINSIC_WCHAR_T)
        // wchar_t is intrinsic
        template <typename Iterator>
        inline void
        construct(wchar_t& attr, Iterator const& first, Iterator const& last)
        {
            attr = *first;
        }
        template <typename Iterator>
        inline void
        construct(unsigned short& attr, Iterator const& first,
            Iterator const& last)
        {
            Iterator first_ = first;
            parse(first_, last, ushort, attr);
        }
#else
        // is wchar_t is not an intrinsic type, treat wchar_t only
        template <typename Iterator>
        inline void
        construct(wchar_t& attr, Iterator const& first, Iterator const& last)
        {
            attr = *first;
        }
#endif

        template <typename Iterator>
        inline void
        construct(short& attr, Iterator const& first, Iterator const& last)
        {
            Iterator first_ = first;
            parse(first_, last, short_, attr);
        }

        template <typename Iterator>
        inline void
        construct(int& attr, Iterator const& first, Iterator const& last)
        {
            Iterator first_ = first;
            parse(first_, last, int_, attr);
        }
        template <typename Iterator>
        inline void
        construct(unsigned int& attr, Iterator const& first,
            Iterator const& last)
        {
            Iterator first_ = first;
            parse(first_, last, uint_, attr);
        }

        template <typename Iterator>
        inline void
        construct(long& attr, Iterator const& first, Iterator const& last)
        {
            Iterator first_ = first;
            parse(first_, last, long_, attr);
        }
        template <typename Iterator>
        inline void
        construct(unsigned long& attr, Iterator const& first,
            Iterator const& last)
        {
            Iterator first_ = first;
            parse(first_, last, ulong, attr);
        }

#ifdef BOOST_HAS_LONG_LONG
        template <typename Iterator>
        inline void
        construct(boost::long_long_type& attr, Iterator const& first,
            Iterator const& last)
        {
            Iterator first_ = first;
            parse(first_, last, long_long, attr);
        }
        template <typename Iterator>
        inline void
        construct(boost::ulong_long_type& attr, Iterator const& first,
            Iterator const& last)
        {
            Iterator first_ = first;
            parse(first_, last, ulong_long, attr);
        }
#endif

        template <typename Iterator>
        inline void
        construct(float& attr, Iterator const& first, Iterator const& last)
        {
            Iterator first_ = first;
            parse(first_, last, float_, attr);
        }

        template <typename Iterator>
        inline void
        construct(double& attr, Iterator const& first, Iterator const& last)
        {
            Iterator first_ = first;
            parse(first_, last, double_, attr);
        }

        template <typename Iterator>
        inline void
        construct(long double& attr, Iterator const& first,
            Iterator const& last)
        {
            Iterator first_ = first;
            parse(first_, last, long_double, attr);
        }
    }
    
}}}}

#endif
