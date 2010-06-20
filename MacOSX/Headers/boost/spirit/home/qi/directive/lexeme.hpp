/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(SPIRIT_LEXEME_MARCH_24_2007_0802AM)
#define SPIRIT_LEXEME_MARCH_24_2007_0802AM

#include <boost/spirit/home/support/component.hpp>
#include <boost/spirit/home/support/unused.hpp>
#include <boost/spirit/home/support/attribute_of.hpp>
#include <boost/spirit/home/qi/domain.hpp>
#include <boost/spirit/home/qi/skip.hpp>

namespace boost { namespace spirit { namespace qi
{
    struct lexeme_director
    {
        template <typename Component, typename Context, typename Iterator>
        struct attribute
        {
            typedef typename
                result_of::subject<Component>::type
            subject_type;

            typedef typename
                traits::attribute_of<
                    qi::domain, subject_type, Context, Iterator>::type
            type;
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
            typedef typename
                result_of::subject<Component>::type::director
            director;

            qi::skip(first, last, skipper);
            return director::parse(
                spirit::subject(component), first, last, context, unused, attr);
        }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            std::string result = "lexeme[";

            typedef typename
                result_of::subject<Component>::type::director
            director;

            result += director::what(subject(component), ctx);
            result += "]";
            return result;
        }
    };
}}}

#endif
