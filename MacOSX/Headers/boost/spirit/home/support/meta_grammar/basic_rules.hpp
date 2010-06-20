/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(BOOST_SPIRIT_BASIC_RULES_JAN_14_2007_1222PM)
#define BOOST_SPIRIT_BASIC_RULES_JAN_14_2007_1222PM

#include <boost/spirit/home/support/meta_grammar/grammar.hpp>
#include <boost/spirit/home/support/meta_grammar/basic_transforms.hpp>
#include <boost/spirit/home/support/component.hpp>
#include <boost/fusion/include/cons.hpp>
#include <boost/xpressive/proto/proto.hpp>
#include <boost/xpressive/proto/transform.hpp>
#include <boost/mpl/identity.hpp>

namespace boost { namespace spirit { namespace meta_grammar
{
    namespace detail
    {
        template <typename Director>
        struct director_identity
        {
            template <typename>
            struct apply : mpl::identity<Director> {};
        };
    }

    ///////////////////////////////////////////////////////////////////////////
    //  A proto rule that composes an empty (the terminal is not saved in
    //  the elements tuple) terminal spirit::component given a domain,
    //  a proto-tag and a director.  Example:
    //
    //      a
    //
    ///////////////////////////////////////////////////////////////////////////
    template <typename Domain, typename Tag, typename Director>
    struct empty_terminal_rule
    : compose_empty<
            proto::terminal<Tag>
          , Domain
          , detail::director_identity<Director>
        >
    {};

    ///////////////////////////////////////////////////////////////////////////
    //  A proto rule that composes a non-empty (the terminal is saved in
    //  the elements tuple) terminal spirit::component given a domain,
    //  a proto-tag and a director.  Example:
    //
    //      a
    //
    ///////////////////////////////////////////////////////////////////////////
    template <typename Domain, typename Tag, typename Director>
    struct terminal_rule
    : compose_single<
            proto::terminal<Tag>
          , Domain
          , detail::director_identity<Director>
        >
    {};

    ///////////////////////////////////////////////////////////////////////////
    //  A proto rule that composes a 1-element spirit::component given a
    //  domain, a proto-tag and a director. No folding takes place. Example:
    //
    //      +a
    //
    ///////////////////////////////////////////////////////////////////////////
    template <
        typename Domain, typename Tag, typename Director
      , typename SubjectGrammar = proto::_
    >
    struct unary_rule
    : compose_single<
            proto::unary_expr<
                Tag
              , SubjectGrammar
            >
          , Domain
          , detail::director_identity<Director>
        >
    {};

    ///////////////////////////////////////////////////////////////////////////
    //  A proto rule that composes a 2-element spirit::component given a
    //  domain, a proto-tag and a director. No folding takes place. Example:
    //
    //      a - b
    //
    ///////////////////////////////////////////////////////////////////////////
    template <
        typename Domain, typename Tag, typename Director
      , typename LeftGrammar = proto::_, typename RightGrammar = proto::_
    >
    struct binary_rule
    : compose_double<
            proto::binary_expr<
                Tag
              , LeftGrammar
              , RightGrammar
            >
          , Domain
          , detail::director_identity<Director>
        >
    {};

    ///////////////////////////////////////////////////////////////////////////
    //  A proto rule that composes a 3-element spirit::component given a
    //  domain, a proto-tag and a director. No folding takes place. Example:
    //
    //      if_else(cond_expr,true_exp,false_expr)
    //
    ///////////////////////////////////////////////////////////////////////////

    template <
    typename Domain, typename Tag, typename Director
    , typename Grammar0 = proto::_, typename Grammar1 = proto::_, typename Grammar2 = proto::_
    >
    struct ternary_rule
    : compose_triple<
            proto::nary_expr<
                Tag
              , Grammar0
              , Grammar1
              , Grammar2
            >
          , Domain
          , detail::director_identity<Director>
        >
    {};

    ///////////////////////////////////////////////////////////////////////////
    //  A proto rule that composes a 1-element spirit::component from a
    //  binary expression. Only the RHS is stored.
    //
    //      a[b]
    //
    ///////////////////////////////////////////////////////////////////////////
    template <
        typename Domain, typename Tag, typename Director
      , typename LeftGrammar = proto::_, typename RightGrammar = proto::_
    >
    struct binary_rule_rhs
    : compose_right<
            proto::binary_expr<
                Tag
              , LeftGrammar
              , RightGrammar
            >
          , Domain
          , detail::director_identity<Director>
        >
    {};

    ///////////////////////////////////////////////////////////////////////////
    //  A proto rule that composes a multi-element spirit::component given a
    //  domain, a proto-tag and a director. All like-operators are folded
    //  into one. Example:
    //
    //      a | b | c
    //
    ///////////////////////////////////////////////////////////////////////////
    template <
        typename Domain, typename Tag, typename Director
      , typename Grammar = proto::_
    >
    struct binary_rule_flat
    : compose_list<
            proto::when<
                proto::binary_expr<Tag, Grammar, Grammar>
              , proto::reverse_fold_tree<
                    proto::_
                  , fusion::nil()
                  , fusion::cons<Grammar, proto::_state>(Grammar, proto::_state)
                >
            >
          , Domain
          , Director
        >
    {};

    ///////////////////////////////////////////////////////////////////////////
    //  A proto rule that composes a 1-element function spirit::component
    //  given a domain, a proto-tag and a director. Example:
    //
    //      f(a)
    ///////////////////////////////////////////////////////////////////////////
    template <
        typename Domain, typename Tag
      , typename Director, typename ArgGrammar = proto::_>
    struct function1_rule
    : compose_function1<
            proto::function<proto::terminal<Tag>, ArgGrammar>
          , Domain
          , Director
        >
    {};

    ///////////////////////////////////////////////////////////////////////////
    //  A proto rule that composes a 2-element function spirit::component
    //  given a domain, a proto-tag and a director. Example:
    //
    //      f(a, b)
    ///////////////////////////////////////////////////////////////////////////
    template <
        typename Domain, typename Tag
      , typename Director, typename ArgGrammar = proto::_>
    struct function2_rule
    : compose_function2<
            proto::function<
                proto::terminal<Tag>
              , ArgGrammar
              , ArgGrammar
            >
          , Domain
          , Director
        >
    {};

    ///////////////////////////////////////////////////////////////////////////
    //  A proto rule for directives. The directive (terminal) tag
    //  is pushed into the modifier state (the Visitor). Example:
    //
    //      directive[a]
    ///////////////////////////////////////////////////////////////////////////
    template <typename Tag, typename SubjectGrammar = proto::_>
    struct deep_directive_meta_grammar
    : meta_grammar::compose_deep_directive<
            proto::when<
                proto::subscript<proto::terminal<Tag>, SubjectGrammar>
              , proto::call<SubjectGrammar(proto::_right)>
            >
        >
    {};

    ///////////////////////////////////////////////////////////////////////////
    //  A proto rule that composes a 2-element spirit::component given a
    //  domain, a proto-tag, a director, and an embedded grammar. 
    //  Example:
    //
    //      directive[p]
    //
    //  The difference to deep_directive_meta_grammar is that it stores both
    //  parts of the expression without modifying the modifier state
    //
    ///////////////////////////////////////////////////////////////////////////
    template <
        typename Domain, typename Tag,
        typename Director, typename EmbeddedGrammar = proto::_
    >
    struct subscript_rule
      : compose_subscript<
            proto::binary_expr<
                proto::tag::subscript, 
                proto::terminal<Tag>, 
                EmbeddedGrammar
            >,
            Domain,
            Director
        >
    {};

    ///////////////////////////////////////////////////////////////////////////
    //  A proto rule that composes a 2-element spirit::component given a
    //  domain, a proto-tag, a director, an argument and an embedded grammar. 
    //  Example:
    //
    //      directive(a)[p]
    //
    ///////////////////////////////////////////////////////////////////////////
    template <
        typename Domain, typename Tag,
        typename Director, typename ArgGrammar = proto::_, 
        typename EmbeddedGrammar = proto::_
    >
    struct subscript_function1_rule
      : compose_subscript_function1<
            proto::binary_expr<
                proto::tag::subscript,
                proto::function<proto::terminal<Tag>, ArgGrammar>,
                EmbeddedGrammar
            >,
            Domain,
            Director
        >
    {};

    ///////////////////////////////////////////////////////////////////////////
    //  A proto rule that composes a 3-element spirit::component given a
    //  domain, a proto-tag, a director, two arguments and an embedded grammar. 
    //  Example:
    //
    //      directive(a, b)[p]
    //
    ///////////////////////////////////////////////////////////////////////////
    template <
        typename Domain, typename Tag,
        typename Director, typename Arg1Grammar = proto::_, 
        typename Arg2Grammar = proto::_, typename EmbeddedGrammar = proto::_
    >
    struct subscript_function2_rule
      : compose_subscript_function2<
            proto::binary_expr<
                proto::tag::subscript,
                proto::function<proto::terminal<Tag>, Arg1Grammar, Arg2Grammar>,
                EmbeddedGrammar
            >,
            Domain,
            Director
        >
    {};

}}}

#endif
