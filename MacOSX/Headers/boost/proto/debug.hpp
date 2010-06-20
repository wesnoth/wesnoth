///////////////////////////////////////////////////////////////////////////////
/// \file debug.hpp
/// Utilities for debugging Proto expression trees
//
//  Copyright 2008 Eric Niebler. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PROTO_DEBUG_HPP_EAN_12_31_2006
#define BOOST_PROTO_DEBUG_HPP_EAN_12_31_2006

#include <boost/proto/detail/prefix.hpp>
#include <boost/preprocessor/iteration/local.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <iomanip>
#include <iostream>
#include <typeinfo>
#include <boost/proto/proto_fwd.hpp>
#include <boost/proto/expr.hpp>
#include <boost/proto/traits.hpp>
#include <boost/proto/detail/dont_care.hpp>
#include <boost/proto/detail/suffix.hpp>

namespace boost { namespace proto
{
    namespace tag
    {
        namespace hidden_detail_
        {
            typedef char (&not_ostream)[sizeof(std::ostream)+1];
            not_ostream operator<<(std::ostream &, detail::dont_care);

            template<typename Tag, std::size_t S>
            struct printable_tag_
            {
                typedef char const *type;
                static type call() { return typeid(Tag).name(); }
            };

            template<typename Tag>
            struct printable_tag_<Tag, sizeof(std::ostream)>
            {
                typedef Tag type;
                static type call() { return Tag(); }
            };

            template<typename Tag>
            struct printable_tag
              : printable_tag_<Tag, sizeof(std::cout << Tag())>
            {};
        }

        /// INTERNAL ONLY
        template<typename Tag>
        inline typename hidden_detail_::printable_tag<Tag>::type proto_tag_name(Tag)
        {
            return hidden_detail_::printable_tag<Tag>::call();
        }

    #define BOOST_PROTO_DEFINE_TAG_NAME(Tag)                                    \
        /** \brief INTERNAL ONLY */                                             \
        inline char const *proto_tag_name(tag::Tag)                             \
        {                                                                       \
            return #Tag;                                                        \
        }                                                                       \
        /**/

        BOOST_PROTO_DEFINE_TAG_NAME(unary_plus)
        BOOST_PROTO_DEFINE_TAG_NAME(negate)
        BOOST_PROTO_DEFINE_TAG_NAME(dereference)
        BOOST_PROTO_DEFINE_TAG_NAME(complement)
        BOOST_PROTO_DEFINE_TAG_NAME(address_of)
        BOOST_PROTO_DEFINE_TAG_NAME(logical_not)
        BOOST_PROTO_DEFINE_TAG_NAME(pre_inc)
        BOOST_PROTO_DEFINE_TAG_NAME(pre_dec)
        BOOST_PROTO_DEFINE_TAG_NAME(post_inc)
        BOOST_PROTO_DEFINE_TAG_NAME(post_dec)
        BOOST_PROTO_DEFINE_TAG_NAME(shift_left)
        BOOST_PROTO_DEFINE_TAG_NAME(shift_right)
        BOOST_PROTO_DEFINE_TAG_NAME(multiplies)
        BOOST_PROTO_DEFINE_TAG_NAME(divides)
        BOOST_PROTO_DEFINE_TAG_NAME(modulus)
        BOOST_PROTO_DEFINE_TAG_NAME(plus)
        BOOST_PROTO_DEFINE_TAG_NAME(minus)
        BOOST_PROTO_DEFINE_TAG_NAME(less)
        BOOST_PROTO_DEFINE_TAG_NAME(greater)
        BOOST_PROTO_DEFINE_TAG_NAME(less_equal)
        BOOST_PROTO_DEFINE_TAG_NAME(greater_equal)
        BOOST_PROTO_DEFINE_TAG_NAME(equal_to)
        BOOST_PROTO_DEFINE_TAG_NAME(not_equal_to)
        BOOST_PROTO_DEFINE_TAG_NAME(logical_or)
        BOOST_PROTO_DEFINE_TAG_NAME(logical_and)
        BOOST_PROTO_DEFINE_TAG_NAME(bitwise_and)
        BOOST_PROTO_DEFINE_TAG_NAME(bitwise_or)
        BOOST_PROTO_DEFINE_TAG_NAME(bitwise_xor)
        BOOST_PROTO_DEFINE_TAG_NAME(comma)
        BOOST_PROTO_DEFINE_TAG_NAME(mem_ptr)
        BOOST_PROTO_DEFINE_TAG_NAME(assign)
        BOOST_PROTO_DEFINE_TAG_NAME(shift_left_assign)
        BOOST_PROTO_DEFINE_TAG_NAME(shift_right_assign)
        BOOST_PROTO_DEFINE_TAG_NAME(multiplies_assign)
        BOOST_PROTO_DEFINE_TAG_NAME(divides_assign)
        BOOST_PROTO_DEFINE_TAG_NAME(modulus_assign)
        BOOST_PROTO_DEFINE_TAG_NAME(plus_assign)
        BOOST_PROTO_DEFINE_TAG_NAME(minus_assign)
        BOOST_PROTO_DEFINE_TAG_NAME(bitwise_and_assign)
        BOOST_PROTO_DEFINE_TAG_NAME(bitwise_or_assign)
        BOOST_PROTO_DEFINE_TAG_NAME(bitwise_xor_assign)
        BOOST_PROTO_DEFINE_TAG_NAME(subscript)
        BOOST_PROTO_DEFINE_TAG_NAME(if_else_)
        BOOST_PROTO_DEFINE_TAG_NAME(function)

    #undef BOOST_PROTO_DEFINE_TAG_NAME
    }

    namespace functional
    {
        /// \brief Pretty-print a Proto expression tree.
        ///
        /// A PolymorphicFunctionObject which accepts a Proto expression
        /// tree and pretty-prints it to an \c ostream for debugging
        /// purposes.
        struct display_expr
        {
            typedef void result_type;

            /// \param sout  The \c ostream to which the expression tree
            ///              will be written.
            /// \param depth The starting indentation depth for this node.
            ///              Children nodes will be displayed at a starting
            ///              depth of <tt>depth+4</tt>.
            explicit display_expr(std::ostream &sout = std::cout, int depth = 0)
              : depth_(depth)
              , first_(true)
              , sout_(sout)
            {}

            /// \brief Pretty-print the current node in a Proto expression
            /// tree.
            template<typename Args>
            void operator()(proto::expr<tag::terminal, Args, 0> const &expr) const
            {
                this->sout_ << std::setw(this->depth_) << (this->first_? "" : ", ")
                    << "terminal(" << proto::value(expr) << ")\n";
                this->first_ = false;
            }

        #define BOOST_PROTO_CHILD(Z, N, DATA)                                                       \
            display(proto::child_c<N>(expr));                                                       \
            /**/

        #define BOOST_PP_LOCAL_MACRO(N)                                                             \
            /** \overload */                                                                        \
            template<typename Tag, typename Args>                                                   \
            void operator()(proto::expr<Tag, Args, N> const &expr) const                            \
            {                                                                                       \
                using namespace tag;                                                                \
                this->sout_ << std::setw(this->depth_) << (this->first_? "" : ", ")                 \
                    << proto_tag_name(Tag()) << "(\n";                                              \
                display_expr display(this->sout_, this->depth_ + 4);                                \
                BOOST_PP_REPEAT(N, BOOST_PROTO_CHILD, _)                                            \
                this->sout_ << std::setw(this->depth_) << "" << ")\n";                              \
                this->first_ = false;                                                               \
            }                                                                                       \
            /**/

        #define BOOST_PP_LOCAL_LIMITS (1, BOOST_PROTO_MAX_ARITY)
        #include BOOST_PP_LOCAL_ITERATE()
        #undef BOOST_PROTO_CHILD

            /// \overload
            ///
            template<typename T>
            void operator()(T const &t) const
            {
                (*this)(t.proto_base());
            }

        private:
            display_expr &operator =(display_expr const &);
            int depth_;
            mutable bool first_;
            std::ostream &sout_;
        };
    }

    /// \brief Pretty-print a Proto expression tree.
    ///
    /// \note Equivalent to <tt>functional::display_expr(0, sout)(expr)</tt>
    /// \param expr The Proto expression tree to pretty-print
    /// \param sout The \c ostream to which the output should be
    ///             written. If not specified, defaults to
    ///             <tt>std::cout</tt>.
    template<typename Expr>
    void display_expr(Expr const &expr, std::ostream &sout)
    {
        functional::display_expr(sout, 0)(expr);
    }

    /// \overload
    ///
    template<typename Expr>
    void display_expr(Expr const &expr)
    {
        functional::display_expr()(expr);
    }

}}

#endif
