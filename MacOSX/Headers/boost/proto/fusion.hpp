///////////////////////////////////////////////////////////////////////////////
/// \file fusion.hpp
/// Make any Proto expression a valid Fusion sequence
//
//  Copyright 2008 Eric Niebler. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PROTO_FUSION_HPP_EAN_11_04_2006
#define BOOST_PROTO_FUSION_HPP_EAN_11_04_2006

#include <boost/proto/detail/prefix.hpp>
#include <boost/config.hpp>
#include <boost/version.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/long.hpp>
#if BOOST_VERSION >= 103500
#include <boost/fusion/include/is_view.hpp>
#include <boost/fusion/include/tag_of_fwd.hpp>
#include <boost/fusion/include/category_of.hpp>
#include <boost/fusion/include/iterator_base.hpp>
#include <boost/fusion/include/intrinsic.hpp>
#include <boost/fusion/include/pop_front.hpp>
#include <boost/fusion/include/reverse.hpp>
#include <boost/fusion/include/single_view.hpp>
#include <boost/fusion/include/transform_view.hpp>
#include <boost/fusion/support/ext_/is_segmented.hpp>
#include <boost/fusion/sequence/intrinsic/ext_/segments.hpp>
#include <boost/fusion/sequence/intrinsic/ext_/size_s.hpp>
#include <boost/fusion/view/ext_/segmented_iterator.hpp>
#else
#include <boost/spirit/fusion/sequence/is_sequence.hpp>
#include <boost/spirit/fusion/sequence/begin.hpp>
#include <boost/spirit/fusion/sequence/end.hpp>
#include <boost/spirit/fusion/sequence/at.hpp>
#include <boost/spirit/fusion/sequence/value_at.hpp>
#include <boost/spirit/fusion/sequence/single_view.hpp>
#include <boost/spirit/fusion/sequence/transform_view.hpp>
#include <boost/proto/detail/reverse.hpp>
#include <boost/proto/detail/pop_front.hpp>
#endif
#include <boost/proto/proto_fwd.hpp>
#include <boost/proto/traits.hpp>
#include <boost/proto/eval.hpp>
#include <boost/proto/detail/suffix.hpp>

#if BOOST_MSVC
#pragma warning(push)
#pragma warning(disable : 4510) // default constructor could not be generated
#pragma warning(disable : 4512) // assignment operator could not be generated
#pragma warning(disable : 4610) // can never be instantiated - user defined constructor required
#endif

namespace boost { namespace proto
{

/// INTERNAL ONLY
///
#define UNREF(x) typename boost::remove_reference<x>::type

    namespace detail
    {

        template<typename Expr, long Pos>
        struct expr_iterator
          : fusion::iterator_base<expr_iterator<Expr, Pos> >
        {
            typedef Expr expr_type;
            typedef typename Expr::proto_tag proto_tag;
            BOOST_STATIC_CONSTANT(long, index = Pos);
            BOOST_PROTO_FUSION_DEFINE_CATEGORY(fusion::random_access_traversal_tag)
            BOOST_PROTO_FUSION_DEFINE_TAG(tag::proto_expr_iterator)

            expr_iterator(Expr const &e)
              : expr(e)
            {}

            Expr const &expr;
        };

        template<typename Expr>
        struct flat_view
        {
            typedef Expr expr_type;
            typedef typename Expr::proto_tag proto_tag;
            BOOST_PROTO_FUSION_DEFINE_CATEGORY(fusion::forward_traversal_tag)
            BOOST_PROTO_FUSION_DEFINE_TAG(tag::proto_flat_view)

            explicit flat_view(Expr &expr)
              : expr_(expr)
            {}

            Expr &expr_;
        };

        template<typename Tag>
        struct as_element
        {
            template<typename Sig>
            struct result;

            template<typename This, typename Expr>
            struct result<This(Expr)>
              : mpl::if_<
                    is_same<Tag, UNREF(Expr)::proto_tag>
                  , flat_view<UNREF(Expr) const>
                  , fusion::single_view<UNREF(Expr) const &>
                >
            {};

            template<typename Expr>
            typename result<as_element(Expr const &)>::type
            operator ()(Expr const &expr) const
            {
                return typename result<as_element(Expr const &)>::type(expr);
            }
        };
    }

    namespace result_of
    {
        template<typename Expr>
        struct flatten
        {
            typedef detail::flat_view<Expr const> type;
        };

        template<typename Expr>
        struct flatten<Expr &>
        {
            typedef detail::flat_view<Expr const> type;
        };
    }

    namespace functional
    {
        /// \brief A PolymorphicFunctionObject type that returns a "flattened"
        /// view of a Proto expression tree.
        ///
        /// A PolymorphicFunctionObject type that returns a "flattened"
        /// view of a Proto expression tree. For a tree with a top-most node
        /// tag of type \c T, the elements of the flattened sequence are
        /// determined by recursing into each child node with the same
        /// tag type and returning those nodes of different type. So for
        /// instance, the Proto expression tree corresponding to the
        /// expression <tt>a | b | c</tt> has a flattened view with elements
        /// [a, b, c], even though the tree is grouped as
        /// <tt>((a | b) | c)</tt>.
        struct flatten
        {
            BOOST_PROTO_CALLABLE()

            template<typename Sig>
            struct result;

            template<typename This, typename Expr>
            struct result<This(Expr)>
            {
                typedef proto::detail::flat_view<UNREF(Expr) const> type;
            };

            template<typename Expr>
            proto::detail::flat_view<Expr const>
            operator ()(Expr const &expr) const
            {
                return proto::detail::flat_view<Expr const>(expr);
            }
        };

        /// \brief A PolymorphicFunctionObject type that invokes the
        /// \c fusion::pop_front() algorithm on its argument.
        ///
        /// A PolymorphicFunctionObject type that invokes the
        /// \c fusion::pop_front() algorithm on its argument. This is
        /// useful for defining a CallableTransform like \c pop_front(_)
        /// which removes the first child from a Proto expression node.
        /// Such a transform might be used as the first argument to the
        /// \c proto::fold\<\> transform; that is, fold all but
        /// the first child.
        struct pop_front
        {
            BOOST_PROTO_CALLABLE()

            template<typename Sig>
            struct result;

            template<typename This, typename Expr>
            struct result<This(Expr)>
            {
                typedef
                    typename fusion::BOOST_PROTO_FUSION_RESULT_OF::pop_front<UNREF(Expr) const>::type
                type;
            };

            template<typename Expr>
            typename fusion::BOOST_PROTO_FUSION_RESULT_OF::pop_front<Expr const>::type
            operator ()(Expr const &expr) const
            {
                return fusion::pop_front(expr);
            }
        };

        /// \brief A PolymorphicFunctionObject type that invokes the
        /// \c fusion::reverse() algorithm on its argument.
        ///
        /// A PolymorphicFunctionObject type that invokes the
        /// \c fusion::reverse() algorithm on its argument. This is
        /// useful for defining a CallableTransform like \c reverse(_)
        /// which reverses the order of the children of a Proto
        /// expression node.
        struct reverse
        {
            BOOST_PROTO_CALLABLE()

            template<typename Sig>
            struct result;

            template<typename This, typename Expr>
            struct result<This(Expr)>
            {
                typedef
                    typename fusion::BOOST_PROTO_FUSION_RESULT_OF::reverse<UNREF(Expr) const>::type
                type;
            };

            template<typename Expr>
            typename fusion::BOOST_PROTO_FUSION_RESULT_OF::reverse<Expr const>::type
            operator ()(Expr const &expr) const
            {
                return fusion::reverse(expr);
            }
        };
    }

    /// \brief A function that returns a "flattened"
    /// view of a Proto expression tree.
    ///
    /// For a tree with a top-most node
    /// tag of type \c T, the elements of the flattened sequence are
    /// determined by recursing into each child node with the same
    /// tag type and returning those nodes of different type. So for
    /// instance, the Proto expression tree corresponding to the
    /// expression <tt>a | b | c</tt> has a flattened view with elements
    /// [a, b, c], even though the tree is grouped as
    /// <tt>((a | b) | c)</tt>.
    template<typename Expr>
    proto::detail::flat_view<Expr const>
    flatten(Expr const &expr)
    {
        return proto::detail::flat_view<Expr const>(expr);
    }

    /// INTERNAL ONLY
    ///
    template<>
    struct is_callable<functional::flatten>
      : mpl::true_
    {};

    /// INTERNAL ONLY
    ///
    template<>
    struct is_callable<functional::pop_front>
      : mpl::true_
    {};

    /// INTERNAL ONLY
    ///
    template<>
    struct is_callable<functional::reverse>
      : mpl::true_
    {};

    /// INTERNAL ONLY
    ///
    template<typename Context>
    struct eval_fun
    {
        explicit eval_fun(Context &ctx)
          : ctx_(ctx)
        {}

        template<typename Sig>
        struct result;

        template<typename This, typename Expr>
        struct result<This(Expr)>
        {
            typedef
                typename proto::result_of::eval<UNREF(Expr), Context>::type
            type;
        };

        template<typename Expr>
        typename proto::result_of::eval<Expr, Context>::type
        operator ()(Expr &expr) const
        {
            return proto::eval(expr, this->ctx_);
        }

    private:
        Context &ctx_;
    };
}}

// Don't bother emitting all this into the Doxygen-generated
// reference section. It's enough to say that Proto expressions
// are valid Fusion sequence without showing all this gunk.
#ifndef BOOST_PROTO_BUILDING_DOCS

namespace boost { namespace fusion
{
    #if BOOST_VERSION < 103500
    template<typename Tag, typename Args, long Arity>
    struct is_sequence<proto::expr<Tag, Args, Arity> >
      : mpl::true_
    {};

    template<typename Tag, typename Args, long Arity>
    struct is_sequence<proto::expr<Tag, Args, Arity> const>
      : mpl::true_
    {};
    #endif

    namespace BOOST_PROTO_FUSION_EXTENSION
    {

        template<typename Tag>
        struct is_view_impl;

        template<>
        struct is_view_impl<proto::tag::proto_flat_view>
        {
            template<typename Sequence>
            struct apply
              : mpl::true_
            {};
        };

        template<>
        struct is_view_impl<proto::tag::proto_expr>
        {
            template<typename Sequence>
            struct apply
              : mpl::false_
            {};
        };

        template<typename Tag>
        struct value_of_impl;

        template<>
        struct value_of_impl<proto::tag::proto_expr_iterator>
        {
            template<
                typename Iterator
              , long Arity = proto::arity_of<typename Iterator::expr_type>::value
            >
            struct apply
            {
                typedef
                    typename proto::result_of::child_c<
                        typename Iterator::expr_type
                      , Iterator::index
                    >::value_type
                type;
            };

            template<typename Iterator>
            struct apply<Iterator, 0>
            {
                typedef
                    typename proto::result_of::value<
                        typename Iterator::expr_type
                    >::value_type
                type;
            };
        };

        #if BOOST_VERSION < 103500
        template<typename Tag>
        struct value_impl;

        template<>
        struct value_impl<proto::tag::proto_expr_iterator>
          : value_of_impl<proto::tag::proto_expr_iterator>
        {};
        #endif

        template<typename Tag>
        struct deref_impl;

        template<>
        struct deref_impl<proto::tag::proto_expr_iterator>
        {
            template<
                typename Iterator
              , long Arity = proto::arity_of<typename Iterator::expr_type>::value
            >
            struct apply
            {
                typedef
                    typename proto::result_of::child_c<
                        typename Iterator::expr_type const &
                      , Iterator::index
                    >::type
                type;

                static type call(Iterator const &iter)
                {
                    return proto::child_c<Iterator::index>(iter.expr);
                }
            };

            template<typename Iterator>
            struct apply<Iterator, 0>
            {
                typedef
                    typename proto::result_of::value<
                        typename Iterator::expr_type const &
                    >::type
                type;

                static type call(Iterator const &iter)
                {
                    return proto::value(iter.expr);
                }
            };
        };

        template<typename Tag>
        struct advance_impl;

        template<>
        struct advance_impl<proto::tag::proto_expr_iterator>
        {
            template<typename Iterator, typename N>
            struct apply
            {
                typedef
                    typename proto::detail::expr_iterator<
                        typename Iterator::expr_type
                      , Iterator::index + N::value
                    >
                type;

                static type call(Iterator const &iter)
                {
                    return type(iter.expr);
                }
            };
        };

        template<typename Tag>
        struct distance_impl;

        template<>
        struct distance_impl<proto::tag::proto_expr_iterator>
        {
            template<typename IteratorFrom, typename IteratorTo>
            struct apply
              : mpl::long_<IteratorTo::index - IteratorFrom::index>
            {};
        };

        template<typename Tag>
        struct next_impl;

        template<>
        struct next_impl<proto::tag::proto_expr_iterator>
        {
            template<typename Iterator>
            struct apply
              : advance_impl<proto::tag::proto_expr_iterator>::template apply<Iterator, mpl::long_<1> >
            {};
        };

        template<typename Tag>
        struct prior_impl;

        template<>
        struct prior_impl<proto::tag::proto_expr_iterator>
        {
            template<typename Iterator>
            struct apply
              : advance_impl<proto::tag::proto_expr_iterator>::template apply<Iterator, mpl::long_<-1> >
            {};
        };

        #if BOOST_VERSION >= 103500
        template<typename Tag>
        struct category_of_impl;

        template<>
        struct category_of_impl<proto::tag::proto_expr>
        {
            template<typename Sequence>
            struct apply
            {
                typedef random_access_traversal_tag type;
            };
        };
        #endif

        template<typename Tag>
        struct size_impl;

        template<>
        struct size_impl<proto::tag::proto_expr>
        {
            template<typename Sequence>
            struct apply
              : mpl::long_<0 == Sequence::proto_arity::value ? 1 : Sequence::proto_arity::value>
            {};
        };

        template<typename Tag>
        struct begin_impl;

        template<>
        struct begin_impl<proto::tag::proto_expr>
        {
            template<typename Sequence>
            struct apply
            {
                typedef proto::detail::expr_iterator<Sequence, 0> type;

                static type call(Sequence const &seq)
                {
                    return type(seq);
                }
            };
        };

        template<typename Tag>
        struct end_impl;

        template<>
        struct end_impl<proto::tag::proto_expr>
        {
            template<typename Sequence>
            struct apply
            {
                typedef
                    proto::detail::expr_iterator<
                        Sequence
                      , 0 == Sequence::proto_arity::value ? 1 : Sequence::proto_arity::value
                    >
                type;

                static type call(Sequence const &seq)
                {
                    return type(seq);
                }
            };
        };

        template<typename Tag>
        struct value_at_impl;

        template<>
        struct value_at_impl<proto::tag::proto_expr>
        {
            template<
                typename Sequence
              , typename Index
              , long Arity = proto::arity_of<Sequence>::value
            >
            struct apply
            {
                typedef
                    typename proto::result_of::child_c<
                        Sequence
                      , Index::value
                    >::value_type
                type;
            };

            template<typename Sequence, typename Index>
            struct apply<Sequence, Index, 0>
            {
                typedef
                    typename proto::result_of::value<
                        Sequence
                    >::value_type
                type;
            };
        };

        template<typename Tag>
        struct at_impl;

        template<>
        struct at_impl<proto::tag::proto_expr>
        {
            template<
                typename Sequence
              , typename Index
              , long Arity = proto::arity_of<Sequence>::value
            >
            struct apply
            {
                typedef
                    typename proto::result_of::child_c<
                        Sequence &
                      , Index::value
                    >::type
                type;

                static type call(Sequence &seq)
                {
                    return proto::child_c<Index::value>(seq);
                }
            };

            template<typename Sequence, typename Index>
            struct apply<Sequence, Index, 0>
            {
                typedef
                    typename proto::result_of::value<
                        Sequence &
                    >::type
                type;

                static type call(Sequence &seq)
                {
                    return proto::value(seq);
                }
            };
        };

        #if BOOST_VERSION >= 103500
        template<typename Tag>
        struct is_segmented_impl;

        template<>
        struct is_segmented_impl<proto::tag::proto_flat_view>
        {
            template<typename Iterator>
            struct apply
              : mpl::true_
            {};
        };

        template<typename Tag>
        struct segments_impl;

        template<>
        struct segments_impl<proto::tag::proto_flat_view>
        {
            template<typename Sequence>
            struct apply
            {
                typedef typename Sequence::proto_tag proto_tag;

                typedef fusion::transform_view<
                    typename Sequence::expr_type
                  , proto::detail::as_element<proto_tag>
                > type;

                static type call(Sequence &sequence)
                {
                    return type(sequence.expr_, proto::detail::as_element<proto_tag>());
                }
            };
        };

        template<>
        struct category_of_impl<proto::tag::proto_flat_view>
        {
            template<typename Sequence>
            struct apply
            {
                typedef forward_traversal_tag type;
            };
        };

        template<>
        struct begin_impl<proto::tag::proto_flat_view>
        {
            template<typename Sequence>
            struct apply
              : fusion::segmented_begin<Sequence>
            {};
        };

        template<>
        struct end_impl<proto::tag::proto_flat_view>
        {
            template<typename Sequence>
            struct apply
              : fusion::segmented_end<Sequence>
            {};
        };

        template<>
        struct size_impl<proto::tag::proto_flat_view>
        {
            template<typename Sequence>
            struct apply
              : fusion::segmented_size<Sequence>
            {};
        };
        #endif

    }

}}

#endif // BOOST_PROTO_BUILDING_DOCS

#undef UNREF

#if BOOST_MSVC
#pragma warning(pop)
#endif

#endif
