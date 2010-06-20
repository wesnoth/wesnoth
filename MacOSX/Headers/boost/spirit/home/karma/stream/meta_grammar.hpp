//  Copyright (c) 2001-2008 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_KARMA_META_GRAMMAR_MAY_01_2007_0313PM)
#define BOOST_SPIRIT_KARMA_META_GRAMMAR_MAY_01_2007_0313PM

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once      // MS compatible compilers support #pragma once
#endif

#include <boost/spirit/home/karma/domain.hpp>
#include <boost/spirit/home/support/meta_grammar.hpp>
#include <boost/utility/enable_if.hpp>

namespace boost { namespace spirit { namespace karma
{
    ///////////////////////////////////////////////////////////////////////////
    // forwards
    /////////////////////////////////////////////////////////////////////////// 
    template <typename Char>
    struct any_stream;
    
    template <typename Char>
    struct stream_director;
    
    struct main_meta_grammar;

    template <typename Expr, typename Enable>
    struct is_valid_expr;

    template <typename Expr, typename Enable>
    struct expr_transform;

    ///////////////////////////////////////////////////////////////////////////
    // get the director for a stream
    ///////////////////////////////////////////////////////////////////////////
    template <typename Tag>
    struct extract_any_stream_director;
    
    template <>
    struct extract_any_stream_director<tag::stream>
    {
        typedef any_stream<char> type;
    };

    template <>
    struct extract_any_stream_director<tag::wstream>
    {
        typedef any_stream<wchar_t> type;
    };

    ///////////////////////////////////////////////////////////////////////////
    template <typename Tag, typename T>
    struct extract_stream_director;
    
    template <typename T>
    struct extract_stream_director<tag::stream, T>
    {
        typedef stream_director<char> type;
    };

    template <typename T>
    struct extract_stream_director<tag::wstream, T>
    {
        typedef stream_director<wchar_t> type;
    };

    ///////////////////////////////////////////////////////////////////////////
    // utility meta-grammar
    ///////////////////////////////////////////////////////////////////////////
    struct utility_meta_grammar 
     :  proto::or_<
            // stream, wstream
            meta_grammar::compose_empty<
                proto::if_<
                    is_stream_tag<proto::_arg, karma::domain>()
                >,
                karma::domain,
                mpl::identity<extract_any_stream_director<mpl::_> >
            >,
            // stream(T), wstream(T)
            meta_grammar::compose_function1_eval<
                proto::function<
                    proto::if_<
                        is_stream_tag<proto::_arg, karma::domain>()
                    >,
                    proto::_
                >,
                karma::domain,
                mpl::identity<extract_stream_director<mpl::_, mpl::_> >
            >
        >
    {
    };

    ///////////////////////////////////////////////////////////////////////////
    //  These specializations non-intrusively hook into the Karma meta-grammar.
    //  (see karma/meta_grammar.hpp)
    ///////////////////////////////////////////////////////////////////////////
    template <typename Expr>
    struct is_valid_expr<
            Expr,
            typename enable_if<
                proto::matches<Expr, utility_meta_grammar> 
            >::type
        >
      : mpl::true_
    {
    };

    template <typename Expr>
    struct expr_transform<
            Expr,
            typename enable_if<
                proto::matches<Expr, utility_meta_grammar> 
            >::type
        >
      : mpl::identity<utility_meta_grammar>
    {
    };
    
}}}

#endif
