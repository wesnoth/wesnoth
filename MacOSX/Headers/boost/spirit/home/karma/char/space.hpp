//  Copyright (c) 2001-2008 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_KARMA_SPACE_MAR_06_2007_0934PM)
#define BOOST_SPIRIT_KARMA_SPACE_MAR_06_2007_0934PM

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once      // MS compatible compilers support #pragma once
#endif

#include <boost/spirit/home/karma/domain.hpp>
#include <boost/spirit/home/karma/delimit.hpp>
#include <boost/spirit/home/karma/detail/generate_to.hpp>
#include <boost/spirit/home/support/char_class.hpp>
#include <boost/spirit/home/support/detail/to_narrow.hpp>
#include <boost/spirit/home/support/iso8859_1.hpp>
#include <boost/spirit/home/support/ascii.hpp>
#include <boost/spirit/home/support/standard.hpp>
#include <boost/spirit/home/support/standard_wide.hpp>

namespace boost { namespace spirit { namespace karma
{
    ///////////////////////////////////////////////////////////////////////////
    //
    //  space
    //      generates a single character from the associated parameter
    //
    ///////////////////////////////////////////////////////////////////////////
    template <typename Tag, typename Char>
    struct any_space_char
    {
        template <typename Component, typename Context, typename Unused>
        struct attribute
        {
            typedef Char type;
        };

        typedef typename Tag::char_set char_set;
        typedef typename Tag::char_class char_class_;

        // space has a parameter attached
        template <typename Component, typename OutputIterator,
            typename Context, typename Delimiter, typename Parameter>
        static bool
        generate(Component const& /*component*/, OutputIterator& sink,
            Context& /*ctx*/, Delimiter const& d, Parameter const& ch)
        {
            using spirit::char_class::classify;
            BOOST_ASSERT(classify<char_set>::is(char_class_(), ch));
            detail::generate_to(sink, ch);
            karma::delimit(sink, d);           // always do post-delimiting
            return true;
        }

        // this space has no parameter attached, just generate a single ' '
        template <typename Component, typename OutputIterator,
            typename Context, typename Delimiter>
        static bool
        generate(Component const&, OutputIterator& sink, Context&,
            Delimiter const& d, unused_type)
        {
            detail::generate_to(sink, ' ');     // generate a single space
            karma::delimit(sink, d);            // always do post-delimiting
            return true;
        }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            return "any-space";
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    //
    //  space(...)
    //      generates a single space character given by a literal it was
    //      initialized from
    //
    ///////////////////////////////////////////////////////////////////////////
    template <typename Tag, typename Char>
    struct literal_space_char
    {
        template <typename Component, typename Context, typename Unused>
        struct attribute
        {
            typedef unused_type type;
        };

        // any_char has a parameter attached
        template <typename Component, typename OutputIterator,
            typename Context, typename Delimiter, typename Parameter>
        static bool
        generate(Component const& component, OutputIterator& sink,
            Context& /*ctx*/, Delimiter const& d, Parameter const& /*param*/)
        {
            detail::generate_to(sink, fusion::at_c<0>(component.elements));
            karma::delimit(sink, d);             // always do post-delimiting
            return true;
        }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            return std::string("space('")
                + spirit::detail::to_narrow_char(
                    fusion::at_c<0>(component.elements))
                + "')";
        }
    };

}}}  // namespace boost::spirit::karma

#endif // !defined(BOOST_SPIRIT_KARMA_CHAR_FEB_21_2007_0543PM)
