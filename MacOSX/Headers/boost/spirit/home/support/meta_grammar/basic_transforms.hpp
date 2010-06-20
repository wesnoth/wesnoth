/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman
    Copyright (c) 2001-2008 Hartmut Kaiser

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(BOOST_SPIRIT_BASIC_TRANSFORMS_JAN_14_2007_1222PM)
#define BOOST_SPIRIT_BASIC_TRANSFORMS_JAN_14_2007_1222PM

#include <boost/spirit/home/support/meta_grammar/grammar.hpp>
#include <boost/spirit/home/support/component.hpp>
#include <boost/spirit/home/support/modifier.hpp>

#include <boost/xpressive/proto/proto.hpp>
#include <boost/xpressive/proto/transform.hpp>

#include <boost/fusion/include/cons.hpp>
#include <boost/fusion/include/list.hpp>
#include <boost/fusion/include/make_cons.hpp>
#include <boost/fusion/include/make_list.hpp>

#include <boost/mpl/apply.hpp>

namespace boost { namespace spirit { namespace meta_grammar
{
    ///////////////////////////////////////////////////////////////////////////
    //  A proto transform for creating empty component meta descriptions
    //  (proto expressions) usable for defining meta grammars. Additionally,
    //  this is used to make the corresponding spirit component.
    //
    //    Grammar:   the proto grammar to use as the base for this component
    //               meta description (i.e.: proto::terminal<Tag>)
    //    Domain:    the domain this proto transform is defined for
    //               (i.e.: qi::domain)
    //    DirectorF: the director is the real component form the specified
    //               domain (i.e.: any_char)
    ///////////////////////////////////////////////////////////////////////////
    template <typename Grammar, typename Domain, typename DirectorF>
    struct compose_empty : Grammar
    {
        template<typename Sig>
        struct result;

        template <typename This, typename Expr, typename State, typename Visitor>
        struct result<This(Expr, State, Visitor)>
        {
            typedef typename proto::result_of::arg<Expr>::type arg_type;

            typedef
                traits::make_component<
                    Domain
                  , typename mpl::apply1<DirectorF, arg_type>::type
                  , fusion::nil
                  , Visitor
                >
            make_component;

            typedef typename make_component::type type;
        };

        template<typename Expr, typename State, typename Visitor>
        typename result<void(Expr, State, Visitor)>::type
        operator ()(Expr const&, State const&, Visitor&) const
        {
            typedef typename result<void(Expr, State, Visitor)>::make_component result;
            return result::call(fusion::nil());
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    //  A proto transform for creating single-element component meta
    //  descriptions (proto expressions) usable for defining meta grammars.
    //
    //    Grammar:   the proto grammar to use as the base for this component
    //               meta description (i.e.: proto::unary_expr<Tag, ...>)
    //    Domain:    the domain this proto transform is defined for
    //               (i.e.: qi::domain)
    //    DirectorF: the director is the real component from the specified
    //               domain (i.e.: negated_char_parser<...>)
    ///////////////////////////////////////////////////////////////////////////
    template <typename Grammar, typename Domain, typename DirectorF>
    struct compose_single : Grammar
    {
        template<typename Sig>
        struct result;

        template <typename This, typename Expr, typename State, typename Visitor>
        struct result<This(Expr, State, Visitor)>
        {
            typedef typename
                proto::result_of::arg<
                    typename Grammar::template result<void(Expr, State, Visitor)>::type
                >::type
            arg_type;

            typedef
                traits::make_component<
                    Domain
                  , typename mpl::apply1<DirectorF, arg_type>::type
                  , typename fusion::result_of::make_cons<arg_type>::type
                  , Visitor
                >
            make_component;

            typedef typename make_component::type type;
        };

        template<typename Expr, typename State, typename Visitor>
        typename result<void(Expr, State, Visitor)>::type
        operator ()(Expr const& expr, State const& state, Visitor& visitor) const
        {
            typedef typename result<void(Expr, State, Visitor)>::make_component result;
            return result::call(
                fusion::make_cons(
                   proto::arg(Grammar()(expr, state, visitor))
                )
            );
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    //  A proto transform for creating double-element component meta
    //  descriptions (proto expressions) usable for defining meta grammars.
    //
    //    Grammar:   the proto grammar to use as the base for this component
    //               meta description (i.e.: proto::binary_expr<Tag, ...>)
    //    Domain:    the domain this proto transform is defined for
    //               (for instance: qi::domain)
    //    DirectorF: the director is the real component form the specified
    //               domain (i.e.: difference)
    ///////////////////////////////////////////////////////////////////////////
    template <typename Grammar, typename Domain, typename DirectorF>
    struct compose_double : Grammar
    {
        template<typename Sig>
        struct result;

        template <typename This, typename Expr, typename State, typename Visitor>
        struct result<This(Expr, State, Visitor)>
        {
            typedef typename
                Grammar::template result<void(Expr, State, Visitor)>::type
            trans;

            typedef typename proto::result_of::left<trans>::type left_type;
            typedef typename proto::result_of::right<trans>::type right_type;
            typedef typename
                fusion::result_of::make_list<left_type, right_type>::type
            list_type;

            typedef
                traits::make_component<
                    Domain
                  , typename mpl::apply1<DirectorF, list_type>::type
                  , list_type
                  , Visitor
                >
            make_component;

            typedef typename make_component::type type;
        };

        template<typename Expr, typename State, typename Visitor>
        typename result<void(Expr, State, Visitor)>::type
        operator ()(Expr const& expr, State const& state, Visitor& visitor) const
        {
            typedef result<void(Expr, State, Visitor)> apply;
            typedef typename apply::make_component result;
            typedef typename apply::list_type list_type;

            typename apply::trans trans = Grammar()(expr, state, visitor);
            return result::call(
                fusion::make_list(proto::left(trans), proto::right(trans))
            );
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    //  A proto transform for creating triple-element component meta
    //  descriptions (proto expressions) usable for defining meta grammars.
    //
    //    Grammar:   the proto grammar to use as the base for this component
    //               meta description (i.e.: proto::nary_expr<Tag,a,b,c>)
    //    Domain:    the domain this proto transform is defined for
    //               (for instance: qi::domain)
    //    DirectorF: the director is the real component form the specified
    //               domain (i.e.: difference)
    ///////////////////////////////////////////////////////////////////////////

    template <typename Grammar, typename Domain, typename DirectorF>
    struct compose_triple : Grammar
    {
        template<typename Sig>
        struct result;

        template <typename This, typename Expr, typename State, typename Visitor>
        struct result<This(Expr, State, Visitor)>
        {
            typedef typename
                Grammar::template result<void(Expr, State, Visitor)>::type
            trans;

            typedef typename proto::result_of::arg_c<trans, 0>::type arg0_type;
            typedef typename proto::result_of::arg_c<trans, 1>::type arg1_type;
            typedef typename proto::result_of::arg_c<trans, 2>::type arg2_type;

            typedef typename
                fusion::result_of::make_list<arg0_type, arg1_type, arg2_type>::type
            list_type;

            typedef
                traits::make_component<
                    Domain
                  , typename mpl::apply1<DirectorF, list_type>::type
                  , list_type
                  , Visitor
                >
            make_component;

            typedef typename make_component::type type;
        };

        template<typename Expr, typename State, typename Visitor>
        typename result<void(Expr, State, Visitor)>::type
        operator ()(Expr const& expr, State const& state, Visitor& visitor) const
        {
            typedef result<void(Expr, State, Visitor)> apply;
            typedef typename apply::make_component result;
            typedef typename apply::list_type list_type;

            typename apply::trans trans = Grammar()(expr, state, visitor);
            return result::call(
                fusion::make_list(proto::arg_c<0>(trans), proto::arg_c<1>(trans), proto::arg_c<2>(trans))
            );
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    //  A proto transform for creating single-element component meta
    //  descriptions (proto expressions) usable for defining meta grammars
    //  Only the RHS is stored.
    //
    //    Grammar:   the proto grammar to use as the base for this component
    //               meta description (i.e.: proto::binary_expr<Tag, ...>)
    //    Domain:    the domain this proto transform is defined for
    //               (for instance: qi::domain)
    //    DirectorF: the director is the real component form the specified
    //               domain (i.e.: difference)
    ///////////////////////////////////////////////////////////////////////////
    template <typename Grammar, typename Domain, typename DirectorF>
    struct compose_right : Grammar
    {
        template<typename Sig>
        struct result;

        template <typename This, typename Expr, typename State, typename Visitor>
        struct result<This(Expr, State, Visitor)>
        {
            typedef typename
                Grammar::template result<void(Expr, State, Visitor)>::type
            trans;

            typedef typename proto::result_of::right<trans>::type right_type;
            typedef typename
                fusion::result_of::make_list<right_type>::type
            list_type;

            typedef
                traits::make_component<
                    Domain
                  , typename mpl::apply1<DirectorF, list_type>::type
                  , list_type
                  , Visitor
                >
            make_component;

            typedef typename make_component::type type;
        };

        template<typename Expr, typename State, typename Visitor>
        typename result<void(Expr, State, Visitor)>::type
        operator ()(Expr const& expr, State const& state, Visitor& visitor) const
        {
            typedef result<void(Expr, State, Visitor)> apply;
            typedef typename apply::make_component result;
            typedef typename apply::list_type list_type;

            typename apply::trans trans = Grammar()(expr, state, visitor);
            return result::call(
                fusion::make_list(proto::right(trans))
            );
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    //  A proto transform that accepts a proto::if_ predicate and
    //  applies a supplied indirect transform if the predicate is true.
    ///////////////////////////////////////////////////////////////////////////
    template <typename Pred, typename TransformF>
    struct if_transform
      : proto::when<proto::if_<Pred>, proto::bind<TransformF> >
    {
    };

    ///////////////////////////////////////////////////////////////////////////
    //  A proto transform that composes components from a fusion::list
    ///////////////////////////////////////////////////////////////////////////
    template <typename Grammar, typename Domain, typename Director>
    struct compose_list : Grammar
    {
        template<typename Sig>
        struct result;

        template <typename This, typename Expr, typename State, typename Visitor>
        struct result<This(Expr, State, Visitor)>
        {
            typedef
                traits::make_component<
                    Domain, Director
                  , typename Grammar::template
                        result<void(Expr, State, Visitor)>::type
                  , Visitor
                >
            make_component;

            typedef typename make_component::type type;
        };

        template<typename Expr, typename State, typename Visitor>
        typename result<void(Expr, State, Visitor)>::type
        operator ()(Expr const& expr, State const& state, Visitor& visitor) const
        {
            typedef typename result<void(Expr, State, Visitor)>::make_component result;
            return result::call(Grammar()(expr, state, visitor));
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    //  A proto transform that composes a single-element component
    //  from a 1-arity proto function expression (e.g. f(x))
    ///////////////////////////////////////////////////////////////////////////
    template <typename Grammar, typename Domain, typename Director>
    struct compose_function1 : Grammar
    {
        template<typename Sig>
        struct result;

        template <typename This, typename Expr, typename State, typename Visitor>
        struct result<This(Expr, State, Visitor)>
        {
            typedef typename
                proto::result_of::arg<
                    typename proto::result_of::arg_c<Expr, 1>::type
                >::type
            arg1;

            typedef
                traits::make_component<
                    Domain, Director
                  , typename fusion::result_of::make_cons<arg1>::type
                  , Visitor
                >
            make_component;

            typedef typename make_component::type type;
        };

        template<typename Expr, typename State, typename Visitor>
        typename result<void(Expr, State, Visitor)>::type
        operator ()(Expr const& expr, State const&, Visitor&) const
        {
            typedef typename result<void(Expr, State, Visitor)>::make_component result;
            return result::call(fusion::make_cons(proto::arg(proto::arg_c<1>(expr))));
        }
    };

    //  Same as compose_function1, except that DirectorF is a meta-function to
    //  be evaluated to get the director
    template <typename Grammar, typename Domain, typename DirectorF>
    struct compose_function1_eval : Grammar
    {
        template<typename Sig>
        struct result;

        template <typename This, typename Expr, typename State, typename Visitor>
        struct result<This(Expr, State, Visitor)>
        {
            typedef typename
                proto::result_of::arg<
                    typename proto::result_of::arg_c<Expr, 0>::type
                >::type
            function;
            typedef typename
                proto::result_of::arg<
                    typename proto::result_of::arg_c<Expr, 1>::type
                >::type
            arg1;

            typedef
                traits::make_component<
                    Domain
                  , typename mpl::apply2<DirectorF, function, arg1>::type
                  , typename fusion::result_of::make_cons<arg1>::type
                  , Visitor
                >
            make_component;

            typedef typename make_component::type type;
        };

        template<typename Expr, typename State, typename Visitor>
        typename result<void(Expr, State, Visitor)>::type
        operator ()(Expr const& expr, State const&, Visitor&) const
        {
            typedef typename result<void(Expr, State, Visitor)>::make_component result;
            return result::call(
                fusion::make_cons(proto::arg(proto::arg_c<1>(expr))));
        }
    };

    //  Same as compose_function1, except that the generated component holds 
    //  not only the function argument, but the function tag as well
    template <typename Grammar, typename Domain, typename DirectorF>
    struct compose_function1_full : Grammar
    {
        template<typename Sig>
        struct result;

        template <typename This, typename Expr, typename State, typename Visitor>
        struct result<This(Expr, State, Visitor)>
        {
            typedef typename
                proto::result_of::arg<
                    typename proto::result_of::arg_c<Expr, 0>::type
                >::type
            function;
            typedef typename
                proto::result_of::arg<
                    typename proto::result_of::arg_c<Expr, 1>::type
                >::type
            arg1;

            typedef
                traits::make_component<
                    Domain
                  , typename mpl::apply2<DirectorF, function, arg1>::type
                  , typename fusion::result_of::make_list<function, arg1>::type
                  , Visitor
                >
            make_component;

            typedef typename make_component::type type;
        };

        template<typename Expr, typename State, typename Visitor>
        typename result<void(Expr, State, Visitor)>::type
        operator ()(Expr const& expr, State const& /*state*/, Visitor& /*visitor*/) const
        {
            typedef typename result<void(Expr, State, Visitor)>::make_component result;
            return result::call(fusion::make_list(
                    proto::arg(proto::arg_c<0>(expr)),
                    proto::arg(proto::arg_c<1>(expr))
                ));
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    //  A proto transform that composes a 2-element component
    //  from a 2-arity proto function expression (e.g. f(x, y))
    ///////////////////////////////////////////////////////////////////////////
    template <typename Grammar, typename Domain, typename Director>
    struct compose_function2 : Grammar
    {
        template<typename Sig>
        struct result;

        template <typename This, typename Expr, typename State, typename Visitor>
        struct result<This(Expr, State, Visitor)>
        {
            typedef typename
                proto::result_of::arg<
                    typename proto::result_of::arg_c<Expr, 1>::type
                >::type
            arg1;

            typedef typename
                proto::result_of::arg<
                    typename proto::result_of::arg_c<Expr, 2>::type
                >::type
            arg2;

            typedef
                traits::make_component<
                    Domain, Director
                  , typename fusion::result_of::make_list<arg1, arg2>::type
                  , Visitor
                >
            make_component;

            typedef typename make_component::type type;
        };

        template<typename Expr, typename State, typename Visitor>
        typename result<void(Expr, State, Visitor)>::type
        operator ()(Expr const& expr, State const& /*state*/, Visitor& /*visitor*/) const
        {
            typedef typename result<void(Expr, State, Visitor)>::make_component result;
            return result::call(fusion::make_list(
                proto::arg(proto::arg_c<1>(expr))
              , proto::arg(proto::arg_c<2>(expr))
            ));
        }
    };

    //  Same as compose_function2, except that DirectorF is a meta-function to
    //  be evaluated to get the director
    template <typename Grammar, typename Domain, typename DirectorF>
    struct compose_function2_eval : Grammar
    {
        template<typename Sig>
        struct result;

        template <typename This, typename Expr, typename State, typename Visitor>
        struct result<This(Expr, State, Visitor)>
        {
            typedef typename
                proto::result_of::arg<
                    typename proto::result_of::arg_c<Expr, 0>::type
                >::type
            function;
            typedef typename
                proto::result_of::arg<
                    typename proto::result_of::arg_c<Expr, 1>::type
                >::type
            arg1;
            typedef typename
                proto::result_of::arg<
                    typename proto::result_of::arg_c<Expr, 2>::type
                >::type
            arg2;

            typedef
                traits::make_component<
                    Domain
                  , typename mpl::apply2<DirectorF, function, arg1>::type
                  , typename fusion::result_of::make_list<arg1, arg2>::type
                  , Visitor
                >
            make_component;

            typedef typename make_component::type type;
        };

        template<typename Expr, typename State, typename Visitor>
        typename result<void(Expr, State, Visitor)>::type
        operator ()(Expr const& expr, State const& /*state*/, Visitor& /*visitor*/) const
        {
            typedef typename result<void(Expr, State, Visitor)>::make_component result;
            return result::call(fusion::make_list(
                proto::arg(proto::arg_c<1>(expr))
              , proto::arg(proto::arg_c<2>(expr))
            ));
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    //  A proto transform for directives. The directive (terminal) tag
    //  is pushed into the modifier state (the Visitor).
    ///////////////////////////////////////////////////////////////////////////
    template <typename Grammar>
    struct compose_deep_directive : Grammar
    {
        template<typename Sig>
        struct result;

        template <typename This, typename Expr, typename State, typename Visitor>
        struct result<This(Expr, State, Visitor)>
        {
            typedef typename
                add_modifier<
                    Visitor
                  , typename proto::result_of::arg<
                        typename proto::result_of::arg_c<Expr, 0>::type
                    >::type
                >::type
            modifier_type;

            typedef typename
                Grammar::template result<void(Expr, State, modifier_type)>::type
            type;
        };

        template<typename Expr, typename State, typename Visitor>
        typename result<void(Expr, State, Visitor)>::type
        operator ()(Expr const& expr, State const& state, Visitor&) const
        {
            typename add_modifier<
                Visitor
              , typename proto::result_of::arg<
                    typename proto::result_of::arg_c<Expr, 0>::type
                >::type
            >::type
            modifier;
            return Grammar()(expr, state, modifier);
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    //  A proto transform for creating double-element component meta 
    //  descriptions (proto expressions) usable for defining meta grammars.
    //  This can be used to handle constructs like: 
    //
    //    directive[p]
    //
    ///////////////////////////////////////////////////////////////////////////
    template <typename Grammar, typename Domain, typename Director>
    struct compose_subscript : Grammar
    {
        template<typename Sig>
        struct result;

        template <typename This, typename Expr, typename State, typename Visitor>
        struct result<This(Expr, State, Visitor)>
        {
            // apply all grammar transformations mandated for the whole 
            // expression
            typedef typename
                Grammar::template result<void(Expr, State, Visitor)>::type
            trans;

            // this calculates the type of the directive
            typedef typename proto::result_of::arg_c<trans, 0>::type directive;
            
            // this calculates the type of the embedded expression
            typedef typename proto::result_of::arg_c<trans, 1>::type embedded;
            
            // this is the type of the contained data
            typedef fusion::list<embedded, directive> list_type;

            typedef
                traits::make_component<Domain, Director, list_type, Visitor>
            make_component;

            typedef typename make_component::type type;
        };

        template<typename Expr, typename State, typename Visitor>
        typename result<void(Expr, State, Visitor)>::type
        operator ()(Expr const& expr, State const& state, Visitor& visitor) const
        {
            typedef result<void(Expr, State, Visitor)> apply;
            typedef typename apply::make_component result;
            typedef typename apply::list_type list_type;
            typename apply::trans trans = Grammar()(expr, state, visitor);

            return result::call(
                list_type(proto::arg_c<1>(trans), proto::arg_c<0>(trans)));
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    //  A proto transform for creating double-element component meta 
    //  descriptions (proto expressions) usable for defining meta grammars.
    //  This can be used to handle constructs like: 
    //
    //    directive(a)[p]
    //
    ///////////////////////////////////////////////////////////////////////////
    template <typename Grammar, typename Domain, typename Director>
    struct compose_subscript_function1 : Grammar
    {
        template<typename Sig>
        struct result;

        template <typename This, typename Expr, typename State, typename Visitor>
        struct result<This(Expr, State, Visitor)>
        {
            // apply all grammar transformations mandated for the whole 
            // expression
            typedef typename
                Grammar::template result<void(Expr, State, Visitor)>::type
            trans;

            // this calculates the type of the embedded expression
            typedef typename proto::result_of::arg_c<trans, 1>::type embedded;
            
            // this calculates the type of the argument of the function
            typedef typename
                proto::result_of::arg_c<
                    typename proto::result_of::arg_c<trans, 0>::type, 1
                >::type
            arg1;

            // this is the type of the contained data
            typedef fusion::list<embedded, arg1> list_type;

            typedef
                traits::make_component<
                    Domain, Director,
                    list_type,
                    Visitor
                >
            make_component;

            typedef typename make_component::type type;
        };

        template<typename Expr, typename State, typename Visitor>
        typename result<void(Expr, State, Visitor)>::type
        operator ()(Expr const& expr, State const& state, Visitor& visitor) const
        {
            typedef result<void(Expr, State, Visitor)> apply;
            typedef typename apply::make_component result;
            typedef typename apply::list_type list_type;
            typename apply::trans trans = Grammar()(expr, state, visitor);

            return result::call(list_type(
                proto::arg_c<1>(trans), 
                proto::arg_c<1>(proto::arg_c<0>(trans))));
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    //  A proto transform for creating triple-element component meta 
    //  descriptions (proto expressions) usable for defining meta grammars.
    //  This can be used to handle constructs like: 
    //
    //    directive(a, b)[p]
    //
    ///////////////////////////////////////////////////////////////////////////
    template <typename Grammar, typename Domain, typename Director>
    struct compose_subscript_function2 : Grammar
    {
        template<typename Sig>
        struct result;

        template <typename This, typename Expr, typename State, typename Visitor>
        struct result<This(Expr, State, Visitor)>
        {
            // apply all grammar transformations mandated for the whole 
            // expression
            typedef typename
                Grammar::template result<void(Expr, State, Visitor)>::type
            trans;

            // this calculates the types of the arguments of the function
            typedef typename proto::result_of::arg_c<trans, 0>::type arg0;
            typedef typename proto::result_of::arg_c<arg0, 1>::type arg1;
            typedef typename proto::result_of::arg_c<arg0, 2>::type arg2;

            // this calculates the type of the embedded expression
            typedef typename proto::result_of::arg_c<trans, 1>::type embedded;
            typedef fusion::list<embedded, arg1, arg2> list_type;

            typedef
                traits::make_component<
                    Domain, Director,
                    list_type,
                    Visitor
                >
            make_component;

            typedef typename make_component::type type;
        };

        template<typename Expr, typename State, typename Visitor>
        typename result<void(Expr, State, Visitor)>::type
        operator ()(Expr const& expr, State const& state, Visitor& visitor) const
        {
            typedef result<void(Expr, State, Visitor)> apply;
            typedef typename apply::make_component result;
            typedef typename apply::list_type list_type;
            typename apply::trans trans = Grammar()(expr, state, visitor);
            typename apply::arg0 arg0 = proto::arg_c<0>(trans);

            return result::call(list_type(
                proto::arg_c<1>(trans), proto::arg_c<1>(arg0), 
                proto::arg_c<2>(arg0)));
        }
    };

}}}

#endif
