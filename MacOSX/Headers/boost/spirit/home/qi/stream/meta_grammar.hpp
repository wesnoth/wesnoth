//  Copyright (c) 2001-2008 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_META_GRAMMAR_MAY_05_2007_1230PM)
#define BOOST_SPIRIT_META_GRAMMAR_MAY_05_2007_1230PM

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once      // MS compatible compilers support #pragma once
#endif

#include <boost/spirit/home/qi/domain.hpp>
#include <boost/spirit/home/support/placeholders.hpp>
#include <boost/spirit/home/support/meta_grammar.hpp>
#include <boost/utility/enable_if.hpp>

namespace boost { namespace spirit 
{
    namespace qi
    {
        template <typename T, typename Char>
        struct stream_tag;
    }

    template <typename T, typename Char>
    struct is_stream_tag<qi::stream_tag<T, Char>, qi::domain> 
      : mpl::true_ {};
}}

namespace boost { namespace spirit { namespace qi
{
    ///////////////////////////////////////////////////////////////////////////
    // forwards
    /////////////////////////////////////////////////////////////////////////// 
    template <typename Char, typename T>
    struct any_stream;
    
    template <typename Char>
    struct stream_director;
    
    struct main_meta_grammar;

    template <typename Expr, typename Enable>
    struct is_valid_expr;

    template <typename Expr, typename Enable>
    struct expr_transform;

    ///////////////////////////////////////////////////////////////////////////
    // stream tag
    ///////////////////////////////////////////////////////////////////////////
    template <typename T, typename Char>
    struct stream_tag
    {
    };

    ///////////////////////////////////////////////////////////////////////////
    // stream specs
    ///////////////////////////////////////////////////////////////////////////
    template <typename T, typename Char = char>
    struct typed_stream
      : proto::terminal<stream_tag<T, Char> >::type
    {
    };

    ///////////////////////////////////////////////////////////////////////////
    // get the director for a stream
    ///////////////////////////////////////////////////////////////////////////
    template <typename Tag>
    struct extract_stream_director;
    
    template <>
    struct extract_stream_director<tag::stream>
    {
        typedef any_stream<char> type;
    };

    template <>
    struct extract_stream_director<tag::wstream>
    {
        typedef any_stream<wchar_t> type;
    };

    template <typename T, typename Char>
    struct extract_stream_director<stream_tag<T, Char> >
    {
        typedef any_stream<Char, T> type;
    };

    ///////////////////////////////////////////////////////////////////////////
    // utility meta-grammar
    ///////////////////////////////////////////////////////////////////////////
    struct utility_meta_grammar :  
        // stream, wstream
        meta_grammar::compose_empty<    
            proto::if_<
                is_stream_tag<proto::_arg, qi::domain>()
            >,
            qi::domain,
            mpl::identity<extract_stream_director<mpl::_> >
        >
    {
    };

    ///////////////////////////////////////////////////////////////////////////
    //  These specializations non-intrusively hooks into the Qi meta-grammar.
    //  (see qi/meta_grammar.hpp)
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
