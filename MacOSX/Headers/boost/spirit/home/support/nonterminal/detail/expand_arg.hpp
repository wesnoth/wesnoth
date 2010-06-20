/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying 
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(BOOST_SPIRIT_EXPAND_ARG_FEB_19_2007_1107AM)
#define BOOST_SPIRIT_EXPAND_ARG_FEB_19_2007_1107AM

#include <boost/mpl/bool.hpp>
#include <boost/mpl/identity.hpp>
#include <boost/mpl/eval_if.hpp>
#include <boost/utility/result_of.hpp>
#include <boost/type_traits/is_scalar.hpp>
#include <boost/variant.hpp>

namespace boost { namespace spirit { namespace detail
{
    ///////////////////////////////////////////////////////////////////////////
    template <typename Context>
    struct expand_arg
    {
        template <typename T>
        struct result_type
        {
            typedef typename
                mpl::eval_if<
                    is_scalar<T>
                  , mpl::identity<T const &>
                  , boost::result_of<T(unused_type, Context)>
                >::type
            type;
        };
        
        template <typename T>
        struct result;
        
        template <typename F, typename A0>
        struct result<F(A0)>
          : result_type<A0> {};

        template <typename F, typename A0>
        struct result<F(A0&)>
          : result_type<A0> {};

        expand_arg(Context& context)
          : context(context)
        {
        }

        template <typename T>
        typename result_type<T>::type
        call(T const& f, mpl::false_) const
        {
            return f(unused, context);
        }

        template <typename T>
        typename result_type<T>::type
        call(T const& val, mpl::true_) const
        {
            return val;
        }

        template <typename T>
        typename result_type<T>::type
        operator()(T const& x) const
        {
            return call(x, is_scalar<T>());
        }

        Context& context;
    };
    
}}}

#endif
