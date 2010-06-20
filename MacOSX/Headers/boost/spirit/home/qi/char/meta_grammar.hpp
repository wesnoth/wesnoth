/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(BOOST_SPIRIT_META_GRAMMAR_FEB_02_2007_0925AM)
#define BOOST_SPIRIT_META_GRAMMAR_FEB_02_2007_0925AM

#include <boost/spirit/home/qi/domain.hpp>
#include <boost/spirit/home/support/placeholders.hpp>
#include <boost/spirit/home/support/meta_grammar.hpp>
#include <boost/spirit/home/support/char_class.hpp>
#include <boost/spirit/home/qi/char/detail/basic_chset.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/shared_ptr.hpp>

namespace boost { namespace spirit { namespace qi
{
    ///////////////////////////////////////////////////////////////////////////
    // forwards
    ///////////////////////////////////////////////////////////////////////////
    template <typename Char>
    struct any_char;

    template <typename Char>
    struct literal_char;

    struct lazy_char;

    template <typename Char>
    struct char_range;

    struct lazy_char_range;

    template <typename Positive>
    struct negated_char_parser;

    template <typename Tag>
    struct char_class;

    struct char_meta_grammar;

    ///////////////////////////////////////////////////////////////////////////
    template <typename Expr, typename Enable>
    struct is_valid_expr;

    template <typename Expr, typename Enable>
    struct expr_transform;

    ///////////////////////////////////////////////////////////////////////////
    //  get the director of an any_char
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

    ///////////////////////////////////////////////////////////////////////////
    // get the director of a character range type
    ///////////////////////////////////////////////////////////////////////////
    template <typename Tag, typename T>
    struct extract_char_range_director;

    template <typename T>
    struct extract_char_range_director<tag::char_, T>
    {
        typedef char_range<T> type;
    };

    template <typename T>
    struct extract_char_range_director<tag::wchar, T>
    {
        typedef char_range<wchar_t> type;
    };

    ///////////////////////////////////////////////////////////////////////////
    // char parser meta-grammars
    ///////////////////////////////////////////////////////////////////////////

    // literals: 'x', L'x'
    struct basic_char_literal_meta_grammar
      : proto::or_<
            proto::terminal<char>
          , proto::terminal<wchar_t>
        >
    {
    };


    // literals: 'x', L'x' and single char strings: "x", L"x"
    struct single_char_literal_meta_grammar
      : proto::or_<
        // plain chars:
            proto::terminal<char>
          , proto::terminal<wchar_t>
        // single char null terminates strings:
          , proto::terminal<char[2]>
          , proto::terminal<char(&)[2]>
          , proto::terminal<wchar_t[2]>
          , proto::terminal<wchar_t(&)[2]>
        >
    {
    };

    // literals: 'x', L'x'
    struct char_literal_meta_grammar
      : proto::or_<
            meta_grammar::terminal_rule<
                qi::domain, char, literal_char<char>
            >
          , meta_grammar::terminal_rule<
                qi::domain, wchar_t, literal_char<wchar_t>
            >
        >
    {
    };

    // literal strings: "hello" (defined in qi/string/meta_grammar.hpp)
    struct basic_string_literal_meta_grammar;

    // std::string(s) (defined in qi/string/meta_grammar.hpp)
    struct basic_std_string_meta_grammar;

    template <typename T>
    struct extract_char; // (defined in qi/string/metagrammar.hpp)

    template <typename Tag, typename T>
    struct extract_chset_director;

    template <typename T>
    struct extract_chset_director<tag::char_, T>
    {
        typedef typename extract_char<T>::type char_type;
        typedef char_set<char_type> type;
    };

    template <typename T>
    struct extract_chset_director<tag::wchar, T>
    {
        typedef typename extract_char<T>::type char_type;
        typedef char_set<char_type> type;
    };

    template <typename Char, typename Elements>
    struct char_set_component
    {
        typedef qi::domain domain;
        typedef char_set<Char> director;
        typedef Elements elements_type;

        char_set_component(Char const* definition)
          : ptr(new detail::basic_chset<Char>())
        {
            Char ch = *definition++;
            while (ch)
            {
                Char next = *definition++;
                if (next == '-')
                {
                    next = *definition++;
                    if (next == 0)
                    {
                        ptr->set(ch);
                        ptr->set('-');
                        break;
                    }
                    ptr->set(ch, next);
                }
                else
                {
                    ptr->set(ch);
                }
                ch = next;
            }
        }

        template <typename CharSetClass> // no-case version
        char_set_component(Char const* definition, CharSetClass)
          : ptr(new detail::basic_chset<Char>())
        {
            Char ch = *definition++;
            while (ch)
            {
                Char next = *definition++;
                if (next == '-')
                {
                    next = *definition++;
                    if (next == 0)
                    {
                        ptr->set(CharSetClass::tolower(ch));
                        ptr->set(CharSetClass::tolower('-'));
                        ptr->set(CharSetClass::toupper(ch));
                        ptr->set(CharSetClass::toupper('-'));
                        break;
                    }
                    ptr->set(CharSetClass::tolower(ch)
                      , CharSetClass::tolower(next));
                    ptr->set(CharSetClass::toupper(ch)
                      , CharSetClass::toupper(next));
                }
                else
                {
                    ptr->set(CharSetClass::tolower(ch));
                    ptr->set(CharSetClass::toupper(ch));
                }
                ch = next;
            }
        }

        boost::shared_ptr<detail::basic_chset<Char> > ptr;
    };

    // char_, char_('x'), char_("x"), char_(f), char_('x', 'z'),
    // char_(L'x'), char_(L'x', L'z'),
    // wchar, wchar('x'), wchar("x"), wchar('x', 'z'),
    // wchar(L'x'), wchar(L'x', L'z')
    // char_("a-z"), wchar("a-z")
    // [w]lit('x'), [w]lit(L'x')
    struct char_meta_grammar1
      : proto::or_<
            // char_, wchar --> any_char
            meta_grammar::compose_empty<
                proto::if_<
                    is_char_tag<proto::_arg, qi::domain>()
                >
              , qi::domain
              , mpl::identity<extract_any_char_director<mpl::_> >
            >
            // char_('x'), wchar(L'x'), char_("x"), wchar(L"x")--> literal_char
          , meta_grammar::compose_function1_eval<
                proto::function<
                    proto::if_<
                        is_char_tag<proto::_arg, qi::domain>()
                    >
                  , single_char_literal_meta_grammar
                >
              , qi::domain
              , mpl::identity<extract_literal_char_director<mpl::_, mpl::_> >
            >
            // lit("x"), wlit(L"x") --> literal_char
          , meta_grammar::compose_function1_eval<
                proto::function<
                    proto::if_<
                        is_lit_tag<proto::_arg, qi::domain>()
                    >
                  , basic_char_literal_meta_grammar
                >
              , qi::domain
              , mpl::identity<extract_literal_char_director<mpl::_, mpl::_> >
            >
            // char_("a-z"), char_(L"a-z"), wchar(L"a-z") --> char_set
          , meta_grammar::compose_function1_eval<
                proto::function<
                    proto::if_<
                        is_char_tag<proto::_arg, qi::domain>()>
                  , proto::or_<basic_string_literal_meta_grammar, basic_std_string_meta_grammar>
                >
              , qi::domain
              , mpl::identity<extract_chset_director<mpl::_, mpl::_> >
            >
            // char_(F()) --> lazy_char
          , meta_grammar::function1_rule<
                qi::domain
              , tag::char_
              , lazy_char
            >
            // char_('x', 'z'), wchar(L'x', L'z') --> char_range
          , meta_grammar::compose_function2_eval<
                proto::function<
                    proto::if_<
                        is_char_tag<proto::_arg, qi::domain>()
                    >
                  , basic_char_literal_meta_grammar
                  , basic_char_literal_meta_grammar
                >
              , qi::domain
              , mpl::identity<extract_char_range_director<mpl::_, mpl::_> >
            >
            // char_(F1(), F2()) --> lazy_char_range
          , meta_grammar::function2_rule<
                qi::domain
              , tag::char_
              , lazy_char_range
            >
        >
    {
    };

    // char_classes: alnum, alpha, cntrl, ... etc.
    struct char_class_meta_grammar
      : proto::or_<
            // alnum, alpha, cntrl, ... etc.
            meta_grammar::compose_empty<
                proto::terminal<spirit::char_class::key<proto::_, proto::_> >
              , qi::domain
              , char_class<mpl::_>
            >
          , meta_grammar::compose_empty<
                proto::terminal<spirit::char_class::lower_case_tag<proto::_> >
              , qi::domain
              , char_class<mpl::_>
            >
          , meta_grammar::compose_empty<
                proto::terminal<spirit::char_class::upper_case_tag<proto::_> >
              , qi::domain
              , char_class<mpl::_>
            >
        >
    {};

    // ~x (where x is a char_parser)
    struct negated_char_meta_grammar
      : meta_grammar::compose_single<
            proto::unary_expr<
                proto::tag::complement
              , char_meta_grammar
            >
          , qi::domain
          , negated_char_parser<mpl::_>
        >
    {
    };

    // main char_meta_grammar
    struct char_meta_grammar
      : proto::or_<
            char_meta_grammar1
          , char_class_meta_grammar
          , char_literal_meta_grammar
          , negated_char_meta_grammar
        >
    {
    };

    ///////////////////////////////////////////////////////////////////////////
    //  These specializations non-intrusively hooks into the RD meta-grammar.
    //  (see qi/meta_grammar.hpp)
    ///////////////////////////////////////////////////////////////////////////
    template <typename Expr>
    struct is_valid_expr<Expr
      , typename enable_if<proto::matches<Expr, char_meta_grammar> >::type>
      : mpl::true_
    {
    };

    template <typename Expr>
    struct expr_transform<Expr
      , typename enable_if<proto::matches<Expr, char_meta_grammar> >::type>
      : mpl::identity<char_meta_grammar>
    {
    };
}}}

#endif
