//  Copyright (c) 2001-2008 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_KARMA_META_GRAMMAR_MAY_04_2007_0853AM)
#define BOOST_SPIRIT_KARMA_META_GRAMMAR_MAY_04_2007_0853AM

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once      // MS compatible compilers support #pragma once
#endif

#include <boost/spirit/home/karma/domain.hpp>
#include <boost/spirit/home/support/placeholders.hpp>
#include <boost/spirit/home/support/meta_grammar.hpp>
#include <boost/spirit/home/support/detail/integer/endian.hpp>
#include <boost/mpl/identity.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/utility/enable_if.hpp>

namespace boost { namespace spirit { namespace karma
{
    ///////////////////////////////////////////////////////////////////////////
    // forwards
    /////////////////////////////////////////////////////////////////////////// 
    template <integer::endianness endian, int bits>
    struct any_binary_director;
    
    template <integer::endianness endian, int bits>
    struct binary_lit_director;

    struct binary_padding_director;
    
    struct main_meta_grammar;

    template <typename Expr, typename Enable>
    struct is_valid_expr;

    template <typename Expr, typename Enable>
    struct expr_transform;

    ///////////////////////////////////////////////////////////////////////////
    // get the director of an integer based binary literal type
    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    struct extract_literal_bin_director
    {
        typedef binary_lit_director<
            boost::integer::native, sizeof(T)*CHAR_BIT
        > type;
    };

    ///////////////////////////////////////////////////////////////////////////
    // get the director of a binary tag
    ///////////////////////////////////////////////////////////////////////////
    template <typename Tag>
    struct extract_binary_director;

    // native endian binaries
    template <>
    struct extract_binary_director<tag::byte>    
    {
        typedef any_binary_director<boost::integer::native, 8> type;
    };

    template <>
    struct extract_binary_director<tag::word>    
    {
        typedef any_binary_director<boost::integer::native, 16> type;
    };

    template <>
    struct extract_binary_director<tag::dword>    
    {
        typedef any_binary_director<boost::integer::native, 32> type;
    };

    // big endian binaries
    template <>
    struct extract_binary_director<tag::big_word>    
    {
        typedef any_binary_director<boost::integer::big, 16> type;
    };

    template <>
    struct extract_binary_director<tag::big_dword>    
    {
        typedef any_binary_director<boost::integer::big, 32> type;
    };

    // little endian binaries
    template <>
    struct extract_binary_director<tag::little_word>    
    {
        typedef any_binary_director<boost::integer::little, 16> type;
    };

    template <>
    struct extract_binary_director<tag::little_dword>    
    {
        typedef any_binary_director<boost::integer::little, 32> type;
    };

#ifdef BOOST_HAS_LONG_LONG
    template <>
    struct extract_binary_director<tag::qword>    
    {
        typedef any_binary_director<boost::integer::native, 64> type;
    };

    template <>
    struct extract_binary_director<tag::big_qword>    
    {
        typedef any_binary_director<boost::integer::big, 64> type;
    };

    template <>
    struct extract_binary_director<tag::little_qword>    
    {
        typedef any_binary_director<boost::integer::little, 64> type;
    };
#endif

    ///////////////////////////////////////////////////////////////////////////
    // get the director of a binary literal tag
    ///////////////////////////////////////////////////////////////////////////
    template <typename Tag, typename T>
    struct extract_binary_lit_director;

    // native endian binaries
    template <typename T>
    struct extract_binary_lit_director<tag::byte, T>    
    {
        typedef binary_lit_director<boost::integer::native, 8> type;
    };

    template <typename T>
    struct extract_binary_lit_director<tag::word, T>    
    {
        typedef binary_lit_director<boost::integer::native, 16> type;
    };

    template <typename T>
    struct extract_binary_lit_director<tag::dword, T>    
    {
        typedef binary_lit_director<boost::integer::native, 32> type;
    };

    // big endian binaries
    template <typename T>
    struct extract_binary_lit_director<tag::big_word, T>    
    {
        typedef binary_lit_director<boost::integer::big, 16> type;
    };

    template <typename T>
    struct extract_binary_lit_director<tag::big_dword, T>    
    {
        typedef binary_lit_director<boost::integer::big, 32> type;
    };

    // little endian binaries
    template <typename T>
    struct extract_binary_lit_director<tag::little_word, T>    
    {
        typedef binary_lit_director<boost::integer::little, 16> type;
    };

    template <typename T>
    struct extract_binary_lit_director<tag::little_dword, T>    
    {
        typedef binary_lit_director<boost::integer::little, 32> type;
    };

#ifdef BOOST_HAS_LONG_LONG
    template <typename T>
    struct extract_binary_lit_director<tag::qword, T>    
    {
        typedef binary_lit_director<boost::integer::native, 64> type;
    };

    template <typename T>
    struct extract_binary_lit_director<tag::big_qword, T>    
    {
        typedef binary_lit_director<boost::integer::big, 64> type;
    };

    template <typename T>
    struct extract_binary_lit_director<tag::little_qword, T>    
    {
        typedef binary_lit_director<boost::integer::little, 64> type;
    };
#endif

    ///////////////////////////////////////////////////////////////////////////
    // binary meta-grammar
    ///////////////////////////////////////////////////////////////////////////
    // literals: 10, 10L, 10LL
    struct int_binary_meta_grammar
      : meta_grammar::compose_empty<
            proto::if_<
                is_int_lit_tag<proto::_arg, karma::domain>()
            >,
            karma::domain,
            mpl::identity<extract_literal_bin_director<mpl::_> >
        >
    {
    };

    struct binary_meta_grammar  
      : proto::or_<
            meta_grammar::compose_empty<
                proto::if_<
                    is_binary_tag<proto::_arg, karma::domain>()
                >,
                karma::domain, 
                mpl::identity<extract_binary_director<mpl::_> > 
            >,
            meta_grammar::compose_function1_eval<
                proto::function<
                    proto::if_<
                        is_binary_tag<proto::_arg, karma::domain>()
                    >,
                    int_binary_meta_grammar
                >,
                karma::domain,
                mpl::identity<extract_binary_lit_director<mpl::_, mpl::_> >
            >,
            meta_grammar::function1_rule<
                karma::domain, tag::pad, binary_padding_director
            >
        >
    {
    };

    ///////////////////////////////////////////////////////////////////////////
    //  These specializations non-intrusively hooks into the Karma meta-grammar.
    //  (see karma/meta_grammar.hpp)
    ///////////////////////////////////////////////////////////////////////////
    template <typename Expr>
    struct is_valid_expr<
            Expr,
            typename enable_if<
                proto::matches<Expr, binary_meta_grammar> 
            >::type
        >
      : mpl::true_
    {
    };

    template <typename Expr>
    struct expr_transform<
            Expr,
            typename enable_if<
                proto::matches<Expr, binary_meta_grammar> 
            >::type
        >
      : mpl::identity<binary_meta_grammar>
    {
    };
    
}}}

#endif
