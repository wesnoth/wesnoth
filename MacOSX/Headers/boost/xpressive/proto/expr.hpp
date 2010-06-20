#ifndef BOOST_PP_IS_ITERATING
    ///////////////////////////////////////////////////////////////////////////////
    /// \file expr.hpp
    /// Contains definition of expr\<\> class template.
    //
    //  Copyright 2008 Eric Niebler. Distributed under the Boost
    //  Software License, Version 1.0. (See accompanying file
    //  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

    #ifndef BOOST_PROTO_EXPR_HPP_EAN_04_01_2005
    #define BOOST_PROTO_EXPR_HPP_EAN_04_01_2005

    #include <boost/xpressive/proto/detail/prefix.hpp>
    #include <boost/preprocessor/cat.hpp>
    #include <boost/preprocessor/arithmetic/dec.hpp>
    #include <boost/preprocessor/selection/max.hpp>
    #include <boost/preprocessor/iteration/iterate.hpp>
    #include <boost/preprocessor/repetition/repeat.hpp>
    #include <boost/preprocessor/repetition/repeat_from_to.hpp>
    #include <boost/preprocessor/repetition/enum_trailing.hpp>
    #include <boost/preprocessor/repetition/enum_params.hpp>
    #include <boost/preprocessor/repetition/enum_binary_params.hpp>
    #include <boost/preprocessor/repetition/enum_trailing_params.hpp>
    #include <boost/preprocessor/repetition/enum_trailing_binary_params.hpp>
    #include <boost/utility/addressof.hpp>
    #include <boost/xpressive/proto/proto_fwd.hpp>
    #include <boost/xpressive/proto/ref.hpp>
    #include <boost/xpressive/proto/args.hpp>
    #include <boost/xpressive/proto/traits.hpp>
    #include <boost/xpressive/proto/detail/suffix.hpp>

    #if defined(_MSC_VER) && (_MSC_VER >= 1020)
    # pragma warning(push)
    # pragma warning(disable : 4510) // default constructor could not be generated
    # pragma warning(disable : 4512) // assignment operator could not be generated
    # pragma warning(disable : 4610) // user defined constructor required
    #endif

    namespace boost { namespace proto
    {

        namespace detail
        {
        /// INTERNAL ONLY
        ///
        #define BOOST_PROTO_ARG(z, n, data)                                                         \
            typedef typename Args::BOOST_PP_CAT(arg, n) BOOST_PP_CAT(proto_arg, n);                 \
            BOOST_PP_CAT(proto_arg, n) BOOST_PP_CAT(arg, n);                                        \
            /**/

        /// INTERNAL ONLY
        ///
        #define BOOST_PROTO_VOID(z, n, data)                                                        \
            typedef void BOOST_PP_CAT(proto_arg, n);                                                \
            /**/

        /// INTERNAL ONLY
        ///
        #define BOOST_PROTO_AS_OP(z, n, data)                                                       \
            proto::as_arg(BOOST_PP_CAT(a,n))                                                        \
            /**/

        /// INTERNAL ONLY
        ///
        #define BOOST_PROTO_UNREF_ARG_TYPE(z, n, data)                                              \
            typename result_of::unref<typename Args::BOOST_PP_CAT(arg, n)>::const_reference         \
            /**/

        /// INTERNAL ONLY
        ///
        #define BOOST_PROTO_UNREF_ARG(z, n, data)                                                   \
            proto::unref(this->BOOST_PP_CAT(arg, n))                                                \
            /**/

            template<typename Tag, typename Arg>
            struct address_of_hack
            {
                typedef address_of_hack type;
            };

            template<typename Expr>
            struct address_of_hack<proto::tag::address_of, ref_<Expr> >
            {
                typedef Expr *type;
            };

            template<typename X, std::size_t N, typename Y>
            void checked_copy(X (&x)[N], Y (&y)[N])
            {
                for(std::size_t i = 0; i < N; ++i)
                {
                    y[i] = x[i];
                }
            }

            template<typename T, std::size_t N>
            struct if_is_array
            {};

            template<typename T, std::size_t N>
            struct if_is_array<T[N], N>
            {
                typedef int type;
            };
        }

        namespace result_of
        {
            /// \brief A helper metafunction for computing the
            /// return type of \c proto::expr\<\>::operator().
            template<typename Sig, typename This>
            struct funop;

        #define BOOST_PP_ITERATION_PARAMS_1 (3, (0, BOOST_PP_DEC(BOOST_PROTO_MAX_FUNCTION_CALL_ARITY), <boost/xpressive/proto/detail/funop.hpp>))
        #include BOOST_PP_ITERATE()
        }

        namespace exprns_
        {
        #define BOOST_PP_ITERATION_PARAMS_1 (3, (0, BOOST_PROTO_MAX_ARITY, <boost/xpressive/proto/expr.hpp>))
        #include BOOST_PP_ITERATE()
        }

        #undef BOOST_PROTO_ARG
        #undef BOOST_PROTO_VOID
        #undef BOOST_PROTO_AS_OP
        #undef BOOST_PROTO_UNREF_ARG_TYPE
        #undef BOOST_PROTO_UNREF_ARG
    }}

    #if defined(_MSC_VER) && (_MSC_VER >= 1020)
    # pragma warning(pop)
    #endif

    #endif // BOOST_PROTO_EXPR_HPP_EAN_04_01_2005

// For gcc 4.4 compatability, we must include the
// BOOST_PP_ITERATION_DEPTH test inside an #else clause.
#else // BOOST_PP_IS_ITERATING
#if BOOST_PP_ITERATION_DEPTH() == 1

    #define ARG_COUNT BOOST_PP_MAX(1, BOOST_PP_ITERATION())
    #define IS_TERMINAL 0 == BOOST_PP_ITERATION()

        /// \brief Representation of a node in an expression tree.
        ///
        /// \c proto::expr\<\> is a node in an expression template tree. It
        /// is a container for its children sub-trees. It also serves as
        /// the terminal nodes of the tree.
        ///
        /// \c Tag is type that represents the operation encoded by
        ///             this expression. It is typically one of the structs
        ///             in the \c boost::proto::tag namespace, but it doesn't
        ///             have to be. If the \c Tag type is \c boost::proto::tag::terminal
        ///             then this \c expr\<\> type represents a leaf in the
        ///             expression tree.
        ///
        /// \c Args is a type list representing the type of the children
        ///             of this expression. It is an instantiation of one
        ///             of \c proto::args1\<\>, \c proto::args2\<\>, etc. The
        ///             children types must all themselves be either \c expr\<\>
        ///             or \c proto::ref_\<proto::expr\<\>\>, unless the \c Tag
        ///             type is \c boost::proto::tag::terminal, in which case
        ///             \c Args must be \c proto::args0\<T\>, where \c T can be any
        ///             type.
        ///
        /// \c proto::expr\<\> is a valid Fusion random-access sequence, where
        /// the elements of the sequence are the children expressions.
        template<typename Tag, typename Args>
        struct expr<Tag, Args, BOOST_PP_ITERATION() >
        {
            typedef Tag proto_tag;
            typedef mpl::long_<BOOST_PP_ITERATION() > proto_arity;
            typedef expr proto_base_expr;
            typedef Args proto_args;
            typedef default_domain proto_domain;
            BOOST_PROTO_DEFINE_FUSION_TAG(proto::tag::proto_expr)
            typedef void proto_is_expr_;
            typedef expr proto_derived_expr;

            BOOST_PP_REPEAT(ARG_COUNT, BOOST_PROTO_ARG, ~)
            BOOST_PP_REPEAT_FROM_TO(ARG_COUNT, BOOST_PROTO_MAX_ARITY, BOOST_PROTO_VOID, ~)

            /// \return *this
            ///
            expr const &proto_base() const
            {
                return *this;
            }

            /// \overload
            ///
            expr &proto_base()
            {
                return *this;
            }

            /// \return A new \c expr\<\> object initialized with the specified
            /// arguments.
            ///
            template<BOOST_PP_ENUM_PARAMS(ARG_COUNT, typename A)>
            static expr make(BOOST_PP_ENUM_BINARY_PARAMS(ARG_COUNT, A, const &a))
            {
                expr that = {BOOST_PP_ENUM_PARAMS(ARG_COUNT, a)};
                return that;
            }

        #if IS_TERMINAL
            /// \overload
            ///
            template<typename A0>
            static expr make(A0 &a0)
            {
                expr that = {a0};
                return that;
            }

            /// \overload
            ///
            template<typename A0, std::size_t N>
            static expr make(A0 (&a0)[N], typename detail::if_is_array<proto_arg0, N>::type = 0)
            {
                expr that;
                detail::checked_copy(a0, that.arg0);
                return that;
            }

            /// \overload
            ///
            template<typename A0, std::size_t N>
            static expr make(A0 const (&a0)[N], typename detail::if_is_array<proto_arg0, N>::type = 0)
            {
                expr that;
                detail::checked_copy(a0, that.arg0);
                return that;
            }
        #endif

        #if 1 == BOOST_PP_ITERATION()
            /// If \c Tag is \c boost::proto::tag::address_of and \c proto_arg0 is
            /// \c proto::ref_\<T\>, then \c address_of_hack_type_ is <tt>T*</tt>.
            /// Otherwise, it is some undefined type.
            typedef typename detail::address_of_hack<Tag, proto_arg0>::type address_of_hack_type_;

            /// \return The address of <tt>this->arg0</tt> if \c Tag is
            /// \c boost::proto::tag::address_of. Otherwise, this function will
            /// fail to compile.
            ///
            /// \attention Proto overloads <tt>operator&</tt>, which means that
            /// proto-ified objects cannot have their addresses taken, unless we use
            /// the following hack to make \c &x implicitly convertible to \c X*.
            operator address_of_hack_type_() const
            {
                return boost::addressof(this->arg0.expr);
            }
        #endif

            /// Assignment
            ///
            /// \param a The rhs.
            /// \return A new \c expr\<\> node representing an assignment of \c a to \c *this.
            template<typename A>
            proto::expr<proto::tag::assign, args2<ref_<expr const>, typename result_of::as_arg<A>::type> > const
            operator =(A &a) const
            {
                proto::expr<proto::tag::assign, args2<ref_<expr const>, typename result_of::as_arg<A>::type> > that = {{*this}, proto::as_arg(a)};
                return that;
            }

            /// \overload
            ///
            template<typename A>
            proto::expr<proto::tag::assign, args2<ref_<expr const>, typename result_of::as_arg<A const>::type> > const
            operator =(A const &a) const
            {
                proto::expr<proto::tag::assign, args2<ref_<expr const>, typename result_of::as_arg<A const>::type> > that = {{*this}, proto::as_arg(a)};
                return that;
            }

        #if IS_TERMINAL
            /// \overload
            ///
            template<typename A>
            proto::expr<proto::tag::assign, args2<ref_<expr>, typename result_of::as_arg<A>::type> > const
            operator =(A &a)
            {
                proto::expr<proto::tag::assign, args2<ref_<expr>, typename result_of::as_arg<A>::type> > that = {{*this}, proto::as_arg(a)};
                return that;
            }

            /// \overload
            ///
            template<typename A>
            proto::expr<proto::tag::assign, args2<ref_<expr>, typename result_of::as_arg<A const>::type> > const
            operator =(A const &a)
            {
                proto::expr<proto::tag::assign, args2<ref_<expr>, typename result_of::as_arg<A const>::type> > that = {{*this}, proto::as_arg(a)};
                return that;
            }
        #endif

            /// Subscript
            ///
            /// \param a The rhs.
            /// \return A new \c expr\<\> node representing \c *this subscripted with \c a.
            template<typename A>
            proto::expr<proto::tag::subscript, args2<ref_<expr const>, typename result_of::as_arg<A>::type> > const
            operator [](A &a) const
            {
                proto::expr<proto::tag::subscript, args2<ref_<expr const>, typename result_of::as_arg<A>::type> > that = {{*this}, proto::as_arg(a)};
                return that;
            }

            /// \overload
            ///
            template<typename A>
            proto::expr<proto::tag::subscript, args2<ref_<expr const>, typename result_of::as_arg<A const>::type> > const
            operator [](A const &a) const
            {
                proto::expr<proto::tag::subscript, args2<ref_<expr const>, typename result_of::as_arg<A const>::type> > that = {{*this}, proto::as_arg(a)};
                return that;
            }

        #if IS_TERMINAL
            /// \overload
            ///
            template<typename A>
            proto::expr<proto::tag::subscript, args2<ref_<expr>, typename result_of::as_arg<A>::type> > const
            operator [](A &a)
            {
                proto::expr<proto::tag::subscript, args2<ref_<expr>, typename result_of::as_arg<A>::type> > that = {{*this}, proto::as_arg(a)};
                return that;
            }

            /// \overload
            ///
            template<typename A>
            proto::expr<proto::tag::subscript, args2<ref_<expr>, typename result_of::as_arg<A const>::type> > const
            operator [](A const &a)
            {
                proto::expr<proto::tag::subscript, args2<ref_<expr>, typename result_of::as_arg<A const>::type> > that = {{*this}, proto::as_arg(a)};
                return that;
            }
        #endif

            /// Encodes the return type of \c expr\<\>::operator(), for use with \c boost::result_of\<\>
            ///
            template<typename Sig>
            struct result
            {
                typedef typename result_of::funop<Sig, expr>::type type;
            };

            /// Function call
            ///
            /// \return A new \c expr\<\> node representing the function invocation of \c (*this)().
            proto::expr<proto::tag::function, args1<ref_<expr const> > > const
            operator ()() const
            {
                proto::expr<proto::tag::function, args1<ref_<expr const> > > that = {{*this}};
                return that;
            }

        #if IS_TERMINAL
            /// \overload
            ///
            proto::expr<proto::tag::function, args1<ref_<expr> > > const
            operator ()()
            {
                proto::expr<proto::tag::function, args1<ref_<expr> > > that = {{*this}};
                return that;
            }
        #endif

    #define BOOST_PP_ITERATION_PARAMS_2 (3, (1, BOOST_PP_DEC(BOOST_PROTO_MAX_FUNCTION_CALL_ARITY), <boost/xpressive/proto/expr.hpp>))
    #include BOOST_PP_ITERATE()
        };

    #undef ARG_COUNT
    #undef IS_TERMINAL

#elif BOOST_PP_ITERATION_DEPTH() == 2

    #define N BOOST_PP_ITERATION()

        /// \overload
        ///
        template<BOOST_PP_ENUM_PARAMS(N, typename A)>
        typename result_of::BOOST_PP_CAT(funop, N)<expr const BOOST_PP_ENUM_TRAILING_PARAMS(N, const A)>::type const
        operator ()(BOOST_PP_ENUM_BINARY_PARAMS(N, A, const &a)) const
        {
            return result_of::BOOST_PP_CAT(funop, N)<expr const BOOST_PP_ENUM_TRAILING_PARAMS(N, const A)>
                ::call(*this BOOST_PP_ENUM_TRAILING_PARAMS(N, a));
        }

        #if IS_TERMINAL
        /// \overload
        ///
        template<BOOST_PP_ENUM_PARAMS(N, typename A)>
        typename result_of::BOOST_PP_CAT(funop, N)<expr BOOST_PP_ENUM_TRAILING_PARAMS(N, const A)>::type const
        operator ()(BOOST_PP_ENUM_BINARY_PARAMS(N, A, const &a))
        {
            return result_of::BOOST_PP_CAT(funop, N)<expr BOOST_PP_ENUM_TRAILING_PARAMS(N, const A)>
                ::call(*this BOOST_PP_ENUM_TRAILING_PARAMS(N, a));
        }
        #endif

    #undef N

#endif // BOOST_PP_ITERATION_DEPTH()
#endif
