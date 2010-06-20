//  Copyright (c) 2001-2008 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_KARMA_META_GRAMMAR_FEB_22_2007_0532PM)
#define BOOST_SPIRIT_KARMA_META_GRAMMAR_FEB_22_2007_0532PM

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once      // MS compatible compilers support #pragma once
#endif

#include <boost/spirit/home/karma/domain.hpp>
#include <boost/spirit/home/support/placeholders.hpp>
#include <boost/spirit/home/support/meta_grammar.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/type_traits/remove_const.hpp>
#include <boost/type_traits/is_convertible.hpp>
#include <string>

namespace boost { namespace spirit { namespace karma
{
    ///////////////////////////////////////////////////////////////////////////
    // forwards
    ///////////////////////////////////////////////////////////////////////////
    template <typename Char>
    struct any_string;

    template <typename Char>
    struct literal_string;

    struct lazy_string;

    struct string_meta_grammar;

    template <typename Expr, typename Enable>
    struct is_valid_expr;

    template <typename Expr, typename Enable>
    struct expr_transform;

    ///////////////////////////////////////////////////////////////////////////
    //
    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    struct extract_char;

    template <typename Char, typename Traits, typename Alloc>
    struct extract_char<std::basic_string<Char, Traits, Alloc> >
    {
        typedef Char type;
    };

    template <typename Char, int N>
    struct extract_char<Char[N]>
    {
        typedef typename remove_const<Char>::type type;
    };

    template <typename Char, int N>
    struct extract_char<Char(&)[N]>
    {
        typedef typename remove_const<Char>::type type;
    };

    template <typename Char>
    struct extract_char<Char*>
    {
        typedef typename remove_const<Char>::type type;
    };

    ///////////////////////////////////////////////////////////////////////////
    // get the director of a string literal type
    ///////////////////////////////////////////////////////////////////////////
    template <typename Tag, typename T>
    struct extract_lit_director_lit;

    template <typename T>
    struct extract_lit_director_lit<tag::lit, T>
    {
        typedef typename extract_char<T>::type char_type;
        typedef literal_string<char_type> type;
    };

    template <typename T>
    struct extract_lit_director_lit<tag::wlit, T>
    {
        typedef typename extract_char<T>::type char_type;
        typedef literal_string<char_type> type;
    };

    ///////////////////////////////////////////////////////////////////////////
    template <typename Tag>
    struct extract_lit_director_plain;

    template <>
    struct extract_lit_director_plain<tag::lit>
    {
        typedef any_string<char> type;
    };

    template <>
    struct extract_lit_director_plain<tag::wlit>
    {
        typedef any_string<wchar_t> type;
    };

    ///////////////////////////////////////////////////////////////////////////
    // string generator meta-grammar
    ///////////////////////////////////////////////////////////////////////////

    // literal strings: "hello"
    struct string_literal_meta_grammar
      : proto::or_<
            meta_grammar::terminal_rule<
                karma::domain, char const*, literal_string<char>
            >,
            meta_grammar::terminal_rule<
                karma::domain, wchar_t const*, literal_string<wchar_t>
            >,
            meta_grammar::terminal_rule<
                karma::domain, char*, literal_string<char>
            >,
            meta_grammar::terminal_rule<
                karma::domain, wchar_t*, literal_string<wchar_t>
            >
        >
    {
    };

    // literal strings: "hello"
    struct basic_string_literal_meta_grammar
      : proto::or_<
            proto::terminal<char const*>,
            proto::terminal<wchar_t const*>
        >
    {
    };

    // std::string(s)
    struct std_string_meta_grammar
      : proto::or_<
            meta_grammar::terminal_rule<
                karma::domain,
                std::basic_string<char, proto::_, proto::_>,
                literal_string<char>
            >,
            meta_grammar::terminal_rule<
                karma::domain,
                std::basic_string<wchar_t, proto::_, proto::_>,
                literal_string<wchar_t>
            >
        >
    {
    };

    // std::string(s)
    struct basic_std_string_meta_grammar
      : proto::or_<
            proto::terminal<std::basic_string<char, proto::_, proto::_> >,
            proto::terminal<std::basic_string<wchar_t, proto::_, proto::_> >
        >
    {
    };

    namespace detail
    {
        // we use this test to detect if the argument to lit is a callable
        // function or not. Only types convertible to int or function/
        // function objects are allowed. Therefore, if T is not convertible
        // to an int, then we have a function/function object.
        template <typename T>
        struct is_not_convertible_to_int
          : mpl::not_<is_convertible<T, int> >
        {
        };
    }

    // this is the string literal meta grammar
    // literal strings: lit, lit("hello")
    struct string_meta_grammar
      : proto::or_<
            string_literal_meta_grammar,
            std_string_meta_grammar,
            meta_grammar::compose_empty<
                proto::if_<
                    is_lit_tag<proto::_arg, karma::domain>()
                >,
                karma::domain,
                mpl::identity<extract_lit_director_plain<mpl::_> >
            >,
            meta_grammar::compose_function1_eval<
                proto::function<
                    proto::if_<
                        is_lit_tag<proto::_arg, karma::domain>()
                    >,
                    proto::or_<
                        basic_string_literal_meta_grammar,
                        basic_std_string_meta_grammar
                    >
                >,
                karma::domain,
                mpl::identity<extract_lit_director_lit<mpl::_, mpl::_> >
            >,
            meta_grammar::function1_rule<
                karma::domain, tag::lit, lazy_string,
                proto::if_<
                    detail::is_not_convertible_to_int<proto::_arg>()
                >
            >
        >
    {
    };

    ///////////////////////////////////////////////////////////////////////////
    //  These specializations non-intrusively hooks into the Karma meta-grammar.
    //  (see karma/meta_grammar.hpp)
    ///////////////////////////////////////////////////////////////////////////
    template <typename Expr>
    struct is_valid_expr<Expr,
        typename enable_if<proto::matches<Expr, string_meta_grammar> >::type>
      : mpl::true_
    {
    };

    template <typename Expr>
    struct expr_transform<Expr,
        typename enable_if<proto::matches<Expr, string_meta_grammar> >::type>
      : mpl::identity<string_meta_grammar>
    {
    };

}}}

#endif
