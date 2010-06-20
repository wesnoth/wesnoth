/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(BOOST_SPIRIT_ARGUMENT_FEB_17_2007_0339PM)
#define BOOST_SPIRIT_ARGUMENT_FEB_17_2007_0339PM

#include <boost/preprocessor/repetition/repeat_from_to.hpp>
#include <boost/preprocessor/arithmetic/inc.hpp>
#include <boost/spirit/home/phoenix/core/actor.hpp>
#include <boost/spirit/home/phoenix/core/argument.hpp>
#include <boost/fusion/include/at.hpp>
#include <boost/fusion/include/size.hpp>
#include <boost/mpl/size.hpp>
#include <boost/mpl/at.hpp>

#if !defined(SPIRIT_ARG_LIMIT)
# define SPIRIT_ARG_LIMIT PHOENIX_LIMIT
#endif

#define SPIRIT_DECLARE_ARG(z, n, data)                                          \
    phoenix::actor<argument<n> > const                                          \
        BOOST_PP_CAT(_, BOOST_PP_INC(n)) = argument<n>();                       \
    phoenix::actor<attribute<n> > const                                         \
        BOOST_PP_CAT(_r, n) = attribute<n>();

namespace boost { namespace spirit
{
    namespace result_of
    {
        template <typename Sequence, int N>
        struct get_arg
        {
            typedef typename
                fusion::result_of::size<Sequence>::type
            sequence_size;

            // report invalid argument not found (N is out of bounds)
            BOOST_MPL_ASSERT_MSG(
                (N < sequence_size::value),
                index_is_out_of_bounds, ());

            typedef typename
                fusion::result_of::at_c<Sequence, N>::type
            type;

            static type call(Sequence& seq)
            {
                return fusion::at_c<N>(seq);
            }
        };

        template <typename Sequence, int N>
        struct get_arg<Sequence&, N> : get_arg<Sequence, N>
        {
        };
    }

    template <int N, typename T>
    typename result_of::get_arg<T, N>::type
    get_arg(T& val)
    {
        return result_of::get_arg<T, N>::call(val);
    }

    struct attribute_context
    {
        typedef mpl::true_ no_nullary;

        template <typename Env>
        struct result
        {
            // FIXME: is this remove_const really necessary?
            typedef typename
                remove_const<
                    typename mpl::at_c<typename Env::args_type, 0>::type
                >::type
            type;
        };

        template <typename Env>
        typename result<Env>::type
        eval(Env const& env) const
        {
            return fusion::at_c<0>(env.args());
        }
    };

    template <int N>
    struct argument
    {
        typedef mpl::true_ no_nullary;

        template <typename Env>
        struct result
        {
            typedef typename
                mpl::at_c<typename Env::args_type, 0>::type
            arg_type;

            typedef typename result_of::get_arg<arg_type, N>::type type;
        };

        template <typename Env>
        typename result<Env>::type
        eval(Env const& env) const
        {
            return get_arg<N>(fusion::at_c<0>(env.args()));
        }
    };

    template <int N>
    struct attribute
    {
        typedef mpl::true_ no_nullary;

        template <typename Env>
        struct result
        {
            typedef typename
                mpl::at_c<typename Env::args_type, 1>::type
            arg_type;

            typedef typename
                result_of::get_arg<
                    typename result_of::get_arg<arg_type, 0>::type
                  , N
                >::type
            type;
        };

        template <typename Env>
        typename result<Env>::type
        eval(Env const& env) const
        {
            return get_arg<N>(get_arg<0>(fusion::at_c<1>(env.args())));
        }
    };

    template <int N>
    struct local_var
    {
        typedef mpl::true_ no_nullary;

        template <typename Env>
        struct result
        {
            typedef typename
                mpl::at_c<typename Env::args_type, 1>::type
            arg_type;

            typedef typename
                result_of::get_arg<
                    typename result_of::get_arg<arg_type, 1>::type
                  , N
                >::type
            type;
        };

        template <typename Env>
        typename result<Env>::type
        eval(Env const& env) const
        {
            return get_arg<N>(get_arg<1>(fusion::at_c<1>(env.args())));
        }
    };

    struct lexer_state
    {
        typedef mpl::true_ no_nullary;

        template <typename Env>
        struct result
        {
            typedef typename 
                mpl::at_c<typename Env::args_type, 3>::type::state_type 
            type;
        };

        template <typename Env>
        typename result<Env>::type
        eval(Env const& env) const
        {
            return fusion::at_c<3>(env.args()).state;
        }
    };
    
    namespace arg_names
    {
    // _0 refers to the whole attribute as generated by the lhs parser
        phoenix::actor<attribute_context> const _0 = attribute_context();

    // _1, _2, ... refer to the attributes of the single components the lhs 
    // parser is composed of
        phoenix::actor<argument<0> > const _1 = argument<0>();
        phoenix::actor<argument<1> > const _2 = argument<1>();
        phoenix::actor<argument<2> > const _3 = argument<2>();

    // 'pass' may be used to make a match fail in retrospective
        phoenix::actor<phoenix::argument<2> > const pass = phoenix::argument<2>();

    // 'id' may be used in a lexer semantic action to refer to the token id 
    // of a matched token 
        phoenix::actor<phoenix::argument<1> > const id = phoenix::argument<1>();
        
    // 'state' may be used in a lexer semantic action to refer to the 
    // current lexer state 
        phoenix::actor<lexer_state> const state = lexer_state();
    
    // _val refers to the 'return' value of a rule
    // _r0, _r1, ... refer to the rule arguments
        phoenix::actor<attribute<0> > const _val = attribute<0>();
        phoenix::actor<attribute<0> > const _r0 = attribute<0>();
        phoenix::actor<attribute<1> > const _r1 = attribute<1>();
        phoenix::actor<attribute<2> > const _r2 = attribute<2>();

    //  Bring in the rest of the arguments and attributes (_4 .. _N+1), using PP
        BOOST_PP_REPEAT_FROM_TO(
            3, SPIRIT_ARG_LIMIT, SPIRIT_DECLARE_ARG, _)

    // _a, _b, ... refer to the local variables of a rule
        phoenix::actor<local_var<0> > const _a = local_var<0>();
        phoenix::actor<local_var<1> > const _b = local_var<1>();
        phoenix::actor<local_var<2> > const _c = local_var<2>();
        phoenix::actor<local_var<3> > const _d = local_var<3>();
        phoenix::actor<local_var<4> > const _e = local_var<4>();
        phoenix::actor<local_var<5> > const _f = local_var<5>();
        phoenix::actor<local_var<6> > const _g = local_var<6>();
        phoenix::actor<local_var<7> > const _h = local_var<7>();
        phoenix::actor<local_var<8> > const _i = local_var<8>();
        phoenix::actor<local_var<9> > const _j = local_var<9>();
    }
}}

#undef SPIRIT_DECLARE_ARG
#endif
