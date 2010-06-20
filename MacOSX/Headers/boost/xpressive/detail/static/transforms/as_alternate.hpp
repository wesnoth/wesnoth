///////////////////////////////////////////////////////////////////////////////
// as_alternate.hpp
//
//  Copyright 2008 Eric Niebler. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_XPRESSIVE_DETAIL_STATIC_TRANSFORMS_AS_ALTERNATE_HPP_EAN_04_01_2007
#define BOOST_XPRESSIVE_DETAIL_STATIC_TRANSFORMS_AS_ALTERNATE_HPP_EAN_04_01_2007

// MS compatible compilers support #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include <boost/config.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/xpressive/proto/proto.hpp>
#include <boost/xpressive/detail/detail_fwd.hpp>
#include <boost/xpressive/detail/static/static.hpp>
#include <boost/xpressive/detail/core/matcher/alternate_matcher.hpp>
#include <boost/xpressive/detail/utility/cons.hpp>

namespace boost { namespace xpressive
{
    namespace detail
    {
        ///////////////////////////////////////////////////////////////////////////////
        // alternates_list
        //   a fusion-compatible sequence of alternate expressions, that also keeps
        //   track of the list's width and purity.
        template<typename Head, typename Tail>
        struct alternates_list
          : fusion::cons<Head, Tail>
        {
            BOOST_STATIC_CONSTANT(std::size_t, width = Head::width == Tail::width ? Head::width : detail::unknown_width::value);
            BOOST_STATIC_CONSTANT(bool, pure = Head::pure && Tail::pure);

            alternates_list(Head const &head, Tail const &tail)
              : fusion::cons<Head, Tail>(head, tail)
            {
            }
        };

        template<typename Head>
        struct alternates_list<Head, fusion::nil>
          : fusion::cons<Head, fusion::nil>
        {
            BOOST_STATIC_CONSTANT(std::size_t, width = Head::width);
            BOOST_STATIC_CONSTANT(bool, pure = Head::pure);

            alternates_list(Head const &head, fusion::nil const &tail)
              : fusion::cons<Head, fusion::nil>(head, tail)
            {
            }
        };
    }

    namespace grammar_detail
    {
        ///////////////////////////////////////////////////////////////////////////////
        // in_alternate_list
        template<typename Grammar>
        struct in_alternate_list : proto::callable
        {
            template<typename Sig> struct result {};

            template<typename This, typename Expr, typename State, typename Visitor>
            struct result<This(Expr, State, Visitor)>
            {
                typedef detail::alternates_list<
                    typename Grammar::template result<void(Expr, detail::alternate_end_xpression, Visitor)>::type
                  , State
                > type;
            };

            template<typename Expr, typename State, typename Visitor>
            typename result<void(Expr, State, Visitor)>::type
            operator ()(Expr const &expr, State const &state, Visitor &visitor) const
            {
                return typename result<void(Expr, State, Visitor)>::type(
                    Grammar()(expr, detail::alternate_end_xpression(), visitor)
                  , state
                );
            }
        };

        ///////////////////////////////////////////////////////////////////////////////
        // as_alternate_matcher
        template<typename Grammar>
        struct as_alternate_matcher : proto::callable
        {
            template<typename Sig> struct result {};

            template<typename This, typename Expr, typename State, typename Visitor>
            struct result<This(Expr, State, Visitor)>
            {
                typedef detail::alternate_matcher<
                    typename Grammar::template result<void(Expr, State, Visitor)>::type
                  , typename Visitor::traits_type
                > type;
            };

            template<typename Expr, typename State, typename Visitor>
            typename result<void(Expr, State, Visitor)>::type
            operator ()(Expr const &expr, State const &state, Visitor &visitor) const
            {
                return typename result<void(Expr, State, Visitor)>::type(
                    Grammar()(expr, state, visitor)
                );
            }
        };

    }

}}

#endif
