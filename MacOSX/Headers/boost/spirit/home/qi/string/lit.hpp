/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(BOOST_SPIRIT_LIT_APR_18_2006_1125PM)
#define BOOST_SPIRIT_LIT_APR_18_2006_1125PM

#include <boost/spirit/home/qi/domain.hpp>
#include <boost/spirit/home/qi/skip.hpp>
#include <boost/spirit/home/qi/detail/string_parse.hpp>
#include <boost/spirit/home/support/char_class.hpp>
#include <boost/spirit/home/support/modifier.hpp>
#include <boost/spirit/home/support/unused.hpp>
#include <boost/spirit/home/support/detail/to_narrow.hpp>
#include <boost/fusion/include/at.hpp>
#include <boost/fusion/include/value_at.hpp>
#include <boost/fusion/include/vector.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <string>

namespace boost { namespace spirit { namespace qi
{
    ///////////////////////////////////////////////////////////////////////////
    // parse literal strings
    ///////////////////////////////////////////////////////////////////////////
    template <typename Char>
    struct literal_string
    {
        template <typename Component, typename Context, typename Iterator>
        struct attribute
        {
            typedef unused_type type;   // literal parsers have no attribute
        };

        template <
            typename Component
          , typename Iterator, typename Context
          , typename Skipper, typename Attribute>
        static bool parse(
            Component const& component
          , Iterator& first, Iterator const& last
          , Context& /*context*/, Skipper const& skipper
          , Attribute& attr)
        {
            qi::skip(first, last, skipper);
            return detail::string_parse(
                fusion::at_c<0>(component.elements)
              , first
              , last
              , attr
            );
        }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            return std::string("\"")
                + spirit::detail::to_narrow_string(
                    fusion::at_c<0>(component.elements))
                + std::string("\"")
            ;
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    // parse lazy strings
    ///////////////////////////////////////////////////////////////////////////
    struct lazy_string
    {
        template <typename Component, typename Context, typename Iterator>
        struct attribute
        {
            typedef typename
                result_of::subject<Component>::type
            subject_type;

            typedef typename
                remove_reference<
                    typename boost::result_of<subject_type(unused_type, Context)>::type
                >::type
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
            qi::skip(first, last, skipper);
            return detail::string_parse(
                fusion::at_c<0>(component.elements)(unused, context)
              , first
              , last
              , attr
            );
        }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            return std::string("\"")
                + spirit::detail::to_narrow_string(
                    fusion::at_c<0>(component.elements)(unused, ctx))
                + std::string("\"")
            ;
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    // no_case literal_string version
    ///////////////////////////////////////////////////////////////////////////
    template <typename Char>
    struct no_case_literal_string
    {
        template <typename Component, typename Context, typename Iterator>
        struct attribute
        {
            typedef unused_type type;   // literal parsers have no attribute
        };

        template <
            typename Component
          , typename Iterator, typename Context
          , typename Skipper, typename Attribute>
        static bool parse(
            Component const& component
          , Iterator& first, Iterator const& last
          , Context& /*context*/, Skipper const& skipper
          , Attribute& attr)
        {
            qi::skip(first, last, skipper);
            return detail::string_parse(
                fusion::at_c<0>(component.elements)
              , fusion::at_c<1>(component.elements)
              , first
              , last
              , attr
            );
        }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            return std::string("case-insensitive \"")
                + spirit::detail::to_narrow_string(
                    fusion::at_c<0>(component.elements))
                + std::string("\"")
            ;
        }
    };
}}}

namespace boost { namespace spirit { namespace traits
{
    ///////////////////////////////////////////////////////////////////////////
    // no_case_literal_string generator
    ///////////////////////////////////////////////////////////////////////////
    template <
        typename Domain, typename Elements, typename Modifier, typename Char
    >
    struct make_modified_component<
        Domain, qi::literal_string<Char>, Elements, Modifier
      , typename enable_if<
            is_member_of_modifier<Modifier, spirit::char_class::no_case_base_tag>
        >::type
    >
    {
        typedef std::basic_string<Char> string_type;
        typedef fusion::vector<string_type, string_type> vector_type;
        typedef
            component<qi::domain, qi::no_case_literal_string<Char>, vector_type>
        type;

        static type
        call(Elements const& elements)
        {
            typedef typename Modifier::char_set char_set;

            Char const* in = fusion::at_c<0>(elements);
            string_type lo(in);
            string_type hi(in);

            typename string_type::iterator loi = lo.begin();
            typename string_type::iterator hii = hi.begin();

            for (; loi != lo.end(); ++loi, ++hii, ++in)
            {
                *loi = char_set::tolower(*loi);
                *hii = char_set::toupper(*hii);
            }

            return type(vector_type(lo, hi));
        }
    };
}}}

#endif
