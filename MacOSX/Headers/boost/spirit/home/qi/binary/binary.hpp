//  Copyright (c) 2001-2008 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_BINARY_MAY_08_2007_0808AM)
#define BOOST_SPIRIT_BINARY_MAY_08_2007_0808AM

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once      // MS compatible compilers support #pragma once
#endif

#include <boost/spirit/home/support/component.hpp>
#include <boost/spirit/home/support/detail/integer/endian.hpp>
#include <boost/spirit/home/support/attribute_of.hpp>
#include <boost/spirit/home/qi/domain.hpp>
#include <boost/spirit/home/qi/detail/assign_to.hpp>
#include <boost/spirit/home/qi/skip.hpp>

namespace boost { namespace spirit { namespace qi
{
    namespace detail
    {
        template <int bits>
        struct integer
        {
#ifdef BOOST_HAS_LONG_LONG
            BOOST_MPL_ASSERT_MSG(
                bits == 8 || bits == 16 || bits == 32 || bits == 64,
                not_supported_binary_size, ());
#else
            BOOST_MPL_ASSERT_MSG(
                bits == 8 || bits == 16 || bits == 32,
                not_supported_binary_size, ());
#endif
        };

        template <>
        struct integer<8>
        {
            typedef uint_least8_t type;
        };

        template <>
        struct integer<16>
        {
            typedef uint_least16_t type;
        };

        template <>
        struct integer<32>
        {
            typedef uint_least32_t type;
        };

#ifdef BOOST_HAS_LONG_LONG
        template <>
        struct integer<64>
        {
            typedef uint_least64_t type;
        };
#endif

        ///////////////////////////////////////////////////////////////////////
        template <boost::integer::endianness bits>
        struct what;

        template <>
        struct what<boost::integer::native>
        {
            static std::string is()
            {
                return "native-endian binary";
            }
        };

        template <>
        struct what<boost::integer::little>
        {
            static char const* is()
            {
                return "little-endian binary";
            }
        };

        template <>
        struct what<boost::integer::big>
        {
            static char const* is()
            {
                return "big-endian binary";
            }
        };
    }

    ///////////////////////////////////////////////////////////////////////////
    template <integer::endianness endian, int bits>
    struct any_binary_director
    {
        template <typename Component, typename Context, typename Iterator>
        struct attribute
        {
            typedef boost::integer::endian<
                endian, typename qi::detail::integer<bits>::type, bits
            > type;
        };

        template <
            typename Component
          , typename Iterator, typename Context
          , typename Skipper, typename Attribute>
        static bool parse(
            Component const&
          , Iterator& first, Iterator const& last
          , Context&, Skipper const& skipper
          , Attribute& attr)
        {
            qi::skip(first, last, skipper);

            typename
                traits::attribute_of<
                    qi::domain, Component, Context, Iterator>::type
            attr_;
            unsigned char* bytes = reinterpret_cast<unsigned char*>(&attr_);

            Iterator it = first;
            for (unsigned int i = 0; i < sizeof(attr_); ++i)
            {
                if (it == last)
                    return false;
                *bytes++ = *it++;
            }

            first = it;
            detail::assign_to(attr_, attr);
            return true;
        }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            return qi::detail::what<endian>::is();
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    template <integer::endianness endian, int bits>
    struct binary_lit_director
    {
        template <typename Component, typename Context, typename Iterator>
        struct attribute
        {
            typedef unused_type type;
        };

        template <
            typename Component
          , typename Iterator, typename Context
          , typename Skipper, typename Attribute>
        static bool parse(
            Component const& component
          , Iterator& first, Iterator const& last
          , Context&, Skipper const& skipper
          , Attribute& attr)
        {
            qi::skip(first, last, skipper);

            boost::integer::endian<
                endian, typename qi::detail::integer<bits>::type, bits
            > attr_ (fusion::at_c<0>(component.elements));
            unsigned char* bytes = reinterpret_cast<unsigned char*>(&attr_);

            Iterator it = first;
            for (unsigned int i = 0; i < sizeof(attr_); ++i)
            {
                if (it == last || *bytes++ != *it++)
                    return false;
            }

            first = it;
            detail::assign_to(attr_, attr);
            return true;
        }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            return qi::detail::what<endian>::is();
        }
    };

}}}

#endif
