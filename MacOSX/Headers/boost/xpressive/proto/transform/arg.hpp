///////////////////////////////////////////////////////////////////////////////
/// \file arg.hpp
/// Contains definition of the argN transforms.
//
//  Copyright 2008 Eric Niebler. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PROTO_TRANSFORM_ARG_HPP_EAN_11_01_2007
#define BOOST_PROTO_TRANSFORM_ARG_HPP_EAN_11_01_2007

#include <boost/xpressive/proto/detail/prefix.hpp>
#include <boost/xpressive/proto/proto_fwd.hpp>
#include <boost/xpressive/proto/traits.hpp>
#include <boost/xpressive/proto/detail/suffix.hpp>

namespace boost { namespace proto
{

    namespace transform
    {

        /// \brief A PrimitiveTransform that returns the current expression
        /// unmodified
        struct expr : proto::callable
        {
            template<typename Sig>
            struct result;

            template<typename This, typename Expr, typename State, typename Visitor>
            struct result<This(Expr, State, Visitor)>
            {
                typedef Expr type;
            };

            /// \param expr_ The current expression.
            /// \return \c expr_
            /// \throw nothrow
            template<typename Expr, typename State, typename Visitor>
            Expr const &
            operator ()(Expr const &expr_, State const &, Visitor &) const
            {
                return expr_;
            }
        };

        /// \brief A PrimitiveTransform that returns the current state
        /// unmodified
        struct state : proto::callable
        {
            template<typename Sig>
            struct result;

            template<typename This, typename Expr, typename State, typename Visitor>
            struct result<This(Expr, State, Visitor)>
            {
                typedef State type;
            };

            /// \param state_ The current state.
            /// \return \c state_
            /// \throw nothrow
            template<typename Expr, typename State, typename Visitor>
            State const &
            operator ()(Expr const &, State const &state_, Visitor &) const
            {
                return state_;
            }
        };

        /// \brief A PrimitiveTransform that returns the current visitor
        /// unmodified
        struct visitor : proto::callable
        {
            template<typename Sig>
            struct result;

            template<typename This, typename Expr, typename State, typename Visitor>
            struct result<This(Expr, State, Visitor)>
            {
                typedef Visitor type;
            };

            /// \param visitor_ The current visitor
            /// \return \c visitor_
            /// \throw nothrow
            template<typename Expr, typename State, typename Visitor>
            Visitor &
            operator ()(Expr const &, State const &, Visitor &visitor_) const
            {
                return visitor_;
            }
        };

        /// \brief A PrimitiveTransform that returns I-th child of the current
        /// expression.
        template<int I>
        struct arg_c : proto::callable
        {
            template<typename Sig>
            struct result;

            template<typename This, typename Expr, typename State, typename Visitor>
            struct result<This(Expr, State, Visitor)>
            {
                typedef typename proto::result_of::arg_c<Expr, I>::type type;
            };

            /// \param expr The current expression.
            /// \return <tt>proto::arg_c\<I\>(expr)</tt>
            /// \throw nothrow
            template<typename Expr, typename State, typename Visitor>
            typename proto::result_of::arg_c<Expr, I>::const_reference
            operator ()(Expr const &expr, State const &, Visitor &) const
            {
                return proto::arg_c<I>(expr);
            }
        };

        /// \brief A unary CallableTransform that wraps its argument
        /// in a \c boost::reference_wrapper\<\>.
        struct _ref : proto::callable
        {
            template<typename Sig>
            struct result;

            template<typename This, typename T>
            struct result<This(T)>
            {
                typedef boost::reference_wrapper<T const> type;
            };

            template<typename This, typename T>
            struct result<This(T &)>
            {
                typedef boost::reference_wrapper<T> type;
            };

            /// \param t The object to wrap
            /// \return <tt>boost::ref(t)</tt>
            /// \throw nothrow
            template<typename T>
            boost::reference_wrapper<T>
            operator ()(T &t) const
            {
                return boost::reference_wrapper<T>(t);
            }

            /// \overload
            ///
            template<typename T>
            boost::reference_wrapper<T const>
            operator ()(T const &t) const
            {
                return boost::reference_wrapper<T const>(t);
            }
        };
    }

    /// \brief A PrimitiveTransform that returns I-th child of the current
    /// expression.
    template<int I>
    struct _arg_c
      : transform::arg_c<I>
    {};

    /// INTERNAL ONLY
    ///
    template<>
    struct is_callable<transform::expr>
      : mpl::true_
    {};

    /// INTERNAL ONLY
    ///
    template<>
    struct is_callable<transform::state>
      : mpl::true_
    {};

    /// INTERNAL ONLY
    ///
    template<>
    struct is_callable<transform::visitor>
      : mpl::true_
    {};

    /// INTERNAL ONLY
    ///
    template<int I>
    struct is_callable<transform::arg_c<I> >
      : mpl::true_
    {};

    /// INTERNAL ONLY
    ///
    template<int I>
    struct is_callable<_arg_c<I> >
      : mpl::true_
    {};

    /// INTERNAL ONLY
    ///
    template<>
    struct is_callable<transform::_ref>
      : mpl::true_
    {};

}}

#endif
