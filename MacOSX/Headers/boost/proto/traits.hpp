#ifndef BOOST_PP_IS_ITERATING
    ///////////////////////////////////////////////////////////////////////////////
    /// \file traits.hpp
    /// Contains definitions for child\<\>, child_c\<\>, left\<\>,
    /// right\<\>, tag_of\<\>, and the helper functions child(), child_c(),
    /// value(), left() and right().
    //
    //  Copyright 2008 Eric Niebler. Distributed under the Boost
    //  Software License, Version 1.0. (See accompanying file
    //  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

    #ifndef BOOST_PROTO_ARG_TRAITS_HPP_EAN_04_01_2005
    #define BOOST_PROTO_ARG_TRAITS_HPP_EAN_04_01_2005

    #include <boost/proto/detail/prefix.hpp>
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
    #include <boost/utility/enable_if.hpp>
    #include <boost/type_traits/is_pod.hpp>
    #include <boost/type_traits/is_same.hpp>
    #include <boost/type_traits/is_function.hpp>
    #include <boost/type_traits/remove_cv.hpp>
    #include <boost/type_traits/remove_const.hpp>
    #include <boost/type_traits/add_reference.hpp>
    #include <boost/proto/proto_fwd.hpp>
    #include <boost/proto/args.hpp>
    #include <boost/proto/tags.hpp>
    #include <boost/proto/transform/pass_through.hpp>
    #include <boost/proto/detail/suffix.hpp>

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

        #if BOOST_WORKAROUND(__GNUC__, == 3) || (__GNUC__ == 4 && __GNUC_MINOR__ == 0)
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
        template<typename T, typename Void>
        struct is_aggregate
          : is_pod<T>
        {};

        /// \brief Specialization of <tt>is_aggregate\<\></tt> that indicates
        /// that objects of <tt>expr\<\></tt> type require aggregate initialization.
        template<typename Tag, typename Args, long N>
        struct is_aggregate<proto::expr<Tag, Args, N>, void>
          : mpl::true_
        {};

        /// INTERNAL ONLY
        template<typename T>
        struct is_aggregate<T, typename T::proto_is_aggregate_>
          : mpl::true_
        {};

        /// TODO document me!
        template<typename T, typename Void BOOST_PROTO_WHEN_BUILDING_DOCS(= void)>
        struct is_transform
          : mpl::false_
        {};

        template<typename T>
        struct is_transform<T, typename T::proto_is_transform_>
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
            /// <tt>BOOST_PROTO_BASIC_EXTENDS()</tt> macro.) Otherwise,
            /// <tt>is_expr\<T\>::::value</tt> is \c false.
            template<typename T, typename Void  BOOST_PROTO_WHEN_BUILDING_DOCS(= void)>
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
            /// <tt>BOOST_PROTO_BASIC_EXTENDS()</tt> macro.) Otherwise,
            /// <tt>is_expr\<T\>::::value</tt> is \c false.
            template<typename T>
            struct is_expr<T, typename T::proto_is_expr_>
              : mpl::true_
            {};
            
            template<typename T>
            struct is_expr<T &, void>
              : is_expr<T>
            {};

            /// \brief A metafunction that returns the tag type of a
            /// Proto expression.
            template<typename Expr>
            struct tag_of
            {
                typedef typename Expr::proto_tag type;
            };

            template<typename Expr>
            struct tag_of<Expr &>
            {
                typedef typename Expr::proto_tag type;
            };

            /// \brief A metafunction that returns the arity of a
            /// Proto expression.
            template<typename Expr>
            struct arity_of
              : Expr::proto_arity
            {};

            template<typename Expr>
            struct arity_of<Expr &>
              : Expr::proto_arity
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
            /// If \c T is a function type, let \c A be <tt>T &</tt>.
            /// Otherwise, let \c A be the type \c T stripped of cv-qualifiers.
            /// Then, the result type <tt>as_expr\<T, Domain\>::::type</tt> is
            /// <tt>boost::result_of\<Domain(expr\< tag::terminal, term\<A\> \>)\>::::type</tt>.
            template<
                typename T
              , typename Domain BOOST_PROTO_WHEN_BUILDING_DOCS(= default_domain)
              , typename Void   BOOST_PROTO_WHEN_BUILDING_DOCS(= void)
              #ifdef BOOST_PROTO_BROKEN_PTS
              , typename Void2  BOOST_PROTO_WHEN_BUILDING_DOCS(= void)
              #endif
            >
            struct as_expr
            {
                typedef
                    typename mpl::eval_if_c<
                        is_function<T>::value
                      , add_reference<T>
                      , remove_cv<T>
                    >::type
                arg0_;
                typedef proto::expr<proto::tag::terminal, term<arg0_> > expr_;
                typedef typename Domain::template result<void(expr_)>::type type;
                typedef type const reference;

                /// INTERNAL ONLY
                ///
                template<typename T2>
                static reference call(T2 &t)
                {
                    return Domain()(expr_::make(t));
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
            struct as_expr<
                T
              , Domain
              , typename T::proto_is_expr_
              #ifdef BOOST_PROTO_BROKEN_PTS
              , typename disable_if<is_same<Domain, typename T::proto_domain> >::type
              #endif
            >
            {
                typedef typename T::proto_derived_expr expr_; // removes the const
                typedef typename Domain::template result<void(expr_)>::type type;
                typedef type const reference;

                /// INTERNAL ONLY
                ///
                template<typename T2>
                static reference call(T2 &t)
                {
                    return Domain()(t);
                }
            };

            template<typename T>
            struct as_expr<
                T
              , typename T::proto_domain
              , typename T::proto_is_expr_
              #ifdef BOOST_PROTO_BROKEN_PTS
              , void
              #endif
            >
            {
                typedef typename T::proto_derived_expr type; // removes the const
                typedef T &reference;

                /// INTERNAL ONLY
                ///
                template<typename T2>
                static T2 &call(T2 &t)
                {
                    return t;
                }
            };

            /// \brief A metafunction that computes the return type of the \c as_child()
            /// function.
            ///
            /// The <tt>as_child\<\></tt> metafunction turns types into Proto types, if
            /// they are not already, by making them Proto terminals held by reference.
            /// Types which are already Proto types are returned by reference.
            ///
            /// This specialization is selected when the type is not yet a Proto type.
            /// The result type <tt>as_child\<T, Domain\>::::type</tt> is
            /// <tt>boost::result_of\<Domain(expr\< tag::terminal, term\<T &\> \>)\>::::type</tt>.
            template<
                typename T
              , typename Domain BOOST_PROTO_WHEN_BUILDING_DOCS(= default_domain)
              , typename Void   BOOST_PROTO_WHEN_BUILDING_DOCS(= void)
              #ifdef BOOST_PROTO_BROKEN_PTS
              , typename Void2  BOOST_PROTO_WHEN_BUILDING_DOCS(= void)
              #endif
            >
            struct as_child
            {
                typedef proto::expr<proto::tag::terminal, term<T &> > expr_;
                typedef typename Domain::template result<void(expr_)>::type type;

                /// INTERNAL ONLY
                ///
                template<typename T2>
                static type call(T2 &t)
                {
                    return Domain()(expr_::make(t));
                }
            };

            /// \brief A metafunction that computes the return type of the \c as_child()
            /// function.
            ///
            /// The <tt>as_child\<\></tt> metafunction turns types into Proto types, if
            /// they are not already, by making them Proto terminals held by reference.
            /// Types which are already Proto types are returned by reference.
            ///
            /// This specialization is selected when the type is already a Proto type.
            /// The result type <tt>as_child\<T, Domain\>::::type</tt> is
            /// <tt>T &</tt>.
            template<typename T, typename Domain>
            struct as_child<
                T
              , Domain
              , typename T::proto_is_expr_
              #ifdef BOOST_PROTO_BROKEN_PTS
              , typename disable_if<is_same<Domain, typename T::proto_domain> >::type
              #endif
            >
            {
                // BUGBUG should be able to hold this guy by reference, no?
                typedef typename Domain::template result<void(T)>::type type;

                /// INTERNAL ONLY
                ///
                template<typename T2>
                static type call(T2 &t)
                {
                    return Domain()(t);
                }
            };

            /// \brief A metafunction that computes the return type of the \c as_child()
            /// function.
            ///
            /// The <tt>as_child\<\></tt> metafunction turns types into Proto types, if
            /// they are not already, by making them Proto terminals held by reference.
            /// Types which are already Proto types are returned by reference.
            ///
            /// This specialization is selected when the type is already a Proto type.
            /// The result type <tt>as_child\<T, Domain\>::::type</tt> is
            /// <tt>T &</tt>.
            template<typename T>
            struct as_child<
                T
              , typename T::proto_domain
              , typename T::proto_is_expr_
              #ifdef BOOST_PROTO_BROKEN_PTS
              , void
              #endif
            >
            {
                typedef T &type;

                /// INTERNAL ONLY
                ///
                template<typename T2>
                static T2 &call(T2 &t)
                {
                    return t;
                }
            };

            /// \brief A metafunction that returns the type of the Nth child
            /// of a Proto expression, where N is an MPL Integral Constant.
            ///
            /// <tt>result_of::child\<Expr, N\></tt> is equivalent to
            /// <tt>result_of::child_c\<Expr, N::value\></tt>.
            template<typename Expr, typename N  BOOST_PROTO_WHEN_BUILDING_DOCS(= mpl::long_<0>) >
            struct child
              : child_c<Expr, N::value>
            {};

            /// \brief A metafunction that returns the type of the value
            /// of a terminal Proto expression.
            ///
            template<typename Expr>
            struct value
            {
                /// The raw type of the Nth child as it is stored within
                /// \c Expr. This may be a value or a reference
                typedef typename Expr::proto_child0 value_type;

                /// The "value" type of the child, suitable for return by value,
                /// computed as follows:
                /// \li <tt>T const(&)[N]</tt> becomes <tt>T const(&)[N]</tt>
                /// \li <tt>T[N]</tt> becomes <tt>T(&)[N]</tt>
                /// \li <tt>T(&)[N]</tt> becomes <tt>T(&)[N]</tt>
                /// \li <tt>R(&)(A0,...)</tt> becomes <tt>R(&)(A0,...)</tt>
                /// \li <tt>T const &</tt> becomes <tt>T</tt>
                /// \li <tt>T &</tt> becomes <tt>T</tt>
                /// \li <tt>T</tt> becomes <tt>T</tt>
                typedef typename Expr::proto_child_ref0::value_type type;
            };

            template<typename Expr>
            struct value<Expr &>
            {
                /// The raw type of the Nth child as it is stored within
                /// \c Expr. This may be a value or a reference
                typedef typename Expr::proto_child0 value_type;

                /// The "reference" type of the child, suitable for return by
                /// reference, computed as follows:
                /// \li <tt>T const(&)[N]</tt> becomes <tt>T const(&)[N]</tt>
                /// \li <tt>T[N]</tt> becomes <tt>T(&)[N]</tt>
                /// \li <tt>T(&)[N]</tt> becomes <tt>T(&)[N]</tt>
                /// \li <tt>R(&)(A0,...)</tt> becomes <tt>R(&)(A0,...)</tt>
                /// \li <tt>T const &</tt> becomes <tt>T const &</tt>
                /// \li <tt>T &</tt> becomes <tt>T &</tt>
                /// \li <tt>T</tt> becomes <tt>T &</tt>
                typedef typename Expr::proto_child_ref0::reference type;
            };

            template<typename Expr>
            struct value<Expr const &>
            {
                /// The raw type of the Nth child as it is stored within
                /// \c Expr. This may be a value or a reference
                typedef typename Expr::proto_child0 value_type;

                /// The "const reference" type of the child, suitable for return by
                /// const reference, computed as follows:
                /// \li <tt>T const(&)[N]</tt> becomes <tt>T const(&)[N]</tt>
                /// \li <tt>T[N]</tt> becomes <tt>T const(&)[N]</tt>
                /// \li <tt>T(&)[N]</tt> becomes <tt>T(&)[N]</tt>
                /// \li <tt>R(&)(A0,...)</tt> becomes <tt>R(&)(A0,...)</tt>
                /// \li <tt>T const &</tt> becomes <tt>T const &</tt>
                /// \li <tt>T &</tt> becomes <tt>T &</tt>
                /// \li <tt>T</tt> becomes <tt>T const &</tt>
                typedef typename Expr::proto_child_ref0::const_reference type;
            };

            // TODO left<> and right<> force the instantiation of Expr.
            // Couldn't we partially specialize them on proto::expr< T, A >
            // and return A::child0 / A::child1?

            /// \brief A metafunction that returns the type of the left child
            /// of a binary Proto expression.
            ///
            /// <tt>result_of::left\<Expr\></tt> is equivalent to
            /// <tt>result_of::child_c\<Expr, 0\></tt>.
            template<typename Expr>
            struct left
              : child_c<Expr, 0>
            {};

            /// \brief A metafunction that returns the type of the right child
            /// of a binary Proto expression.
            ///
            /// <tt>result_of::right\<Expr\></tt> is equivalent to
            /// <tt>result_of::child_c\<Expr, 1\></tt>.
            template<typename Expr>
            struct right
              : child_c<Expr, 1>
            {};

        } // namespace result_of

        namespace op
        {
            /// \brief A metafunction for generating terminal expression types,
            /// a grammar element for matching terminal expressions, and a
            /// PrimitiveTransform that returns the current expression unchanged.
            template<typename T>
            struct terminal : transform<terminal<T>, empty_base>
            {
                typedef proto::expr<proto::tag::terminal, term<T> > type;
                typedef type proto_base_expr;

                template<typename Expr, typename State, typename Data>
                struct impl : transform_impl<Expr, State, Data>
                {
                    typedef Expr result_type;

                    /// \param expr The current expression
                    /// \pre <tt>matches\<Expr, terminal\<T\> \>::::value</tt> is \c true.
                    /// \return \c expr
                    /// \throw nothrow
                    typename impl::expr_param operator ()(
                        typename impl::expr_param expr
                      , typename impl::state_param
                      , typename impl::data_param
                    ) const
                    {
                        return expr;
                    }
                };

                /// INTERNAL ONLY
                typedef proto::tag::terminal proto_tag;
                /// INTERNAL ONLY
                typedef T proto_child0;
            };

            /// \brief A metafunction for generating ternary conditional expression types,
            /// a grammar element for matching ternary conditional expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U, typename V>
            struct if_else_ : transform<if_else_<T, U, V>, empty_base>
            {
                typedef proto::expr<proto::tag::if_else_, list3<T, U, V> > type;
                typedef type proto_base_expr;

                template<typename Expr, typename State, typename Data>
                struct impl
                  : pass_through<if_else_>::template impl<Expr, State, Data>
                {};

                /// INTERNAL ONLY
                typedef proto::tag::if_else_ proto_tag;
                /// INTERNAL ONLY
                typedef T proto_child0;
                /// INTERNAL ONLY
                typedef U proto_child1;
                /// INTERNAL ONLY
                typedef V proto_child2;
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
            struct unary_expr : transform<unary_expr<Tag, T>, empty_base>
            {
                typedef proto::expr<Tag, list1<T> > type;
                typedef type proto_base_expr;

                template<typename Expr, typename State, typename Data>
                struct impl
                  : pass_through<unary_expr>::template impl<Expr, State, Data>
                {};

                /// INTERNAL ONLY
                typedef Tag proto_tag;
                /// INTERNAL ONLY
                typedef T proto_child0;
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
            struct binary_expr : transform<binary_expr<Tag, T, U>, empty_base>
            {
                typedef proto::expr<Tag, list2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Expr, typename State, typename Data>
                struct impl
                  : pass_through<binary_expr>::template impl<Expr, State, Data>
                {};

                /// INTERNAL ONLY
                typedef Tag proto_tag;
                /// INTERNAL ONLY
                typedef T proto_child0;
                /// INTERNAL ONLY
                typedef U proto_child1;
            };

            /// \brief A metafunction for generating unary plus expression types,
            /// a grammar element for matching unary plus expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T>
            struct unary_plus : transform<unary_plus<T>, empty_base>
            {
                typedef proto::expr<proto::tag::unary_plus, list1<T> > type;
                typedef type proto_base_expr;

                template<typename Expr, typename State, typename Data>
                struct impl
                  : pass_through<unary_plus>::template impl<Expr, State, Data>
                {};

                /// INTERNAL ONLY
                typedef proto::tag::unary_plus proto_tag;
                /// INTERNAL ONLY
                typedef T proto_child0;
            };

            /// \brief A metafunction for generating unary minus expression types,
            /// a grammar element for matching unary minus expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T>
            struct negate : transform<negate<T>, empty_base>
            {
                typedef proto::expr<proto::tag::negate, list1<T> > type;
                typedef type proto_base_expr;

                template<typename Expr, typename State, typename Data>
                struct impl
                  : pass_through<negate>::template impl<Expr, State, Data>
                {};

                /// INTERNAL ONLY
                typedef proto::tag::negate proto_tag;
                /// INTERNAL ONLY
                typedef T proto_child0;
            };

            /// \brief A metafunction for generating defereference expression types,
            /// a grammar element for matching dereference expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T>
            struct dereference : transform<dereference<T>, empty_base>
            {
                typedef proto::expr<proto::tag::dereference, list1<T> > type;
                typedef type proto_base_expr;

                template<typename Expr, typename State, typename Data>
                struct impl
                  : pass_through<dereference>::template impl<Expr, State, Data>
                {};

                /// INTERNAL ONLY
                typedef proto::tag::dereference proto_tag;
                /// INTERNAL ONLY
                typedef T proto_child0;
            };

            /// \brief A metafunction for generating complement expression types,
            /// a grammar element for matching complement expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T>
            struct complement : transform<complement<T>, empty_base>
            {
                typedef proto::expr<proto::tag::complement, list1<T> > type;
                typedef type proto_base_expr;

                template<typename Expr, typename State, typename Data>
                struct impl
                  : pass_through<complement>::template impl<Expr, State, Data>
                {};

                /// INTERNAL ONLY
                typedef proto::tag::complement proto_tag;
                /// INTERNAL ONLY
                typedef T proto_child0;
            };

            /// \brief A metafunction for generating address_of expression types,
            /// a grammar element for matching address_of expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T>
            struct address_of : transform<address_of<T>, empty_base>
            {
                typedef proto::expr<proto::tag::address_of, list1<T> > type;
                typedef type proto_base_expr;

                template<typename Expr, typename State, typename Data>
                struct impl
                  : pass_through<address_of>::template impl<Expr, State, Data>
                {};

                /// INTERNAL ONLY
                typedef proto::tag::address_of proto_tag;
                /// INTERNAL ONLY
                typedef T proto_child0;
            };

            /// \brief A metafunction for generating logical_not expression types,
            /// a grammar element for matching logical_not expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T>
            struct logical_not : transform<logical_not<T>, empty_base>
            {
                typedef proto::expr<proto::tag::logical_not, list1<T> > type;
                typedef type proto_base_expr;

                template<typename Expr, typename State, typename Data>
                struct impl
                  : pass_through<logical_not>::template impl<Expr, State, Data>
                {};

                /// INTERNAL ONLY
                typedef proto::tag::logical_not proto_tag;
                /// INTERNAL ONLY
                typedef T proto_child0;
            };

            /// \brief A metafunction for generating pre-increment expression types,
            /// a grammar element for matching pre-increment expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T>
            struct pre_inc : transform<pre_inc<T>, empty_base>
            {
                typedef proto::expr<proto::tag::pre_inc, list1<T> > type;
                typedef type proto_base_expr;

                template<typename Expr, typename State, typename Data>
                struct impl
                  : pass_through<pre_inc>::template impl<Expr, State, Data>
                {};

                /// INTERNAL ONLY
                typedef proto::tag::pre_inc proto_tag;
                /// INTERNAL ONLY
                typedef T proto_child0;
            };

            /// \brief A metafunction for generating pre-decrement expression types,
            /// a grammar element for matching pre-decrement expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T>
            struct pre_dec : transform<pre_dec<T>, empty_base>
            {
                typedef proto::expr<proto::tag::pre_dec, list1<T> > type;
                typedef type proto_base_expr;

                template<typename Expr, typename State, typename Data>
                struct impl
                  : pass_through<pre_dec>::template impl<Expr, State, Data>
                {};

                /// INTERNAL ONLY
                typedef proto::tag::pre_dec proto_tag;
                /// INTERNAL ONLY
                typedef T proto_child0;
            };

            /// \brief A metafunction for generating post-increment expression types,
            /// a grammar element for matching post-increment expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T>
            struct post_inc : transform<post_inc<T>, empty_base>
            {
                typedef proto::expr<proto::tag::post_inc, list1<T> > type;
                typedef type proto_base_expr;

                template<typename Expr, typename State, typename Data>
                struct impl
                  : pass_through<post_inc>::template impl<Expr, State, Data>
                {};

                /// INTERNAL ONLY
                typedef proto::tag::post_inc proto_tag;
                /// INTERNAL ONLY
                typedef T proto_child0;
            };

            /// \brief A metafunction for generating post-decrement expression types,
            /// a grammar element for matching post-decrement expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T>
            struct post_dec : transform<post_dec<T>, empty_base>
            {
                typedef proto::expr<proto::tag::post_dec, list1<T> > type;
                typedef type proto_base_expr;

                template<typename Expr, typename State, typename Data>
                struct impl
                  : pass_through<post_dec>::template impl<Expr, State, Data>
                {};

                /// INTERNAL ONLY
                typedef proto::tag::post_dec proto_tag;
                /// INTERNAL ONLY
                typedef T proto_child0;
            };

            /// \brief A metafunction for generating left-shift expression types,
            /// a grammar element for matching left-shift expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U>
            struct shift_left : transform<shift_left<T, U>, empty_base>
            {
                typedef proto::expr<proto::tag::shift_left, list2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Expr, typename State, typename Data>
                struct impl
                  : pass_through<shift_left>::template impl<Expr, State, Data>
                {};

                /// INTERNAL ONLY
                typedef proto::tag::shift_left proto_tag;
                /// INTERNAL ONLY
                typedef T proto_child0;
                /// INTERNAL ONLY
                typedef U proto_child1;
            };

            /// \brief A metafunction for generating right-shift expression types,
            /// a grammar element for matching right-shift expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U>
            struct shift_right : transform<shift_right<T, U>, empty_base>
            {
                typedef proto::expr<proto::tag::shift_right, list2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Expr, typename State, typename Data>
                struct impl
                  : pass_through<shift_right>::template impl<Expr, State, Data>
                {};

                /// INTERNAL ONLY
                typedef proto::tag::shift_right proto_tag;
                /// INTERNAL ONLY
                typedef T proto_child0;
                /// INTERNAL ONLY
                typedef U proto_child1;
            };

            /// \brief A metafunction for generating multiplies expression types,
            /// a grammar element for matching multiplies expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U>
            struct multiplies : transform<multiplies<T, U>, empty_base>
            {
                typedef proto::expr<proto::tag::multiplies, list2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Expr, typename State, typename Data>
                struct impl
                  : pass_through<multiplies>::template impl<Expr, State, Data>
                {};

                /// INTERNAL ONLY
                typedef proto::tag::multiplies proto_tag;
                /// INTERNAL ONLY
                typedef T proto_child0;
                /// INTERNAL ONLY
                typedef U proto_child1;
            };

            /// \brief A metafunction for generating divides expression types,
            /// a grammar element for matching divides expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U>
            struct divides : transform<divides<T, U>, empty_base>
            {
                typedef proto::expr<proto::tag::divides, list2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Expr, typename State, typename Data>
                struct impl
                  : pass_through<divides>::template impl<Expr, State, Data>
                {};

                /// INTERNAL ONLY
                typedef proto::tag::divides proto_tag;
                /// INTERNAL ONLY
                typedef T proto_child0;
                /// INTERNAL ONLY
                typedef U proto_child1;
            };

            /// \brief A metafunction for generating modulus expression types,
            /// a grammar element for matching modulus expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U>
            struct modulus : transform<modulus<T, U>, empty_base>
            {
                typedef proto::expr<proto::tag::modulus, list2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Expr, typename State, typename Data>
                struct impl
                  : pass_through<modulus>::template impl<Expr, State, Data>
                {};

                /// INTERNAL ONLY
                typedef proto::tag::modulus proto_tag;
                /// INTERNAL ONLY
                typedef T proto_child0;
                /// INTERNAL ONLY
                typedef U proto_child1;
            };

            /// \brief A metafunction for generating binary plus expression types,
            /// a grammar element for matching binary plus expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U>
            struct plus : transform<plus<T, U>, empty_base>
            {
                typedef proto::expr<proto::tag::plus, list2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Expr, typename State, typename Data>
                struct impl
                  : pass_through<plus>::template impl<Expr, State, Data>
                {};

                /// INTERNAL ONLY
                typedef proto::tag::plus proto_tag;
                /// INTERNAL ONLY
                typedef T proto_child0;
                /// INTERNAL ONLY
                typedef U proto_child1;
            };

            /// \brief A metafunction for generating binary minus expression types,
            /// a grammar element for matching binary minus expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U>
            struct minus : transform<minus<T, U>, empty_base>
            {
                typedef proto::expr<proto::tag::minus, list2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Expr, typename State, typename Data>
                struct impl
                  : pass_through<minus>::template impl<Expr, State, Data>
                {};

                /// INTERNAL ONLY
                typedef proto::tag::minus proto_tag;
                /// INTERNAL ONLY
                typedef T proto_child0;
                /// INTERNAL ONLY
                typedef U proto_child1;
            };

            /// \brief A metafunction for generating less expression types,
            /// a grammar element for matching less expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U>
            struct less : transform<less<T, U>, empty_base>
            {
                typedef proto::expr<proto::tag::less, list2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Expr, typename State, typename Data>
                struct impl
                  : pass_through<less>::template impl<Expr, State, Data>
                {};

                /// INTERNAL ONLY
                typedef proto::tag::less proto_tag;
                /// INTERNAL ONLY
                typedef T proto_child0;
                /// INTERNAL ONLY
                typedef U proto_child1;
            };

            /// \brief A metafunction for generating greater expression types,
            /// a grammar element for matching greater expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U>
            struct greater : transform<greater<T, U>, empty_base>
            {
                typedef proto::expr<proto::tag::greater, list2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Expr, typename State, typename Data>
                struct impl
                  : pass_through<greater>::template impl<Expr, State, Data>
                {};

                /// INTERNAL ONLY
                typedef proto::tag::greater proto_tag;
                /// INTERNAL ONLY
                typedef T proto_child0;
                /// INTERNAL ONLY
                typedef U proto_child1;
            };

            /// \brief A metafunction for generating less-or-equal expression types,
            /// a grammar element for matching less-or-equal expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U>
            struct less_equal : transform<less_equal<T, U>, empty_base>
            {
                typedef proto::expr<proto::tag::less_equal, list2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Expr, typename State, typename Data>
                struct impl
                  : pass_through<less_equal>::template impl<Expr, State, Data>
                {};

                /// INTERNAL ONLY
                typedef proto::tag::less_equal proto_tag;
                /// INTERNAL ONLY
                typedef T proto_child0;
                /// INTERNAL ONLY
                typedef U proto_child1;
            };

            /// \brief A metafunction for generating greater-or-equal expression types,
            /// a grammar element for matching greater-or-equal expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U>
            struct greater_equal : transform<greater_equal<T, U>, empty_base>
            {
                typedef proto::expr<proto::tag::greater_equal, list2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Expr, typename State, typename Data>
                struct impl
                  : pass_through<greater_equal>::template impl<Expr, State, Data>
                {};

                /// INTERNAL ONLY
                typedef proto::tag::greater_equal proto_tag;
                /// INTERNAL ONLY
                typedef T proto_child0;
                /// INTERNAL ONLY
                typedef U proto_child1;
            };

            /// \brief A metafunction for generating equal-to expression types,
            /// a grammar element for matching equal-to expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U>
            struct equal_to : transform<equal_to<T, U>, empty_base>
            {
                typedef proto::expr<proto::tag::equal_to, list2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Expr, typename State, typename Data>
                struct impl
                  : pass_through<equal_to>::template impl<Expr, State, Data>
                {};

                /// INTERNAL ONLY
                typedef proto::tag::equal_to proto_tag;
                /// INTERNAL ONLY
                typedef T proto_child0;
                /// INTERNAL ONLY
                typedef U proto_child1;
            };

            /// \brief A metafunction for generating not-equal-to expression types,
            /// a grammar element for matching not-equal-to expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U>
            struct not_equal_to : transform<not_equal_to<T, U>, empty_base>
            {
                typedef proto::expr<proto::tag::not_equal_to, list2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Expr, typename State, typename Data>
                struct impl
                  : pass_through<not_equal_to>::template impl<Expr, State, Data>
                {};

                /// INTERNAL ONLY
                typedef proto::tag::not_equal_to proto_tag;
                /// INTERNAL ONLY
                typedef T proto_child0;
                /// INTERNAL ONLY
                typedef U proto_child1;
            };

            /// \brief A metafunction for generating logical-or expression types,
            /// a grammar element for matching logical-or expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U>
            struct logical_or : transform<logical_or<T, U>, empty_base>
            {
                typedef proto::expr<proto::tag::logical_or, list2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Expr, typename State, typename Data>
                struct impl
                  : pass_through<logical_or>::template impl<Expr, State, Data>
                {};

                /// INTERNAL ONLY
                typedef proto::tag::logical_or proto_tag;
                /// INTERNAL ONLY
                typedef T proto_child0;
                /// INTERNAL ONLY
                typedef U proto_child1;
            };

            /// \brief A metafunction for generating logical-and expression types,
            /// a grammar element for matching logical-and expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U>
            struct logical_and : transform<logical_and<T, U>, empty_base>
            {
                typedef proto::expr<proto::tag::logical_and, list2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Expr, typename State, typename Data>
                struct impl
                  : pass_through<logical_and>::template impl<Expr, State, Data>
                {};

                /// INTERNAL ONLY
                typedef proto::tag::logical_and proto_tag;
                /// INTERNAL ONLY
                typedef T proto_child0;
                /// INTERNAL ONLY
                typedef U proto_child1;
            };

            /// \brief A metafunction for generating bitwise-and expression types,
            /// a grammar element for matching bitwise-and expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U>
            struct bitwise_and : transform<bitwise_and<T, U>, empty_base>
            {
                typedef proto::expr<proto::tag::bitwise_and, list2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Expr, typename State, typename Data>
                struct impl
                  : pass_through<bitwise_and>::template impl<Expr, State, Data>
                {};

                /// INTERNAL ONLY
                typedef proto::tag::bitwise_and proto_tag;
                /// INTERNAL ONLY
                typedef T proto_child0;
                /// INTERNAL ONLY
                typedef U proto_child1;
            };

            /// \brief A metafunction for generating bitwise-or expression types,
            /// a grammar element for matching bitwise-or expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U>
            struct bitwise_or : transform<bitwise_or<T, U>, empty_base>
            {
                typedef proto::expr<proto::tag::bitwise_or, list2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Expr, typename State, typename Data>
                struct impl
                  : pass_through<bitwise_or>::template impl<Expr, State, Data>
                {};

                /// INTERNAL ONLY
                typedef proto::tag::bitwise_or proto_tag;
                /// INTERNAL ONLY
                typedef T proto_child0;
                /// INTERNAL ONLY
                typedef U proto_child1;
            };

            /// \brief A metafunction for generating bitwise-xor expression types,
            /// a grammar element for matching bitwise-xor expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U>
            struct bitwise_xor : transform<bitwise_xor<T, U>, empty_base>
            {
                typedef proto::expr<proto::tag::bitwise_xor, list2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Expr, typename State, typename Data>
                struct impl
                  : pass_through<bitwise_xor>::template impl<Expr, State, Data>
                {};

                /// INTERNAL ONLY
                typedef proto::tag::bitwise_xor proto_tag;
                /// INTERNAL ONLY
                typedef T proto_child0;
                /// INTERNAL ONLY
                typedef U proto_child1;
            };

            /// \brief A metafunction for generating comma expression types,
            /// a grammar element for matching comma expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U>
            struct comma : transform<comma<T, U>, empty_base>
            {
                typedef proto::expr<proto::tag::comma, list2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Expr, typename State, typename Data>
                struct impl
                  : pass_through<comma>::template impl<Expr, State, Data>
                {};

                /// INTERNAL ONLY
                typedef proto::tag::comma proto_tag;
                /// INTERNAL ONLY
                typedef T proto_child0;
                /// INTERNAL ONLY
                typedef U proto_child1;
            };

            template<typename T, typename U>
            struct mem_ptr : transform<mem_ptr<T, U>, empty_base>
            {
                typedef proto::expr<proto::tag::mem_ptr, list2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Expr, typename State, typename Data>
                struct impl
                  : pass_through<mem_ptr>::template impl<Expr, State, Data>
                {};

                /// INTERNAL ONLY
                typedef proto::tag::mem_ptr proto_tag;
                /// INTERNAL ONLY
                typedef T proto_child0;
                /// INTERNAL ONLY
                typedef U proto_child1;
            };

            /// \brief A metafunction for generating assignment expression types,
            /// a grammar element for matching assignment expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U>
            struct assign : transform<assign<T, U>, empty_base>
            {
                typedef proto::expr<proto::tag::assign, list2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Expr, typename State, typename Data>
                struct impl
                  : pass_through<assign>::template impl<Expr, State, Data>
                {};

                /// INTERNAL ONLY
                typedef proto::tag::assign proto_tag;
                /// INTERNAL ONLY
                typedef T proto_child0;
                /// INTERNAL ONLY
                typedef U proto_child1;
            };

            /// \brief A metafunction for generating left-shift-assign expression types,
            /// a grammar element for matching left-shift-assign expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U>
            struct shift_left_assign : transform<shift_left_assign<T, U>, empty_base>
            {
                typedef proto::expr<proto::tag::shift_left_assign, list2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Expr, typename State, typename Data>
                struct impl
                  : pass_through<shift_left_assign>::template impl<Expr, State, Data>
                {};

                /// INTERNAL ONLY
                typedef proto::tag::shift_left_assign proto_tag;
                /// INTERNAL ONLY
                typedef T proto_child0;
                /// INTERNAL ONLY
                typedef U proto_child1;
            };

            /// \brief A metafunction for generating right-shift-assign expression types,
            /// a grammar element for matching right-shift-assign expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U>
            struct shift_right_assign : transform<shift_right_assign<T, U>, empty_base>
            {
                typedef proto::expr<proto::tag::shift_right_assign, list2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Expr, typename State, typename Data>
                struct impl
                  : pass_through<shift_right_assign>::template impl<Expr, State, Data>
                {};

                /// INTERNAL ONLY
                typedef proto::tag::shift_right_assign proto_tag;
                /// INTERNAL ONLY
                typedef T proto_child0;
                /// INTERNAL ONLY
                typedef U proto_child1;
            };

            /// \brief A metafunction for generating multiplies-assign expression types,
            /// a grammar element for matching multiplies-assign expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U>
            struct multiplies_assign : transform<multiplies_assign<T, U>, empty_base>
            {
                typedef proto::expr<proto::tag::multiplies_assign, list2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Expr, typename State, typename Data>
                struct impl
                  : pass_through<multiplies_assign>::template impl<Expr, State, Data>
                {};

                /// INTERNAL ONLY
                typedef proto::tag::multiplies_assign proto_tag;
                /// INTERNAL ONLY
                typedef T proto_child0;
                /// INTERNAL ONLY
                typedef U proto_child1;
            };

            /// \brief A metafunction for generating divides-assign expression types,
            /// a grammar element for matching divides-assign expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U>
            struct divides_assign : transform<divides_assign<T, U>, empty_base>
            {
                typedef proto::expr<proto::tag::divides_assign, list2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Expr, typename State, typename Data>
                struct impl
                  : pass_through<divides_assign>::template impl<Expr, State, Data>
                {};

                /// INTERNAL ONLY
                typedef proto::tag::divides_assign proto_tag;
                /// INTERNAL ONLY
                typedef T proto_child0;
                /// INTERNAL ONLY
                typedef U proto_child1;
            };

            /// \brief A metafunction for generating modulus-assign expression types,
            /// a grammar element for matching modulus-assign expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U>
            struct modulus_assign : transform<modulus_assign<T, U>, empty_base>
            {
                typedef proto::expr<proto::tag::modulus_assign, list2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Expr, typename State, typename Data>
                struct impl
                  : pass_through<modulus_assign>::template impl<Expr, State, Data>
                {};

                /// INTERNAL ONLY
                typedef proto::tag::modulus_assign proto_tag;
                /// INTERNAL ONLY
                typedef T proto_child0;
                /// INTERNAL ONLY
                typedef U proto_child1;
            };

            /// \brief A metafunction for generating plus-assign expression types,
            /// a grammar element for matching plus-assign expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U>
            struct plus_assign : transform<plus_assign<T, U>, empty_base>
            {
                typedef proto::expr<proto::tag::plus_assign, list2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Expr, typename State, typename Data>
                struct impl
                  : pass_through<plus_assign>::template impl<Expr, State, Data>
                {};

                /// INTERNAL ONLY
                typedef proto::tag::plus_assign proto_tag;
                /// INTERNAL ONLY
                typedef T proto_child0;
                /// INTERNAL ONLY
                typedef U proto_child1;
            };

            /// \brief A metafunction for generating minus-assign expression types,
            /// a grammar element for matching minus-assign expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U>
            struct minus_assign : transform<minus_assign<T, U>, empty_base>
            {
                typedef proto::expr<proto::tag::minus_assign, list2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Expr, typename State, typename Data>
                struct impl
                  : pass_through<minus_assign>::template impl<Expr, State, Data>
                {};

                /// INTERNAL ONLY
                typedef proto::tag::minus_assign proto_tag;
                /// INTERNAL ONLY
                typedef T proto_child0;
                /// INTERNAL ONLY
                typedef U proto_child1;
            };

            /// \brief A metafunction for generating bitwise-and-assign expression types,
            /// a grammar element for matching bitwise-and-assign expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U>
            struct bitwise_and_assign : transform<bitwise_and_assign<T, U>, empty_base>
            {
                typedef proto::expr<proto::tag::bitwise_and_assign, list2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Expr, typename State, typename Data>
                struct impl
                  : pass_through<bitwise_and_assign>::template impl<Expr, State, Data>
                {};

                /// INTERNAL ONLY
                typedef proto::tag::bitwise_and_assign proto_tag;
                /// INTERNAL ONLY
                typedef T proto_child0;
                /// INTERNAL ONLY
                typedef U proto_child1;
            };

            /// \brief A metafunction for generating bitwise-or-assign expression types,
            /// a grammar element for matching bitwise-or-assign expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U>
            struct bitwise_or_assign : transform<bitwise_or_assign<T, U>, empty_base>
            {
                typedef proto::expr<proto::tag::bitwise_or_assign, list2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Expr, typename State, typename Data>
                struct impl
                  : pass_through<bitwise_or_assign>::template impl<Expr, State, Data>
                {};

                /// INTERNAL ONLY
                typedef proto::tag::bitwise_or_assign proto_tag;
                /// INTERNAL ONLY
                typedef T proto_child0;
                /// INTERNAL ONLY
                typedef U proto_child1;
            };

            /// \brief A metafunction for generating bitwise-xor-assign expression types,
            /// a grammar element for matching bitwise-xor-assign expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U>
            struct bitwise_xor_assign : transform<bitwise_xor_assign<T, U>, empty_base>
            {
                typedef proto::expr<proto::tag::bitwise_xor_assign, list2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Expr, typename State, typename Data>
                struct impl
                  : pass_through<bitwise_xor_assign>::template impl<Expr, State, Data>
                {};

                /// INTERNAL ONLY
                typedef proto::tag::bitwise_xor_assign proto_tag;
                /// INTERNAL ONLY
                typedef T proto_child0;
                /// INTERNAL ONLY
                typedef U proto_child1;
            };

            /// \brief A metafunction for generating subscript expression types,
            /// a grammar element for matching subscript expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U>
            struct subscript : transform<subscript<T, U>, empty_base>
            {
                typedef proto::expr<proto::tag::subscript, list2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Expr, typename State, typename Data>
                struct impl
                  : pass_through<subscript>::template impl<Expr, State, Data>
                {};

                /// INTERNAL ONLY
                typedef proto::tag::subscript proto_tag;
                /// INTERNAL ONLY
                typedef T proto_child0;
                /// INTERNAL ONLY
                typedef U proto_child1;
            };

            /// \brief A metafunction for generating virtual data member expression
            /// types, a grammar element for matching member expressions, and a
            /// PrimitiveTransform that dispatches to the <tt>pass_through\<\></tt>
            /// transform.
            template<typename T, typename U>
            struct member : transform<member<T, U>, empty_base>
            {
                typedef expr<tag::member, list2<T, U> > type;
                typedef type proto_base_expr;

                template<typename Expr, typename State, typename Data>
                struct impl
                  : pass_through<member>::template impl<Expr, State, Data>
                {};

                /// INTERNAL ONLY
                typedef proto::tag::member proto_tag;
                /// INTERNAL ONLY
                typedef T proto_child0;
                /// INTERNAL ONLY
                typedef U proto_child1;
            };

        } // namespace op

    #define BOOST_PROTO_CHILD(Z, N, DATA)                                                           \
        /** INTERNAL ONLY */                                                                        \
        typedef BOOST_PP_CAT(DATA, N) BOOST_PP_CAT(proto_child, N);                                 \
        /**/

    #define BOOST_PP_ITERATION_PARAMS_1 (3, (0, BOOST_PROTO_MAX_ARITY, <boost/proto/traits.hpp>))
    #include BOOST_PP_ITERATE()

    #undef BOOST_PROTO_CHILD
    #undef BOOST_PROTO_IMPLICIT_ARG

        namespace functional
        {
            /// \brief A callable PolymorphicFunctionObject that is
            /// equivalent to the \c as_expr() function.
            template<typename Domain    BOOST_PROTO_WHEN_BUILDING_DOCS(= default_domain)>
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
                typename result_of::as_expr<T[N_], Domain>::reference
                operator ()(T (&t)[N_]) const
                {
                    return result_of::as_expr<T[N_], Domain>::call(t);
                }

                template<typename T, std::size_t N_>
                typename result_of::as_expr<T const[N_], Domain>::reference
                operator ()(T const (&t)[N_]) const
                {
                    return result_of::as_expr<T const[N_], Domain>::call(t);
                }
                #endif
            };

            /// \brief A callable PolymorphicFunctionObject that is
            /// equivalent to the \c as_child() function.
            template<typename Domain    BOOST_PROTO_WHEN_BUILDING_DOCS(= default_domain)>
            struct as_child
            {
                BOOST_PROTO_CALLABLE()

                template<typename Sig>
                struct result;

                template<typename This, typename T>
                struct result<This(T)>
                {
                    typedef typename remove_reference<T>::type unref_type;
                    typedef typename result_of::as_child<unref_type, Domain>::type type;
                };

                /// \brief Wrap an object in a Proto terminal if it isn't a
                /// Proto expression already.
                /// \param t The object to wrap.
                /// \return <tt>proto::as_child\<Domain\>(t)</tt>
                template<typename T>
                typename result_of::as_child<T, Domain>::type
                operator ()(T &t) const
                {
                    return result_of::as_child<T, Domain>::call(t);
                }

                /// \overload
                ///
                template<typename T>
                typename result_of::as_child<T const, Domain>::type
                operator ()(T const &t) const
                {
                    return result_of::as_child<T const, Domain>::call(t);
                }
            };

            /// \brief A callable PolymorphicFunctionObject that is
            /// equivalent to the \c child_c() function.
            template<long N>
            struct child_c
            {
                BOOST_PROTO_CALLABLE()

                template<typename Sig>
                struct result;

                template<typename This, typename Expr>
                struct result<This(Expr)>
                {
                    typedef typename result_of::child_c<Expr, N>::type type;
                };

                /// \brief Return the Nth child of the given expression.
                /// \param expr The expression node.
                /// \pre <tt>is_expr\<Expr\>::::value</tt> is \c true
                /// \pre <tt>N == 0 || N \< Expr::proto_arity::value</tt>
                /// \return <tt>proto::child_c\<N\>(expr)</tt>
                /// \throw nothrow
                template<typename Expr>
                typename result_of::child_c<Expr &, N>::type
                operator ()(Expr &expr) const
                {
                    return result_of::child_c<Expr &, N>::call(expr);
                }

                /// \overload
                ///
                template<typename Expr>
                typename result_of::child_c<Expr const &, N>::type
                operator ()(Expr const &expr) const
                {
                    return result_of::child_c<Expr const &, N>::call(expr);
                }
            };

            /// \brief A callable PolymorphicFunctionObject that is
            /// equivalent to the \c child() function.
            ///
            /// A callable PolymorphicFunctionObject that is
            /// equivalent to the \c child() function. \c N is required
            /// to be an MPL Integral Constant.
            template<typename N BOOST_PROTO_WHEN_BUILDING_DOCS(= mpl::long_<0>) >
            struct child
            {
                BOOST_PROTO_CALLABLE()

                template<typename Sig>
                struct result;

                template<typename This, typename Expr>
                struct result<This(Expr)>
                {
                    typedef typename result_of::child<Expr, N>::type type;
                };

                /// \brief Return the Nth child of the given expression.
                /// \param expr The expression node.
                /// \pre <tt>is_expr\<Expr\>::::value</tt> is \c true
                /// \pre <tt>N::value == 0 || N::value \< Expr::proto_arity::value</tt>
                /// \return <tt>proto::child\<N\>(expr)</tt>
                /// \throw nothrow
                template<typename Expr>
                typename result_of::child<Expr &, N>::type
                operator ()(Expr &expr) const
                {
                    return result_of::child<Expr &, N>::call(expr);
                }

                /// \overload
                ///
                template<typename Expr>
                typename result_of::child<Expr const &, N>::type
                operator ()(Expr const &expr) const
                {
                    return result_of::child<Expr const &, N>::call(expr);
                }
            };

            /// \brief A callable PolymorphicFunctionObject that is
            /// equivalent to the \c value() function.
            struct value
            {
                BOOST_PROTO_CALLABLE()

                template<typename Sig>
                struct result;

                template<typename This, typename Expr>
                struct result<This(Expr)>
                {
                    typedef typename result_of::value<Expr>::type type;
                };

                /// \brief Return the value of the given terminal expression.
                /// \param expr The terminal expression node.
                /// \pre <tt>is_expr\<Expr\>::::value</tt> is \c true
                /// \pre <tt>0 == Expr::proto_arity::value</tt>
                /// \pre <tt>Expr::proto_tag</tt> is <tt>tag::terminal</tt>
                /// \return <tt>proto::value(expr)</tt>
                /// \throw nothrow
                template<typename Expr>
                typename result_of::value<Expr &>::type
                operator ()(Expr &expr) const
                {
                    return expr.proto_base().child0;
                }

                /// \overload
                ///
                template<typename Expr>
                typename result_of::value<Expr const &>::type
                operator ()(Expr const &expr) const
                {
                    return expr.proto_base().child0;
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
                    typedef typename result_of::left<Expr>::type type;
                };

                /// \brief Return the left child of the given binary expression.
                /// \param expr The expression node.
                /// \pre <tt>is_expr\<Expr\>::::value</tt> is \c true
                /// \pre <tt>2 == Expr::proto_arity::value</tt>
                /// \return <tt>proto::left(expr)</tt>
                /// \throw nothrow
                template<typename Expr>
                typename result_of::left<Expr &>::type
                operator ()(Expr &expr) const
                {
                    return expr.proto_base().child0;
                }

                /// \overload
                ///
                template<typename Expr>
                typename result_of::left<Expr const &>::type
                operator ()(Expr const &expr) const
                {
                    return expr.proto_base().child0;
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
                    typedef typename result_of::right<Expr>::type type;
                };

                /// \brief Return the right child of the given binary expression.
                /// \param expr The expression node.
                /// \pre <tt>is_expr\<Expr\>::::value</tt> is \c true
                /// \pre <tt>2 == Expr::proto_arity::value</tt>
                /// \return <tt>proto::right(expr)</tt>
                /// \throw nothrow
                template<typename Expr>
                typename result_of::right<Expr &>::type
                operator ()(Expr &expr) const
                {
                    return expr.proto_base().child1;
                }

                template<typename Expr>
                typename result_of::right<Expr const &>::type
                operator ()(Expr const &expr) const
                {
                    return expr.proto_base().child1;
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
        /// If \c T is a function type, let \c A be <tt>T &</tt>. Otherwise, let
        /// \c A be the type \c T stripped of cv-qualifiers. Then, \c as_expr()
        /// returns <tt>Domain()(terminal\<A\>::::type::make(t))</tt>.
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
        /// terminals (by reference) and returns Proto expression types by
        /// reference
        ///
        /// The <tt>as_child()</tt> function turns objects into Proto terminals if
        /// they are not Proto expression types already. Non-Proto types are
        /// held by reference. Types which are already Proto types are simply
        /// returned as-is.
        ///
        /// This function can be called either with an explicitly specified
        /// \c Domain parameter (i.e., <tt>as_child\<Domain\>(t)</tt>), or
        /// without (i.e., <tt>as_child(t)</tt>). If no domain is
        /// specified, \c default_domain is assumed.
        ///
        /// If <tt>is_expr\<T\>::::value</tt> is \c true, then the argument is
        /// returned as-is. Otherwise, \c as_child() returns
        /// <tt>Domain()(terminal\<T &\>::::type::make(t))</tt>.
        ///
        /// \param t The object to wrap.
        template<typename T>
        typename result_of::as_child<T>::type
        as_child(T &t BOOST_PROTO_DISABLE_IF_IS_CONST(T))
        {
            return result_of::as_child<T>::call(t);
        }

        /// \overload
        ///
        template<typename T>
        typename result_of::as_child<T const>::type
        as_child(T const &t)
        {
            return result_of::as_child<T const>::call(t);
        }

        /// \overload
        ///
        template<typename Domain, typename T>
        typename result_of::as_child<T, Domain>::type
        as_child(T &t BOOST_PROTO_DISABLE_IF_IS_CONST(T))
        {
            return result_of::as_child<T, Domain>::call(t);
        }

        /// \overload
        ///
        template<typename Domain, typename T>
        typename result_of::as_child<T const, Domain>::type
        as_child(T const &t)
        {
            return result_of::as_child<T const, Domain>::call(t);
        }

        /// \brief Return the Nth child of the specified Proto expression.
        ///
        /// Return the Nth child of the specified Proto expression. If
        /// \c N is not specified, as in \c child(expr), then \c N is assumed
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
        typename result_of::child<Expr &, N>::type
        child(Expr &expr BOOST_PROTO_DISABLE_IF_IS_CONST(Expr))
        {
            return result_of::child<Expr &, N>::call(expr);
        }

        /// \overload
        ///
        template<typename N, typename Expr>
        typename result_of::child<Expr const &, N>::type
        child(Expr const &expr)
        {
            return result_of::child<Expr const &, N>::call(expr);
        }

        /// \overload
        ///
        template<typename Expr2>
        typename Expr2::proto_base_expr::proto_child_ref0::reference
        child(Expr2 &expr2 BOOST_PROTO_DISABLE_IF_IS_CONST(Expr2))
        {
            return expr2.proto_base().child0;
        }

        /// \overload
        ///
        template<typename Expr2>
        typename Expr2::proto_base_expr::proto_child_ref0::const_reference
        child(Expr2 const &expr2)
        {
            return expr2.proto_base().child0;
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
        typename result_of::child_c<Expr &, N>::type
        child_c(Expr &expr BOOST_PROTO_DISABLE_IF_IS_CONST(Expr))
        {
            return result_of::child_c<Expr &, N>::call(expr);
        }

        /// \overload
        ///
        template<long N, typename Expr>
        typename result_of::child_c<Expr const &, N>::type
        child_c(Expr const &expr)
        {
            return result_of::child_c<Expr const &, N>::call(expr);
        }

        /// \brief Return the value stored within the specified Proto
        /// terminal expression.
        ///
        /// Return the the value stored within the specified Proto
        /// terminal expression. The value is returned by
        /// reference.
        ///
        /// \param expr The Proto terminal expression.
        /// \pre \c Expr::proto_tag is \c tag::terminal.
        /// \pre <tt>N::value == 0</tt>
        /// \throw nothrow
        /// \return A reference to the terminal's value
        template<typename Expr>
        typename result_of::value<Expr &>::type
        value(Expr &expr BOOST_PROTO_DISABLE_IF_IS_CONST(Expr))
        {
            return expr.proto_base().child0;
        }

        /// \overload
        ///
        template<typename Expr>
        typename result_of::value<Expr const &>::type
        value(Expr const &expr)
        {
            return expr.proto_base().child0;
        }

        /// \brief Return the left child of the specified binary Proto
        /// expression.
        ///
        /// Return the left child of the specified binary Proto expression. The
        /// child is returned by reference.
        ///
        /// \param expr The Proto expression.
        /// \pre <tt>is_expr\<Expr\>::::value</tt> is \c true.
        /// \pre <tt>2 == Expr::proto_arity::value</tt>
        /// \throw nothrow
        /// \return A reference to the left child
        template<typename Expr>
        typename result_of::left<Expr &>::type
        left(Expr &expr BOOST_PROTO_DISABLE_IF_IS_CONST(Expr))
        {
            return expr.proto_base().child0;
        }

        /// \overload
        ///
        template<typename Expr>
        typename result_of::left<Expr const &>::type
        left(Expr const &expr)
        {
            return expr.proto_base().child0;
        }

        /// \brief Return the right child of the specified binary Proto
        /// expression.
        ///
        /// Return the right child of the specified binary Proto expression. The
        /// child is returned by reference.
        ///
        /// \param expr The Proto expression.
        /// \pre <tt>is_expr\<Expr\>::::value</tt> is \c true.
        /// \pre <tt>2 == Expr::proto_arity::value</tt>
        /// \throw nothrow
        /// \return A reference to the right child
        template<typename Expr>
        typename result_of::right<Expr &>::type
        right(Expr &expr BOOST_PROTO_DISABLE_IF_IS_CONST(Expr))
        {
            return expr.proto_base().child1;
        }

        /// \overload
        ///
        template<typename Expr>
        typename result_of::right<Expr const &>::type
        right(Expr const &expr)
        {
            return expr.proto_base().child1;
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
        struct is_callable<functional::as_child<Domain> >
          : mpl::true_
        {};

        /// INTERNAL ONLY
        ///
        template<long N>
        struct is_callable<functional::child_c<N> >
          : mpl::true_
        {};

        /// INTERNAL ONLY
        ///
        template<typename N>
        struct is_callable<functional::child<N> >
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
              : transform<
                    function<
                        BOOST_PP_ENUM_PARAMS(N, A)
                        BOOST_PP_ENUM_TRAILING_PARAMS(BOOST_PP_SUB(BOOST_PROTO_MAX_ARITY, N), void BOOST_PP_INTERCEPT), void
                    >
                  , empty_base
                >
            {
                typedef proto::expr<proto::tag::function, BOOST_PP_CAT(list, N)<BOOST_PP_ENUM_PARAMS(N, A)> > type;
                typedef type proto_base_expr;

                template<typename Expr, typename State, typename Data>
                struct impl
                  : pass_through<function>::template impl<Expr, State, Data>
                {};

                /// INTERNAL ONLY
                typedef proto::tag::function proto_tag;
                BOOST_PP_REPEAT(N, BOOST_PROTO_CHILD, A)
                BOOST_PP_REPEAT_FROM_TO(
                    N
                  , BOOST_PROTO_MAX_ARITY
                  , BOOST_PROTO_CHILD
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
              : transform<
                    nary_expr<
                        Tag
                        BOOST_PP_ENUM_TRAILING_PARAMS(N, A)
                        BOOST_PP_ENUM_TRAILING_PARAMS(BOOST_PP_SUB(BOOST_PROTO_MAX_ARITY, N), void BOOST_PP_INTERCEPT), void
                    >
                  , empty_base
                >
            {
                typedef proto::expr<Tag, BOOST_PP_CAT(list, N)<BOOST_PP_ENUM_PARAMS(N, A)> > type;
                typedef type proto_base_expr;

                template<typename Expr, typename State, typename Data>
                struct impl
                  : pass_through<nary_expr>::template impl<Expr, State, Data>
                {};

                /// INTERNAL ONLY
                typedef Tag proto_tag;
                BOOST_PP_REPEAT(N, BOOST_PROTO_CHILD, A)
                BOOST_PP_REPEAT_FROM_TO(
                    N
                  , BOOST_PROTO_MAX_ARITY
                  , BOOST_PROTO_CHILD
                  , detail::if_vararg<BOOST_PP_CAT(A, BOOST_PP_DEC(N))> BOOST_PP_INTERCEPT
                )
            };

        } // namespace op

        namespace detail
        {
            template<
                template<BOOST_PP_ENUM_PARAMS(N, typename BOOST_PP_INTERCEPT)> class T
              , BOOST_PP_ENUM_PARAMS(N, typename A)
            >
            struct is_callable_<T<BOOST_PP_ENUM_PARAMS(N, A)> BOOST_MPL_AUX_LAMBDA_ARITY_PARAM(N)>
              : is_same<BOOST_PP_CAT(A, BOOST_PP_DEC(N)), callable>
            {};
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
            struct child_c<Expr, N>
            {
                /// The raw type of the Nth child as it is stored within
                /// \c Expr. This may be a value or a reference
                typedef typename Expr::BOOST_PP_CAT(proto_child, N) value_type;

                /// The "value" type of the child, suitable for return by value,
                /// computed as follows:
                /// \li <tt>T const &</tt> becomes <tt>T</tt>
                /// \li <tt>T &</tt> becomes <tt>T</tt>
                /// \li <tt>T</tt> becomes <tt>T</tt>
                typedef typename Expr::BOOST_PP_CAT(proto_child_ref, N)::value_type type;
            };

            template<typename Expr>
            struct child_c<Expr &, N>
            {
                /// The raw type of the Nth child as it is stored within
                /// \c Expr. This may be a value or a reference
                typedef typename Expr::BOOST_PP_CAT(proto_child, N) value_type;

                /// The "reference" type of the child, suitable for return by
                /// reference, computed as follows:
                /// \li <tt>T const &</tt> becomes <tt>T const &</tt>
                /// \li <tt>T &</tt> becomes <tt>T &</tt>
                /// \li <tt>T</tt> becomes <tt>T &</tt>
                typedef typename Expr::BOOST_PP_CAT(proto_child_ref, N)::reference type;

                /// INTERNAL ONLY
                ///
                static type call(Expr &expr)
                {
                    return expr.proto_base().BOOST_PP_CAT(child, N);
                }
            };

            template<typename Expr>
            struct child_c<Expr const &, N>
            {
                /// The raw type of the Nth child as it is stored within
                /// \c Expr. This may be a value or a reference
                typedef typename Expr::BOOST_PP_CAT(proto_child, N) value_type;

                /// The "const reference" type of the child, suitable for return by
                /// const reference, computed as follows:
                /// \li <tt>T const &</tt> becomes <tt>T const &</tt>
                /// \li <tt>T &</tt> becomes <tt>T &</tt>
                /// \li <tt>T</tt> becomes <tt>T const &</tt>
                typedef typename Expr::BOOST_PP_CAT(proto_child_ref, N)::const_reference type;

                /// INTERNAL ONLY
                ///
                static type call(Expr const &expr)
                {
                    return expr.proto_base().BOOST_PP_CAT(child, N);
                }
            };
        }

    #undef N

#endif
