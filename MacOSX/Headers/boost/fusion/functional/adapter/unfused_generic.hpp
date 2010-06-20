/*=============================================================================
    Copyright (c) 2006-2007 Tobias Schwinger
  
    Use modification and distribution are subject to the Boost Software 
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt).
==============================================================================*/

#if !defined(BOOST_FUSION_FUNCTIONAL_ADAPTER_UNFUSED_GENERIC_HPP_INCLUDED)
#if !defined(BOOST_PP_IS_ITERATING)

#include <boost/config.hpp>
#include <boost/detail/workaround.hpp>

#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/iteration/iterate.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/enum_binary_params.hpp>
#include <boost/preprocessor/facilities/intercept.hpp>

#include <boost/fusion/container/vector/vector.hpp>

#include <boost/fusion/functional/adapter/limits.hpp>
#include <boost/fusion/functional/adapter/detail/access.hpp>

#include <boost/utility/result_of.hpp>

namespace boost { namespace fusion
{
    template <class Function> class unfused_generic;

    //----- ---- --- -- - -  -   -

    template <class Function> 
    class unfused_generic
    {
        Function fnc_transformed;

        typedef typename detail::qf_c<Function>::type function_c;
        typedef typename detail::qf<Function>::type function;

        typedef typename detail::call_param<Function>::type func_const_fwd_t;

      public:

        inline explicit unfused_generic(func_const_fwd_t f = Function())
            : fnc_transformed(f)
        { }

        template <typename Sig>
        struct result;

        typedef typename boost::result_of<
            function_c(fusion::vector0 &) >::type call_const_0_result;

        inline call_const_0_result operator()() const
        {
            fusion::vector0 arg;
            return this->fnc_transformed(arg);
        }

        typedef typename boost::result_of< 
            function (fusion::vector0 &) >::type call_0_result;

        inline call_0_result operator()() 
        {
            fusion::vector0 arg;
            return this->fnc_transformed(arg);
        }

        #define BOOST_FUSION_CODE(tpl_params,arg_types,params,args)             \
        template <tpl_params>                                                  \
        inline typename boost::result_of<function_c(                           \
            BOOST_PP_CAT(fusion::vector,N)<arg_types> & )>::type               \
        operator()(params) const                                               \
        {                                                                      \
            BOOST_PP_CAT(fusion::vector,N)<arg_types> arg(args);               \
            return this->fnc_transformed(arg);                                 \
        }                                                                      \
        template <tpl_params>                                                  \
        inline typename boost::result_of<function(                             \
            BOOST_PP_CAT(fusion::vector,N)<arg_types> & )>::type               \
        operator()(params)                                                     \
        {                                                                      \
            BOOST_PP_CAT(fusion::vector,N)<arg_types> arg(args);               \
            return this->fnc_transformed(arg);                                 \
        }

        #define  BOOST_PP_INDIRECT_SELF                                        \
            <boost/fusion/functional/adapter/unfused_generic.hpp>
        #define  BOOST_PP_FILENAME_1                                           \
            <boost/fusion/functional/adapter/detail/pow2_explode.hpp>
        #define  BOOST_PP_ITERATION_LIMITS                                     \
            (1,BOOST_FUSION_UNFUSED_GENERIC_MAX_ARITY)
        #define  N BOOST_PP_ITERATION_1
        #include BOOST_PP_ITERATE()
        #undef   N

        #undef BOOST_FUSION_CODE
    };
}}

namespace boost 
{
    template<class F>
    struct result_of<boost::fusion::unfused_generic<F> const ()>
    {
        typedef typename boost::fusion::unfused_generic<F>::call_const_0_result type;
    };
    template<class F>
    struct result_of<boost::fusion::unfused_generic<F>()>
    {
        typedef typename boost::fusion::unfused_generic<F>::call_0_result type;
    };
}

#define BOOST_FUSION_FUNCTIONAL_ADAPTER_UNFUSED_GENERIC_HPP_INCLUDED
#else // defined(BOOST_PP_IS_ITERATING)
///////////////////////////////////////////////////////////////////////////////
//
//  Preprocessor vertical repetition code
//
///////////////////////////////////////////////////////////////////////////////

#include <boost/fusion/functional/adapter/detail/pt_def.hpp>

#if BOOST_PP_SLOT_1() == 0 
        template <class Self, BOOST_PP_ENUM_PARAMS(N,typename T)>
        struct result
            < Self const (BOOST_PP_ENUM_PARAMS(N,T)) >
            : boost::result_of<function_c(
                BOOST_PP_CAT(fusion::vector,N)< BOOST_PP_ENUM_BINARY_PARAMS(N,
                   typename detail::gref<T,>::type BOOST_PP_INTERCEPT) > & )>
        { };

        template <class Self, BOOST_PP_ENUM_PARAMS(N,typename T)>
        struct result
            < Self(BOOST_PP_ENUM_PARAMS(N,T)) >
            : boost::result_of<function(
                BOOST_PP_CAT(fusion::vector,N)< BOOST_PP_ENUM_BINARY_PARAMS(N,
                   typename detail::gref<T,>::type BOOST_PP_INTERCEPT) > & )>
        { };
#endif

#if BOOST_WORKAROUND(BOOST_MSVC,BOOST_TESTED_AT(1400)) 
        template <BOOST_PP_ENUM_PARAMS(N,typename T)>
        inline typename boost::result_of<function_c(
            BOOST_PP_CAT(fusion::vector,N)<BOOST_PP_ENUM_PARAMS(N,PT)> & )>::type
        operator()(BOOST_PP_ENUM_BINARY_PARAMS(N,PT,a)) const
        {
            BOOST_PP_CAT(fusion::vector,N)<BOOST_PP_ENUM_PARAMS(N,PT)>
                arg(BOOST_PP_ENUM_PARAMS(N,a));
            return this->fnc_transformed(arg);
        }
        template <BOOST_PP_ENUM_PARAMS(N,typename T)>
        inline typename boost::result_of<function(
            BOOST_PP_CAT(fusion::vector,N)<BOOST_PP_ENUM_PARAMS(N,PT)> & )>::type
        operator()(BOOST_PP_ENUM_BINARY_PARAMS(N,PT,a))
        {
            BOOST_PP_CAT(fusion::vector,N)<BOOST_PP_ENUM_PARAMS(N,PT)>
                arg(BOOST_PP_ENUM_PARAMS(N,a));
            return this->fnc_transformed(arg);
        }
#else
        BOOST_FUSION_CODE(BOOST_PP_ENUM_PARAMS(N,typename T),
            BOOST_PP_ENUM_PARAMS(N,PT), BOOST_PP_ENUM_BINARY_PARAMS(N,PT,a),
            BOOST_PP_ENUM_PARAMS(N,a) )
        // ...generates uglier code but is faster - it caches ENUM_*
#endif

#include <boost/fusion/functional/adapter/detail/pt_undef.hpp>

#endif // defined(BOOST_PP_IS_ITERATING)
#endif

