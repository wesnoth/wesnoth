/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(BOOST_SPIRIT_CHAR_PARSER_APR_16_2006_0906AM)
#define BOOST_SPIRIT_CHAR_PARSER_APR_16_2006_0906AM

#include <string>
#include <boost/spirit/home/qi/skip.hpp>
#include <boost/spirit/home/qi/detail/assign_to.hpp>
#include <boost/spirit/home/support/unused.hpp>
#include <boost/fusion/include/at.hpp>

#include <boost/detail/iterator.hpp> // for boost::detail::iterator_traits

namespace boost { namespace spirit { namespace qi
{
    template <typename Derived, typename Char = unused_type>
    struct char_parser
    {
        typedef Char char_type;

        // if Char is unused_type, Derived must supply its own attribute metafunction
        template <typename Component, typename Context, typename Iterator>
        struct attribute
        {
            typedef Char type;
        };

        template <
            typename Component
          , typename Iterator, typename Context
          , typename Skipper, typename Attribute>
        static bool parse(
            Component const& component
          , Iterator& first, Iterator const& last
          , Context& context, Skipper const& skipper
          , Attribute& attr)
        {
            qi::skip(first, last, skipper);

            if (first != last && Derived::test(component, *first, context))
            {
                qi::detail::assign_to(*first, attr);
                ++first;
                return true;
            }
            return false;
        }

        // char_parser subclasses are required to
        // implement test:

        template <typename Component, typename CharParam, typename Context>
        bool test(Component const& component, CharParam ch, Context& context);
    };

    template <typename Positive>
    struct negated_char_parser :
        char_parser<
            negated_char_parser<Positive>, typename Positive::director::char_type
        >
    {
        template <typename Component, typename CharParam, typename Context>
        static bool test(Component const& component, CharParam ch, Context& context)
        {
            return !Positive::director::test(
                fusion::at_c<0>(component.elements), ch, context);
        }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            return std::string("not ")
                + Positive::director::what(fusion::at_c<0>(component.elements), ctx);
        }
    };
}}}

#endif
