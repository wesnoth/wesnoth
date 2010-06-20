#ifndef BOOST_PP_IS_ITERATING
    ///////////////////////////////////////////////////////////////////////////////
    /// \file bind.hpp
    /// Contains definition of the bind<> transform.
    //
    //  Copyright 2008 Eric Niebler. Distributed under the Boost
    //  Software License, Version 1.0. (See accompanying file
    //  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

    #ifndef BOOST_PROTO_TRANSFORM_BIND_HPP_EAN_12_02_2007
    #define BOOST_PROTO_TRANSFORM_BIND_HPP_EAN_12_02_2007

    #include <boost/xpressive/proto/detail/prefix.hpp>
    #include <boost/preprocessor/iteration/iterate.hpp>
    #include <boost/preprocessor/repetition/enum_params.hpp>
    #include <boost/preprocessor/repetition/enum_trailing_params.hpp>
    #include <boost/xpressive/proto/proto_fwd.hpp>
    #include <boost/xpressive/proto/transform/make.hpp>
    #include <boost/xpressive/proto/transform/call.hpp>
    #include <boost/xpressive/proto/detail/suffix.hpp>

    namespace boost { namespace proto
    {

        namespace transform
        {
            /// \brief A PrimitiveTransform that uses <tt>make\<\></tt> to build
            /// a CallableTransform, and then uses <tt>call\<\></tt> to apply it.
            ///
            /// <tt>bind\<\></tt> is useful as a higher-order transform, when the
            /// transform to be applied depends on the current state of the
            /// transformation. The invocation of the <tt>make\<\></tt> transform
            /// evaluates any nested transforms, and the resulting type is treated
            /// as a CallableTransform, which is evaluated with <tt>call\<\></tt>.
            template<typename Object>
            struct bind : proto::callable
            {
                template<typename Sig>
                struct result;

                template<typename This, typename Expr, typename State, typename Visitor>
                struct result<This(Expr, State, Visitor)>
                {
                    typedef typename make<Object>::template result<void(Expr, State, Visitor)>::type fun;
                    typedef call<fun> impl;
                    typedef typename impl::template result<void(Expr, State, Visitor)>::type type;
                };

                /// Build a CallableTransform by applying <tt>make\<\></tt>
                /// and evaluate it with <tt>call\<\></tt>
                ///
                /// \param expr The current expression
                /// \param state The current state
                /// \param visitor An arbitrary visitor
                /// \return <tt>result\<void(Expr, State, Visitor)\>::::impl()(expr, state, visitor)</tt>
                template<typename Expr, typename State, typename Visitor>
                typename result<void(Expr, State, Visitor)>::type
                operator ()(Expr const &expr, State const &state, Visitor &visitor) const
                {
                    return typename result<void(Expr, State, Visitor)>::impl()(expr, state, visitor);
                }
            };

        #define BOOST_PP_ITERATION_PARAMS_1 (3, (0, BOOST_PROTO_MAX_ARITY, <boost/xpressive/proto/transform/bind.hpp>))
        #include BOOST_PP_ITERATE()

        }

        /// INTERNAL ONLY
        ///
        template<typename Object>
        struct is_callable<transform::bind<Object> >
          : mpl::true_
        {};

    }}

    #endif

#else

    #define N BOOST_PP_ITERATION()

            /// \brief A PrimitiveTransform that uses <tt>make\<\></tt> to build
            /// a CallableTransform, and then uses <tt>call\<\></tt> to apply it.
            ///
            /// <tt>bind\<\></tt> is useful as a higher-order transform, when the
            /// transform to be applied depends on the current state of the
            /// transformation. The invocation of the <tt>make\<\></tt> transform
            /// evaluates any nested transforms, and the resulting type is treated
            /// as a CallableTransform, which is evaluated with <tt>call\<\></tt>.
            template<typename Object BOOST_PP_ENUM_TRAILING_PARAMS(N, typename A)>
            struct bind<Object(BOOST_PP_ENUM_PARAMS(N, A))> : proto::callable
            {
                template<typename Sig>
                struct result;

                template<typename This, typename Expr, typename State, typename Visitor>
                struct result<This(Expr, State, Visitor)>
                {
                    typedef typename make<Object>::template result<void(Expr, State, Visitor)>::type fun;
                    typedef call<fun(BOOST_PP_ENUM_PARAMS(N, A))> impl;
                    typedef typename impl::template result<void(Expr, State, Visitor)>::type type;
                };

                /// Build a CallableTransform by applying <tt>make\<\></tt>
                /// and evaluate it with <tt>call\<\></tt>
                ///
                /// \param expr The current expression
                /// \param state The current state
                /// \param visitor An arbitrary visitor
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
