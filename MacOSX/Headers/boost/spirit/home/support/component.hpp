/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(BOOST_SPIRIT_COMPONENT_JAN_14_2007_1102AM)
#define BOOST_SPIRIT_COMPONENT_JAN_14_2007_1102AM

#include <boost/spirit/home/support/unused.hpp>
#include <boost/spirit/home/support/meta_grammar/grammar.hpp>
#include <boost/xpressive/proto/proto.hpp>
#include <boost/fusion/include/at.hpp>
#include <boost/fusion/include/value_at.hpp>
#include <boost/mpl/void.hpp>
#include <boost/mpl/identity.hpp>
#include <boost/mpl/apply.hpp>

namespace boost { namespace spirit
{
    ///////////////////////////////////////////////////////////////////////////
    //  component generalizes a spirit component. A component can be a parser,
    //  a primitive-parser, a composite-parser, a generator, etc.
    //  A component has:
    //
    //      1) Domain: The world it operates on (purely a type e.g. qi::domain).
    //      2) Director: Its Director (purely a type e.g. qi::sequence)
    //      3) Elements: For composites, a tuple of components
    //                   For primitives, a tuple of arbitrary information
    //
    ///////////////////////////////////////////////////////////////////////////
    template <typename Domain, typename Director, typename Elements>
    struct component
    {
        typedef Domain domain;
        typedef Director director;
        typedef Elements elements_type;

        component()
        {
        }

        component(Elements const& elements)
          : elements(elements)
        {
        }

        template <typename Elements2>
        component(component<Domain, Director, Elements2> const& other)
          : elements(other.elements)
        {
            // allow copy from components with compatible elements
        }

        elements_type elements;
    };

    ///////////////////////////////////////////////////////////////////////////
    //  Utils for extracting child components
    ///////////////////////////////////////////////////////////////////////////
    namespace result_of
    {
        template <typename Component>
        struct subject
        {
            typedef typename
                fusion::result_of::value_at_c<
                    typename Component::elements_type, 0>::type
            type;
        };

        template <typename Component>
        struct left
        {
            typedef typename
                fusion::result_of::value_at_c<
                    typename Component::elements_type, 0>::type
            type;
        };

        template <typename Component>
        struct right
        {
            typedef typename
                fusion::result_of::value_at_c<
                    typename Component::elements_type, 1>::type
            type;
        };

        template <typename Component>
        struct argument1
        {
            typedef typename
                fusion::result_of::value_at_c<
                    typename Component::elements_type, 1>::type
            type;
        };

        template <typename Component>
        struct argument2
        {
            typedef typename
                fusion::result_of::value_at_c<
                    typename Component::elements_type, 2>::type
            type;
        };

        template<typename Component, int N>
        struct arg_c
            : fusion::result_of::value_at_c<
                    typename Component::elements_type, N>
        {};

    }

    template <typename Component>
    typename fusion::result_of::at_c<
        typename Component::elements_type const, 0>::type
    inline subject(Component const& c)
    {
        return fusion::at_c<0>(c.elements);
    }

    template <typename Component>
    typename fusion::result_of::at_c<
        typename Component::elements_type const, 0>::type
    inline left(Component const& c)
    {
        return fusion::at_c<0>(c.elements);
    }

    template <typename Component>
    typename fusion::result_of::at_c<
        typename Component::elements_type const, 1>::type
    inline right(Component const& c)
    {
        return fusion::at_c<1>(c.elements);
    }

    template <typename Component>
    typename fusion::result_of::at_c<
        typename Component::elements_type const, 1>::type
    inline argument1(Component const& c)
    {
        return fusion::at_c<1>(c.elements);
    }

    template <typename Component>
    typename fusion::result_of::at_c<
        typename Component::elements_type const, 2>::type
    inline argument2(Component const& c)
    {
        return fusion::at_c<2>(c.elements);
    }

    template <int N, typename Component>
    typename fusion::result_of::at_c<
        typename Component::elements_type const, N>::type
    inline arg_c(Component const& c)
    {
        return fusion::at_c<N>(c.elements);
    }

    ///////////////////////////////////////////////////////////////////////////
    //  Test if Expr conforms to the grammar of Domain. If Expr is already
    //  a component, return mpl::true_.
    ///////////////////////////////////////////////////////////////////////////
    namespace traits
    {
        template <typename Domain, typename Expr>
        struct is_component
          : proto::matches<
                typename proto::result_of::as_expr<Expr>::type
              , typename meta_grammar::grammar<Domain>::type
            >
        {
        };

        template <typename Domain, typename Director, typename Elements>
        struct is_component<Domain, component<Domain, Director, Elements> > :
            mpl::true_
        {
        };
    }

    ///////////////////////////////////////////////////////////////////////////
    //  Convert an arbitrary expression to a spirit component. There's
    //  a metafunction in namespace result_of and a function in main
    //  spirit namespace. If Expr is already a component, return it as-is.
    ///////////////////////////////////////////////////////////////////////////
    namespace result_of
    {
        template <
            typename Domain, typename Expr, typename State = unused_type,
            typename Visitor = unused_type
        >
        struct as_component
        {
            typedef typename meta_grammar::grammar<Domain>::type grammar;
            typedef typename proto::result_of::as_expr<Expr>::type proto_xpr;

            typedef typename
                grammar::template result<
                    void(proto_xpr, State, Visitor)
                >::type
            type;
        };

        // special case for arrays
        template <
            typename Domain, typename T, int N,
            typename State, typename Visitor>
        struct as_component<Domain, T[N], State, Visitor>
        {
            typedef typename meta_grammar::grammar<Domain>::type grammar;
            typedef typename proto::result_of::as_expr<T const*>::type proto_xpr;

            typedef typename
                grammar::template result<
                    void(proto_xpr, State, Visitor)
                >::type
            type;
        };

        // special case for components
        template <typename Domain, typename Director, typename Elements>
        struct as_component<Domain, component<Domain, Director, Elements> > :
            mpl::identity<component<Domain, Director, Elements> >
        {
        };
    }

    template <typename Domain, typename Expr>
    inline typename result_of::as_component<Domain, Expr>::type
    as_component(Domain, Expr const& xpr)
    {
        unused_type unused;
        typedef typename result_of::as_component<Domain, Expr>::grammar grammar;
        return grammar()(proto::as_expr(xpr), unused, unused);
    }

    template <typename Domain, typename Expr, typename State, typename Visitor>
    inline typename result_of::as_component<Domain, Expr>::type
    as_component(Domain, Expr const& xpr, State const& state, Visitor& visitor)
    {
        typedef typename
            result_of::as_component<Domain, Expr, State, Visitor>::grammar
        grammar;
        return grammar()(proto::as_expr(xpr), state, visitor);
    }

    template <typename Domain, typename Director, typename Elements>
    inline component<Domain, Director, Elements> const&
    as_component(Domain, component<Domain, Director, Elements> const& component)
    {
        return component;
    }

    ///////////////////////////////////////////////////////////////////////////
    //  Create a component. This is a customization point. Components are
    //  not created directly; they are created through make_component.
    //  Clients may customize this to direct the creation of a component.
    //
    //  The extra Modifier template parameter may be used to direct the
    //  creation of the component. This is the Visitor parameter in Proto
    //  transforms.
    //
    //  (see also: modifier.hpp)
    ///////////////////////////////////////////////////////////////////////////
    namespace traits
    {
        template <
            typename Domain, typename Director
          , typename Elements, typename Modifier, typename Enable = void>
        struct make_component
          : mpl::identity<component<Domain, Director, Elements> >
        {
            static component<Domain, Director, Elements>
            call(Elements const& elements)
            {
                return component<Domain, Director, Elements>(elements);
            }
        };
    }
}}

#endif
