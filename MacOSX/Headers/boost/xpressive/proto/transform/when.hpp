#ifndef BOOST_PP_IS_ITERATING
    ///////////////////////////////////////////////////////////////////////////////
    /// \file when.hpp
    /// Definition of when transform.
    //
    //  Copyright 2008 Eric Niebler. Distributed under the Boost
    //  Software License, Version 1.0. (See accompanying file
    //  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

    #ifndef BOOST_PROTO_TRANSFORM_WHEN_HPP_EAN_10_29_2007
    #define BOOST_PROTO_TRANSFORM_WHEN_HPP_EAN_10_29_2007

    #include <boost/xpressive/proto/detail/prefix.hpp>
    #include <boost/preprocessor/cat.hpp>
    #include <boost/preprocessor/repetition/enum_params.hpp>
    #include <boost/preprocessor/repetition/enum_trailing_params.hpp>
    #include <boost/preprocessor/iteration/iterate.hpp>
    #include <boost/mpl/if.hpp>
    #include <boost/xpressive/proto/proto_fwd.hpp>
    #include <boost/xpressive/proto/traits.hpp>
    #include <boost/xpressive/proto/transform/call.hpp>
    #include <boost/xpressive/proto/transform/make.hpp>
    #include <boost/xpressive/proto/detail/suffix.hpp>

    namespace boost { namespace proto { namespace transform
    {
        /// \brief A grammar element and a PrimitiveTransform that associates
        /// a transform with the grammar.
        ///
        /// Use <tt>when\<\></tt> to override a grammar's default transform
        /// with a custom transform. It is for used when composing larger
        /// transforms by associating smaller transforms with individual
        /// rules in your grammar, as in the following transform which
        /// counts the number of terminals in an expression.
        ///
        /// \code
        /// // Count the terminals in an expression tree.
        /// // Must be invoked with initial state == mpl::int_<0>().
        /// struct CountLeaves
        ///   : or_<
        ///         when<terminal<_>, mpl::next<_state>()>
        ///       , otherwise<fold<_, _state, CountLeaves> >
        ///     >
        /// {};
        /// \endcode
        ///
        /// In <tt>when\<G, T\></tt>, when \c T is a class type it is a
        /// PrimitiveTransform and the following equivalencies hold:
        ///
        /// <tt>when\<G,T\>::::result\<void(E,S,V)\>::::type</tt> is the same as
        /// <tt>T::result\<void(E,S,V)\>::::type</tt>.
        ///
        /// <tt>when\<G,T\>()(e,s,v)</tt> is the same as
        /// <tt>T()(e,s,v)</tt>.
        template<typename Grammar, typename PrimitiveTransform BOOST_PROTO_FOR_DOXYGEN_ONLY(= Grammar)>
        struct when
          : PrimitiveTransform
        {
            typedef typename Grammar::proto_base_expr proto_base_expr;
        };

        /// \brief A specialization that treats function pointer Transforms as
        /// if they were function type Transforms.
        ///
        /// This specialization requires that \c Fun is actually a function type.
        ///
        /// This specialization is required for nested transforms such as
        /// <tt>when\<G, T0(T1(_))\></tt>. In C++, functions that are used as
        /// parameters to other functions automatically decay to funtion
        /// pointer types. In other words, the type <tt>T0(T1(_))</tt> is
        /// indistinguishable from <tt>T0(T1(*)(_))</tt>. This specialization
        /// is required to handle these nested function pointer type transforms
        /// properly.
        template<typename Grammar, typename Fun>
        struct when<Grammar, Fun *>
          : when<Grammar, Fun>
        {};

        /// \brief Syntactic sugar for <tt>when\<_, Fun\></tt>, for use
        /// in grammars to handle all the cases not yet handled.
        ///
        /// Use <tt>otherwise\<T\></tt> in your grammars as a synonym for
        /// <tt>when\<_, T\></tt> as in the following transform which
        /// counts the number of terminals in an expression.
        ///
        /// \code
        /// // Count the terminals in an expression tree.
        /// // Must be invoked with initial state == mpl::int_<0>().
        /// struct CountLeaves
        ///   : or_<
        ///         when<terminal<_>, mpl::next<_state>()>
        ///       , otherwise<fold<_, _state, CountLeaves> >
        ///     >
        /// {};
        /// \endcode
        template<typename Fun>
        struct otherwise
          : when<_, Fun>
        {};

        #define BOOST_PP_ITERATION_PARAMS_1 (3, (0, BOOST_PROTO_MAX_ARITY, <boost/xpressive/proto/transform/when.hpp>))
        #include BOOST_PP_ITERATE()

    }}} // namespace boost::proto::transform

    #endif

#else

    #define N BOOST_PP_ITERATION()

        /// \brief A grammar element and a PrimitiveTransform that associates
        /// a transform with the grammar.
        ///
        /// Use <tt>when\<\></tt> to override a grammar's default transform
        /// with a custom transform. It is for used when composing larger
        /// transforms by associating smaller transforms with individual
        /// rules in your grammar, as in the following transform which
        /// counts the number of terminals in an expression.
        ///
        /// \code
        /// // Count the terminals in an expression tree.
        /// // Must be invoked with initial state == mpl::int_<0>().
        /// struct CountLeaves
        ///   : or_<
        ///         when<terminal<_>, mpl::next<_state>()>
        ///       , otherwise<fold<_, _state, CountLeaves> >
        ///     >
        /// {};
        /// \endcode
        ///
        /// The <tt>when\<G, R(A0,A1,...)\></tt> form accepts either a
        /// CallableTransform or an ObjectTransform as its second parameter.
        /// <tt>when\<\></tt> uses <tt>is_callable\<R\>::::value</tt> to
        /// distinguish between the two, and uses <tt>call\<\></tt> to
        /// evaluate CallableTransforms and <tt>make\<\></tt> to evaluate
        /// ObjectTransforms.
        template<typename Grammar, typename R BOOST_PP_ENUM_TRAILING_PARAMS(N, typename A)>
        struct when<Grammar, R(BOOST_PP_ENUM_PARAMS(N, A))>
          : proto::callable
        {
            typedef typename Grammar::proto_base_expr proto_base_expr;

            // Note: do not evaluate is_callable<R> in this scope.
            // R may be an incomplete type at this point.

            template<typename Sig>
            struct result;

            template<typename This, typename Expr, typename State, typename Visitor>
            struct result<This(Expr, State, Visitor)>
            {
                typedef call<R(BOOST_PP_ENUM_PARAMS(N, A))> call_;
                typedef make<R(BOOST_PP_ENUM_PARAMS(N, A))> make_;

                typedef
                    typename mpl::if_c<
                        // OK to evaluate is_callable<R> here.
                        // R should be compete by now.
                        is_callable<R>::value
                      , call_                       // "R" is a function to call
                      , make_                       // "R" is an object to construct
                    >::type
                impl;

                typedef typename impl::template result<void(Expr, State, Visitor)>::type type;
            };

            /// Evaluate <tt>R(A0,A1,...)</tt> as a transform either with
            /// <tt>call\<\></tt> or with <tt>make\<\></tt> depending on
            /// whether <tt>is_callable\<R\>::::value</tt> is \c true or
            /// \c false.
            ///
            /// \param expr The current expression
            /// \param state The current state
            /// \param visitor An arbitrary visitor
            /// \pre <tt>matches\<Expr, Grammar\>::::value</tt> is \c true
            /// \return <tt>result\<void(Expr, State, Visitor)\>::::impl()(expr, state, visitor)</tt>
            template<typename Expr, typename State, typename Visitor>
            typename result<void(Expr, State, Visitor)>::type
            operator ()(Expr const &expr, State const &state, Visitor &visitor) const
            {
                return typename result<void(Expr, State, Visitor)>::impl()(expr, state, visitor);
            }
        };

    #undef N

#endif
