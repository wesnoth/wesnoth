///////////////////////////////////////////////////////////////////////////////
// as_action.hpp
//
//  Copyright 2008 Eric Niebler.
//  Copyright 2008 David Jenkins.
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_XPRESSIVE_DETAIL_STATIC_TRANSFORMS_AS_ACTION_HPP_EAN_04_05_2007
#define BOOST_XPRESSIVE_DETAIL_STATIC_TRANSFORMS_AS_ACTION_HPP_EAN_04_05_2007

// MS compatible compilers support #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include <boost/mpl/sizeof.hpp>
#include <boost/mpl/min_max.hpp>
#include <boost/mpl/apply_wrap.hpp>
#include <boost/xpressive/detail/detail_fwd.hpp>
#include <boost/xpressive/detail/core/matcher/attr_end_matcher.hpp>
#include <boost/xpressive/detail/static/static.hpp>
#include <boost/xpressive/detail/static/transforms/as_quantifier.hpp>
#include <boost/xpressive/proto/proto.hpp>
#include <boost/xpressive/proto/transform.hpp>

namespace boost { namespace xpressive { namespace detail
{
    ///////////////////////////////////////////////////////////////////////////////
    // read_attr
    //  Placeholder that knows the slot number of an attribute as well as the type
    //  of the object stored in it.
    template<typename Nbr, typename Matcher>
    struct read_attr
    {
        typedef Nbr nbr_type;
        typedef Matcher matcher_type;
    };

    template<typename Nbr, typename Matcher>
    struct read_attr<Nbr, Matcher &>
    {
        typedef Nbr nbr_type;
        typedef Matcher matcher_type;
    };

}}}

namespace boost { namespace xpressive { namespace grammar_detail
{
    ///////////////////////////////////////////////////////////////////////////////
    // FindAttr
    //  Look for patterns like (a1= terminal<RHS>) and return the type of the RHS.
    template<typename Nbr>
    struct FindAttr
      : or_<
            // Ignore nested actions, because attributes are scoped
            when< subscript<_, _>,                                  _state >
          , when< terminal<_>,                                      _state >
          , when< proto::assign<terminal<detail::attribute_placeholder<Nbr> >, _>, call<_arg(_right)> >
          , otherwise< fold<_, _state, FindAttr<Nbr> > >
        >
    {};

    ///////////////////////////////////////////////////////////////////////////////
    // as_read_attr
    //  For patterns like (a1 = RHS)[ref(i) = a1], transform to
    //  (a1 = RHS)[ref(i) = read_attr<1, RHS>] so that when reading the attribute
    //  we know what type is stored in the attribute slot.
    struct as_read_attr : proto::callable
    {
        template<typename Sig> struct result {};

        template<typename This, typename Expr, typename State, typename Visitor>
        struct result<This(Expr, State, Visitor)>
        {
            typedef
                typename proto::result_of::as_expr<
                    detail::read_attr<
                        typename Expr::proto_arg0::nbr_type
                      , typename FindAttr<typename Expr::proto_arg0::nbr_type>::template result<void(
                            State
                          , mpl::void_
                          , int
                        )>::type
                    >
                >::type
            type;
        };

        template<typename Expr, typename State, typename Visitor>
        typename result<void(Expr, State, Visitor)>::type
        operator ()(Expr const &, State const &, Visitor &) const
        {
            typename result<void(Expr, State, Visitor)>::type that = {{}};
            return that;
        }
    };

    ///////////////////////////////////////////////////////////////////////////////
    // by_value
    //  Store all terminals within an action by value to avoid dangling references.
    struct by_value : proto::callable
    {
        template<typename Sig> struct result {};

        template<typename This, typename Expr, typename State, typename Visitor>
        struct result<This(Expr, State, Visitor)>
        {
            typedef
                typename proto::result_of::as_expr<
                    typename proto::result_of::arg<Expr>::type
                >::type
            type;
        };

        template<typename Expr, typename State, typename Visitor>
        typename result<void(Expr, State, Visitor)>::type
        operator ()(Expr const &expr, State const &, Visitor &) const
        {
            return proto::as_expr(proto::arg(expr));
        }
    };

    ///////////////////////////////////////////////////////////////////////////////
    // DeepCopy
    //  Turn all refs into values, and also bind all attribute placeholders with
    //  the types from which they are being assigned.
    struct DeepCopy
      : or_<
            when< terminal<detail::attribute_placeholder<_> >,  as_read_attr>
          , when< terminal<_>,                          by_value >
          , otherwise< nary_expr<_, vararg<DeepCopy> > >
        >
    {};

    ///////////////////////////////////////////////////////////////////////////////
    // attr_nbr
    //  For an attribute placeholder, return the attribute's slot number.
    struct attr_nbr : proto::callable
    {
        template<typename Sig> struct result {};

        template<typename This, typename Expr, typename State, typename Visitor>
        struct result<This(Expr, State, Visitor)>
        {
            typedef typename Expr::proto_arg0::nbr_type::type type;
        };
    };

    struct max_attr;

    ///////////////////////////////////////////////////////////////////////////////
    // MaxAttr
    //  In an action (rx)[act], find the largest attribute slot being used.
    struct MaxAttr
      : or_<
            when< terminal<detail::attribute_placeholder<_> >, attr_nbr>
          , when< terminal<_>, make<mpl::int_<0> > >
            // Ignore nested actions, because attributes are scoped:
          , when< subscript<_, _>, make<mpl::int_<0> > >
          , otherwise< fold<_, make<mpl::int_<0> >, max_attr> >
        >
    {};

    ///////////////////////////////////////////////////////////////////////////////
    // max_attr
    //  Take the maximum of the current attr slot number and the state.
    struct max_attr : proto::callable
    {
        template<typename Sig> struct result {};

        template<typename This, typename Expr, typename State, typename Visitor>
        struct result<This(Expr, State, Visitor)>
        {
            typedef typename mpl::max<State, typename MaxAttr::template result<void(Expr, State, Visitor)>::type >::type type;
        };
    };

    ///////////////////////////////////////////////////////////////////////////////
    // as_attr_matcher
    //  turn a1=matcher into attr_matcher<Matcher>(1)
    struct as_attr_matcher : proto::callable
    {
        template<typename Sig> struct result {};

        template<typename This, typename Expr, typename State, typename Visitor>
        struct result<This(Expr, State, Visitor)>
        {
            typedef
                detail::attr_matcher<
                    typename proto::result_of::arg<typename Expr::proto_arg1>::type
                  , typename Visitor::traits_type
                  , typename Visitor::icase_type
                >
            type;
        };

        template<typename Expr, typename State, typename Visitor>
        typename result<void(Expr, State, Visitor)>::type
        operator ()(Expr const &expr, State const &, Visitor &visitor) const
        {
            return typename result<void(Expr, State, Visitor)>::type(
                Expr::proto_arg0::proto_base_expr::proto_arg0::nbr_type::value
              , proto::arg(proto::right(expr))
              , visitor.traits()
            );
        }
    };

    ///////////////////////////////////////////////////////////////////////////////
    // add_attrs
    //  Wrap an expression in attr_begin_matcher/attr_end_matcher pair
    struct add_attrs : proto::callable
    {
        template<typename Sig> struct result {};

        template<typename This, typename Expr, typename State, typename Visitor>
        struct result<This(Expr, State, Visitor)>
        {
            typedef
                typename shift_right<
                    typename terminal<
                        detail::attr_begin_matcher<typename MaxAttr::template result<void(Expr, mpl::int_<0>, int)>::type >
                    >::type
                  , typename shift_right<
                        Expr
                      , terminal<detail::attr_end_matcher>::type
                    >::type
                >::type
            type;
        };

        template<typename Expr, typename State, typename Visitor>
        typename result<void(Expr, State, Visitor)>::type
        operator ()(Expr const &expr, State const &, Visitor &) const
        {
            detail::attr_begin_matcher<typename MaxAttr::template result<void(Expr, mpl::int_<0>, int)>::type > begin;
            detail::attr_end_matcher end;
            typename result<void(Expr, State, Visitor)>::type that = {{begin}, {expr, {end}}};
            return that;
        }
    };

    ///////////////////////////////////////////////////////////////////////////////
    // InsertAttrs
    struct InsertAttrs
      : if_<MaxAttr, add_attrs, _>
    {};

    ///////////////////////////////////////////////////////////////////////////////
    // CheckAssertion
    struct CheckAssertion
      : proto::function<terminal<detail::check_tag>, _>
    {};

    ///////////////////////////////////////////////////////////////////////////////
    // action_transform
    //  Turn A[B] into (mark_begin(n) >> A >> mark_end(n) >> action_matcher<B>(n))
    //  If A and B use attributes, wrap the above expression in
    //  a attr_begin_matcher<Count> / attr_end_matcher pair, where Count is
    //  the number of attribute slots used by the pattern/action.
    struct as_action : proto::callable
    {
        template<typename Sig> struct result {};

        template<typename This, typename Expr, typename State, typename Visitor>
        struct result<This(Expr, State, Visitor)>
        {
            typedef typename proto::result_of::left<Expr>::type expr_type;
            typedef typename proto::result_of::right<Expr>::type action_type;
            typedef typename DeepCopy::template result<void(action_type, expr_type, int)>::type action_copy_type;

            typedef
                typename InsertMark::template result<void(expr_type, State, Visitor)>::type
            marked_expr_type;

            typedef
                typename mpl::if_<
                    proto::matches<action_type, CheckAssertion>
                  , detail::predicate_matcher<action_copy_type>
                  , detail::action_matcher<action_copy_type>
                >::type
            matcher_type;

            typedef
                typename proto::shift_right<
                    marked_expr_type
                  , typename proto::terminal<matcher_type>::type
                >::type
            no_attr_type;

            typedef
                typename InsertAttrs::template result<void(no_attr_type, State, Visitor)>::type
            type;
        };

        template<typename Expr, typename State, typename Visitor>
        typename result<void(Expr, State, Visitor)>::type
        operator ()(Expr const &expr, State const &state, Visitor &visitor) const
        {
            typedef result<void(Expr, State, Visitor)> apply_type;
            typedef typename apply_type::matcher_type matcher_type;

            int dummy = 0;
            typename apply_type::marked_expr_type marked_expr =
                InsertMark()(proto::left(expr), state, visitor);

            typename apply_type::no_attr_type that =
            {
                marked_expr
              , {
                    matcher_type
                    (
                        DeepCopy()(proto::right(expr), proto::left(expr), dummy)
                      , proto::arg(proto::left(marked_expr)).mark_number_
                    )
                }
            };

            return InsertAttrs()(that, state, visitor);
        }
    };

}}}

#endif
