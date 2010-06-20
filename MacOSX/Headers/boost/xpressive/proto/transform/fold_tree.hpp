///////////////////////////////////////////////////////////////////////////////
/// \file fold_tree.hpp
/// Contains definition of the fold_tree<> and reverse_fold_tree<> transforms.
//
//  Copyright 2008 Eric Niebler. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PROTO_TRANSFORM_FOLD_TREE_HPP_EAN_11_05_2007
#define BOOST_PROTO_TRANSFORM_FOLD_TREE_HPP_EAN_11_05_2007

#include <boost/xpressive/proto/detail/prefix.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/xpressive/proto/proto_fwd.hpp>
#include <boost/xpressive/proto/traits.hpp>
#include <boost/xpressive/proto/matches.hpp>
#include <boost/xpressive/proto/transform/fold.hpp>
#include <boost/xpressive/proto/detail/suffix.hpp>

namespace boost { namespace proto
{
    namespace transform
    {
        namespace detail
        {
            template<typename Tag>
            struct has_tag : proto::callable
            {
                template<typename Sig, typename EnableIf = Tag>
                struct result
                  : mpl::false_
                {};

                template<typename This, typename Expr, typename State, typename Visitor>
                struct result<This(Expr, State, Visitor), typename Expr::proto_tag>
                  : mpl::true_
                {};
            };

            template<typename Tag, typename Fun>
            struct fold_tree_
              : if_<has_tag<Tag>, fold<_, _state, fold_tree_<Tag, Fun> >, Fun>
            {};

            template<typename Tag, typename Fun>
            struct reverse_fold_tree_
              : if_<has_tag<Tag>, reverse_fold<_, _state, reverse_fold_tree_<Tag, Fun> >, Fun>
            {};
        }

        /// \brief A PrimitiveTransform that recursively applies the
        /// <tt>fold\<\></tt> transform to sub-trees that all share a common
        /// tag type.
        ///
        /// <tt>fold_tree\<\></tt> is useful for flattening trees into lists;
        /// for example, you might use <tt>fold_tree\<\></tt> to flatten an
        /// expression tree like <tt>a | b | c</tt> into a Fusion list like
        /// <tt>cons(c, cons(b, cons(a)))</tt>.
        ///
        /// <tt>fold_tree\<\></tt> is easily understood in terms of a
        /// <tt>recurse_if_\<\></tt> helper, defined as follows:
        ///
        /// \code
        /// template<typename Tag, typename Fun>
        /// struct recurse_if_
        ///   : if_<
        ///         // If the current node has type type "Tag" ...
        ///         is_same<tag_of<_>, Tag>()
        ///         // ... recurse, otherwise ...
        ///       , fold<_, _state, recurse_if_<Tag, Fun> >
        ///         // ... apply the Fun transform.
        ///       , Fun
        ///     >
        /// {};
        /// \endcode
        ///
        /// With <tt>recurse_if_\<\></tt> as defined above,
        /// <tt>fold_tree\<Sequence, State0, Fun\>()(expr, state, visitor)</tt> is
        /// equivalent to
        /// <tt>fold<Sequence, State0, recurse_if_<Expr::proto_tag, Fun> >()(expr, state, visitor).</tt>
        /// It has the effect of folding a tree front-to-back, recursing into
        /// child nodes that share a tag type with the parent node.
        template<typename Sequence, typename State0, typename Fun>
        struct fold_tree
          : proto::callable
        {
            template<typename Sig>
            struct result;

            template<typename This, typename Expr, typename State, typename Visitor>
            struct result<This(Expr, State, Visitor)>
            {
                /// \brief <tt>recurse_if_\<Expr::proto_tag, Fun\></tt>, as described below.
                typedef
                    detail::fold_tree_<typename Expr::proto_tag, Fun>
                recurse_if_;

                typedef fold<Sequence, State0, recurse_if_> impl;
                typedef typename impl::template result<void(Expr, State, Visitor)>::type type;
            };

            /// Let \c R be <tt>recurse_if_\<Expr::proto_tag,Fun\></tt> as described below.
            /// This function returns <tt>fold\<Sequence, State0, R\>()(expr, state, visitor)</tt>.
            ///
            /// \param expr The current expression
            /// \param state The current state
            /// \param visitor An arbitrary visitor
            template<typename Expr, typename State, typename Visitor>
            typename result<void(Expr, State, Visitor)>::type
            operator ()(Expr const &expr, State const &state, Visitor &visitor) const
            {
                typedef
                    detail::fold_tree_<typename Expr::proto_tag, Fun>
                recurse_if_;

                return fold<Sequence, State0, recurse_if_>()(expr, state, visitor);
            }
        };

        /// \brief A PrimitiveTransform that recursively applies the
        /// <tt>reverse_fold\<\></tt> transform to sub-trees that all share
        /// a common tag type.
        ///
        /// <tt>reverse_fold_tree\<\></tt> is useful for flattening trees into
        /// lists; for example, you might use <tt>reverse_fold_tree\<\></tt> to
        /// flatten an expression tree like <tt>a | b | c</tt> into a Fusion list
        /// like <tt>cons(a, cons(b, cons(c)))</tt>.
        ///
        /// <tt>reverse_fold_tree\<\></tt> is easily understood in terms of a
        /// <tt>recurse_if_\<\></tt> helper, defined as follows:
        ///
        /// \code
        /// template<typename Tag, typename Fun>
        /// struct recurse_if_
        ///   : if_<
        ///         // If the current node has type type "Tag" ...
        ///         is_same<tag_of<_>, Tag>()
        ///         // ... recurse, otherwise ...
        ///       , reverse_fold<_, _state, recurse_if_<Tag, Fun> >
        ///         // ... apply the Fun transform.
        ///       , Fun
        ///     >
        /// {};
        /// \endcode
        ///
        /// With <tt>recurse_if_\<\></tt> as defined above,
        /// <tt>reverse_fold_tree\<Sequence, State0, Fun\>()(expr, state, visitor)</tt> is
        /// equivalent to
        /// <tt>reverse_fold<Sequence, State0, recurse_if_<Expr::proto_tag, Fun> >()(expr, state, visitor).</tt>
        /// It has the effect of folding a tree back-to-front, recursing into
        /// child nodes that share a tag type with the parent node.
        template<typename Sequence, typename State0, typename Fun>
        struct reverse_fold_tree
          : proto::callable
        {
            template<typename Sig>
            struct result;

            template<typename This, typename Expr, typename State, typename Visitor>
            struct result<This(Expr, State, Visitor)>
            {
                /// \brief <tt>recurse_if_\<Expr::proto_tag, Fun\></tt>, as described below.
                typedef
                    detail::reverse_fold_tree_<typename Expr::proto_tag, Fun>
                recurse_if_;
                
                typedef reverse_fold<Sequence, State0, recurse_if_> impl;
                typedef typename impl::template result<void(Expr, State, Visitor)>::type type;
            };

            /// Let \c R be <tt>recurse_if_\<Expr::proto_tag,Fun\></tt> as described below.
            /// This function returns <tt>reverse_fold\<Sequence, State0, R\>()(expr, state, visitor)</tt>.
            ///
            /// \param expr The current expression
            /// \param state The current state
            /// \param visitor An arbitrary visitor
            template<typename Expr, typename State, typename Visitor>
            typename result<void(Expr, State, Visitor)>::type
            operator ()(Expr const &expr, State const &state, Visitor &visitor) const
            {
                typedef
                    detail::reverse_fold_tree_<typename Expr::proto_tag, Fun>
                recurse_if_;

                return reverse_fold<Sequence, State0, recurse_if_>()(expr, state, visitor);
            }
        };
    }

    /// INTERNAL ONLY
    ///
    template<typename Sequence, typename State0, typename Fun>
    struct is_callable<transform::fold_tree<Sequence, State0, Fun> >
      : mpl::true_
    {};

    /// INTERNAL ONLY
    ///
    template<typename Sequence, typename State0, typename Fun>
    struct is_callable<transform::reverse_fold_tree<Sequence, State0, Fun> >
      : mpl::true_
    {};
}}

#endif
