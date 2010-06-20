///////////////////////////////////////////////////////////////////////////////
// as_independent.hpp
//
//  Copyright 2008 Eric Niebler. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_XPRESSIVE_DETAIL_STATIC_TRANSFORMS_AS_INDEPENDENT_HPP_EAN_04_05_2007
#define BOOST_XPRESSIVE_DETAIL_STATIC_TRANSFORMS_AS_INDEPENDENT_HPP_EAN_04_05_2007

// MS compatible compilers support #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include <boost/mpl/sizeof.hpp>
#include <boost/xpressive/detail/detail_fwd.hpp>
#include <boost/xpressive/detail/static/static.hpp>
#include <boost/xpressive/proto/proto.hpp>
#include <boost/xpressive/proto/transform.hpp>

namespace boost { namespace xpressive { namespace detail
{
    struct keeper_tag
    {};

    struct lookahead_tag
    {};

    struct lookbehind_tag
    {};
}}}

namespace boost { namespace xpressive { namespace grammar_detail
{
    // A grammar that only accepts static regexes that
    // don't have semantic actions.
    struct NotHasAction
      : proto::switch_<struct NotHasActionCases>
    {};

    struct NotHasActionCases
    {
        template<typename Tag, int Dummy = 0>
        struct case_
          : proto::nary_expr<Tag, proto::vararg<NotHasAction> >
        {};

        template<int Dummy>
        struct case_<proto::tag::terminal, Dummy>
          : not_< or_<
                proto::terminal<detail::tracking_ptr<detail::regex_impl<_> > >,
                proto::terminal<reference_wrapper<_> >
            > >
        {};

        template<int Dummy>
        struct case_<proto::tag::comma, Dummy>
          : proto::_    // because (set='a','b') can't contain an action
        {};

        template<int Dummy>
        struct case_<proto::tag::complement, Dummy>
          : proto::_    // because in ~X, X can't contain an unscoped action
        {};

        template<int Dummy>
        struct case_<detail::lookahead_tag, Dummy>
          : proto::_    // because actions in lookaheads are scoped
        {};

        template<int Dummy>
        struct case_<detail::lookbehind_tag, Dummy>
          : proto::_    // because actions in lookbehinds are scoped
        {};

        template<int Dummy>
        struct case_<detail::keeper_tag, Dummy>
          : proto::_    // because actions in keepers are scoped
        {};

        template<int Dummy>
        struct case_<proto::tag::subscript, Dummy>
          : proto::subscript<detail::set_initializer_type, _>
        {}; // only accept set[...], not actions!
    };

    struct IndependentEndXpression
      : or_<
            when<NotHasAction, detail::true_xpression()>
          , otherwise<detail::independent_end_xpression()>
        >
    {};

    template<typename Grammar>
    struct as_lookahead : proto::callable
    {
        template<typename Sig> struct result {};

        template<typename This, typename Expr, typename State, typename Visitor>
        struct result<This(Expr, State, Visitor)>
        {
            typedef typename proto::result_of::arg<Expr>::type arg_type;
            
            typedef
                typename Grammar::template result<void(
                    arg_type
                  , typename IndependentEndXpression::result<void(arg_type, int, int)>::type
                  , Visitor
                )>::type
            xpr_type;
            typedef detail::lookahead_matcher<xpr_type> type;
        };

        template<typename Expr, typename State, typename Visitor>
        typename result<void(Expr, State, Visitor)>::type
        operator ()(Expr const &expr, State const &, Visitor &visitor) const
        {
            typedef result<void(Expr, State, Visitor)> result_type;
            int i = 0;
            return typename result_type::type(
                Grammar()(
                    proto::arg(expr)
                  , IndependentEndXpression()(proto::arg(expr), i, i)
                  , visitor
                )
              , false
            );
        }
    };

    template<typename Grammar>
    struct as_lookbehind : proto::callable
    {
        template<typename Sig> struct result {};

        template<typename This, typename Expr, typename State, typename Visitor>
        struct result<This(Expr, State, Visitor)>
        {
            typedef typename proto::result_of::arg<Expr>::type arg_type;
            typedef
                typename Grammar::template result<void(
                    arg_type
                  , typename IndependentEndXpression::result<void(arg_type, int, int)>::type
                  , Visitor
                )>::type
            xpr_type;
            typedef detail::lookbehind_matcher<xpr_type> type;
        };

        template<typename Expr, typename State, typename Visitor>
        typename result<void(Expr, State, Visitor)>::type
        operator ()(Expr const &expr, State const &, Visitor &visitor) const
        {
            typedef typename result<void(Expr, State, Visitor)>::xpr_type xpr_type;
            int i = 0;
            xpr_type const &expr2 = Grammar()(
                proto::arg(expr)
              , IndependentEndXpression()(proto::arg(expr), i, i)
              , visitor
            );
            std::size_t width = expr2.get_width().value();
            return detail::lookbehind_matcher<xpr_type>(expr2, width, false);
        }
    };

    template<typename Grammar>
    struct as_keeper : proto::callable
    {
        template<typename Sig> struct result {};

        template<typename This, typename Expr, typename State, typename Visitor>
        struct result<This(Expr, State, Visitor)>
        {
            typedef typename proto::result_of::arg<Expr>::type arg_type;
            typedef detail::keeper_matcher<
                typename Grammar::template result<void(
                    arg_type
                  , typename IndependentEndXpression::result<void(arg_type, int, int)>::type
                  , Visitor
                )>::type
            > type;
        };

        template<typename Expr, typename State, typename Visitor>
        typename result<void(Expr, State, Visitor)>::type
        operator ()(Expr const &expr, State const &, Visitor &visitor) const
        {
            int i = 0;
            return typename result<void(Expr, State, Visitor)>::type(
                Grammar()(
                    proto::arg(expr)
                  , IndependentEndXpression()(proto::arg(expr), i, i)
                  , visitor
                )
            );
        }
    };

}}}

#endif
