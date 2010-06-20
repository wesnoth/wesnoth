//  Copyright (c) 2001-2008 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_KARMA_BINARY_MAY_04_2007_0904AM)
#define BOOST_SPIRIT_KARMA_BINARY_MAY_04_2007_0904AM

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once      // MS compatible compilers support #pragma once
#endif

#include <boost/spirit/home/support/component.hpp>
#include <boost/spirit/home/support/detail/integer/endian.hpp>
#include <boost/spirit/home/support/attribute_of.hpp>
#include <boost/spirit/home/karma/domain.hpp>
#include <boost/spirit/home/karma/detail/generate_to.hpp>
#include <boost/spirit/home/karma/delimit.hpp>

namespace boost { namespace spirit { namespace karma
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
        template <typename Component, typename Context, typename Unused>
        struct attribute
        {
            typedef boost::integer::endian<
                endian, typename karma::detail::integer<bits>::type, bits
            > type;
        };

        template <typename Component, typename OutputIterator,
            typename Context, typename Delimiter, typename Parameter>
        static bool
        generate(Component const& /*component*/, OutputIterator& sink,
            Context& /*ctx*/, Delimiter const& d, Parameter const& param)
        {
            typename traits::attribute_of<
                karma::domain, Component, Context>::type p (param);
            unsigned char const* bytes =
                reinterpret_cast<unsigned char const*>(&p);

            for (unsigned int i = 0; i < sizeof(p); ++i)
                detail::generate_to(sink, *bytes++);

            karma::delimit(sink, d);           // always do post-delimiting
            return true;
        }

        // this any_byte_director has no parameter attached, it needs to have
        // been initialized from a direct literal
        template <typename Component, typename OutputIterator,
            typename Context, typename Delimiter>
        static bool
        generate(Component const&, OutputIterator&, Context&, Delimiter const&,
            unused_type)
        {
            BOOST_MPL_ASSERT_MSG(false,
                binary_generator_not_usable_without_attribute, ());
            return false;
        }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            return karma::detail::what<endian>::is();
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    template <integer::endianness endian, int bits>
    struct binary_lit_director
    {
        template <typename Component, typename Context, typename Unused>
        struct attribute
        {
            typedef unused_type type;
        };

        template <typename Component, typename OutputIterator,
            typename Context, typename Delimiter, typename Parameter>
        static bool
        generate(Component const& component, OutputIterator& sink,
            Context& /*ctx*/, Delimiter const& d, Parameter const& /*param*/)
        {
            boost::integer::endian<
                endian, typename karma::detail::integer<bits>::type, bits
            > p (fusion::at_c<0>(component.elements));

            unsigned char const* bytes =
                reinterpret_cast<unsigned char const*>(&p);

            for (unsigned int i = 0; i < sizeof(p); ++i)
                detail::generate_to(sink, *bytes++);

            karma::delimit(sink, d);           // always do post-delimiting
            return true;
        }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            return karma::detail::what<endian>::is();
        }
    };

}}}

#endif
