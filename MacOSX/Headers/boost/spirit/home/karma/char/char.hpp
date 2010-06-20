//  Copyright (c) 2001-2008 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_KARMA_CHAR_FEB_21_2007_0543PM)
#define BOOST_SPIRIT_KARMA_CHAR_FEB_21_2007_0543PM

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once      // MS compatible compilers support #pragma once
#endif

#include <boost/spirit/home/karma/domain.hpp>
#include <boost/spirit/home/karma/delimit.hpp>
#include <boost/spirit/home/karma/detail/generate_to.hpp>
#include <boost/spirit/home/support/modifier.hpp>
#include <boost/spirit/home/support/char_class.hpp>
#include <boost/spirit/home/support/detail/to_narrow.hpp>
#include <boost/spirit/home/support/iso8859_1.hpp>
#include <boost/spirit/home/support/ascii.hpp>
#include <boost/fusion/include/at.hpp>
#include <boost/fusion/include/value_at.hpp>
#include <boost/fusion/include/vector.hpp>
#include <boost/fusion/include/cons.hpp>
#include <boost/utility/enable_if.hpp>

namespace boost { namespace spirit { namespace karma
{
    ///////////////////////////////////////////////////////////////////////////
    //
    //  any_char
    //      generates a single character from the associated parameter
    //
    //      Note: this generator has to have an associated parameter
    //
    ///////////////////////////////////////////////////////////////////////////
    template <typename Char>
    struct any_char
    {
        template <typename Component, typename Context, typename Unused>
        struct attribute
        {
            typedef Char type;
        };

        // any_char has a parameter attached
        template <typename Component, typename OutputIterator,
            typename Context, typename Delimiter, typename Parameter>
        static bool
        generate(Component const&, OutputIterator& sink,
            Context& /*ctx*/, Delimiter const& d, Parameter const& param)
        {
            detail::generate_to(sink, param);
            karma::delimit(sink, d);           // always do post-delimiting
            return true;
        }

        // this any_char has no parameter attached, it needs to have been
        // initialized from a direct literal
        template <typename Component, typename OutputIterator,
            typename Context, typename Delimiter>
        static bool
        generate(Component const&, OutputIterator&, Context&, Delimiter const&,
            unused_type)
        {
            BOOST_MPL_ASSERT_MSG(false, char__not_usable_without_attribute, ());
            return false;
        }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            return "any-char";
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    //
    //  literal_char
    //      generates a single character given by a literal it was initialized
    //      from
    //
    ///////////////////////////////////////////////////////////////////////////
    template <typename Char>
    struct literal_char
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
            return std::string("'")
                + spirit::detail::to_narrow_char(
                    fusion::at_c<0>(component.elements))
                + '\'';
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    //
    //  lazy_char
    //      generates a single character given by a functor it was initialized
    //      from
    //
    ///////////////////////////////////////////////////////////////////////////
    struct lazy_char
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
            Context& ctx, Delimiter const& d, Parameter const& /*param*/)
        {
            detail::generate_to(sink,
                fusion::at_c<0>(component.elements)(unused, ctx));
            karma::delimit(sink, d);             // always do post-delimiting
            return true;
        }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            return "char";
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    //
    //  lower and upper case variants of any_char with an associated parameter
    //      note: this generator has to have a parameter associated
    //
    ///////////////////////////////////////////////////////////////////////////
    template <typename Char, typename Tag>
    struct case_any_char
    {
        template <typename Component, typename Context, typename Unused>
        struct attribute
        {
            typedef Char type;
        };

        typedef typename Tag::char_set char_set;
        typedef typename Tag::char_class char_class_;

        // case_any_char has a parameter attached
        template <typename Component, typename OutputIterator,
            typename Context, typename Delimiter, typename Parameter>
        static bool
        generate(Component const& /*component*/, OutputIterator& sink,
            Context& /*ctx*/, Delimiter const& d, Parameter const& param)
        {
            using spirit::char_class::convert;
            Char p = convert<char_set>::to(char_class_(), param);
            detail::generate_to(sink, p);
            karma::delimit(sink, d);           // always do post-delimiting
            return true;
        }

        // this case_any_char has no parameter attached, it needs to have been
        // initialized from a direct literal
        template <typename Component, typename OutputIterator,
            typename Context, typename Delimiter>
        static bool
        generate(Component const&, OutputIterator&, Context&, Delimiter const&,
            unused_type)
        {
            BOOST_MPL_ASSERT_MSG(false, char__not_usable_without_attribute, ());
            return false;
        }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            std::string result;
            result = std::string("any-") +
                spirit::char_class::what<char_set>::is(char_class_()) +
                "case-char";
            return result;
        }
    };

}}}  // namespace boost::spirit::karma

namespace boost { namespace spirit { namespace traits
{
    ///////////////////////////////////////////////////////////////////////////
    // lower_case and upper_case any_char and literal_char generators
    ///////////////////////////////////////////////////////////////////////////
    template <typename Domain, typename Elements, typename Modifier,
        typename Char>
    struct make_modified_component<
        Domain, karma::literal_char<Char>, Elements, Modifier,
        typename enable_if<
            is_member_of_modifier<Modifier, spirit::char_class::lower_case_base_tag>
        >::type
    >
    {
        typedef typename
            fusion::result_of::value_at_c<Elements, 0>::type
        char_type;
        typedef fusion::vector<char_type> vector_type;

        typedef component<
            karma::domain, karma::literal_char<Char>, vector_type>
        type;

        static type
        call(Elements const& elements)
        {
            typedef typename Modifier::char_set char_set;

            char_type ch = fusion::at_c<0>(elements);
            vector_type v(char_set::tolower(ch));
            return type(v);
        }
    };

    template <typename Domain, typename Elements, typename Modifier,
        typename Char>
    struct make_modified_component<
        Domain, karma::literal_char<Char>, Elements, Modifier,
        typename enable_if<
            is_member_of_modifier<Modifier, spirit::char_class::upper_case_base_tag>
        >::type
    >
    {
        typedef typename
            fusion::result_of::value_at_c<Elements, 0>::type
        char_type;
        typedef fusion::vector<char_type> vector_type;

        typedef
            component<karma::domain, karma::literal_char<Char>, vector_type>
        type;

        static type
        call(Elements const& elements)
        {
            typedef typename Modifier::char_set char_set;

            char_type ch = fusion::at_c<0>(elements);
            vector_type v(char_set::toupper(ch));
            return type(v);
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    // lower_case and upper case_any_char conversions
    ///////////////////////////////////////////////////////////////////////////
    template <typename Domain, typename Elements, typename Modifier,
        typename Char>
    struct make_modified_component<
        Domain, karma::any_char<Char>, Elements, Modifier,
        typename enable_if<
            is_member_of_modifier<Modifier, spirit::char_class::lower_case_base_tag>
        >::type
    >
    {
        typedef typename Modifier::char_set char_set;
        typedef spirit::char_class::tag::lower char_class_;
        typedef spirit::char_class::key<char_set, char_class_> key_tag;

        typedef component<
            karma::domain, karma::case_any_char<Char, key_tag>, fusion::nil>
        type;

        static type
        call(Elements const&)
        {
            return type(fusion::nil());
        }
    };

    template <typename Domain, typename Elements, typename Modifier,
        typename Char>
    struct make_modified_component<
        Domain, karma::any_char<Char>, Elements, Modifier,
        typename enable_if<
            is_member_of_modifier<Modifier, spirit::char_class::upper_case_base_tag>
        >::type
    >
    {
        typedef typename Modifier::char_set char_set;
        typedef spirit::char_class::tag::upper char_class_;
        typedef spirit::char_class::key<char_set, char_class_> key_tag;

        typedef component<
            karma::domain, karma::case_any_char<Char, key_tag>, fusion::nil>
        type;

        static type
        call(Elements const&)
        {
            return type(fusion::nil());
        }
    };

}}}   // namespace boost::spirit::traits

#endif // !defined(BOOST_SPIRIT_KARMA_CHAR_FEB_21_2007_0543PM)
