//  Copyright (c) 2001-2007 Joel de Guzman
//  Copyright (c) 2001-2008 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_KARMA_RULE_MAR_05_2007_0455PM)
#define BOOST_SPIRIT_KARMA_RULE_MAR_05_2007_0455PM

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once      // MS compatible compilers support #pragma once
#endif

#include <boost/spirit/home/support/unused.hpp>
#include <boost/spirit/home/karma/nonterminal/nonterminal.hpp>
#include <boost/spirit/home/karma/nonterminal/grammar_fwd.hpp>
#include <boost/spirit/home/karma/nonterminal/detail/rule.hpp>
#include <boost/spirit/home/karma/domain.hpp>
#include <boost/spirit/home/karma/detail/output_iterator.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/assert.hpp>

#if defined(BOOST_MSVC)
# pragma warning(push)
# pragma warning(disable: 4355) // 'this' : used in base member initializer list warning
#endif

namespace boost { namespace spirit { namespace karma
{
    template <typename OutputIterator, typename T0 = unused_type,
        typename T1 = unused_type, typename T2 = unused_type>
    struct rule
      : make_nonterminal<rule<OutputIterator, T0, T1, T2>, T0, T1, T2>::type
    {
        typedef
            make_nonterminal<rule<OutputIterator, T0, T1, T2>, T0, T1, T2>
        make_nonterminal_;

        typedef typename make_nonterminal_::delimiter_type delimiter_type;
        typedef typename make_nonterminal_::type base_type;
        typedef detail::output_iterator<OutputIterator> iterator_type;
        typedef rule<OutputIterator, T0, T1, T2> self_type;

        typedef
            detail::virtual_component_base<
                iterator_type,
                typename base_type::context_type,
                delimiter_type
            >
        virtual_component;

        typedef intrusive_ptr<virtual_component> pointer_type;

        rule() {}
        ~rule() {}

        rule(rule const& rhs)
          : ptr(rhs.ptr)
        {
        }

        rule& operator=(rule const& rhs)
        {
            ptr = rhs.ptr;
            return *this;
        }

        template <typename Expr>
        rule& operator=(Expr const& xpr)
        {
            typedef
                spirit::traits::is_component<karma::domain, Expr>
            is_component;

            // report invalid expression error as early as possible
//             BOOST_MPL_ASSERT_MSG(
//                 is_component::value,
//                 xpr_is_not_convertible_to_a_generator, ());

            // temp workaround for mpl problem
            BOOST_STATIC_ASSERT(is_component::value);

            define(xpr, mpl::false_());
            return *this;
        }

        template <typename Expr>
        friend rule& operator%=(rule& r, Expr const& xpr)
        {
            typedef
                spirit::traits::is_component<karma::domain, Expr>
            is_component;

            // report invalid expression error as early as possible
//             BOOST_MPL_ASSERT_MSG(
//                 is_component::value,
//                 xpr_is_not_convertible_to_a_generator, ());

            // temp workaround for mpl problem
            BOOST_STATIC_ASSERT(is_component::value);

            r.define(xpr, mpl::true_());
            return r;
        }

        self_type alias() const
        {
            self_type result;
            result.define(*this, mpl::false_());
            return result;
        }

        typename
            make_nonterminal_holder<
                nonterminal_object<self_type>
              , self_type
            >::type
        copy() const
        {
            typename
                make_nonterminal_holder<
                    nonterminal_object<self_type>
                  , self_type
                >::type
            result = {{*this}};
            return result;
        }

        std::string name() const
        {
            return name_;
        }

        void name(std::string const& str)
        {
            name_ = str;
        }

    private:

        template <typename Iterator_, typename T0_, typename T1_, typename T2_>
        friend struct grammar;

        template <typename Expr, typename Auto>
        void define(Expr const& xpr, Auto)
        {
            typedef typename
                result_of::as_component<karma::domain, Expr>::type
            component;
            typedef
                detail::virtual_component<
                    iterator_type,
                    component,
                    typename base_type::context_type,
                    delimiter_type,
                    Auto
                >
            virtual_component;
            ptr = new virtual_component(spirit::as_component(karma::domain(), xpr));
        }

        template <typename OutputIterator_, typename Context, typename Delimiter>
        bool generate(
            OutputIterator_& sink, Context& context, Delimiter const& delim) const
        {
            // If the following line produces a compilation error stating the
            // 3rd parameter is not convertible to the expected type, then you
            // are probably trying to use this rule instance with a delimiter 
            // which is not compatible with the delimiter type used while 
            // defining the type of this rule instance.
            return ptr->generate(sink, context, delim);
        }

        std::string what() const
        {
            if (name_.empty())
            {
                if (ptr)
                {
                    return "unnamed-rule";
                }
                else
                {
                    return "empty-rule";
                }
            }
            else
            {
                return name_;
            }
        }

        friend struct nonterminal_director;
        pointer_type ptr;
        std::string name_;
    };

}}}

#if defined(BOOST_MSVC)
# pragma warning(pop)
#endif

#endif
