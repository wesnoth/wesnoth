///////////////////////////////////////////////////////////////////////////////
// as_modifier.hpp
//
//  Copyright 2008 Eric Niebler. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_XPRESSIVE_DETAIL_STATIC_TRANSFORMS_AS_MODIFIER_HPP_EAN_04_05_2007
#define BOOST_XPRESSIVE_DETAIL_STATIC_TRANSFORMS_AS_MODIFIER_HPP_EAN_04_05_2007

// MS compatible compilers support #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include <boost/mpl/sizeof.hpp>
#include <boost/xpressive/detail/detail_fwd.hpp>
#include <boost/xpressive/detail/static/static.hpp>
#include <boost/xpressive/proto/proto.hpp>

#define UNCV(x) typename remove_const<x>::type
#define UNREF(x) typename remove_reference<x>::type
#define UNCVREF(x) UNCV(UNREF(x))

namespace boost { namespace xpressive { namespace detail
{
    ///////////////////////////////////////////////////////////////////////////////
    // regex operator tags
    struct modifier_tag
    {};

}}}

namespace boost { namespace xpressive { namespace grammar_detail
{

    ///////////////////////////////////////////////////////////////////////////////
    // as_modifier
    template<typename Grammar>
    struct as_modifier : proto::callable
    {
        template<typename Sig> struct result {};

        template<typename This, typename Expr, typename State, typename Visitor>
        struct result<This(Expr, State, Visitor)>
        {
            typedef typename proto::result_of::arg<typename proto::result_of::left<Expr>::type>::type modifier_type;
            typedef typename modifier_type::BOOST_NESTED_TEMPLATE apply<Visitor>::type visitor_type;
            typedef typename Grammar::template result<void(typename proto::result_of::right<Expr>::type, State, visitor_type)>::type type;
        };

        template<typename Expr, typename State, typename Visitor>
        typename result<void(Expr, State, Visitor)>::type
        operator ()(Expr const &expr, State const &state, Visitor &visitor) const
        {
            typedef result<void(Expr, State, Visitor)> result_;
            typedef typename result_::visitor_type new_visitor_type;
            new_visitor_type new_visitor(proto::arg(proto::left(expr)).call(visitor));
            return Grammar()(proto::right(expr), state, new_visitor);
        }
    };

}}}

#undef UNCV
#undef UNREF
#undef UNCVREF

#endif
