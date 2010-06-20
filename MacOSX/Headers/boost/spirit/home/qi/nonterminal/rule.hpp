/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(BOOST_SPIRIT_RULE_FEB_12_2007_1020AM)
#define BOOST_SPIRIT_RULE_FEB_12_2007_1020AM

#include <boost/spirit/home/support/unused.hpp>
#include <boost/spirit/home/qi/nonterminal/nonterminal.hpp>
#include <boost/spirit/home/qi/nonterminal/grammar_fwd.hpp>
#include <boost/spirit/home/qi/nonterminal/detail/rule.hpp>
#include <boost/spirit/home/qi/nonterminal/detail/error_handler.hpp>
#include <boost/spirit/home/qi/domain.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/type_traits/is_convertible.hpp>

#if defined(BOOST_MSVC)
# pragma warning(push)
# pragma warning(disable: 4355) // 'this' : used in base member initializer list warning
#endif

namespace boost { namespace spirit { namespace qi
{
    namespace detail { struct rule_decorator; }

    template <
        typename Iterator
      , typename T0 = unused_type
      , typename T1 = unused_type
      , typename T2 = unused_type
    >
    struct rule
      : make_nonterminal<rule<Iterator, T0, T1, T2>, T0, T1, T2>::type
    {
        typedef
            make_nonterminal<rule<Iterator, T0, T1, T2>, T0, T1, T2>
        make_nonterminal_;

        typedef typename make_nonterminal_::skipper_type skipper_type;
        typedef typename make_nonterminal_::type base_type;
        typedef Iterator iterator_type;
        typedef rule<Iterator, T0, T1, T2> self_type;

        typedef
            virtual_component_base<
                Iterator
              , typename base_type::context_type
              , skipper_type
            >
        virtual_component;

        typedef intrusive_ptr<virtual_component> pointer_type;

        rule(std::string const& name_ = std::string())
          : name_(name_) {}

        ~rule() {}

        rule(rule const& rhs)
          : ptr(rhs.ptr)
          , name_(rhs.name_)
        {
        }

        rule& operator=(rule const& rhs)
        {
            ptr = rhs.ptr;
            name_ = rhs.name_;
            return *this;
        }

        template <typename Expr>
        rule& operator=(Expr const& xpr)
        {
            typedef spirit::traits::is_component<qi::domain, Expr> is_component;

            // report invalid expression error as early as possible
//             BOOST_MPL_ASSERT_MSG(
//                 is_component::value,
//                 xpr_is_not_convertible_to_a_parser, ());

            // temp workaround for mpl problem
            BOOST_STATIC_ASSERT(is_component::value);

            define(xpr, mpl::false_());
            return *this;
        }

        template <typename Expr>
        friend rule& operator%=(rule& r, Expr const& xpr)
        {
            typedef spirit::traits::is_component<qi::domain, Expr> is_component;

            // report invalid expression error as early as possible
            //~ BOOST_MPL_ASSERT_MSG(
                //~ is_component::value,
                //~ xpr_is_not_convertible_to_a_parser, ());

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

        std::string const& name() const
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

        friend struct detail::rule_decorator;

        template <typename Expr, typename Auto>
        void define(Expr const& xpr, Auto)
        {
            typedef typename
                result_of::as_component<qi::domain, Expr>::type
            component;
            typedef
                detail::virtual_component<
                    Iterator
                  , component
                  , typename base_type::context_type
                  , skipper_type
                  , Auto
                >
            virtual_component;
            ptr = new virtual_component(spirit::as_component(qi::domain(), xpr));
        }

        template <typename Iterator_, typename Context, typename Skipper>
        bool parse(
            Iterator_& first, Iterator_ const& last
          , Context& context, Skipper const& skipper) const
        {
            // If the following line produces a compilation error stating the
            // 4th parameter is not convertible to the expected type, then you
            // are probably trying to use this rule instance with a skipper
            // which is not compatible with the skipper type used while
            // defining the type of this rule instance.
            return ptr->parse(first, last, context, skipper);
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

    // Decoration support: create a new virtual component and link it as
    // first element in the chain of virtual components associated with this
    // rule. Returns the previous topmost virtual component in the chain.
    // We provide support from 1 to 5 arguments.

    namespace detail
    {
        struct rule_decorator
        {
            template <typename Decorator, typename Rule, typename A1>
            typename Rule::pointer_type
            static call(Rule& r, A1 const& a1)
            {
                typename Rule::pointer_type old (r.ptr);
                r.ptr.reset(new Decorator(r.ptr, a1));
                return old;
            }

            template <typename Decorator, typename Rule, typename A1, typename A2>
            typename Rule::pointer_type
            static call(Rule& r, A1 const& a1, A2 const& a2)
            {
                typename Rule::pointer_type old (r.ptr);
                r.ptr.reset(new Decorator(r.ptr, a1, a2));
                return old;
            }

            template <typename Decorator, typename Rule
              , typename A1, typename A2, typename A3
            >
            typename Rule::pointer_type
            static call(Rule& r
              , A1 const& a1, A2 const& a2, A3 const& a3)
            {
                typename Rule::pointer_type old (r.ptr);
                r.ptr.reset(new Decorator(r.ptr, a1, a2, a3));
                return old;
            }

            template <typename Decorator, typename Rule
              , typename A1, typename A2, typename A3, typename A4
            >
            typename Rule::pointer_type
            static call(Rule& r
              , A1 const& a1, A2 const& a2
              , A3 const& a3, A4 const& a4)
            {
                typename Rule::pointer_type old (r.ptr);
                r.ptr.reset(new Decorator(r.ptr, a1, a2, a3, a4));
                return old;
            }

            template <typename Decorator, typename Rule
              , typename A1, typename A2, typename A3, typename A4, typename A5
            >
            typename Rule::pointer_type
            static call(Rule& r
              , A1 const& a1, A2 const& a2
              , A3 const& a3, A4 const& a4, A5 const& a5)
            {
                typename Rule::pointer_type old (r.ptr);
                r.ptr.reset(new Decorator(r.ptr, a1, a2, a3, a4, a5));
                return old;
            }
        };
    }

    template <typename Decorator
      , typename Iterator, typename T0, typename T1, typename T2
      , typename A1>
    typename rule<Iterator, T0, T1, T2>::pointer_type
    decorate(rule<Iterator, T0, T1, T2>& r
      , A1 const& a1)
    {
        return detail::rule_decorator::
            template call<Decorator>(r, a1);
    }

    template <typename Decorator
      , typename Iterator, typename T0, typename T1, typename T2
      , typename A1, typename A2
    >
    typename rule<Iterator, T0, T1, T2>::pointer_type
    decorate(rule<Iterator, T0, T1, T2>& r
      , A1 const& a1, A2 const& a2)
    {
        return detail::rule_decorator::
            template call<Decorator>(r, a1, a2);
    }

    template <typename Decorator
      , typename Iterator, typename T0, typename T1, typename T2
      , typename A1, typename A2, typename A3
    >
    typename rule<Iterator, T0, T1, T2>::pointer_type
    decorate(rule<Iterator, T0, T1, T2>& r
      , A1 const& a1, A2 const& a2, A3 const& a3)
    {
        return detail::rule_decorator::
            template call<Decorator>(r, a1, a2, a3);
    }

    template <typename Decorator
      , typename Iterator, typename T0, typename T1, typename T2
      , typename A1, typename A2, typename A3, typename A4
    >
    typename rule<Iterator, T0, T1, T2>::pointer_type
    decorate(rule<Iterator, T0, T1, T2>& r
      , A1 const& a1, A2 const& a2
      , A3 const& a3, A4 const& a4)
    {
        return detail::rule_decorator::
            template call<Decorator>(r, a1, a2, a3, a4);
    }

    template <typename Decorator
      , typename Iterator, typename T0, typename T1, typename T2
      , typename A1, typename A2, typename A3, typename A4, typename A5
    >
    typename rule<Iterator, T0, T1, T2>::pointer_type
    decorate(rule<Iterator, T0, T1, T2>& r
      , A1 const& a1, A2 const& a2
      , A3 const& a3, A4 const& a4, A5 const& a5)
    {
        return detail::rule_decorator::
            template call<Decorator>(r, a1, a2, a3, a4, a5);
    }

    // Error handling support
    template <
        error_handler_result action
      , typename Iterator, typename T0, typename T1, typename T2
      , typename F>
    void on_error(rule<Iterator, T0, T1, T2>& r, F f)
    {
        typedef
            rule<Iterator, T0, T1, T2>
        rule_type;

        typedef
            detail::error_handler<
                Iterator
              , typename rule_type::base_type::context_type
              , typename rule_type::skipper_type
              , F
              , action>
        error_handler;
        decorate<error_handler>(r, f);
    }

    // Error handling support when <action> is not
    // specified. We will default to <fail>.
    template <typename Iterator, typename T0, typename T1
      , typename T2, typename F>
    void on_error(rule<Iterator, T0, T1, T2>& r, F f)
    {
        on_error<fail>(r, f);
    }

}}}

#if defined(BOOST_MSVC)
# pragma warning(pop)
#endif

#endif
