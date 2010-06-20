/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(BOOST_SPIRIT_NONTERMINAL_DIRECTOR_FEB_19_2007_0259PM)
#define BOOST_SPIRIT_NONTERMINAL_DIRECTOR_FEB_19_2007_0259PM

#include <boost/spirit/home/support/nonterminal/nonterminal.hpp>
#include <boost/spirit/home/support/nonterminal/detail/expand_arg.hpp>
#include <boost/spirit/home/qi/domain.hpp>
#include <boost/spirit/home/support/component.hpp>
#include <boost/spirit/home/support/detail/values.hpp>
#include <boost/fusion/include/transform.hpp>
#include <boost/fusion/include/join.hpp>
#include <boost/fusion/include/single_view.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/mpl/at.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/remove_const.hpp>

namespace boost { namespace spirit { namespace qi
{
    struct nonterminal_director
    {
        template <typename Component, typename Context, typename Iterator>
        struct attribute
        {
            typedef typename result_of::subject<Component>::type nonterminal_holder;
            typedef typename nonterminal_holder::nonterminal_type::attr_type type;
        };

        template <
            typename NonterminalContext, typename Nonterminal
          , typename Iterator, typename Context
          , typename Skipper, typename Attribute>
        static bool parse_nonterminal(
            nonterminal_object<Nonterminal> const& x
          , Iterator& first, Iterator const& last
          , Context& /*caller_context*/, Skipper const& skipper
          , Attribute& attr)
        {
            // the nonterminal_holder holds an actual nonterminal_object
            typedef typename Nonterminal::locals_type locals_type;
            fusion::single_view<Attribute&> front(attr);
            NonterminalContext context(front, locals_type());
            return x.obj.parse(first, last, context, skipper);
        }

        template <
            typename NonterminalContext, typename Nonterminal
          , typename Iterator, typename Context
          , typename Skipper, typename Attribute>
        static bool parse_nonterminal(
            Nonterminal const* ptr
          , Iterator& first, Iterator const& last
          , Context& /*caller_context*/, Skipper const& skipper
          , Attribute& attr)
        {
            // the nonterminal_holder holds a pointer to a nonterminal
            typedef typename Nonterminal::locals_type locals_type;
            fusion::single_view<Attribute&> front(attr);
            NonterminalContext context(front, locals_type());
            return ptr->parse(first, last, context, skipper);
        }

        template <
            typename NonterminalContext, typename Nonterminal, typename FSequence
          , typename Iterator, typename Context
          , typename Skipper, typename Attribute>
        static bool parse_nonterminal(
            parameterized_nonterminal<Nonterminal, FSequence> const& x
          , Iterator& first, Iterator const& last
          , Context& caller_context, Skipper const& skipper
          , Attribute& attr)
        {
            // the nonterminal_holder holds a parameterized_nonterminal
            typedef typename Nonterminal::locals_type locals_type;
            fusion::single_view<Attribute&> front(attr);
            NonterminalContext context(
                fusion::join(
                    front
                  , fusion::transform(
                        x.fseq
                      , spirit::detail::expand_arg<Context>(caller_context)
                    )
                )
              , locals_type()
            );
            return x.ptr->parse(first, last, context, skipper);
        }

        template <
            typename Component
          , typename Iterator, typename Context
          , typename Skipper, typename Attribute>
        static bool parse(
            Component const& component
          , Iterator& first, Iterator const& last
          , Context& context, Skipper const& skipper
          , Attribute& attr_)
        {
            // main entry point

            typedef typename
                result_of::subject<Component>::type
            nonterminal_holder;

            //  The overall context_type consist of a tuple with:
            //      1) a tuple of the return value and parameters
            //      2) the locals
            //  if no signature is specified the first tuple contains
            //  an unused_type element at position zero only.

            typedef typename
                nonterminal_holder::nonterminal_type::context_type
            context_type;

            //  attr_type is the return type as specified by the associated
            //  nonterminal signature, if no signature is specified this is
            //  the unused_type
            typedef typename
                nonterminal_holder::nonterminal_type::attr_type
            attr_type;

            // create an attribute if one is not supplied
            typename mpl::if_<
                is_same<typename remove_const<Attribute>::type, unused_type>
              , attr_type
              , Attribute&>::type
            attr = spirit::detail::make_value<attr_type>::call(attr_);

            return parse_nonterminal<context_type>(
                subject(component).held
              , first, last, context, skipper, attr
            );
        }

        template <typename Nonterminal>
        static std::string what_nonterminal(nonterminal_object<Nonterminal> const& x)
        {
            // the nonterminal_holder holds an actual nonterminal_object
            return x.obj.what();
        }

        template <typename Nonterminal>
        static std::string what_nonterminal(Nonterminal const* ptr)
        {
            // the nonterminal_holder holds a pointer to a nonterminal
            return ptr->what();
        }

        template <typename Nonterminal, typename FSequence>
        static std::string what_nonterminal(
            parameterized_nonterminal<Nonterminal, FSequence> const& x)
        {
            // the nonterminal_holder holds a parameterized_nonterminal
            return x.ptr->what();
        }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            return what_nonterminal(subject(component).held);
        }
    };
}}}

#endif
