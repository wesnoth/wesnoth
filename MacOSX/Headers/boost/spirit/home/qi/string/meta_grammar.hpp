/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(BOOST_SPIRIT_META_GRAMMAR_FEB_03_2007_0356PM)
#define BOOST_SPIRIT_META_GRAMMAR_FEB_03_2007_0356PM

#include <boost/spirit/home/qi/domain.hpp>
#include <boost/spirit/home/support/placeholders.hpp>
#include <boost/spirit/home/support/meta_grammar.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/remove_const.hpp>
#include <boost/type_traits/is_convertible.hpp>
#include <string>

namespace boost { namespace spirit { namespace qi
{
    ///////////////////////////////////////////////////////////////////////////
    // forwards
    ///////////////////////////////////////////////////////////////////////////
    template<typename Char>
    struct literal_string;

    struct lazy_string;

    template <typename Filter>
    struct symbols_director;

    struct string_meta_grammar;

    template <typename Expr, typename Enable>
    struct is_valid_expr;

    template <typename Expr, typename Enable>
    struct expr_transform;

    template <typename Lookup>
    struct symbols_lookup;

    ///////////////////////////////////////////////////////////////////////////
    // get the director of a string literal type
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

    template <typename Tag, typename T>
    struct extract_lit_director;

    template <typename T>
    struct extract_lit_director<tag::lit, T>
    {
        typedef typename extract_char<T>::type char_type;
        typedef literal_string<char_type> type;
    };

    template <typename T>
    struct extract_lit_director<tag::wlit, T>
    {
        typedef typename extract_char<T>::type char_type;
        typedef literal_string<char_type> type;
    };

    ///////////////////////////////////////////////////////////////////////////
    // string parser meta-grammars
    ///////////////////////////////////////////////////////////////////////////

    // literal strings: "hello"
    struct string_literal_meta_grammar
      : proto::or_<
            meta_grammar::terminal_rule<
                qi::domain, char const*, literal_string<char> >
          , meta_grammar::terminal_rule<
                qi::domain, char*, literal_string<char> >
          , meta_grammar::terminal_rule<
                qi::domain, wchar_t const*, literal_string<wchar_t> >
          , meta_grammar::terminal_rule<
                qi::domain, wchar_t*, literal_string<wchar_t> >

        >
    {
    };

    // literal strings: "hello"
    struct basic_string_literal_meta_grammar
      : proto::or_<
            proto::terminal<char const*>
          , proto::terminal<wchar_t const*>
        >
    {
    };

    // std::string(s)
    struct basic_std_string_meta_grammar
      : proto::or_<
            proto::terminal<std::basic_string<char, proto::_, proto::_> >
          , proto::terminal<std::basic_string<wchar_t, proto::_, proto::_> >
        >
    {
    };

    // std::string(s)
    struct std_string_meta_grammar
      : proto::or_<
            meta_grammar::terminal_rule<
                qi::domain
              , std::basic_string<char, proto::_, proto::_>
              , literal_string<char> >
          , meta_grammar::terminal_rule<
                qi::domain
              , std::basic_string<wchar_t, proto::_, proto::_>
              , literal_string<wchar_t> >
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
        {};
    }

    // strings: "hello", lit("hello"), lit(str), lit(f), symbols
    struct string_meta_grammar
      : proto::or_<
            string_literal_meta_grammar
          , std_string_meta_grammar
          , meta_grammar::compose_function1_eval<
                proto::function<
                    proto::if_<
                        is_lit_tag<proto::_arg, qi::domain>()>
                  , proto::or_<basic_string_literal_meta_grammar, basic_std_string_meta_grammar>
                >
              , qi::domain
              , mpl::identity<extract_lit_director<mpl::_, mpl::_> >
            >
          , meta_grammar::function1_rule<
                qi::domain
              , tag::lit
              , lazy_string
              , proto::if_<
                    detail::is_not_convertible_to_int<proto::_arg >() >
            >
          , meta_grammar::terminal_rule<
                qi::domain
              , symbols_lookup<proto::_>
              , symbols_director<>
            >
        >
    {
    };

    ///////////////////////////////////////////////////////////////////////////
    //  These specializations non-intrusively hooks into the RD meta-grammar.
    //  (see qi/meta_grammar.hpp)
    ///////////////////////////////////////////////////////////////////////////
    template <typename Expr>
    struct is_valid_expr<Expr
      , typename enable_if<proto::matches<Expr, string_meta_grammar> >::type>
      : mpl::true_
    {
    };

    template <typename Expr>
    struct expr_transform<Expr
      , typename enable_if<proto::matches<Expr, string_meta_grammar> >::type>
      : mpl::identity<string_meta_grammar>
    {
    };
}}}

#endif
