///////////////////////////////////////////////////////////////////////////////
// as_sequence.hpp
//
//  Copyright 2008 Eric Niebler. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_XPRESSIVE_DETAIL_STATIC_TRANSFORMS_AS_SEQUENCE_HPP_EAN_04_01_2007
#define BOOST_XPRESSIVE_DETAIL_STATIC_TRANSFORMS_AS_SEQUENCE_HPP_EAN_04_01_2007

// MS compatible compilers support #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include <boost/mpl/assert.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/xpressive/detail/detail_fwd.hpp>
#include <boost/xpressive/detail/static/static.hpp>

namespace boost { namespace xpressive { namespace grammar_detail
{
    template<typename Grammar>
    struct in_sequence : proto::callable
    {
        template<typename Sig> struct result {};

        template<typename This, typename Expr, typename State, typename Visitor>
        struct result<This(Expr, State, Visitor)>
        {
            typedef detail::static_xpression<
                typename Grammar::template result<void(Expr, State, Visitor)>::type
              , State
            > type;
        };

        template<typename Expr, typename State, typename Visitor>
        typename result<void(Expr, State, Visitor)>::type
        operator ()(Expr const &expr, State const &state, Visitor &visitor) const
        {
            return typename result<void(Expr, State, Visitor)>::type(
                Grammar()(expr, state, visitor)
              , state
            );
        }
    };

}}}

#endif
