#ifndef BOOST_PP_IS_ITERATING
    ///////////////////////////////////////////////////////////////////////////////
    /// \file traits.hpp
    /// Contains definitions for arg\<\>, arg_c\<\>, left\<\>,
    /// right\<\>, tag_of\<\>, and the helper functions arg(), arg_c(),
    /// left() and right().
    //
    //  Copyright 2008 Eric Niebler. Distributed under the Boost
    //  Software License, Version 1.0. (See accompanying file
    //  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

    #ifndef BOOST_PROTO_ARG_TRAITS_HPP_EAN_04_01_2005
    #define BOOST_PROTO_ARG_TRAITS_HPP_EAN_04_01_2005

    #include <boost/xpressive/proto/detail/prefix.hpp>
    #include <boost/config.hpp>
    #include <boost/detail/workaround.hpp>
    #include <boost/preprocessor/iteration/iterate.hpp>
    #include <boost/preprocessor/repetition/enum.hpp>
    #include <boost/preprocessor/repetition/enum_params.hpp>
    #include <boost/preprocessor/repetition/enum_trailing.hpp>
    #include <boost/preprocessor/repetition/enum_trailing_params.hpp>
    #include <boost/preprocessor/repetition/repeat.hpp>
    #include <boost/preprocessor/repetition/repeat_from_to.hpp>
    #include <boost/preprocessor/facilities/intercept.hpp>
    #include <boost/preprocessor/arithmetic/sub.hpp>
    #include <boost/ref.hpp>
    #include <boost/mpl/if.hpp>
    #include <boost/mpl/or.hpp>
    #include <boost/mpl/bool.hpp>
    #include <boost/mpl/eval_if.hpp>
    #include <boost/mpl/aux_/template_arity.hpp>
    #include <boost/mpl/aux_/lambda_arity_param.hpp>
    #include <boost/static_assert.hpp>
    #include <boost/utility/result_of.hpp>
    #include <boost/type_traits/is_pod.hpp>
    #include <boost/type_traits/is_same.hpp>
    #include <boost/type_traits/is_array.hpp>
    #include <boost/type_traits/is_function.hpp>
    #include <boost/type_traits/remove_cv.hpp>
    #include <boost/type_traits/remove_const.hpp>
    #include <boost/type_traits/add_reference.hpp>
    #include <boost/xpressive/proto/proto_fwd.hpp>
    #include <boost/xpressive/proto/ref.hpp>
    #include <boost/xpressive/proto/args.hpp>
    #include <boost/xpressive/proto/tags.hpp>
    #include <boost/xpressive/proto/transform/pass_through.hpp>
    #include <boost/xpressive/proto/detail/suffix.hpp>

    #if BOOST_WORKAROUND( BOOST_MSVC, == 1310 )
        #define BOOST_PROTO_IS_ARRAY_(T) boost::is_array<typename boost::remove_const<T>::type>
    #else
        #define BOOST_PROTO_IS_ARRAY_(T) boost::is_array<T>
    #endif

    #if BOOST_WORKAROUND( BOOST_MSVC, >= 1400 )
        #pragma warning(push)
        #pragma warning(disable: 4180) // warning C4180: qualifier applied to function type has no meaning; ignored
    #endif

    namespace boost { namespace proto
    {
        namespace detail
        {
            template<typename T, typename Void = void>
            struct if_vararg
            {};

            template<typename T>
            struct if_vararg<T, typename T::proto_is_vararg_>
              : T
            {};

            template<typename T, typename Void = void>
            struct is_callable2_
              : mpl::false_
            {};

            template<typename T>
            struct is_callable2_<T, typename T::proto_is_callable_>
              : mpl::true_
            {};

            template<typename T BOOST_MPL_AUX_LAMBDA_ARITY_PARAM(long Arity = mpl::aux::template_arity<T>::value)>
            struct is_callable_
              : is_callable2_<T>
            {};
        }

        /// \brief Boolean metafunction which detects whether a type is
        /// a callable function object type or not.
        ///
        /// <tt>is_callable\<\></tt> is used by the <tt>when\<\></tt> transform
        /// to determine whether a function type <tt>R(A1,A2,...AN)</tt> is a
        /// callable transform or an object transform. (The former are evaluated
        /// using <tt>call\<\></tt> and the later with <tt>make\<\></tt>.) If
        /// <tt>is_callable\<R\>::::value</tt> is \c true, the function type is
        /// a callable transform; otherwise, it is an object transform.
        ///
        /// Unless specialized for a type \c T, <tt>is_callable\<T\>::::value</tt>
        /// is computed as follows:
        ///
        /// \li If \c T is a template type <tt>X\<Y0,Y1,...YN\></tt>, where all \c Yx
        /// are types for \c x in <tt>[0,N]</tt>, <tt>is_callable\<T\>::::value</tt>
        /// is <tt>is_same\<YN, proto::callable\>::::value</tt>.
        /// \li If \c T has a nested type \c proto_is_callable_ that is a typedef
        /// for \c void, <tt>is_callable\<T\>::::value</tt> is \c true. (Note: this is
        /// the case for any type that derives from \c proto::callable.)
        /// \li Otherwise, <tt>is_callable\<T\>::::value</tt> is \c false.
        template<typename T>
        struct is_callable
          : proto::detail::is_callable_<T>
        {};

        /// INTERNAL ONLY
        ///
        template<>
        struct is_callable<proto::_>
          : mpl::true_
        {};

        /// INTERNAL ONLY
        ///
        template<>
        struct is_callable<proto::callable>
          : mpl::false_
        {};

        #if BOOST_WORKAROUND(__GNUC__, == 3)
        // work around GCC bug
        template<typename Tag, typename Args, long N>
        struct is_callable<proto::expr<Tag, Args, N> >
          : mpl::false_
        {};
        #endif

        /// \brief A Boolean metafunction that indicates whether a type requires
        /// aggregate initialization.
        ///
        /// <tt>is_aggregate\<\></tt> is used by the <tt>make\<\></tt> transform
        /// to determine how to construct an object of some type \c T, given some
        /// initialization arguments <tt>a0,a1,...aN</tt>.
        /// If <tt>is_aggregate\<T\>::::value</tt> is \c true, then an object of
        /// type T will be initialized as <tt>T t = {a0,a1,...aN};</tt>. Otherwise,
        /// it will be initialized as <tt>T t(a0,a1,...aN)</tt>.
        template<typename T>
        struct is_aggregate
          : is_pod<T>
        {};

        /// \brief Specialization of <tt>is_aggregate\<\></tt> that indicates
        /// that objects of <tt>expr\<\></tt> type require aggregate initialization.
        template<typename Tag, typename Args, long N>
        struct is_aggregate<proto::expr<Tag, Args, N> >
          : mpl::true_
        {};

        namespace result_of
        {
            /// \brief A Boolean metafunction that indicates whether a given
            /// type \c T is a Proto expression type.
            ///
            /// If \c T has a nested type \c proto_is_expr_ that is a typedef
            /// for \c void, <tt>is_expr\<T\>::::value</tt> is \c true. (Note, this
            /// is the case for <tt>proto::expr\<\></tt>, any type that is derived
            /// from <tt>proto::extends\<\></tt> or that uses the
            /// <tt>BOOST_PROTO_EXTENDS()</tt> macro.) Otherwise,
            /// <tt>is_expr\<T\>::::value</tt> is \c false.
            template<typename T, typename Void  BOOST_PROTO_FOR_DOXYGEN_ONLY(= void)>
            struct is_expr
              : mpl::false_
            {};

            /// \brief A Boolean metafunction that indicates whether a given
            /// type \c T is a Proto expression type.
            ///
            /// If \c T has a nested type \c proto_is_expr_ that is a typedef
            /// for \c void, <tt>is_expr\<T\>::::value</tt> is \c true. (Note, this
            /// is the case for <tt>proto::expr\<\></tt>, any type that is derived
            /// from <tt>proto::extends\<\></tt> or that uses the
            /// <tt>BOOST_PROTO_EXTENDS()</tt> macro.) Otherwise,
            /// <tt>is_expr\<T\>::::value</tt> is \c false.
            template<typename T>
            struct is_expr<T, typename T::proto_is_expr_>
              : mpl::true_
            {};

            /// \brief A metafunction that returns the tag type of a
            /// Proto expression.
            template<typename Expr>
            struct tag_of
            {
                typedef typename Expr::proto_tag type;
            };

            /// INTERNAL ONLY
            ///
            template<typename T, typename Void  BOOST_PROTO_FOR_DOXYGEN_ONLY(= void)>
            struct is_ref
              : mpl::false_
            {};

            /// INTERNAL ONLY
            ///
            template<typename T>
            struct is_ref<T, typename T::proto_is_ref_>
              : mpl::true_
            {};

            /// \brief A metafunction that computes the return type of the \c as_expr()
            /// function.
            ///
            /// The <tt>as_expr\<\></tt> metafunction turns types into Proto types, if
            /// they are not already, by making them Proto terminals held by value if
            /// possible. Types which are already Proto types are left alone.
            ///
            /// This specialization is selected when the type is not yet a Proto type.
            /// The resulting terminal type is calculated as follows:
            ///
            /// If \c T is an array type or a function type, let \c A be <tt>T &</tt>.
            /// Otherwise, let \c A be the type \c T stripped of cv-qualifiers.
            /// Then, the result type <tt>as_expr\<T, Domain\>::::type</tt> is
            /// <tt>Domain::apply\< expr\< tag::terminal, args0\<A\> \> \>::::type</tt>.
            template<
                typename T
              , typename Domain BOOST_PROTO_FOR_DOXYGEN_ONLY(= default_domain)
              , typename Void   BOOST_PROTO_FOR_DOXYGEN_ONLY(= void)
            >
            struct as_expr
            {
                typedef mpl::or_<BOOST_PROTO_IS_ARRAY_(T), is_function<T> > is_unstorable_;
                typedef typename mpl::eval_if<is_unstorable_, add_reference<T>, remove_cv<T> >::type arg0_;
                typedef proto::expr<proto::tag::terminal, args0<arg0_> > expr_;
                typedef typename Domain::template apply<expr_>::type type;
                typedef type const reference;

                /// INTERNAL ONLY
                ///
                template<typename T2>
                static reference call(T2 &t)
                {
                    return Domain::make(expr_::make(t));
                }
            };

            /// \brief A metafunction that computes the return type of the \c as_expr()
            /// function.
            ///
            /// The <tt>as_expr\<\></tt> metafunction turns types into Proto types, if
            /// they are not already, by making them Proto terminals held by value if
            /// possible. Types which are already Proto types are left alone.
            ///
            /// This specialization is selected when the type is already a Proto type.
            /// The result type <tt>as_expr\<T, Domain\>::::type</tt> is \c T stripped
            /// of cv-qualifiers.
            template<typename T, typename Domain>
            struct as_expr<T, Domain, typename T::proto_is_expr_>
            {
                typedef typename T::proto_derived_expr type;
                typedef T &reference;

                /// INTERNAL ONLY
                ///
                template<typename T2>
                static reference call(T2 &t)
                {
                    return t;
                }
            };

            /// \brief A metafunction that computes the return type of the \c as_arg()
            /// function.
            ///
            /// The <tt>as_arg\<\></tt> metafunction turns types into Proto types, if
            /// they are not already, by making them Proto terminals held by reference.
            /// Types which are already Proto types are wrapped in <tt>proto::ref_\<\></tt>.
            ///
            /// This specialization is selected when the type is not yet a Proto type.
            /// The result type <tt>as_arg\<T, Domain\>::::type</tt> is
            /// <tt>Domain::apply\< expr\< tag::terminal, args0\<T &\> \> \>::::type</tt>.
            template<
                typename T
              , typename Domain BOOST_PROTO_FOR_DOXYGEN_ONLY(= default_domain)
              , typename Void   BOOST_PROTO_FOR_DOXYGEN_ONLY(= void)
            >
            struct as_arg
            {
                typedef proto::expr<proto::tag::terminal, args0<T &> > expr_;
                typedef typename Domain::template apply<expr_>::type type;

                /// INTERNAL ONLY
                ///
                template<typename T2>
                static type call(T2 &t)
                {
                    return Domain::make(expr_::make(t));
                }
            };

            /// \brief A metafunction that computes the return type of the \c as_arg()
            /// function.
            ///
            /// The <tt>as_arg\<\></tt> metafunction turns types into Proto types, if
            /// they are not already, by making them Proto terminals held by reference.
            /// Types which are already Proto types are wrapped in <tt>proto::ref_\<\></tt>.
            ///
            /// This specialization is selected when the type is already a Proto type.
            /// The result type <tt>as_arg\<T, Domain\>::::type</tt> is
            /// <tt>proto::ref_\<T\></tt>.
            template<typename T, typename Domain>
            struct as_arg<T, Domain, typename T::proto_is_expr_>
            {
                typedef ref_<T> type;

                /// INTERNAL ONLY
                ///
                template<typename T2>
                static type call(T2 &t)
                {
                    return type::make(t);
                }
            };

            /// \brief A metafunction that returns the type of the Nth child
            /// of a Proto expression, where N is an MPL Integral Constant.
            ///
            /// <tt>result_of::arg\<Expr, N\></tt> is equivalent to
            /// <tt>result_of::arg_c\<Expr, N::value\></tt>.
            template<typename Expr, typename N  BOOST_PROTO_FOR_DOXYGEN_ONLY(= mpl::long_<0>) >
            struct arg
              : arg_c<Expr, N::value>
            {};

            // TODO left<> and right<> force the instantiation of Expr.
            // Couldn't we partially specialize them on proto::expr< T, A >
            // and ref_< proto::expr< T, A > > and return A::arg0 / A::arg1?

            /// \brief A metafunction that returns the type of the left child
            /// of a binary Proto expression.
            ///
            /// <tt>result_of::left\<Expr\></tt> is equivalent to
            /// <tt>result_of::arg_c\<Expr, 0\></tt>.
            template<typename Expr>
            struct left
            {
                typedef typename Expr::proto_arg0 wrapped_type;
                typedef typename unref<wrapped_type>::type type;
                typedef typename unref<wrapped_type>::reference reference;
                typedef typename unref<wrapped_type>::const_reference const_reference;
            };

            /// \brief A metafunction that returns the type of the right child
            /// of a binary Proto expression.
            ///
            /// <tt>result_of::right\<Expr\></tt> is equivalent to
            /// <tt>result_of::arg_c\<Expr, 1\></tt>.
            template<typename Expr>
            struct right
            {
                typedef typename Expr::proto_arg1 wrapped_type;
                typedef typename unref<wrapped_type>::type type;
                typedef typename unref<wrapped_type>::reference reference;
                typedef typename unref<wrapped_type>::const_reference const_reference;
            };

        } // namespace result_of

        namespace op
        {
            /// \brief A metafunction for generating terminal expression types,
            /// a grammar element for matching terminal expressions, and a
            /// PrimitiveTransform that returns the current expression unchanged.
            template<typename T>
            struct terminal
            {
                typedef proto::expr<proto::tag::terminal, args0<T> > type;
                typedef type proto_base_expr;

                template<typename Sig>
                struct result;

                template<typename This, typename Expr, typename State, typename Visitor>
                struct result<This(Expr, State, Visitor)>
                {
                    typedef Expr type;
                };

                /// \param expr The current expression
                /// \pre <tt>matches\<Expr, terminal\<T\> \>::::value</tt> is \c true.
                /// \return \c expr
                /// \throw nothrow
                template<typename Expr, typename State, typename Visitor>
                Expr const &operator ()(Expr const &expr, State const &, Visitor &) const
                {
                    return expr;
                }

                /// INTERNAL ONLY
                typedef proto::tag::terminal proto_tag;
                /// INTERNAL ONLY
                typedef T proto_arg0;
            };

            /// \brief A metafunction for generating ternary conditional expression types,
            /// a grammar element for matching ternary conditional expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U, typename V>
            struct if_else_
            {
                typedef proto::expr<proto::tag::if_else_, args3<T, U, V> > type;
                typedef type proto_base_expr;

                template<typename Sig>
                struct result
                {
                    typedef
                        typename pass_through<if_else_>::template result<Sig>::type
                    type;
                };

                /// \param expr The current expression
                /// \param state The current state
                /// \param visitor An arbitrary visitor
                /// \pre <tt>matches\<Expr, if_else_\<T,U,V\> \>::::value</tt> is \c true.
                /// \return <tt>pass_through\<if_else_\<T,U,V\> \>()(expr, state, visitor)</tt>
                template<typename Expr, typename State, typename Visitor>
                typename result<void(Expr, State, Visitor)>::type
                operator ()(Expr const &expr, State const &state, Visitor &visitor) const
                {
                    return pass_through<if_else_>()(expr, state, visitor);
                }

                /// INTERNAL ONLY
                typedef proto::tag::if_else_ proto_tag;
                /// INTERNAL ONLY
                typedef T proto_arg0;
                /// INTERNAL ONLY
                typedef U proto_arg1;
                /// INTERNAL ONLY
                typedef V proto_arg2;
            };

            /// \brief A metafunction for generating unary expression types with a
            /// specified tag type,
            /// a grammar element for matching unary expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            ///
            /// Use <tt>unary_expr\<_, _\></tt> as a grammar element to match any
            /// unary expression.
            template<typename Tag, typename T>
            struct unary_expr
            {
                typedef proto::expr<Tag, args1<T> > type;
                typedef type proto_base_expr;

                template<typename Sig>
                struct result
                {
                    typedef
                        typename pass_through<unary_expr>::template result<Sig>::type
                    type;
                };

                /// \param expr The current expression
                /// \param state The current state
                /// \param visitor An arbitrary visitor
                /// \pre <tt>matches\<Expr, unary_expr\<Tag, T\> \>::::value</tt> is \c true.
                /// \return <tt>pass_through\<unary_expr\<Tag, T\> \>()(expr, state, visitor)</tt>
                template<typename Expr, typename State, typename Visitor>
                typename result<void(Expr, State, Visitor)>::type
                operator ()(Expr const &expr, State const &state, Visitor &visitor) const
                {
                    return pass_through<unary_expr>()(expr, state, visitor);
                }

                /// INTERNAL ONLY
                typedef Tag proto_tag;
                /// INTERNAL ONLY
                typedef T proto_arg0;
            };

            /// \brief A metafunction for generating binary expression types with a
            /// specified tag type,
            /// a grammar element for matching binary expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            ///
            /// Use <tt>binary_expr\<_, _, _\></tt> as a grammar element to match any
            /// binary expression.
            template<typename Tag, typename T, typename U>
            struct binary_expr
            {
                typedef proto::expr<Tag, args2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Sig>
                struct result
                {
                    typedef
                        typename pass_through<binary_expr>::template result<Sig>::type
                    type;
                };

                /// \param expr The current expression
                /// \param state The current state
                /// \param visitor An arbitrary visitor
                /// \pre <tt>matches\<Expr, binary_expr\<Tag,T,U\> \>::::value</tt> is \c true.
                /// \return <tt>pass_through\<binary_expr\<Tag,T,U\> \>()(expr, state, visitor)</tt>
                template<typename Expr, typename State, typename Visitor>
                typename result<void(Expr, State, Visitor)>::type
                operator ()(Expr const &expr, State const &state, Visitor &visitor) const
                {
                    return pass_through<binary_expr>()(expr, state, visitor);
                }

                /// INTERNAL ONLY
                typedef Tag proto_tag;
                /// INTERNAL ONLY
                typedef T proto_arg0;
                /// INTERNAL ONLY
                typedef U proto_arg1;
            };

            /// \brief A metafunction for generating unary plus expression types,
            /// a grammar element for matching unary plus expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T>
            struct posit
            {
                typedef proto::expr<proto::tag::posit, args1<T> > type;
                typedef type proto_base_expr;

                template<typename Sig>
                struct result
                {
                    typedef
                        typename pass_through<posit>::template result<Sig>::type
                    type;
                };

                /// \param expr The current expression
                /// \param state The current state
                /// \param visitor An arbitrary visitor
                /// \pre <tt>matches\<Expr, posit\<T\> \>::::value</tt> is \c true.
                /// \return <tt>pass_through\<posit\<T\> \>()(expr, state, visitor)</tt>
                template<typename Expr, typename State, typename Visitor>
                typename result<void(Expr, State, Visitor)>::type
                operator ()(Expr const &expr, State const &state, Visitor &visitor) const
                {
                    return pass_through<posit>()(expr, state, visitor);
                }

                /// INTERNAL ONLY
                typedef proto::tag::posit proto_tag;
                /// INTERNAL ONLY
                typedef T proto_arg0;
            };

            /// \brief A metafunction for generating unary minus expression types,
            /// a grammar element for matching unary minus expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T>
            struct negate
            {
                typedef proto::expr<proto::tag::negate, args1<T> > type;
                typedef type proto_base_expr;

                template<typename Sig>
                struct result
                {
                    typedef
                        typename pass_through<negate>::template result<Sig>::type
                    type;
                };

                /// \param expr The current expression
                /// \param state The current state
                /// \param visitor An arbitrary visitor
                /// \pre <tt>matches\<Expr, negate\<T\> \>::::value</tt> is \c true.
                /// \return <tt>pass_through\<negate\<T\> \>()(expr, state, visitor)</tt>
                template<typename Expr, typename State, typename Visitor>
                typename result<void(Expr, State, Visitor)>::type
                operator ()(Expr const &expr, State const &state, Visitor &visitor) const
                {
                    return pass_through<negate>()(expr, state, visitor);
                }

                /// INTERNAL ONLY
                typedef proto::tag::negate proto_tag;
                /// INTERNAL ONLY
                typedef T proto_arg0;
            };

            /// \brief A metafunction for generating defereference expression types,
            /// a grammar element for matching dereference expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T>
            struct dereference
            {
                typedef proto::expr<proto::tag::dereference, args1<T> > type;
                typedef type proto_base_expr;

                template<typename Sig>
                struct result
                {
                    typedef
                        typename pass_through<dereference>::template result<Sig>::type
                    type;
                };

                /// \param expr The current expression
                /// \param state The current state
                /// \param visitor An arbitrary visitor
                /// \pre <tt>matches\<Expr, dereference\<T\> \>::::value</tt> is \c true.
                /// \return <tt>pass_through\<dereference\<T\> \>()(expr, state, visitor)</tt>
                template<typename Expr, typename State, typename Visitor>
                typename result<void(Expr, State, Visitor)>::type
                operator ()(Expr const &expr, State const &state, Visitor &visitor) const
                {
                    return pass_through<dereference>()(expr, state, visitor);
                }

                /// INTERNAL ONLY
                typedef proto::tag::dereference proto_tag;
                /// INTERNAL ONLY
                typedef T proto_arg0;
            };

            /// \brief A metafunction for generating complement expression types,
            /// a grammar element for matching complement expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T>
            struct complement
            {
                typedef proto::expr<proto::tag::complement, args1<T> > type;
                typedef type proto_base_expr;

                template<typename Sig>
                struct result
                {
                    typedef
                        typename pass_through<complement>::template result<Sig>::type
                    type;
                };

                /// \param expr The current expression
                /// \param state The current state
                /// \param visitor An arbitrary visitor
                /// \pre <tt>matches\<Expr, complement\<T\> \>::::value</tt> is \c true.
                /// \return <tt>pass_through\<complement\<T\> \>()(expr, state, visitor)</tt>
                template<typename Expr, typename State, typename Visitor>
                typename result<void(Expr, State, Visitor)>::type
                operator ()(Expr const &expr, State const &state, Visitor &visitor) const
                {
                    return pass_through<complement>()(expr, state, visitor);
                }

                /// INTERNAL ONLY
                typedef proto::tag::complement proto_tag;
                /// INTERNAL ONLY
                typedef T proto_arg0;
            };

            /// \brief A metafunction for generating address_of expression types,
            /// a grammar element for matching address_of expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T>
            struct address_of
            {
                typedef proto::expr<proto::tag::address_of, args1<T> > type;
                typedef type proto_base_expr;

                template<typename Sig>
                struct result
                {
                    typedef
                        typename pass_through<address_of>::template result<Sig>::type
                    type;
                };

                /// \param expr The current expression
                /// \param state The current state
                /// \param visitor An arbitrary visitor
                /// \pre <tt>matches\<Expr, address_of\<T\> \>::::value</tt> is \c true.
                /// \return <tt>pass_through\<address_of\<T\> \>()(expr, state, visitor)</tt>
                template<typename Expr, typename State, typename Visitor>
                typename result<void(Expr, State, Visitor)>::type
                operator ()(Expr const &expr, State const &state, Visitor &visitor) const
                {
                    return pass_through<address_of>()(expr, state, visitor);
                }

                /// INTERNAL ONLY
                typedef proto::tag::address_of proto_tag;
                /// INTERNAL ONLY
                typedef T proto_arg0;
            };

            /// \brief A metafunction for generating logical_not expression types,
            /// a grammar element for matching logical_not expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T>
            struct logical_not
            {
                typedef proto::expr<proto::tag::logical_not, args1<T> > type;
                typedef type proto_base_expr;

                template<typename Sig>
                struct result
                {
                    typedef
                        typename pass_through<logical_not>::template result<Sig>::type
                    type;
                };

                /// \param expr The current expression
                /// \param state The current state
                /// \param visitor An arbitrary visitor
                /// \pre <tt>matches\<Expr, logical_not\<T\> \>::::value</tt> is \c true.
                /// \return <tt>pass_through\<logical_not\<T\> \>()(expr, state, visitor)</tt>
                template<typename Expr, typename State, typename Visitor>
                typename result<void(Expr, State, Visitor)>::type
                operator ()(Expr const &expr, State const &state, Visitor &visitor) const
                {
                    return pass_through<logical_not>()(expr, state, visitor);
                }

                /// INTERNAL ONLY
                typedef proto::tag::logical_not proto_tag;
                /// INTERNAL ONLY
                typedef T proto_arg0;
            };

            /// \brief A metafunction for generating pre-increment expression types,
            /// a grammar element for matching pre-increment expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T>
            struct pre_inc
            {
                typedef proto::expr<proto::tag::pre_inc, args1<T> > type;
                typedef type proto_base_expr;

                template<typename Sig>
                struct result
                {
                    typedef
                        typename pass_through<pre_inc>::template result<Sig>::type
                    type;
                };

                /// \param expr The current expression
                /// \param state The current state
                /// \param visitor An arbitrary visitor
                /// \pre <tt>matches\<Expr, pre_inc\<T\> \>::::value</tt> is \c true.
                /// \return <tt>pass_through\<pre_inc\<T\> \>()(expr, state, visitor)</tt>
                template<typename Expr, typename State, typename Visitor>
                typename result<void(Expr, State, Visitor)>::type
                operator ()(Expr const &expr, State const &state, Visitor &visitor) const
                {
                    return pass_through<pre_inc>()(expr, state, visitor);
                }

                /// INTERNAL ONLY
                typedef proto::tag::pre_inc proto_tag;
                /// INTERNAL ONLY
                typedef T proto_arg0;
            };

            /// \brief A metafunction for generating pre-decrement expression types,
            /// a grammar element for matching pre-decrement expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T>
            struct pre_dec
            {
                typedef proto::expr<proto::tag::pre_dec, args1<T> > type;
                typedef type proto_base_expr;

                template<typename Sig>
                struct result
                {
                    typedef
                        typename pass_through<pre_dec>::template result<Sig>::type
                    type;
                };

                /// \param expr The current expression
                /// \param state The current state
                /// \param visitor An arbitrary visitor
                /// \pre <tt>matches\<Expr, pre_dec\<T\> \>::::value</tt> is \c true.
                /// \return <tt>pass_through\<pre_dec\<T\> \>()(expr, state, visitor)</tt>
                template<typename Expr, typename State, typename Visitor>
                typename result<void(Expr, State, Visitor)>::type
                operator ()(Expr const &expr, State const &state, Visitor &visitor) const
                {
                    return pass_through<pre_dec>()(expr, state, visitor);
                }

                /// INTERNAL ONLY
                typedef proto::tag::pre_dec proto_tag;
                /// INTERNAL ONLY
                typedef T proto_arg0;
            };

            /// \brief A metafunction for generating post-increment expression types,
            /// a grammar element for matching post-increment expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T>
            struct post_inc
            {
                typedef proto::expr<proto::tag::post_inc, args1<T> > type;
                typedef type proto_base_expr;

                template<typename Sig>
                struct result
                {
                    typedef
                        typename pass_through<post_inc>::template result<Sig>::type
                    type;
                };

                /// \param expr The current expression
                /// \param state The current state
                /// \param visitor An arbitrary visitor
                /// \pre <tt>matches\<Expr, post_inc\<T\> \>::::value</tt> is \c true.
                /// \return <tt>pass_through\<post_inc\<T\> \>()(expr, state, visitor)</tt>
                template<typename Expr, typename State, typename Visitor>
                typename result<void(Expr, State, Visitor)>::type
                operator ()(Expr const &expr, State const &state, Visitor &visitor) const
                {
                    return pass_through<post_inc>()(expr, state, visitor);
                }

                /// INTERNAL ONLY
                typedef proto::tag::post_inc proto_tag;
                /// INTERNAL ONLY
                typedef T proto_arg0;
            };

            /// \brief A metafunction for generating post-decrement expression types,
            /// a grammar element for matching post-decrement expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T>
            struct post_dec
            {
                typedef proto::expr<proto::tag::post_dec, args1<T> > type;
                typedef type proto_base_expr;

                template<typename Sig>
                struct result
                {
                    typedef
                        typename pass_through<post_dec>::template result<Sig>::type
                    type;
                };

                /// \param expr The current expression
                /// \param state The current state
                /// \param visitor An arbitrary visitor
                /// \pre <tt>matches\<Expr, post_dec\<T\> \>::::value</tt> is \c true.
                /// \return <tt>pass_through\<post_dec\<T\> \>()(expr, state, visitor)</tt>
                template<typename Expr, typename State, typename Visitor>
                typename result<void(Expr, State, Visitor)>::type
                operator ()(Expr const &expr, State const &state, Visitor &visitor) const
                {
                    return pass_through<post_dec>()(expr, state, visitor);
                }

                /// INTERNAL ONLY
                typedef proto::tag::post_dec proto_tag;
                /// INTERNAL ONLY
                typedef T proto_arg0;
            };

            /// \brief A metafunction for generating left-shift expression types,
            /// a grammar element for matching left-shift expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U>
            struct shift_left
            {
                typedef proto::expr<proto::tag::shift_left, args2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Sig>
                struct result
                {
                    typedef
                        typename pass_through<shift_left>::template result<Sig>::type
                    type;
                };

                /// \param expr The current expression
                /// \param state The current state
                /// \param visitor An arbitrary visitor
                /// \pre <tt>matches\<Expr, shift_left\<T,U\> \>::::value</tt> is \c true.
                /// \return <tt>pass_through\<shift_left\<T,U\> \>()(expr, state, visitor)</tt>
                template<typename Expr, typename State, typename Visitor>
                typename result<void(Expr, State, Visitor)>::type
                operator ()(Expr const &expr, State const &state, Visitor &visitor) const
                {
                    return pass_through<shift_left>()(expr, state, visitor);
                }

                /// INTERNAL ONLY
                typedef proto::tag::shift_left proto_tag;
                /// INTERNAL ONLY
                typedef T proto_arg0;
                /// INTERNAL ONLY
                typedef U proto_arg1;
            };

            /// \brief A metafunction for generating right-shift expression types,
            /// a grammar element for matching right-shift expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U>
            struct shift_right
            {
                typedef proto::expr<proto::tag::shift_right, args2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Sig>
                struct result
                {
                    typedef
                        typename pass_through<shift_right>::template result<Sig>::type
                    type;
                };

                /// \param expr The current expression
                /// \param state The current state
                /// \param visitor An arbitrary visitor
                /// \pre <tt>matches\<Expr, shift_right\<T,U\> \>::::value</tt> is \c true.
                /// \return <tt>pass_through\<shift_right\<T,U\> \>()(expr, state, visitor)</tt>
                template<typename Expr, typename State, typename Visitor>
                typename result<void(Expr, State, Visitor)>::type
                operator ()(Expr const &expr, State const &state, Visitor &visitor) const
                {
                    return pass_through<shift_right>()(expr, state, visitor);
                }

                /// INTERNAL ONLY
                typedef proto::tag::shift_right proto_tag;
                /// INTERNAL ONLY
                typedef T proto_arg0;
                /// INTERNAL ONLY
                typedef U proto_arg1;
            };

            /// \brief A metafunction for generating multiplies expression types,
            /// a grammar element for matching multiplies expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U>
            struct multiplies
            {
                typedef proto::expr<proto::tag::multiplies, args2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Sig>
                struct result
                {
                    typedef
                        typename pass_through<multiplies>::template result<Sig>::type
                    type;
                };

                /// \param expr The current expression
                /// \param state The current state
                /// \param visitor An arbitrary visitor
                /// \pre <tt>matches\<Expr, multiplies\<T,U\> \>::::value</tt> is \c true.
                /// \return <tt>pass_through\<multiplies\<T,U\> \>()(expr, state, visitor)</tt>
                template<typename Expr, typename State, typename Visitor>
                typename result<void(Expr, State, Visitor)>::type
                operator ()(Expr const &expr, State const &state, Visitor &visitor) const
                {
                    return pass_through<multiplies>()(expr, state, visitor);
                }

                /// INTERNAL ONLY
                typedef proto::tag::multiplies proto_tag;
                /// INTERNAL ONLY
                typedef T proto_arg0;
                /// INTERNAL ONLY
                typedef U proto_arg1;
            };

            /// \brief A metafunction for generating divides expression types,
            /// a grammar element for matching divides expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U>
            struct divides
            {
                typedef proto::expr<proto::tag::divides, args2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Sig>
                struct result
                {
                    typedef
                        typename pass_through<divides>::template result<Sig>::type
                    type;
                };

                /// \param expr The current expression
                /// \param state The current state
                /// \param visitor An arbitrary visitor
                /// \pre <tt>matches\<Expr, divides\<T,U\> \>::::value</tt> is \c true.
                /// \return <tt>pass_through\<divides\<T,U\> \>()(expr, state, visitor)</tt>
                template<typename Expr, typename State, typename Visitor>
                typename result<void(Expr, State, Visitor)>::type
                operator ()(Expr const &expr, State const &state, Visitor &visitor) const
                {
                    return pass_through<divides>()(expr, state, visitor);
                }

                /// INTERNAL ONLY
                typedef proto::tag::divides proto_tag;
                /// INTERNAL ONLY
                typedef T proto_arg0;
                /// INTERNAL ONLY
                typedef U proto_arg1;
            };

            /// \brief A metafunction for generating modulus expression types,
            /// a grammar element for matching modulus expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U>
            struct modulus
            {
                typedef proto::expr<proto::tag::modulus, args2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Sig>
                struct result
                {
                    typedef
                        typename pass_through<modulus>::template result<Sig>::type
                    type;
                };

                /// \param expr The current expression
                /// \param state The current state
                /// \param visitor An arbitrary visitor
                /// \pre <tt>matches\<Expr, modulus\<T,U\> \>::::value</tt> is \c true.
                /// \return <tt>pass_through\<modulus\<T,U\> \>()(expr, state, visitor)</tt>
                template<typename Expr, typename State, typename Visitor>
                typename result<void(Expr, State, Visitor)>::type
                operator ()(Expr const &expr, State const &state, Visitor &visitor) const
                {
                    return pass_through<modulus>()(expr, state, visitor);
                }

                /// INTERNAL ONLY
                typedef proto::tag::modulus proto_tag;
                /// INTERNAL ONLY
                typedef T proto_arg0;
                /// INTERNAL ONLY
                typedef U proto_arg1;
            };

            /// \brief A metafunction for generating binary plus expression types,
            /// a grammar element for matching binary plus expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U>
            struct plus
            {
                typedef proto::expr<proto::tag::plus, args2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Sig>
                struct result
                {
                    typedef
                        typename pass_through<plus>::template result<Sig>::type
                    type;
                };

                /// \param expr The current expression
                /// \param state The current state
                /// \param visitor An arbitrary visitor
                /// \pre <tt>matches\<Expr, plus\<T,U\> \>::::value</tt> is \c true.
                /// \return <tt>pass_through\<plus\<T,U\> \>()(expr, state, visitor)</tt>
                template<typename Expr, typename State, typename Visitor>
                typename result<void(Expr, State, Visitor)>::type
                operator ()(Expr const &expr, State const &state, Visitor &visitor) const
                {
                    return pass_through<plus>()(expr, state, visitor);
                }

                /// INTERNAL ONLY
                typedef proto::tag::plus proto_tag;
                /// INTERNAL ONLY
                typedef T proto_arg0;
                /// INTERNAL ONLY
                typedef U proto_arg1;
            };

            /// \brief A metafunction for generating binary minus expression types,
            /// a grammar element for matching binary minus expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U>
            struct minus
            {
                typedef proto::expr<proto::tag::minus, args2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Sig>
                struct result
                {
                    typedef
                        typename pass_through<minus>::template result<Sig>::type
                    type;
                };

                /// \param expr The current expression
                /// \param state The current state
                /// \param visitor An arbitrary visitor
                /// \pre <tt>matches\<Expr, minus\<T,U\> \>::::value</tt> is \c true.
                /// \return <tt>pass_through\<minus\<T,U\> \>()(expr, state, visitor)</tt>
                template<typename Expr, typename State, typename Visitor>
                typename result<void(Expr, State, Visitor)>::type
                operator ()(Expr const &expr, State const &state, Visitor &visitor) const
                {
                    return pass_through<minus>()(expr, state, visitor);
                }

                /// INTERNAL ONLY
                typedef proto::tag::minus proto_tag;
                /// INTERNAL ONLY
                typedef T proto_arg0;
                /// INTERNAL ONLY
                typedef U proto_arg1;
            };

            /// \brief A metafunction for generating less expression types,
            /// a grammar element for matching less expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U>
            struct less
            {
                typedef proto::expr<proto::tag::less, args2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Sig>
                struct result
                {
                    typedef
                        typename pass_through<less>::template result<Sig>::type
                    type;
                };

                /// \param expr The current expression
                /// \param state The current state
                /// \param visitor An arbitrary visitor
                /// \pre <tt>matches\<Expr, less\<T,U\> \>::::value</tt> is \c true.
                /// \return <tt>pass_through\<less\<T,U\> \>()(expr, state, visitor)</tt>
                template<typename Expr, typename State, typename Visitor>
                typename result<void(Expr, State, Visitor)>::type
                operator ()(Expr const &expr, State const &state, Visitor &visitor) const
                {
                    return pass_through<less>()(expr, state, visitor);
                }

                /// INTERNAL ONLY
                typedef proto::tag::less proto_tag;
                /// INTERNAL ONLY
                typedef T proto_arg0;
                /// INTERNAL ONLY
                typedef U proto_arg1;
            };

            /// \brief A metafunction for generating greater expression types,
            /// a grammar element for matching greater expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U>
            struct greater
            {
                typedef proto::expr<proto::tag::greater, args2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Sig>
                struct result
                {
                    typedef
                        typename pass_through<greater>::template result<Sig>::type
                    type;
                };

                /// \param expr The current expression
                /// \param state The current state
                /// \param visitor An arbitrary visitor
                /// \pre <tt>matches\<Expr, greater\<T,U\> \>::::value</tt> is \c true.
                /// \return <tt>pass_through\<greater\<T,U\> \>()(expr, state, visitor)</tt>
                template<typename Expr, typename State, typename Visitor>
                typename result<void(Expr, State, Visitor)>::type
                operator ()(Expr const &expr, State const &state, Visitor &visitor) const
                {
                    return pass_through<greater>()(expr, state, visitor);
                }

                /// INTERNAL ONLY
                typedef proto::tag::greater proto_tag;
                /// INTERNAL ONLY
                typedef T proto_arg0;
                /// INTERNAL ONLY
                typedef U proto_arg1;
            };

            /// \brief A metafunction for generating less-or-equal expression types,
            /// a grammar element for matching less-or-equal expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U>
            struct less_equal
            {
                typedef proto::expr<proto::tag::less_equal, args2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Sig>
                struct result
                {
                    typedef
                        typename pass_through<less_equal>::template result<Sig>::type
                    type;
                };

                /// \param expr The current expression
                /// \param state The current state
                /// \param visitor An arbitrary visitor
                /// \pre <tt>matches\<Expr, less_equal\<T,U\> \>::::value</tt> is \c true.
                /// \return <tt>pass_through\<less_equal\<T,U\> \>()(expr, state, visitor)</tt>
                template<typename Expr, typename State, typename Visitor>
                typename result<void(Expr, State, Visitor)>::type
                operator ()(Expr const &expr, State const &state, Visitor &visitor) const
                {
                    return pass_through<less_equal>()(expr, state, visitor);
                }

                /// INTERNAL ONLY
                typedef proto::tag::less_equal proto_tag;
                /// INTERNAL ONLY
                typedef T proto_arg0;
                /// INTERNAL ONLY
                typedef U proto_arg1;
            };

            /// \brief A metafunction for generating greater-or-equal expression types,
            /// a grammar element for matching greater-or-equal expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U>
            struct greater_equal
            {
                typedef proto::expr<proto::tag::greater_equal, args2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Sig>
                struct result
                {
                    typedef
                        typename pass_through<greater_equal>::template result<Sig>::type
                    type;
                };

                /// \param expr The current expression
                /// \param state The current state
                /// \param visitor An arbitrary visitor
                /// \pre <tt>matches\<Expr, greater_equal\<T,U\> \>::::value</tt> is \c true.
                /// \return <tt>pass_through\<greater_equal\<T,U\> \>()(expr, state, visitor)</tt>
                template<typename Expr, typename State, typename Visitor>
                typename result<void(Expr, State, Visitor)>::type
                operator ()(Expr const &expr, State const &state, Visitor &visitor) const
                {
                    return pass_through<greater_equal>()(expr, state, visitor);
                }

                /// INTERNAL ONLY
                typedef proto::tag::greater_equal proto_tag;
                /// INTERNAL ONLY
                typedef T proto_arg0;
                /// INTERNAL ONLY
                typedef U proto_arg1;
            };

            /// \brief A metafunction for generating equal-to expression types,
            /// a grammar element for matching equal-to expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U>
            struct equal_to
            {
                typedef proto::expr<proto::tag::equal_to, args2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Sig>
                struct result
                {
                    typedef
                        typename pass_through<equal_to>::template result<Sig>::type
                    type;
                };

                /// \param expr The current expression
                /// \param state The current state
                /// \param visitor An arbitrary visitor
                /// \pre <tt>matches\<Expr, equal_to\<T,U\> \>::::value</tt> is \c true.
                /// \return <tt>pass_through\<equal_to\<T,U\> \>()(expr, state, visitor)</tt>
                template<typename Expr, typename State, typename Visitor>
                typename result<void(Expr, State, Visitor)>::type
                operator ()(Expr const &expr, State const &state, Visitor &visitor) const
                {
                    return pass_through<equal_to>()(expr, state, visitor);
                }

                /// INTERNAL ONLY
                typedef proto::tag::equal_to proto_tag;
                /// INTERNAL ONLY
                typedef T proto_arg0;
                /// INTERNAL ONLY
                typedef U proto_arg1;
            };

            /// \brief A metafunction for generating not-equal-to expression types,
            /// a grammar element for matching not-equal-to expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U>
            struct not_equal_to
            {
                typedef proto::expr<proto::tag::not_equal_to, args2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Sig>
                struct result
                {
                    typedef
                        typename pass_through<not_equal_to>::template result<Sig>::type
                    type;
                };

                /// \param expr The current expression
                /// \param state The current state
                /// \param visitor An arbitrary visitor
                /// \pre <tt>matches\<Expr, not_equal_to\<T,U\> \>::::value</tt> is \c true.
                /// \return <tt>pass_through\<not_equal_to\<T,U\> \>()(expr, state, visitor)</tt>
                template<typename Expr, typename State, typename Visitor>
                typename result<void(Expr, State, Visitor)>::type
                operator ()(Expr const &expr, State const &state, Visitor &visitor) const
                {
                    return pass_through<not_equal_to>()(expr, state, visitor);
                }

                /// INTERNAL ONLY
                typedef proto::tag::not_equal_to proto_tag;
                /// INTERNAL ONLY
                typedef T proto_arg0;
                /// INTERNAL ONLY
                typedef U proto_arg1;
            };

            /// \brief A metafunction for generating logical-or expression types,
            /// a grammar element for matching logical-or expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U>
            struct logical_or
            {
                typedef proto::expr<proto::tag::logical_or, args2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Sig>
                struct result
                {
                    typedef
                        typename pass_through<logical_or>::template result<Sig>::type
                    type;
                };

                /// \param expr The current expression
                /// \param state The current state
                /// \param visitor An arbitrary visitor
                /// \pre <tt>matches\<Expr, logical_or\<T,U\> \>::::value</tt> is \c true.
                /// \return <tt>pass_through\<logical_or\<T,U\> \>()(expr, state, visitor)</tt>
                template<typename Expr, typename State, typename Visitor>
                typename result<void(Expr, State, Visitor)>::type
                operator ()(Expr const &expr, State const &state, Visitor &visitor) const
                {
                    return pass_through<logical_or>()(expr, state, visitor);
                }

                /// INTERNAL ONLY
                typedef proto::tag::logical_or proto_tag;
                /// INTERNAL ONLY
                typedef T proto_arg0;
                /// INTERNAL ONLY
                typedef U proto_arg1;
            };

            /// \brief A metafunction for generating logical-and expression types,
            /// a grammar element for matching logical-and expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U>
            struct logical_and
            {
                typedef proto::expr<proto::tag::logical_and, args2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Sig>
                struct result
                {
                    typedef
                        typename pass_through<logical_and>::template result<Sig>::type
                    type;
                };

                /// \param expr The current expression
                /// \param state The current state
                /// \param visitor An arbitrary visitor
                /// \pre <tt>matches\<Expr, logical_and\<T,U\> \>::::value</tt> is \c true.
                /// \return <tt>pass_through\<logical_and\<T,U\> \>()(expr, state, visitor)</tt>
                template<typename Expr, typename State, typename Visitor>
                typename result<void(Expr, State, Visitor)>::type
                operator ()(Expr const &expr, State const &state, Visitor &visitor) const
                {
                    return pass_through<logical_and>()(expr, state, visitor);
                }

                /// INTERNAL ONLY
                typedef proto::tag::logical_and proto_tag;
                /// INTERNAL ONLY
                typedef T proto_arg0;
                /// INTERNAL ONLY
                typedef U proto_arg1;
            };

            /// \brief A metafunction for generating bitwise-and expression types,
            /// a grammar element for matching bitwise-and expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U>
            struct bitwise_and
            {
                typedef proto::expr<proto::tag::bitwise_and, args2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Sig>
                struct result
                {
                    typedef
                        typename pass_through<bitwise_and>::template result<Sig>::type
                    type;
                };

                /// \param expr The current expression
                /// \param state The current state
                /// \param visitor An arbitrary visitor
                /// \pre <tt>matches\<Expr, bitwise_and\<T,U\> \>::::value</tt> is \c true.
                /// \return <tt>pass_through\<bitwise_and\<T,U\> \>()(expr, state, visitor)</tt>
                template<typename Expr, typename State, typename Visitor>
                typename result<void(Expr, State, Visitor)>::type
                operator ()(Expr const &expr, State const &state, Visitor &visitor) const
                {
                    return pass_through<bitwise_and>()(expr, state, visitor);
                }

                /// INTERNAL ONLY
                typedef proto::tag::bitwise_and proto_tag;
                /// INTERNAL ONLY
                typedef T proto_arg0;
                /// INTERNAL ONLY
                typedef U proto_arg1;
            };

            /// \brief A metafunction for generating bitwise-or expression types,
            /// a grammar element for matching bitwise-or expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U>
            struct bitwise_or
            {
                typedef proto::expr<proto::tag::bitwise_or, args2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Sig>
                struct result
                {
                    typedef
                        typename pass_through<bitwise_or>::template result<Sig>::type
                    type;
                };

                /// \param expr The current expression
                /// \param state The current state
                /// \param visitor An arbitrary visitor
                /// \pre <tt>matches\<Expr, bitwise_or\<T,U\> \>::::value</tt> is \c true.
                /// \return <tt>pass_through\<bitwise_or\<T,U\> \>()(expr, state, visitor)</tt>
                template<typename Expr, typename State, typename Visitor>
                typename result<void(Expr, State, Visitor)>::type
                operator ()(Expr const &expr, State const &state, Visitor &visitor) const
                {
                    return pass_through<bitwise_or>()(expr, state, visitor);
                }

                /// INTERNAL ONLY
                typedef proto::tag::bitwise_or proto_tag;
                /// INTERNAL ONLY
                typedef T proto_arg0;
                /// INTERNAL ONLY
                typedef U proto_arg1;
            };

            /// \brief A metafunction for generating bitwise-xor expression types,
            /// a grammar element for matching bitwise-xor expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U>
            struct bitwise_xor
            {
                typedef proto::expr<proto::tag::bitwise_xor, args2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Sig>
                struct result
                {
                    typedef
                        typename pass_through<bitwise_xor>::template result<Sig>::type
                    type;
                };

                /// \param expr The current expression
                /// \param state The current state
                /// \param visitor An arbitrary visitor
                /// \pre <tt>matches\<Expr, bitwise_xor\<T,U\> \>::::value</tt> is \c true.
                /// \return <tt>pass_through\<bitwise_xor\<T,U\> \>()(expr, state, visitor)</tt>
                template<typename Expr, typename State, typename Visitor>
                typename result<void(Expr, State, Visitor)>::type
                operator ()(Expr const &expr, State const &state, Visitor &visitor) const
                {
                    return pass_through<bitwise_xor>()(expr, state, visitor);
                }

                /// INTERNAL ONLY
                typedef proto::tag::bitwise_xor proto_tag;
                /// INTERNAL ONLY
                typedef T proto_arg0;
                /// INTERNAL ONLY
                typedef U proto_arg1;
            };

            /// \brief A metafunction for generating comma expression types,
            /// a grammar element for matching comma expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U>
            struct comma
            {
                typedef proto::expr<proto::tag::comma, args2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Sig>
                struct result
                {
                    typedef
                        typename pass_through<comma>::template result<Sig>::type
                    type;
                };

                /// \param expr The current expression
                /// \param state The current state
                /// \param visitor An arbitrary visitor
                /// \pre <tt>matches\<Expr, comma\<T,U\> \>::::value</tt> is \c true.
                /// \return <tt>pass_through\<comma\<T,U\> \>()(expr, state, visitor)</tt>
                template<typename Expr, typename State, typename Visitor>
                typename result<void(Expr, State, Visitor)>::type
                operator ()(Expr const &expr, State const &state, Visitor &visitor) const
                {
                    return pass_through<comma>()(expr, state, visitor);
                }

                /// INTERNAL ONLY
                typedef proto::tag::comma proto_tag;
                /// INTERNAL ONLY
                typedef T proto_arg0;
                /// INTERNAL ONLY
                typedef U proto_arg1;
            };

            template<typename T, typename U>
            struct mem_ptr
            {
                typedef proto::expr<proto::tag::mem_ptr, args2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Sig>
                struct result
                {
                    typedef
                        typename pass_through<mem_ptr>::template result<Sig>::type
                    type;
                };

                /// \param expr The current expression
                /// \param state The current state
                /// \param visitor An arbitrary visitor
                /// \pre <tt>matches\<Expr, mem_ptr\<T,U\> \>::::value</tt> is \c true.
                /// \return <tt>pass_through\<mem_ptr\<T,U\> \>()(expr, state, visitor)</tt>
                template<typename Expr, typename State, typename Visitor>
                typename result<void(Expr, State, Visitor)>::type
                operator ()(Expr const &expr, State const &state, Visitor &visitor) const
                {
                    return pass_through<mem_ptr>()(expr, state, visitor);
                }

                /// INTERNAL ONLY
                typedef proto::tag::mem_ptr proto_tag;
                /// INTERNAL ONLY
                typedef T proto_arg0;
                /// INTERNAL ONLY
                typedef U proto_arg1;
            };

            /// \brief A metafunction for generating assignment expression types,
            /// a grammar element for matching assignment expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U>
            struct assign
            {
                typedef proto::expr<proto::tag::assign, args2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Sig>
                struct result
                {
                    typedef
                        typename pass_through<assign>::template result<Sig>::type
                    type;
                };

                /// \param expr The current expression
                /// \param state The current state
                /// \param visitor An arbitrary visitor
                /// \pre <tt>matches\<Expr, assign\<T,U\> \>::::value</tt> is \c true.
                /// \return <tt>pass_through\<assign\<T,U\> \>()(expr, state, visitor)</tt>
                template<typename Expr, typename State, typename Visitor>
                typename result<void(Expr, State, Visitor)>::type
                operator ()(Expr const &expr, State const &state, Visitor &visitor) const
                {
                    return pass_through<assign>()(expr, state, visitor);
                }

                /// INTERNAL ONLY
                typedef proto::tag::assign proto_tag;
                /// INTERNAL ONLY
                typedef T proto_arg0;
                /// INTERNAL ONLY
                typedef U proto_arg1;
            };

            /// \brief A metafunction for generating left-shift-assign expression types,
            /// a grammar element for matching left-shift-assign expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U>
            struct shift_left_assign
            {
                typedef proto::expr<proto::tag::shift_left_assign, args2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Sig>
                struct result
                {
                    typedef
                        typename pass_through<shift_left_assign>::template result<Sig>::type
                    type;
                };

                /// \param expr The current expression
                /// \param state The current state
                /// \param visitor An arbitrary visitor
                /// \pre <tt>matches\<Expr, shift_left_assign\<T,U\> \>::::value</tt> is \c true.
                /// \return <tt>pass_through\<shift_left_assign\<T,U\> \>()(expr, state, visitor)</tt>
                template<typename Expr, typename State, typename Visitor>
                typename result<void(Expr, State, Visitor)>::type
                operator ()(Expr const &expr, State const &state, Visitor &visitor) const
                {
                    return pass_through<shift_left_assign>()(expr, state, visitor);
                }

                /// INTERNAL ONLY
                typedef proto::tag::shift_left_assign proto_tag;
                /// INTERNAL ONLY
                typedef T proto_arg0;
                /// INTERNAL ONLY
                typedef U proto_arg1;
            };

            /// \brief A metafunction for generating right-shift-assign expression types,
            /// a grammar element for matching right-shift-assign expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U>
            struct shift_right_assign
            {
                typedef proto::expr<proto::tag::shift_right_assign, args2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Sig>
                struct result
                {
                    typedef
                        typename pass_through<shift_right_assign>::template result<Sig>::type
                    type;
                };

                /// \param expr The current expression
                /// \param state The current state
                /// \param visitor An arbitrary visitor
                /// \pre <tt>matches\<Expr, shift_right_assign\<T,U\> \>::::value</tt> is \c true.
                /// \return <tt>pass_through\<shift_right_assign\<T,U\> \>()(expr, state, visitor)</tt>
                template<typename Expr, typename State, typename Visitor>
                typename result<void(Expr, State, Visitor)>::type
                operator ()(Expr const &expr, State const &state, Visitor &visitor) const
                {
                    return pass_through<shift_right_assign>()(expr, state, visitor);
                }

                /// INTERNAL ONLY
                typedef proto::tag::shift_right_assign proto_tag;
                /// INTERNAL ONLY
                typedef T proto_arg0;
                /// INTERNAL ONLY
                typedef U proto_arg1;
            };

            /// \brief A metafunction for generating multiplies-assign expression types,
            /// a grammar element for matching multiplies-assign expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U>
            struct multiplies_assign
            {
                typedef proto::expr<proto::tag::multiplies_assign, args2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Sig>
                struct result
                {
                    typedef
                        typename pass_through<multiplies_assign>::template result<Sig>::type
                    type;
                };

                /// \param expr The current expression
                /// \param state The current state
                /// \param visitor An arbitrary visitor
                /// \pre <tt>matches\<Expr, multiplies_assign\<T,U\> \>::::value</tt> is \c true.
                /// \return <tt>pass_through\<multiplies_assign\<T,U\> \>()(expr, state, visitor)</tt>
                template<typename Expr, typename State, typename Visitor>
                typename result<void(Expr, State, Visitor)>::type
                operator ()(Expr const &expr, State const &state, Visitor &visitor) const
                {
                    return pass_through<multiplies_assign>()(expr, state, visitor);
                }

                /// INTERNAL ONLY
                typedef proto::tag::multiplies_assign proto_tag;
                /// INTERNAL ONLY
                typedef T proto_arg0;
                /// INTERNAL ONLY
                typedef U proto_arg1;
            };

            /// \brief A metafunction for generating divides-assign expression types,
            /// a grammar element for matching divides-assign expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U>
            struct divides_assign
            {
                typedef proto::expr<proto::tag::divides_assign, args2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Sig>
                struct result
                {
                    typedef
                        typename pass_through<divides_assign>::template result<Sig>::type
                    type;
                };

                /// \param expr The current expression
                /// \param state The current state
                /// \param visitor An arbitrary visitor
                /// \pre <tt>matches\<Expr, divides_assign\<T,U\> \>::::value</tt> is \c true.
                /// \return <tt>pass_through\<divides_assign\<T,U\> \>()(expr, state, visitor)</tt>
                template<typename Expr, typename State, typename Visitor>
                typename result<void(Expr, State, Visitor)>::type
                operator ()(Expr const &expr, State const &state, Visitor &visitor) const
                {
                    return pass_through<divides_assign>()(expr, state, visitor);
                }

                /// INTERNAL ONLY
                typedef proto::tag::divides_assign proto_tag;
                /// INTERNAL ONLY
                typedef T proto_arg0;
                /// INTERNAL ONLY
                typedef U proto_arg1;
            };

            /// \brief A metafunction for generating modulus-assign expression types,
            /// a grammar element for matching modulus-assign expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U>
            struct modulus_assign
            {
                typedef proto::expr<proto::tag::modulus_assign, args2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Sig>
                struct result
                {
                    typedef
                        typename pass_through<modulus_assign>::template result<Sig>::type
                    type;
                };

                /// \param expr The current expression
                /// \param state The current state
                /// \param visitor An arbitrary visitor
                /// \pre <tt>matches\<Expr, modulus_assign\<T,U\> \>::::value</tt> is \c true.
                /// \return <tt>pass_through\<modulus_assign\<T,U\> \>()(expr, state, visitor)</tt>
                template<typename Expr, typename State, typename Visitor>
                typename result<void(Expr, State, Visitor)>::type
                operator ()(Expr const &expr, State const &state, Visitor &visitor) const
                {
                    return pass_through<modulus_assign>()(expr, state, visitor);
                }

                /// INTERNAL ONLY
                typedef proto::tag::modulus_assign proto_tag;
                /// INTERNAL ONLY
                typedef T proto_arg0;
                /// INTERNAL ONLY
                typedef U proto_arg1;
            };

            /// \brief A metafunction for generating plus-assign expression types,
            /// a grammar element for matching plus-assign expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U>
            struct plus_assign
            {
                typedef proto::expr<proto::tag::plus_assign, args2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Sig>
                struct result
                {
                    typedef
                        typename pass_through<plus_assign>::template result<Sig>::type
                    type;
                };

                /// \param expr The current expression
                /// \param state The current state
                /// \param visitor An arbitrary visitor
                /// \pre <tt>matches\<Expr, plus_assign\<T,U\> \>::::value</tt> is \c true.
                /// \return <tt>pass_through\<plus_assign\<T,U\> \>()(expr, state, visitor)</tt>
                template<typename Expr, typename State, typename Visitor>
                typename result<void(Expr, State, Visitor)>::type
                operator ()(Expr const &expr, State const &state, Visitor &visitor) const
                {
                    return pass_through<plus_assign>()(expr, state, visitor);
                }

                /// INTERNAL ONLY
                typedef proto::tag::plus_assign proto_tag;
                /// INTERNAL ONLY
                typedef T proto_arg0;
                /// INTERNAL ONLY
                typedef U proto_arg1;
            };

            /// \brief A metafunction for generating minus-assign expression types,
            /// a grammar element for matching minus-assign expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U>
            struct minus_assign
            {
                typedef proto::expr<proto::tag::minus_assign, args2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Sig>
                struct result
                {
                    typedef
                        typename pass_through<minus_assign>::template result<Sig>::type
                    type;
                };

                /// \param expr The current expression
                /// \param state The current state
                /// \param visitor An arbitrary visitor
                /// \pre <tt>matches\<Expr, minus_assign\<T,U\> \>::::value</tt> is \c true.
                /// \return <tt>pass_through\<minus_assign\<T,U\> \>()(expr, state, visitor)</tt>
                template<typename Expr, typename State, typename Visitor>
                typename result<void(Expr, State, Visitor)>::type
                operator ()(Expr const &expr, State const &state, Visitor &visitor) const
                {
                    return pass_through<minus_assign>()(expr, state, visitor);
                }

                /// INTERNAL ONLY
                typedef proto::tag::minus_assign proto_tag;
                /// INTERNAL ONLY
                typedef T proto_arg0;
                /// INTERNAL ONLY
                typedef U proto_arg1;
            };

            /// \brief A metafunction for generating bitwise-and-assign expression types,
            /// a grammar element for matching bitwise-and-assign expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U>
            struct bitwise_and_assign
            {
                typedef proto::expr<proto::tag::bitwise_and_assign, args2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Sig>
                struct result
                {
                    typedef
                        typename pass_through<bitwise_and_assign>::template result<Sig>::type
                    type;
                };

                /// \param expr The current expression
                /// \param state The current state
                /// \param visitor An arbitrary visitor
                /// \pre <tt>matches\<Expr, bitwise_and_assign\<T,U\> \>::::value</tt> is \c true.
                /// \return <tt>pass_through\<bitwise_and_assign\<T,U\> \>()(expr, state, visitor)</tt>
                template<typename Expr, typename State, typename Visitor>
                typename result<void(Expr, State, Visitor)>::type
                operator ()(Expr const &expr, State const &state, Visitor &visitor) const
                {
                    return pass_through<bitwise_and_assign>()(expr, state, visitor);
                }

                /// INTERNAL ONLY
                typedef proto::tag::bitwise_and_assign proto_tag;
                /// INTERNAL ONLY
                typedef T proto_arg0;
                /// INTERNAL ONLY
                typedef U proto_arg1;
            };

            /// \brief A metafunction for generating bitwise-or-assign expression types,
            /// a grammar element for matching bitwise-or-assign expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U>
            struct bitwise_or_assign
            {
                typedef proto::expr<proto::tag::bitwise_or_assign, args2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Sig>
                struct result
                {
                    typedef
                        typename pass_through<bitwise_or_assign>::template result<Sig>::type
                    type;
                };

                /// \param expr The current expression
                /// \param state The current state
                /// \param visitor An arbitrary visitor
                /// \pre <tt>matches\<Expr, bitwise_or_assign\<T,U\> \>::::value</tt> is \c true.
                /// \return <tt>pass_through\<bitwise_or_assign\<T,U\> \>()(expr, state, visitor)</tt>
                template<typename Expr, typename State, typename Visitor>
                typename result<void(Expr, State, Visitor)>::type
                operator ()(Expr const &expr, State const &state, Visitor &visitor) const
                {
                    return pass_through<bitwise_or_assign>()(expr, state, visitor);
                }

                /// INTERNAL ONLY
                typedef proto::tag::bitwise_or_assign proto_tag;
                /// INTERNAL ONLY
                typedef T proto_arg0;
                /// INTERNAL ONLY
                typedef U proto_arg1;
            };

            /// \brief A metafunction for generating bitwise-xor-assign expression types,
            /// a grammar element for matching bitwise-xor-assign expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U>
            struct bitwise_xor_assign
            {
                typedef proto::expr<proto::tag::bitwise_xor_assign, args2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Sig>
                struct result
                {
                    typedef
                        typename pass_through<bitwise_xor_assign>::template result<Sig>::type
                    type;
                };

                /// \param expr The current expression
                /// \param state The current state
                /// \param visitor An arbitrary visitor
                /// \pre <tt>matches\<Expr, bitwise_xor_assign\<T,U\> \>::::value</tt> is \c true.
                /// \return <tt>pass_through\<bitwise_xor_assign\<T,U\> \>()(expr, state, visitor)</tt>
                template<typename Expr, typename State, typename Visitor>
                typename result<void(Expr, State, Visitor)>::type
                operator ()(Expr const &expr, State const &state, Visitor &visitor) const
                {
                    return pass_through<bitwise_xor_assign>()(expr, state, visitor);
                }

                /// INTERNAL ONLY
                typedef proto::tag::bitwise_xor_assign proto_tag;
                /// INTERNAL ONLY
                typedef T proto_arg0;
                /// INTERNAL ONLY
                typedef U proto_arg1;
            };

            /// \brief A metafunction for generating subscript expression types,
            /// a grammar element for matching subscript expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U>
            struct subscript
            {
                typedef proto::expr<proto::tag::subscript, args2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Sig>
                struct result
                {
                    typedef
                        typename pass_through<subscript>::template result<Sig>::type
                    type;
                };

                /// \param expr The current expression
                /// \param state The current state
                /// \param visitor An arbitrary visitor
                /// \pre <tt>matches\<Expr, subscript\<T,U\> \>::::value</tt> is \c true.
                /// \return <tt>pass_through\<subscript\<T,U\> \>()(expr, state, visitor)</tt>
                template<typename Expr, typename State, typename Visitor>
                typename result<void(Expr, State, Visitor)>::type
                operator ()(Expr const &expr, State const &state, Visitor &visitor) const
                {
                    return pass_through<subscript>()(expr, state, visitor);
                }

                /// INTERNAL ONLY
                typedef proto::tag::subscript proto_tag;
                /// INTERNAL ONLY
                typedef T proto_arg0;
                /// INTERNAL ONLY
                typedef U proto_arg1;
            };

        } // namespace op

    #define BOOST_PROTO_ARG(z, n, data)                                                             \
        /** INTERNAL ONLY */                                                                        \
        typedef BOOST_PP_CAT(data, n) BOOST_PP_CAT(proto_arg, n);                                   \
        /**/

    #define BOOST_PROTO_IMPLICIT_ARG(z, n, data)                                                    \
        BOOST_PP_CAT(data, n) &BOOST_PP_CAT(a, n);                                                  \
        /**/

    #define BOOST_PP_ITERATION_PARAMS_1 (3, (0, BOOST_PROTO_MAX_ARITY, <boost/xpressive/proto/traits.hpp>))
    #include BOOST_PP_ITERATE()

    #undef BOOST_PROTO_ARG
    #undef BOOST_PROTO_IMPLICIT_ARG

        namespace functional
        {
            /// \brief A callable PolymorphicFunctionObject that is
            /// equivalent to the \c as_expr() function.
            template<typename Domain    BOOST_PROTO_FOR_DOXYGEN_ONLY(= default_domain)>
            struct as_expr
            {
                BOOST_PROTO_CALLABLE()

                template<typename Sig>
                struct result;

                template<typename This, typename T>
                struct result<This(T)>
                {
                    typedef typename remove_reference<T>::type unref_type;
                    typedef typename result_of::as_expr<unref_type, Domain>::type type;
                };

                /// \brief Wrap an object in a Proto terminal if it isn't a
                /// Proto expression already.
                /// \param t The object to wrap.
                /// \return <tt>proto::as_expr\<Domain\>(t)</tt>
                template<typename T>
                typename result_of::as_expr<T, Domain>::reference
                operator ()(T &t) const
                {
                    return result_of::as_expr<T, Domain>::call(t);
                }

                /// \overload
                ///
                template<typename T>
                typename result_of::as_expr<T const, Domain>::reference
                operator ()(T const &t) const
                {
                    return result_of::as_expr<T const, Domain>::call(t);
                }

                #if BOOST_WORKAROUND(BOOST_MSVC, == 1310)
                template<typename T, std::size_t N_>
                typename result_of::as_expr<T(&)[N_], Domain>::reference
                operator ()(T (&t)[N_]) const
                {
                    return result_of::as_expr<T(&)[N_], Domain>::call(t);
                }

                template<typename T, std::size_t N_>
                typename result_of::as_expr<T const(&)[N_], Domain>::reference
                operator ()(T const (&t)[N_]) const
                {
                    return result_of::as_expr<T const(&)[N_], Domain>::call(t);
                }
                #endif
            };

            /// \brief A callable PolymorphicFunctionObject that is
            /// equivalent to the \c as_arg() function.
            template<typename Domain    BOOST_PROTO_FOR_DOXYGEN_ONLY(= default_domain)>
            struct as_arg
            {
                BOOST_PROTO_CALLABLE()

                template<typename Sig>
                struct result;

                template<typename This, typename T>
                struct result<This(T)>
                {
                    typedef typename remove_reference<T>::type unref_type;
                    typedef typename result_of::as_arg<unref_type, Domain>::type type;
                };

                /// \brief Wrap an object in a Proto terminal if it isn't a
                /// Proto expression already.
                /// \param t The object to wrap.
                /// \return <tt>proto::as_arg\<Domain\>(t)</tt>
                template<typename T>
                typename result_of::as_arg<T, Domain>::type
                operator ()(T &t) const
                {
                    return result_of::as_arg<T, Domain>::call(t);
                }

                /// \overload
                ///
                template<typename T>
                typename result_of::as_arg<T const, Domain>::type
                operator ()(T const &t) const
                {
                    return result_of::as_arg<T const, Domain>::call(t);
                }
            };

            /// \brief A callable PolymorphicFunctionObject that is
            /// equivalent to the \c arg_c() function.
            template<long N>
            struct arg_c
            {
                BOOST_PROTO_CALLABLE()

                template<typename Sig>
                struct result;

                template<typename This, typename Expr>
                struct result<This(Expr)>
                {
                    typedef BOOST_PROTO_UNCVREF(Expr) uncvref_type;
                    typedef typename result_of::arg_c<uncvref_type, N>::type type;
                };

                /// \brief Return the Nth child of the given expression.
                /// \param expr The expression node.
                /// \pre <tt>is_expr\<Expr\>::::value</tt> is \c true
                /// \pre <tt>N == 0 || N \< Expr::proto_arity::value</tt>
                /// \return <tt>proto::arg_c\<N\>(expr)</tt>
                /// \throw nothrow
                template<typename Expr>
                typename result_of::arg_c<Expr, N>::reference
                operator ()(Expr &expr) const
                {
                    return result_of::arg_c<Expr, N>::call(expr);
                }

                /// \overload
                ///
                template<typename Expr>
                typename result_of::arg_c<Expr, N>::const_reference
                operator ()(Expr const &expr) const
                {
                    return result_of::arg_c<Expr, N>::call(expr);
                }
            };

            /// \brief A callable PolymorphicFunctionObject that is
            /// equivalent to the \c arg() function.
            ///
            /// A callable PolymorphicFunctionObject that is
            /// equivalent to the \c arg() function. \c N is required
            /// to be an MPL Integral Constant.
            template<typename N BOOST_PROTO_FOR_DOXYGEN_ONLY(= mpl::long_<0>) >
            struct arg
            {
                BOOST_PROTO_CALLABLE()

                template<typename Sig>
                struct result;

                template<typename This, typename Expr>
                struct result<This(Expr)>
                {
                    typedef BOOST_PROTO_UNCVREF(Expr) uncvref_type;
                    typedef typename result_of::arg<uncvref_type, N>::type type;
                };

                /// \brief Return the Nth child of the given expression.
                /// \param expr The expression node.
                /// \pre <tt>is_expr\<Expr\>::::value</tt> is \c true
                /// \pre <tt>N::value == 0 || N::value \< Expr::proto_arity::value</tt>
                /// \return <tt>proto::arg\<N\>(expr)</tt>
                /// \throw nothrow
                template<typename Expr>
                typename result_of::arg<Expr, N>::reference
                operator ()(Expr &expr) const
                {
                    return result_of::arg<Expr, N>::call(expr);
                }

                /// \overload
                ///
                template<typename Expr>
                typename result_of::arg<Expr, N>::const_reference operator ()(Expr const &expr) const
                {
                    return result_of::arg<Expr, N>::call(expr);
                }
            };

            /// \brief A callable PolymorphicFunctionObject that is
            /// equivalent to the \c left() function.
            struct left
            {
                BOOST_PROTO_CALLABLE()

                template<typename Sig>
                struct result;

                template<typename This, typename Expr>
                struct result<This(Expr)>
                {
                    typedef BOOST_PROTO_UNCVREF(Expr) uncvref_type;
                    typedef typename result_of::left<uncvref_type>::type type;
                };

                /// \brief Return the left child of the given binary expression.
                /// \param expr The expression node.
                /// \pre <tt>is_expr\<Expr\>::::value</tt> is \c true
                /// \pre <tt>2 == Expr::proto_arity::value</tt>
                /// \return <tt>proto::left(expr)</tt>
                /// \throw nothrow
                template<typename Expr>
                typename result_of::left<Expr>::reference
                operator ()(Expr &expr) const
                {
                    return proto::unref(expr.proto_base().arg0);
                }

                /// \overload
                ///
                template<typename Expr>
                typename result_of::left<Expr>::const_reference
                operator ()(Expr const &expr) const
                {
                    return proto::unref(expr.proto_base().arg0);
                }
            };

            /// \brief A callable PolymorphicFunctionObject that is
            /// equivalent to the \c right() function.
            struct right
            {
                BOOST_PROTO_CALLABLE()

                template<typename Sig>
                struct result;

                template<typename This, typename Expr>
                struct result<This(Expr)>
                {
                    typedef BOOST_PROTO_UNCVREF(Expr) uncvref_type;
                    typedef typename result_of::right<uncvref_type>::type type;
                };

                /// \brief Return the right child of the given binary expression.
                /// \param expr The expression node.
                /// \pre <tt>is_expr\<Expr\>::::value</tt> is \c true
                /// \pre <tt>2 == Expr::proto_arity::value</tt>
                /// \return <tt>proto::right(expr)</tt>
                /// \throw nothrow
                template<typename Expr>
                typename result_of::right<Expr>::reference
                operator ()(Expr &expr) const
                {
                    return proto::unref(expr.proto_base().arg1);
                }

                template<typename Expr>
                typename result_of::right<Expr>::const_reference
                operator ()(Expr const &expr) const
                {
                    return proto::unref(expr.proto_base().arg1);
                }
            };

        }

        /// \brief A function that wraps non-Proto expression types in Proto
        /// terminals and leaves Proto expression types alone.
        ///
        /// The <tt>as_expr()</tt> function turns objects into Proto terminals if
        /// they are not Proto expression types already. Non-Proto types are
        /// held by value, if possible. Types which are already Proto types are
        /// left alone and returned by reference.
        ///
        /// This function can be called either with an explicitly specified
        /// \c Domain parameter (i.e., <tt>as_expr\<Domain\>(t)</tt>), or 
        /// without (i.e., <tt>as_expr(t)</tt>). If no domain is
        /// specified, \c default_domain is assumed.
        ///
        /// If <tt>is_expr\<T\>::::value</tt> is \c true, then the argument is
        /// returned unmodified, by reference. Otherwise, the argument is wrapped
        /// in a Proto terminal expression node according to the following rules.
        /// If \c T is an array type or a function type, let \c A be <tt>T &</tt>.
        /// Otherwise, let \c A be the type \c T stripped of cv-qualifiers.
        /// Then, \c as_expr() returns
        /// <tt>Domain::make(terminal\<A\>::::type::make(t))</tt>.
        ///
        /// \param t The object to wrap.
        template<typename T>
        typename result_of::as_expr<T>::reference
        as_expr(T &t BOOST_PROTO_DISABLE_IF_IS_CONST(T))
        {
            return result_of::as_expr<T>::call(t);
        }

        /// \overload
        ///
        template<typename T>
        typename result_of::as_expr<T const>::reference
        as_expr(T const &t)
        {
            return result_of::as_expr<T const>::call(t);
        }

        /// \overload
        ///
        template<typename Domain, typename T>
        typename result_of::as_expr<T, Domain>::reference
        as_expr(T &t BOOST_PROTO_DISABLE_IF_IS_CONST(T))
        {
            return result_of::as_expr<T, Domain>::call(t);
        }

        /// \overload
        ///
        template<typename Domain, typename T>
        typename result_of::as_expr<T const, Domain>::reference
        as_expr(T const &t)
        {
            return result_of::as_expr<T const, Domain>::call(t);
        }

        /// \brief A function that wraps non-Proto expression types in Proto
        /// terminals (by reference) and wraps Proto expression types in
        /// <tt>ref_\<\></tt>.
        ///
        /// The <tt>as_arg()</tt> function turns objects into Proto terminals if
        /// they are not Proto expression types already. Non-Proto types are
        /// held by reference. Types which are already Proto types are wrapped
        /// in <tt>ref_\<\></tt>.
        ///
        /// This function can be called either with an explicitly specified
        /// \c Domain parameter (i.e., <tt>as_arg\<Domain\>(t)</tt>), or 
        /// without (i.e., <tt>as_arg(t)</tt>). If no domain is
        /// specified, \c default_domain is assumed.
        ///
        /// If <tt>is_expr\<T\>::::value</tt> is \c true, then the argument is
        /// wrapped in <tt>ref_\<\></tt>, which holds the argument by reference.
        /// Otherwise, \c as_arg() returns
        /// <tt>Domain::make(terminal\<T &\>::::type::make(t))</tt>.
        ///
        /// \param t The object to wrap.
        template<typename T>
        typename result_of::as_arg<T>::type
        as_arg(T &t BOOST_PROTO_DISABLE_IF_IS_CONST(T))
        {
            return result_of::as_arg<T>::call(t);
        }

        /// \overload
        ///
        template<typename T>
        typename result_of::as_arg<T const>::type
        as_arg(T const &t)
        {
            return result_of::as_arg<T const>::call(t);
        }

        /// \overload
        ///
        template<typename Domain, typename T>
        typename result_of::as_arg<T, Domain>::type
        as_arg(T &t BOOST_PROTO_DISABLE_IF_IS_CONST(T))
        {
            return result_of::as_arg<T, Domain>::call(t);
        }

        /// \overload
        ///
        template<typename Domain, typename T>
        typename result_of::as_arg<T const, Domain>::type
        as_arg(T const &t)
        {
            return result_of::as_arg<T const, Domain>::call(t);
        }

        /// \brief Return the Nth child of the specified Proto expression.
        /// 
        /// Return the Nth child of the specified Proto expression. If
        /// \c N is not specified, as in \c arg(expr), then \c N is assumed
        /// to be <tt>mpl::long_\<0\></tt>. The child is returned by
        /// reference. If the expression is holding the child in a
        /// <tt>ref_\<\></tt> wrapper, it is unwrapped before it is returned.
        ///
        /// \param expr The Proto expression.
        /// \pre <tt>is_expr\<Expr\>::::value</tt> is \c true.
        /// \pre \c N is an MPL Integral Constant.
        /// \pre <tt>N::value == 0 || N::value \< Expr::proto_arity::value</tt>
        /// \throw nothrow
        /// \return A reference to the Nth child
        template<typename N, typename Expr>
        typename result_of::arg<Expr, N>::reference
        arg(Expr &expr BOOST_PROTO_DISABLE_IF_IS_CONST(Expr))
        {
            return result_of::arg<Expr, N>::call(expr);
        }

        /// \overload
        ///
        template<typename N, typename Expr>
        typename result_of::arg<Expr, N>::const_reference
        arg(Expr const &expr)
        {
            return result_of::arg<Expr, N>::call(expr);
        }

        /// \overload
        ///
        template<typename Expr2>
        typename result_of::unref<typename Expr2::proto_base_expr::proto_arg0>::reference
        arg(Expr2 &expr2 BOOST_PROTO_DISABLE_IF_IS_CONST(Expr2))
        {
            return proto::unref(expr2.proto_base().arg0);
        }

        /// \overload
        ///
        template<typename Expr2>
        typename result_of::unref<typename Expr2::proto_base_expr::proto_arg0>::const_reference
        arg(Expr2 const &expr2)
        {
            return proto::unref(expr2.proto_base().arg0);
        }

        /// \brief Return the Nth child of the specified Proto expression.
        /// 
        /// Return the Nth child of the specified Proto expression. The child
        /// is returned by reference. If the expression is holding the child in
        /// a <tt>ref_\<\></tt> wrapper, it is unwrapped before it is returned.
        ///
        /// \param expr The Proto expression.
        /// \pre <tt>is_expr\<Expr\>::::value</tt> is \c true.
        /// \pre <tt>N == 0 || N \< Expr::proto_arity::value</tt>
        /// \throw nothrow
        /// \return A reference to the Nth child
        template<long N, typename Expr>
        typename result_of::arg_c<Expr, N>::reference
        arg_c(Expr &expr BOOST_PROTO_DISABLE_IF_IS_CONST(Expr))
        {
            return result_of::arg_c<Expr, N>::call(expr);
        }

        /// \overload
        ///
        template<long N, typename Expr>
        typename result_of::arg_c<Expr, N>::const_reference
        arg_c(Expr const &expr)
        {
            return result_of::arg_c<Expr, N>::call(expr);
        }

        /// \brief Return the left child of the specified binary Proto
        /// expression.
        /// 
        /// Return the left child of the specified binary Proto expression. The
        /// child is returned by reference. If the expression is holding the
        /// child in a <tt>ref_\<\></tt> wrapper, it is unwrapped before it is
        /// returned.
        ///
        /// \param expr The Proto expression.
        /// \pre <tt>is_expr\<Expr\>::::value</tt> is \c true.
        /// \pre <tt>2 == Expr::proto_arity::value</tt>
        /// \throw nothrow
        /// \return A reference to the left child
        template<typename Expr>
        typename result_of::left<Expr>::reference
        left(Expr &expr BOOST_PROTO_DISABLE_IF_IS_CONST(Expr))
        {
            return proto::unref(expr.proto_base().arg0);
        }

        /// \overload
        ///
        template<typename Expr>
        typename result_of::left<Expr>::const_reference
        left(Expr const &expr)
        {
            return proto::unref(expr.proto_base().arg0);
        }

        /// \brief Return the right child of the specified binary Proto
        /// expression.
        /// 
        /// Return the right child of the specified binary Proto expression. The
        /// child is returned by reference. If the expression is holding the
        /// child in a <tt>ref_\<\></tt> wrapper, it is unwrapped before it is
        /// returned.
        ///
        /// \param expr The Proto expression.
        /// \pre <tt>is_expr\<Expr\>::::value</tt> is \c true.
        /// \pre <tt>2 == Expr::proto_arity::value</tt>
        /// \throw nothrow
        /// \return A reference to the right child
        template<typename Expr>
        typename result_of::right<Expr>::reference
        right(Expr &expr BOOST_PROTO_DISABLE_IF_IS_CONST(Expr))
        {
            return proto::unref(expr.proto_base().arg1);
        }

        /// \overload
        ///
        template<typename Expr>
        typename result_of::right<Expr>::const_reference
        right(Expr const &expr)
        {
            return proto::unref(expr.proto_base().arg1);
        }

        /// INTERNAL ONLY
        ///
        template<typename Domain>
        struct is_callable<functional::as_expr<Domain> >
          : mpl::true_
        {};

        /// INTERNAL ONLY
        ///
        template<typename Domain>
        struct is_callable<functional::as_arg<Domain> >
          : mpl::true_
        {};

        /// INTERNAL ONLY
        ///
        template<long N>
        struct is_callable<functional::arg_c<N> >
          : mpl::true_
        {};

        /// INTERNAL ONLY
        ///
        template<typename N>
        struct is_callable<functional::arg<N> >
          : mpl::true_
        {};

    }}

    #if BOOST_WORKAROUND( BOOST_MSVC, >= 1400 )
        #pragma warning(pop)
    #endif

    #endif

#else // PP_IS_ITERATING

    #define N BOOST_PP_ITERATION()
    #if N > 0
        namespace op
        {
            /// \brief A metafunction for generating function-call expression types,
            /// a grammar element for matching function-call expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<BOOST_PP_ENUM_PARAMS(N, typename A)>
            struct function<
                BOOST_PP_ENUM_PARAMS(N, A)
                BOOST_PP_ENUM_TRAILING_PARAMS(BOOST_PP_SUB(BOOST_PROTO_MAX_ARITY, N), void BOOST_PP_INTERCEPT), void
            >
            {
                typedef proto::expr<proto::tag::function, BOOST_PP_CAT(args, N)<BOOST_PP_ENUM_PARAMS(N, A)> > type;
                typedef type proto_base_expr;

                template<typename Sig>
                struct result
                {
                    typedef
                        typename pass_through<function>::template result<Sig>::type
                    type;
                };

                /// \param expr The current expression
                /// \param state The current state
                /// \param visitor An arbitrary visitor
                /// \pre <tt>matches\<Expr, function\>::::value</tt> is \c true.
                /// \return <tt>pass_through\<function\>()(expr, state, visitor)</tt>
                template<typename Expr, typename State, typename Visitor>
                typename result<void(Expr, State, Visitor)>::type
                operator ()(Expr const &expr, State const &state, Visitor &visitor) const
                {
                    return pass_through<function>()(expr, state, visitor);
                }

                /// INTERNAL ONLY
                typedef proto::tag::function proto_tag;
                BOOST_PP_REPEAT(N, BOOST_PROTO_ARG, A)
                BOOST_PP_REPEAT_FROM_TO(
                    N
                  , BOOST_PROTO_MAX_ARITY
                  , BOOST_PROTO_ARG
                  , detail::if_vararg<BOOST_PP_CAT(A, BOOST_PP_DEC(N))> BOOST_PP_INTERCEPT
                )
            };

            /// \brief A metafunction for generating n-ary expression types with a
            /// specified tag type,
            /// a grammar element for matching n-ary expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            ///
            /// Use <tt>nary_expr\<_, vararg\<_\> \></tt> as a grammar element to match any
            /// n-ary expression; that is, any non-terminal.
            template<typename Tag BOOST_PP_ENUM_TRAILING_PARAMS(N, typename A)>
            struct nary_expr<
                Tag
                BOOST_PP_ENUM_TRAILING_PARAMS(N, A)
                BOOST_PP_ENUM_TRAILING_PARAMS(BOOST_PP_SUB(BOOST_PROTO_MAX_ARITY, N), void BOOST_PP_INTERCEPT), void
            >
            {
                typedef proto::expr<Tag, BOOST_PP_CAT(args, N)<BOOST_PP_ENUM_PARAMS(N, A)> > type;
                typedef type proto_base_expr;

                template<typename Sig>
                struct result
                {
                    typedef
                        typename pass_through<nary_expr>::template result<Sig>::type
                    type;
                };

                /// \param expr The current expression
                /// \param state The current state
                /// \param visitor An arbitrary visitor
                /// \pre <tt>matches\<Expr, nary_expr\>::::value</tt> is \c true.
                /// \return <tt>pass_through\<nary_expr\>()(expr, state, visitor)</tt>
                template<typename Expr, typename State, typename Visitor>
                typename result<void(Expr, State, Visitor)>::type
                operator ()(Expr const &expr, State const &state, Visitor &visitor) const
                {
                    return pass_through<nary_expr>()(expr, state, visitor);
                }

                /// INTERNAL ONLY
                typedef Tag proto_tag;
                BOOST_PP_REPEAT(N, BOOST_PROTO_ARG, A)
                BOOST_PP_REPEAT_FROM_TO(
                    N
                  , BOOST_PROTO_MAX_ARITY
                  , BOOST_PROTO_ARG
                  , detail::if_vararg<BOOST_PP_CAT(A, BOOST_PP_DEC(N))> BOOST_PP_INTERCEPT
                )
            };

        } // namespace op

        namespace detail
        {
            template<BOOST_PP_ENUM_PARAMS(N, typename A)>
            struct BOOST_PP_CAT(implicit_expr_, N)
            {
                BOOST_PP_REPEAT(N, BOOST_PROTO_IMPLICIT_ARG, A)

                template<typename Tag, typename Args, long Arity>
                operator proto::expr<Tag, Args, Arity> () const
                {
                    proto::expr<Tag, Args, Arity> that = {BOOST_PP_ENUM_PARAMS(N, a)};
                    return that;
                }
            };

            template<
                template<BOOST_PP_ENUM_PARAMS(N, typename BOOST_PP_INTERCEPT)> class T
              , BOOST_PP_ENUM_PARAMS(N, typename A)
            >
            struct is_callable_<T<BOOST_PP_ENUM_PARAMS(N, A)> BOOST_MPL_AUX_LAMBDA_ARITY_PARAM(N)>
              : is_same<BOOST_PP_CAT(A, BOOST_PP_DEC(N)), callable>
            {};
        }

        /// INTERNAL ONLY
        template<BOOST_PP_ENUM_PARAMS(N, typename A)>
        detail::BOOST_PP_CAT(implicit_expr_, N)<BOOST_PP_ENUM_PARAMS(N, A)>
        implicit_expr(BOOST_PP_ENUM_BINARY_PARAMS(N, A, &a))
        {
            detail::BOOST_PP_CAT(implicit_expr_, N)<BOOST_PP_ENUM_PARAMS(N, A)> that
                = {BOOST_PP_ENUM_PARAMS(N, a)};
            return that;
        }

    #endif

        namespace result_of
        {
            /// \brief A metafunction that returns the type of the Nth child
            /// of a Proto expression.
            ///
            /// A metafunction that returns the type of the Nth child
            /// of a Proto expression. \c N must be 0 or less than
            /// \c Expr::proto_arity::value.
            template<typename Expr>
            struct arg_c<Expr, N>
            {
                /// The raw type of the Nth child as it is stored within
                /// \c Expr. This may be a value, a reference, or a Proto 
                /// <tt>ref_\<\></tt> wrapper.
                typedef typename Expr::BOOST_PP_CAT(proto_arg, N) wrapped_type;

                /// The "value" type of the child, suitable for return by value,
                /// computed as follows:
                /// \li <tt>ref_\<T const\></tt> becomes <tt>T</tt>
                /// \li <tt>ref_\<T\></tt> becomes <tt>T</tt>
                /// \li <tt>T const(&)[N]</tt> becomes <tt>T const(&)[N]</tt>
                /// \li <tt>T(&)[N]</tt> becomes <tt>T(&)[N]</tt>
                /// \li <tt>R(&)(A0,...)</tt> becomes <tt>R(&)(A0,...)</tt>
                /// \li <tt>T const &</tt> becomes <tt>T</tt>
                /// \li <tt>T &</tt> becomes <tt>T</tt>
                /// \li <tt>T</tt> becomes <tt>T</tt>
                typedef typename unref<wrapped_type>::type type;

                /// The "reference" type of the child, suitable for return by
                /// reference, computed as follows:
                /// \li <tt>ref_\<T const\></tt> becomes <tt>T const &</tt>
                /// \li <tt>ref_\<T\></tt> becomes <tt>T &</tt>
                /// \li <tt>T const(&)[N]</tt> becomes <tt>T const(&)[N]</tt>
                /// \li <tt>T(&)[N]</tt> becomes <tt>T(&)[N]</tt>
                /// \li <tt>R(&)(A0,...)</tt> becomes <tt>R(&)(A0,...)</tt>
                /// \li <tt>T const &</tt> becomes <tt>T const &</tt>
                /// \li <tt>T &</tt> becomes <tt>T &</tt>
                /// \li <tt>T</tt> becomes <tt>T &</tt>
                typedef typename unref<wrapped_type>::reference reference;

                /// The "const reference" type of the child, suitable for return by
                /// const reference, computed as follows:
                /// \li <tt>ref_\<T const\></tt> becomes <tt>T const &</tt>
                /// \li <tt>ref_\<T\></tt> becomes <tt>T &</tt>
                /// \li <tt>T const(&)[N]</tt> becomes <tt>T const(&)[N]</tt>
                /// \li <tt>T(&)[N]</tt> becomes <tt>T(&)[N]</tt>
                /// \li <tt>R(&)(A0,...)</tt> becomes <tt>R(&)(A0,...)</tt>
                /// \li <tt>T const &</tt> becomes <tt>T const &</tt>
                /// \li <tt>T &</tt> becomes <tt>T &</tt>
                /// \li <tt>T</tt> becomes <tt>T const &</tt>
                typedef typename unref<wrapped_type>::const_reference const_reference;

                /// INTERNAL ONLY
                ///
                static reference call(typename Expr::proto_derived_expr &expr)
                {
                    return proto::unref(expr.proto_base().BOOST_PP_CAT(arg, N));
                }

                /// INTERNAL ONLY
                ///
                static const_reference call(typename Expr::proto_derived_expr const &expr)
                {
                    return proto::unref(expr.proto_base().BOOST_PP_CAT(arg, N));
                }
            };
        }

    #undef N

#endif
