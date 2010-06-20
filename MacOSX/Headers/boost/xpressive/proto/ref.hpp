///////////////////////////////////////////////////////////////////////////////
/// \file ref.hpp
/// Utility for storing a sub-expr by reference
//
//  Copyright 2008 Eric Niebler. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PROTO_REF_HPP_EAN_04_01_2005
#define BOOST_PROTO_REF_HPP_EAN_04_01_2005

#include <boost/xpressive/proto/detail/prefix.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/type_traits/is_const.hpp>
#include <boost/xpressive/proto/proto_fwd.hpp>
#include <boost/xpressive/proto/detail/suffix.hpp>

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma warning(push)
# pragma warning(disable : 4510) // default constructor could not be generated
# pragma warning(disable : 4512) // assignment operator could not be generated
# pragma warning(disable : 4610) // user defined constructor required
#endif

namespace boost { namespace proto
{

#define BOOST_PROTO_ARG(z, n, data)                                                                 \
    typedef                                                                                         \
        typename Expr::BOOST_PP_CAT(proto_arg, n)                                                   \
    BOOST_PP_CAT(proto_arg, n);                                                                     \
    /**/

    namespace refns_
    {
        /// \brief A simple reference wrapper for a Proto expression type,
        /// used by <tt>expr\<\></tt> to hold children expressions by reference.
        ///
        /// <tt>ref_\<\></tt> is used by <tt>expr\<\></tt> to hold children
        /// expression types by reference. It forwards enough of the child
        /// expression's interface so that <tt>expr\<\></tt> can handle children
        /// uniformly regardless of whether it is stored by reference or by
        /// value.
        ///
        /// This type is largely an implementation detail.
        template<typename Expr>
        struct ref_
        {
            typedef typename Expr::proto_base_expr proto_base_expr;
            typedef typename Expr::proto_tag proto_tag;
            typedef typename Expr::proto_args proto_args;
            typedef typename Expr::proto_arity proto_arity;
            typedef typename Expr::proto_domain proto_domain;
            typedef void proto_is_ref_;
            typedef void proto_is_expr_;
            typedef Expr proto_derived_expr;

            BOOST_PP_REPEAT(BOOST_PROTO_MAX_ARITY, BOOST_PROTO_ARG, _)

            typename mpl::if_c<
                is_const<Expr>::value
              , proto_base_expr const &
              , proto_base_expr &
            >::type
            proto_base() const
            {
                return this->expr.proto_base();
            }

            static ref_<Expr> make(Expr &expr)
            {
                ref_<Expr> that = {expr};
                return that;
            }

            Expr &expr;
        };

        // ref_-to-ref_ is not allowed. this will cause a compile error.
        /// INTERNAL ONLY
        template<typename Expr>
        struct ref_<ref_<Expr> >
        {};
    }

#undef BOOST_PROTO_ARG

    namespace result_of
    {
        /// \brief Trait for stripping top-level references
        /// and reference wrappers.
        template<typename T>
        struct unref
        {
            typedef T type;                     ///< Suitable for return by value
            typedef T &reference;               ///< Suitable for return by reference
            typedef T const &const_reference;   ///< Suitable for return by const reference
        };

        /// \brief Trait for stripping top-level references
        /// and reference wrappers.
        template<typename T>
        struct unref<ref_<T> >
        {
            typedef T type;                     ///< Suitable for return by value
            typedef T &reference;               ///< Suitable for return by reference
            typedef T &const_reference;         ///< Suitable for return by const reference
        };

        /// \brief Trait for stripping top-level references
        /// and reference wrappers.
        template<typename T>
        struct unref<ref_<T const> >
        {
            typedef T type;                     ///< Suitable for return by value
            typedef T const &reference;         ///< Suitable for return by reference
            typedef T const &const_reference;   ///< Suitable for return by const reference
        };

        /// \brief Trait for stripping top-level references
        /// and reference wrappers.
        template<typename T>
        struct unref<T &>
        {
            typedef T type;                     ///< Suitable for return by value
            typedef T &reference;               ///< Suitable for return by reference
            typedef T &const_reference;         ///< Suitable for return by const reference
        };

        /// \brief Trait for stripping top-level references
        /// and reference wrappers.
        template<typename T>
        struct unref<T const &>
        {
            typedef T type;                     ///< Suitable for return by value
            typedef T const &reference;         ///< Suitable for return by reference
            typedef T const &const_reference;   ///< Suitable for return by const reference
        };

        /// \brief Trait for stripping top-level references
        /// and reference wrappers.
        template<typename T, std::size_t N>
        struct unref<T (&)[N]>
        {
            typedef T (&type)[N];               ///< Suitable for return by value
            typedef T (&reference)[N];          ///< Suitable for return by reference
            typedef T (&const_reference)[N];    ///< Suitable for return by const reference
        };

        /// \brief Trait for stripping top-level references
        /// and reference wrappers.
        template<typename T, std::size_t N>
        struct unref<T const (&)[N]>
        {
            typedef T const (&type)[N];             ///< Suitable for return by value
            typedef T const (&reference)[N];        ///< Suitable for return by reference
            typedef T const (&const_reference)[N];  ///< Suitable for return by const reference
        };
    }

    namespace functional
    {
        /// \brief A callable PolymorphicFunctionObject equivalent
        /// to the <tt>proto::unref()</tt> function that removes
        /// top-level reference wrappers.
        struct unref
        {
            BOOST_PROTO_CALLABLE()

            template<typename T>
            struct result;

            template<typename This, typename T>
            struct result<This(T)>
            {
                typedef BOOST_PROTO_UNCVREF(T) uncvref_type;
                typedef typename result_of::unref<uncvref_type>::type type;
            };

            /// \brief Remove a top-level <tt>ref_\<\></tt> reference wrapper,
            /// if it exists.
            /// \param t The object to unwrap
            /// \return If \c T t is a <tt>ref_\<\></tt>, return <tt>t.expr</tt>.
            /// Otherwise, return \c t.
            template<typename T>
            T &operator()(T &t) const
            {
                return t;
            }

            /// \overload
            ///
            template<typename T>
            T const &operator()(T const &t) const
            {
                return t;
            }

            /// \overload
            ///
            template<typename T>
            T &operator()(ref_<T> &t) const
            {
                return t.expr;
            }

            /// \overload
            ///
            template<typename T>
            T &operator()(ref_<T> const &t) const
            {
                return t.expr;
            }
        };
    }

    /// \brief Remove a top-level <tt>ref_\<\></tt> reference wrapper, if
    /// it exists.
    /// \param t The object to unwrap
    /// \throw nothrow
    /// \return If \c T t is a <tt>ref_\<\></tt>, return <tt>t.expr</tt>.
    /// Otherwise, return \c t.
    template<typename T>
    T &unref(T &t BOOST_PROTO_DISABLE_IF_IS_CONST(T))
    {
        return t;
    }

    /// \overload
    ///
    template<typename T>
    T const &unref(T const &t)
    {
        return t;
    }

    /// \overload
    ///
    template<typename T>
    T &unref(ref_<T> &t)
    {
        return t.expr;
    }

    /// \overload
    ///
    template<typename T>
    T &unref(ref_<T> const &t)
    {
        return t.expr;
    }
}}

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma warning(pop)
#endif

#endif
