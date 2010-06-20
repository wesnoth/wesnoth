/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(BOOST_SPIRIT_CHAR_APR_16_2006_1051AM)
#define BOOST_SPIRIT_CHAR_APR_16_2006_1051AM

#include <boost/spirit/home/qi/char/char_parser.hpp>
#include <boost/spirit/home/qi/char/detail/get_char.hpp>
#include <boost/spirit/home/qi/domain.hpp>
#include <boost/fusion/include/at.hpp>
#include <boost/fusion/include/value_at.hpp>
#include <boost/fusion/include/vector.hpp>
#include <boost/spirit/home/support/modifier.hpp>
#include <boost/spirit/home/support/char_class.hpp>
#include <boost/spirit/home/support/detail/to_narrow.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <boost/foreach.hpp>
#include <boost/mpl/print.hpp>

namespace boost { namespace spirit { namespace qi
{
    ///////////////////////////////////////////////////////////////////////////
    // parse any character
    ///////////////////////////////////////////////////////////////////////////
    template <typename Char>
    struct any_char : char_parser<any_char<Char>, Char>
    {
        template <typename Component, typename CharParam, typename Context>
        static bool test(Component const&, CharParam, Context&)
        {
            return true;
        }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            return "any-char";
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    // parse a single character
    ///////////////////////////////////////////////////////////////////////////
    template <typename Char>
    struct literal_char : char_parser<literal_char<Char>, Char>
    {
        template <typename Component, typename Context, typename Iterator>
        struct attribute
        {
            typedef unused_type type;   // literal parsers have no attribute
        };

        template <typename Component, typename CharParam, typename Context>
        static bool test(Component const& component, CharParam ch, Context&)
        {
            return detail::get_char(fusion::at_c<0>(component.elements)) == ch;
        }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            return std::string("'")
                + spirit::detail::to_narrow_char(
                    detail::get_char(fusion::at_c<0>(component.elements)))
                + '\'';
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    // parse a character set
    ///////////////////////////////////////////////////////////////////////////
    template <typename Char>
    struct char_set : char_parser<char_set<Char>, Char>
    {
        template <typename Component, typename CharParam, typename Context>
        static bool test(Component const& component, CharParam ch, Context&)
        {
            return component.ptr->test(ch);
        }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            return "char-set";
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    // parse a lazy character
    ///////////////////////////////////////////////////////////////////////////
    struct lazy_char : char_parser<lazy_char>
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

        template <typename Component, typename CharParam, typename Context>
        static bool test(Component const& component, CharParam ch, Context& context)
        {
            return fusion::at_c<0>(component.elements)(unused, context) == ch;
        }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            return std::string("'")
                + spirit::detail::to_narrow_char(
                    fusion::at_c<0>(component.elements)(unused, ctx))
                + '\'';
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    // parse a character range
    ///////////////////////////////////////////////////////////////////////////
    template <typename Char>
    struct char_range : char_parser<char_range<Char>, Char>
    {
        template <typename Component, typename CharParam, typename Context>
        static bool test(Component const& component, CharParam ch, Context&)
        {
            return
                !(ch < fusion::at_c<0>(component.elements)) &&
                !(fusion::at_c<1>(component.elements) < ch);
        }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            std::string result;
            result += std::string("'") + fusion::at_c<0>(component.elements) + '\'';
            result += "...";
            result += std::string("'") + fusion::at_c<1>(component.elements) + '\'';
            return result;
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    // parse a lazy character range
    ///////////////////////////////////////////////////////////////////////////
    struct lazy_char_range : char_parser<lazy_char_range>
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

        template <typename Component, typename CharParam, typename Context>
        static bool test(Component const& component, CharParam ch, Context& context)
        {
            return
                !(ch < fusion::at_c<0>(component.elements)(unused, context)) &&
                !(fusion::at_c<1>(component.elements)(unused, context) < ch);
        }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            return "char-range";
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    // no_case literal_char version
    ///////////////////////////////////////////////////////////////////////////
    template <typename Char>
    struct no_case_literal_char : char_parser<no_case_literal_char<Char>, Char>
    {
        template <typename Component, typename Context, typename Iterator>
        struct attribute
        {
            typedef unused_type type;   // literal parsers have no attribute
        };

        template <typename Component, typename CharParam, typename Context>
        static bool test(Component const& component, CharParam ch, Context&)
        {
            return detail::get_char(fusion::at_c<0>(component.elements)) == ch
                || detail::get_char(fusion::at_c<1>(component.elements)) == ch
            ;
        }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            std::string result;
            result += std::string("'")
                + spirit::detail::to_narrow_char(
                    detail::get_char(fusion::at_c<0>(component.elements))) + '\'';
            result += " or ";
            result += std::string("'") +
                spirit::detail::to_narrow_char(
                    detail::get_char(fusion::at_c<1>(component.elements))) + '\'';
            return result;
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    // no_case char_range version
    ///////////////////////////////////////////////////////////////////////////
    template <typename Char>
    struct no_case_char_range : char_parser<no_case_char_range<Char>, Char>
    {
        template <typename Component, typename CharParam, typename Context>
        static bool test(Component const& component, CharParam ch, Context&)
        {
            return
                (!(ch < fusion::at_c<0>(component.elements)) &&
                 !(fusion::at_c<1>(component.elements) < ch))
            ||  (!(ch < fusion::at_c<2>(component.elements)) &&
                 !(fusion::at_c<3>(component.elements) < ch))
            ;
        }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            std::string result;
            result += std::string("'") + fusion::at_c<0>(component.elements) + '\'';
            result += "...";
            result += std::string("'") + fusion::at_c<1>(component.elements) + '\'';
            result += " or ";
            result += std::string("'") + fusion::at_c<2>(component.elements) + '\'';
            result += "...";
            result += std::string("'") + fusion::at_c<3>(component.elements) + '\'';
            return result;
        }
    };

    template <typename Char, typename Elements>
    struct char_set_component;
}}}

namespace boost { namespace spirit { namespace traits
{
    ///////////////////////////////////////////////////////////////////////////
    // char_set_component generator
    ///////////////////////////////////////////////////////////////////////////
    template <typename Char, typename Elements, typename Modifier>
    struct make_component<qi::domain, qi::char_set<Char>, Elements, Modifier
      , typename disable_if<
            is_member_of_modifier<Modifier, spirit::char_class::no_case_base_tag>
        >::type
    > : mpl::identity<qi::char_set_component<Char, Elements> >
    {
        static qi::char_set_component<Char, Elements>
        call(Elements const& elements)
        {
            return qi::char_set_component<Char, Elements>(
                fusion::at_c<0>(elements));
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    // no_case char_set_component generator
    ///////////////////////////////////////////////////////////////////////////
    template <
        typename Domain, typename Elements, typename Modifier, typename Char
    >
    struct make_modified_component<
        Domain, qi::char_set<Char>, Elements, Modifier
      , typename enable_if<
            is_member_of_modifier<Modifier, spirit::char_class::no_case_base_tag>
        >::type
    >
    {
        typedef qi::char_set_component<Char, Elements> type;
        typedef typename Modifier::char_set char_set;

        static type
        call(Elements const& elements)
        {
            return qi::char_set_component<Char, Elements>(
                fusion::at_c<0>(elements), char_set());
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    // no_case_literal_char generator
    ///////////////////////////////////////////////////////////////////////////
    template <
        typename Domain, typename Elements, typename Modifier, typename Char
    >
    struct make_modified_component<
        Domain, qi::literal_char<Char>, Elements, Modifier
      , typename enable_if<
            is_member_of_modifier<Modifier, spirit::char_class::no_case_base_tag>
        >::type
    >
    {
        typedef fusion::vector<Char, Char> vector_type;
        typedef
            component<qi::domain, qi::no_case_literal_char<Char>, vector_type>
        type;

        static type
        call(Elements const& elements)
        {
            typedef typename Modifier::char_set char_set;

            Char ch = qi::detail::get_char(fusion::at_c<0>(elements));
            vector_type v(
                char_set::tolower(ch)
              , char_set::toupper(ch)
            );
            return type(v);
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    // no_case_char_range generator
    ///////////////////////////////////////////////////////////////////////////
    template <
        typename Domain, typename Elements, typename Modifier, typename Char
    >
    struct make_modified_component<
        Domain, qi::char_range<Char>, Elements, Modifier
      , typename enable_if<
            is_member_of_modifier<Modifier, spirit::char_class::no_case_base_tag>
        >::type
    >
    {
        typedef fusion::vector<Char, Char, Char, Char> vector_type;
        typedef
            component<qi::domain, qi::no_case_char_range<Char>, vector_type>
        type;

        static type
        call(Elements const& elements)
        {
            typedef typename Modifier::char_set char_set;

            Char first = fusion::at_c<0>(elements);
            Char last = fusion::at_c<1>(elements);
            vector_type v(
                char_set::tolower(first)
              , char_set::tolower(last)
              , char_set::toupper(first)
              , char_set::toupper(last)
            );
            return type(v);
        }
    };
}}}

#endif
