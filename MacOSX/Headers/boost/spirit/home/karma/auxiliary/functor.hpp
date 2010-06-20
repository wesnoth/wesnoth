//  Copyright (c) 2001-2008 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_KARMA_FUNCTOR_APR_01_2007_1038AM)
#define BOOST_SPIRIT_KARMA_FUNCTOR_APR_01_2007_1038AM

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once      // MS compatible compilers support #pragma once
#endif

#include <boost/spirit/home/support/component.hpp>
#include <boost/spirit/home/support/unused.hpp>
#include <boost/spirit/home/support/auxiliary/functor_holder.hpp>
#include <boost/spirit/home/support/auxiliary/meta_function_holder.hpp>
#include <boost/spirit/home/support/detail/values.hpp>
#include <boost/spirit/home/karma/delimit.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/lambda.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/remove_const.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace boost { namespace spirit 
{ 
    namespace karma
    {
        template <typename Functor, typename ParameterMF = Functor>
        class functor_generator;
    }
    
    namespace result_of
    {
        template <typename Functor>
        struct as_generator
        {
            typedef karma::functor_generator<Functor> type;
        };

        template <typename Functor, typename ParameterMF>
        struct as_generator_mf
        {
            typedef karma::functor_generator<Functor, ParameterMF> type;
        };
    }

}}  // boost::spirit

///////////////////////////////////////////////////////////////////////////////
namespace boost { namespace spirit { namespace karma
{
    ///////////////////////////////////////////////////////////////////////////
    //  This struct may be used as a base class for a user defined functor
    ///////////////////////////////////////////////////////////////////////////
    struct functor_base
    {
        ///////////////////////////////////////////////////////////////////////
        //  The return value of a karma functor is always bool
        ///////////////////////////////////////////////////////////////////////
        template <typename Parameter, typename OutputIterator>
        struct result
        {
            typedef bool type;
        };

// FIXME: It will be possible to specify the return value as a typedef, but for 
//        that Phoenix will have to be fixed.
//         typedef bool result_type;
        
        ///////////////////////////////////////////////////////////////////////
        //  The expected parameter type of a functor has to be defined using a
        //  embedded apply metafunction. Normally this will be overloaded by 
        //  the derived class, but the default is unused type.
        ///////////////////////////////////////////////////////////////////////
        template <typename Context>
        struct apply
        {
            typedef spirit::unused_type type;
        };
    };
    
    ///////////////////////////////////////////////////////////////////////////
    //  The functor generator template may be used to create new generators
    //  without having to dig into the implementation details of Karma
    ///////////////////////////////////////////////////////////////////////////
    template <typename Functor, typename ParameterMF>
    class functor_generator
      : public proto::extends<
            typename make_functor_holder<
                functor_generator<Functor, ParameterMF> const*, 
                functor_generator<Functor, ParameterMF>
            >::type,
            functor_generator<Functor, ParameterMF>
        > 
    {
    private:
        typedef functor_generator<Functor, ParameterMF> self_type;
        typedef typename 
            make_functor_holder<self_type const*, self_type>::type 
        functor_tag;
        typedef proto::extends<functor_tag, self_type> base_type;

    public:
        template <typename Context>
        struct result 
          : mpl::apply<ParameterMF, Context>
        {};

    private:
        // generate function just delegates to the functor supplied function
        template <typename OutputIterator, typename Context, typename Parameter>
        bool 
        generate (OutputIterator& sink, Context& ctx, Parameter const& p) const
        {
            // create an attribute if none is supplied
            typedef typename result<Context>::type parameter_type;
            typename mpl::if_<
                is_same<typename remove_const<Parameter>::type, unused_type>,
                parameter_type,
                Parameter const&
            >::type
            param = spirit::detail::make_value<parameter_type>::call(p);

            return functor(param, ctx, sink);
        }

        friend struct functor_director;
        
    public:
        explicit functor_generator()
          : base_type(make_tag())
        {
        }

        functor_generator(Functor const& functor_)
          : base_type(make_tag()), functor(functor_)
        {
        }

        functor_generator(Functor const& functor_, ParameterMF const& mf)
          : base_type(make_tag()), functor(functor_), mf_(mf)
        {
        }

    private:
        functor_tag make_tag() const
        {
            functor_tag xpr = {{ this }};
            return xpr;
        }
        
        Functor functor;
        meta_function_holder<Functor, ParameterMF> mf_;
    };

    ///////////////////////////////////////////////////////////////////////////
    //  The as_generator generator function may be used to create a functor 
    //  generator from a function object (some callable item).
    //  The supplied functor needs to expose
    // 
    //    - an embedded result meta function: 
    //
    //          template <typename Parameter, typename OutputIterator>
    //          struct result
    //          {
    //              typedef bool type;
    //          };
    //
    //      which declares 'bool' as the result type of the defined function
    //      operator and
    //
    //    - an embedded apply meta function:
    //
    //          template <typename Context>
    //          struct apply
    //          {
    //              typedef unspecified type;
    //          };
    //
    //      which declares the given type as the expected attribute type for 
    //      the generator to create.
    ///////////////////////////////////////////////////////////////////////////
    template <typename Functor>
    inline typename result_of::as_generator<Functor>::type
    as_generator(Functor const& func)
    {
        return functor_generator<Functor>(func);
    }

    ///////////////////////////////////////////////////////////////////////////
    //  The as_generator_mf generator function is equivalent to the function
    //  as_generator above except that the user has to explicitly specify a
    //  type exposing an embedded apply meta function declaring the expected
    //  parameter type for the generator to create.
    ///////////////////////////////////////////////////////////////////////////
    template <typename ParameterMF, typename Functor>
    inline typename result_of::as_generator_mf<Functor, ParameterMF>::type
    as_generator_mf(Functor const& func, ParameterMF const& mf)
    {
        return functor_generator<Functor, ParameterMF>(func, mf);
    }

    template <typename ParameterMF, typename Functor>
    inline typename result_of::as_generator_mf<Functor, ParameterMF>::type
    as_generator_mf(Functor const& func)
    {
        return functor_generator<Functor, ParameterMF>(func, ParameterMF());
    }

}}}

#endif
