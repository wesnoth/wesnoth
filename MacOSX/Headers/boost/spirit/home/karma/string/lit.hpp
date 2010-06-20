//  Copyright (c) 2001-2008 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_KARMA_LIT_FEB_22_2007_0534PM)
#define BOOST_SPIRIT_KARMA_LIT_FEB_22_2007_0534PM

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once      // MS compatible compilers support #pragma once
#endif

#include <boost/spirit/home/karma/domain.hpp>
#include <boost/spirit/home/karma/delimit.hpp>
#include <boost/spirit/home/karma/detail/string_generate.hpp>
#include <boost/spirit/home/support/char_class.hpp>
#include <boost/spirit/home/support/modifier.hpp>
#include <boost/fusion/include/at.hpp>
#include <boost/fusion/include/value_at.hpp>
#include <boost/fusion/include/vector.hpp>
#include <string>

namespace boost { namespace spirit { namespace karma
{
    ///////////////////////////////////////////////////////////////////////////
    // generate literal strings from a given parameter
    ///////////////////////////////////////////////////////////////////////////
    template <typename Char>
    struct any_string
    {
        template <typename Component, typename Context, typename Unused>
        struct attribute
        {
            typedef std::basic_string<Char> type;
        };

        // lit has a parameter attached
        template <typename Component, typename OutputIterator,
            typename Context, typename Delimiter, typename Parameter>
        static bool
        generate(Component const& /*component*/, OutputIterator& sink,
            Context& /*ctx*/, Delimiter const& d, Parameter const& param)
        {
            bool result = detail::string_generate(sink, param);
            if (result)
                karma::delimit(sink, d);           // always do post-delimiting
            return result;
        }

        // this lit has no parameter attached, it needs to have been
        // initialized from a direct literal
        template <typename Component, typename OutputIterator,
            typename Context, typename Delimiter>
        static bool
        generate(Component const& component, OutputIterator& sink,
            Context& /*ctx*/, Delimiter const& d, unused_type)
        {
            BOOST_MPL_ASSERT_MSG(false, lit_not_usable_without_attribute, ());
            return false;
        }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            return "any-string";
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    // generate literal strings
    ///////////////////////////////////////////////////////////////////////////
    template <typename Char>
    struct literal_string
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
            bool result = detail::string_generate(sink,
                fusion::at_c<0>(component.elements));

            karma::delimit(sink, d);           // always do post-delimiting
            return result;
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
    // lazy string generation
    ///////////////////////////////////////////////////////////////////////////
    struct lazy_string
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
            Context& ctx, Delimiter const& d, Parameter const& /*param*/)
        {
            bool result = detail::string_generate(sink,
                fusion::at_c<0>(component.elements)(unused, ctx));

            karma::delimit(sink, d);           // always do post-delimiting
            return result;
        }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            return "string";
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    // generate literal strings from a given parameter
    ///////////////////////////////////////////////////////////////////////////
    template <typename Char, typename Tag>
    struct case_any_string
    {
        template <typename Component, typename Context, typename Unused>
        struct attribute
        {
            typedef std::basic_string<Char> type;
        };

        // case_any_string has a parameter attached
        template <typename Component, typename OutputIterator,
            typename Context, typename Delimiter, typename Parameter>
        static bool
        generate(Component const& /*component*/, OutputIterator& sink,
            Context& /*ctx*/, Delimiter const& d, Parameter const& param)
        {
            bool result = detail::string_generate(sink, param, Tag());
            karma::delimit(sink, d);           // always do post-delimiting
            return result;
        }

        // this case_any_string has no parameter attached, it needs to have been
        // initialized from a direct literal
        template <typename Component, typename OutputIterator,
            typename Context, typename Delimiter>
        static bool
        generate(Component const& component, OutputIterator& sink,
            Context& /*ctx*/, Delimiter const& d, unused_type)
        {
            BOOST_MPL_ASSERT_MSG(false, lit_not_usable_without_attribute, ());
            return false;
        }


        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            typedef typename Tag::char_set char_set;
            typedef typename Tag::char_class char_class_;
            return std::string("any-") +
                spirit::char_class::what<char_set>::is(char_class_())
                + "case-string";
        }
    };

}}}

namespace boost { namespace spirit { namespace traits
{
    ///////////////////////////////////////////////////////////////////////////
    // lower_case and upper_case literal_string generator
    ///////////////////////////////////////////////////////////////////////////
    template <typename Domain, typename Elements, typename Modifier,
        typename Char>
    struct make_modified_component<
        Domain, karma::literal_string<Char>, Elements, Modifier,
        typename enable_if<
            is_member_of_modifier<Modifier, spirit::char_class::lower_case_base_tag>
        >::type
    >
    {
        typedef std::basic_string<Char> string_type;
        typedef fusion::vector<string_type> vector_type;

        typedef
            component<karma::domain, karma::literal_string<Char>, vector_type>
        type;

        static type
        call(Elements const& elements)
        {
            typedef typename Modifier::char_set char_set;

            string_type val(fusion::at_c<0>(elements));
            typename string_type::iterator end = val.end();
            for (typename string_type::iterator it = val.begin();
                 it != end; ++it)
            {
                *it = char_set::tolower(*it);
            }

            return type(vector_type(val));
        }
    };

    template <typename Domain, typename Elements, typename Modifier,
        typename Char>
    struct make_modified_component<
        Domain, karma::literal_string<Char>, Elements, Modifier,
        typename enable_if<
            is_member_of_modifier<Modifier, spirit::char_class::upper_case_base_tag>
        >::type
    >
    {
        typedef std::basic_string<Char> string_type;
        typedef fusion::vector<string_type> vector_type;

        typedef
            component<karma::domain, karma::literal_string<Char>, vector_type>
        type;

        static type
        call(Elements const& elements)
        {
            typedef typename Modifier::char_set char_set;

            string_type val(fusion::at_c<0>(elements));
            typename string_type::iterator end = val.end();
            for (typename string_type::iterator it = val.begin();
                 it != end; ++it)
            {
                *it = char_set::toupper(*it);
            }

            return type(vector_type(val));
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    // lower and upper case_any_string conversions
    ///////////////////////////////////////////////////////////////////////////
    template <typename Domain, typename Elements, typename Modifier,
        typename Char>
    struct make_modified_component<
        Domain, karma::any_string<Char>, Elements, Modifier,
        typename enable_if<
            is_member_of_modifier<Modifier, spirit::char_class::lower_case_base_tag>
        >::type
    >
    {
        typedef typename Modifier::char_set char_set;
        typedef spirit::char_class::tag::lower char_class_;
        typedef spirit::char_class::key<char_set, char_class_> key_tag;

        typedef component<
            karma::domain, karma::case_any_string<Char, key_tag>, fusion::nil
        > type;

        static type
        call(Elements const&)
        {
            return type(fusion::nil());
        }
    };

    template <typename Domain, typename Elements, typename Modifier,
        typename Char>
    struct make_modified_component<
        Domain, karma::any_string<Char>, Elements, Modifier,
        typename enable_if<
            is_member_of_modifier<Modifier, spirit::char_class::upper_case_base_tag>
        >::type
    >
    {
        typedef typename Modifier::char_set char_set;
        typedef spirit::char_class::tag::upper char_class_;
        typedef spirit::char_class::key<char_set, char_class_> key_tag;

        typedef component<
            karma::domain, karma::case_any_string<Char, key_tag>, fusion::nil
        > type;

        static type
        call(Elements const&)
        {
            return type(fusion::nil());
        }
    };

}}}

#endif
