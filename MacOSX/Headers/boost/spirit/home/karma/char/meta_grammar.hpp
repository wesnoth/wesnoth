//  Copyright (c) 2001-2007 Joel de Guzman
//  Copyright (c) 2001-2008 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_KARMA_META_GRAMMAR_FEB_21_2007_0742AM)
#define BOOST_SPIRIT_KARMA_META_GRAMMAR_FEB_21_2007_0742AM

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once      // MS compatible compilers support #pragma once
#endif

#include <boost/spirit/home/karma/domain.hpp>
#include <boost/spirit/home/support/placeholders.hpp>
#include <boost/spirit/home/support/meta_grammar.hpp>
#include <boost/spirit/home/support/char_class.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/mpl/bool.hpp>

namespace boost { namespace spirit
{
    ///////////////////////////////////////////////////////////////////////////
    // test if a tag is a character literal type
    ///////////////////////////////////////////////////////////////////////////
    template <typename CharSet>
    struct is_char_tag<
            spirit::char_class::key<CharSet, char_class::tag::space>,
            karma::domain
        >
      : mpl::true_
    {};
}}

namespace boost { namespace spirit { namespace karma
{
    ///////////////////////////////////////////////////////////////////////////
    // forward declarations
    ///////////////////////////////////////////////////////////////////////////
    template <typename Char>
    struct any_char;

    template <typename Char>
    struct literal_char;

    struct lazy_char;

    template <typename Tag, typename Char>
    struct any_space_char;

    template <typename Tag, typename Char>
    struct literal_space_char;

    struct char_meta_grammar;

    template <typename Expr, typename Enable>
    struct is_valid_expr;

    template <typename Expr, typename Enable>
    struct expr_transform;

    ///////////////////////////////////////////////////////////////////////////
    // get the director of a character literal type
    ///////////////////////////////////////////////////////////////////////////
    template <typename Tag, typename T>
    struct extract_literal_char_director;

    template <typename T>
    struct extract_literal_char_director<tag::char_, T>
    {
        typedef literal_char<T> type;
    };

    template <typename T>
    struct extract_literal_char_director<tag::wchar, T>
    {
        typedef literal_char<wchar_t> type;
    };

    template <typename T>
    struct extract_literal_char_director<tag::lit, T>
    {
        typedef literal_char<T> type;
    };

    template <typename T>
    struct extract_literal_char_director<tag::wlit, T>
    {
        typedef literal_char<wchar_t> type;
    };

    template <typename CharSet, typename T>
    struct extract_literal_char_director<
        spirit::char_class::key<CharSet, char_class::tag::space>, T
    >
    {
        typedef
            spirit::char_class::key<CharSet, char_class::tag::space>
        key_type;
        typedef literal_space_char<key_type, T> type;
    };

    ///////////////////////////////////////////////////////////////////////////
    // get the director of a plain character type
    ///////////////////////////////////////////////////////////////////////////
    template <typename Tag>
    struct extract_any_char_director;

    template <>
    struct extract_any_char_director<tag::char_>
    {
        typedef any_char<char> type;
    };

    template <>
    struct extract_any_char_director<tag::wchar>
    {
        typedef any_char<wchar_t> type;
    };

    template <typename CharSet>
    struct extract_any_char_director<
        spirit::char_class::key<CharSet, char_class::tag::space>
    >
    {
        typedef typename CharSet::char_type char_type;
        typedef
            spirit::char_class::key<CharSet, char_class::tag::space>
        key_type;
        typedef any_space_char<key_type, char_type> type;
    };

    ///////////////////////////////////////////////////////////////////////////
    // char generator meta-grammars
    ///////////////////////////////////////////////////////////////////////////

    // literals: 'x', L'x'
    struct char_literal_meta_grammar
      : proto::or_<
            meta_grammar::terminal_rule<
                karma::domain, char, literal_char<char> 
            >,
            meta_grammar::terminal_rule<
                karma::domain, wchar_t, literal_char<wchar_t> 
            >
        >
    {
    };

    // literals: 'x', L'x'
    struct basic_char_literal_meta_grammar
      : proto::or_<
            proto::terminal<char>,
            proto::terminal<wchar_t>
        >
    {
    };

    // char_, wchar, space
    // char_('x'), char_(L'x'), wchar('x'), wchar(L'x'), space(' ')
    struct char_meta_grammar1
      : proto::or_<
            // char_, wchar, space
            meta_grammar::compose_empty<
                proto::if_<
                    is_char_tag<proto::_arg, karma::domain>()
                >,
                karma::domain,
                mpl::identity<extract_any_char_director<mpl::_> >
            >,
            // char_('x'), wchar(L'x'), space(' ')
            meta_grammar::compose_function1_eval<
                proto::function<
                    proto::if_<
                        is_char_tag<proto::_arg, karma::domain>()
                    >,
                    basic_char_literal_meta_grammar
                >,
                karma::domain,
                mpl::identity<extract_literal_char_director<mpl::_, mpl::_> >
            >,
            // lit('x'), wlit('x')
            meta_grammar::compose_function1_eval<
                proto::function<
                    proto::if_<
                        is_lit_tag<proto::_arg, karma::domain>()
                    >,
                    basic_char_literal_meta_grammar
                >,
                karma::domain,
                mpl::identity<extract_literal_char_director<mpl::_, mpl::_> >
            >,
            // char_(val('y'))
            meta_grammar::function1_rule<
                karma::domain, tag::char_, lazy_char
            >
        >
    {
    };

    // main char_meta_grammar
    struct char_meta_grammar
      : proto::or_<
            char_literal_meta_grammar,
            char_meta_grammar1
        >
    {
    };

    ///////////////////////////////////////////////////////////////////////////
    //  These specializations non-intrusively hook into the Karma meta-grammar.
    //  (see karma/meta_grammar.hpp)
    ///////////////////////////////////////////////////////////////////////////
    template <typename Expr>
    struct is_valid_expr<Expr,
        typename enable_if<proto::matches<Expr, char_meta_grammar> >::type>
      : mpl::true_
    {
    };

    template <typename Expr>
    struct expr_transform<Expr,
        typename enable_if<proto::matches<Expr, char_meta_grammar> >::type>
      : mpl::identity<char_meta_grammar>
    {
    };

}}}

#endif
