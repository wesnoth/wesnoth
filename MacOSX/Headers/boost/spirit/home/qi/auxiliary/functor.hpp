//  Copyright (c) 2001-2008 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_FUNCTOR_APR_01_2007_0817AM)
#define BOOST_SPIRIT_FUNCTOR_APR_01_2007_0817AM

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once      // MS compatible compilers support #pragma once
#endif

#include <boost/spirit/home/support/component.hpp>
#include <boost/spirit/home/support/unused.hpp>
#include <boost/spirit/home/support/detail/values.hpp>
#include <boost/spirit/home/support/auxiliary/functor_holder.hpp>
#include <boost/spirit/home/support/auxiliary/meta_function_holder.hpp>
#include <boost/spirit/home/qi/skip.hpp>
#include <boost/mpl/if.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/remove_const.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace boost { namespace spirit 
{ 
    namespace qi
    {
        template <typename Functor, typename ParameterMF = Functor>
        class functor_parser;
    }
    
    namespace result_of
    {
        template <typename Functor>
        struct as_parser
        {
            typedef qi::functor_parser<Functor> type;
        };

        template <typename ParameterMF, typename Functor>
        struct as_parser_mf
        {
            typedef qi::functor_parser<Functor, ParameterMF> type;
        };
    }
        
}}  // boost::spirit

///////////////////////////////////////////////////////////////////////////////
namespace boost { namespace spirit { namespace qi
{
    ///////////////////////////////////////////////////////////////////////////
    //  This struct may be used as a base class for a user defined functor
    ///////////////////////////////////////////////////////////////////////////
    struct functor_base
    {
        ///////////////////////////////////////////////////////////////////////
        //  The return value of a qi functor is always bool
        ///////////////////////////////////////////////////////////////////////
        template <typename Attribute, typename Iterator, typename Context>
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
        template <typename Iterator, typename Context>
        struct apply
        {
            typedef spirit::unused_type type;
        };
    };

    ///////////////////////////////////////////////////////////////////////////
    template <typename Functor, typename ParameterMF>
    class functor_parser
      : public proto::extends<
            typename make_functor_holder<
                functor_parser<Functor, ParameterMF> const*, 
                functor_parser<Functor, ParameterMF>
            >::type,
            functor_parser<Functor, ParameterMF>
        >
    {
    private:
        typedef functor_parser<Functor, ParameterMF> self_type;
        typedef typename
            make_functor_holder<self_type const*, self_type>::type
        functor_tag;
        typedef proto::extends<functor_tag, self_type> base_type;

    public:
        template <typename Iterator, typename Context>
        struct result 
          : mpl::apply<ParameterMF, Iterator, Context>
        {};

    private:
        // parse function just delegates to the functor supplied function
        template <typename Iterator, typename Context, typename Attribute>
        bool
        parse (Iterator& first, Iterator const& last, Context& ctx,
            Attribute& attr_) const
        {
            // create an attribute if none is supplied
            typedef typename result<Iterator, Context>::type attr_type;
            typename mpl::if_<
                is_same<typename remove_const<Attribute>::type, unused_type>,
                attr_type,
                Attribute&
            >::type
            attr = spirit::detail::make_value<attr_type>::call(attr_);

            return functor(attr, ctx, first, last);
        }

        friend struct functor_director;

    public:
        explicit functor_parser()
          : base_type(make_tag())
        {
        }

        functor_parser(Functor const& functor_)
          : base_type(make_tag()), functor(functor_)
        {
        }

        functor_parser(Functor const& functor_, ParameterMF const& mf)
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
    //  The as_parser generator function may be used to create a functor 
    //  parser from a function object (some callable item).
    //  The supplied functor needs to expose
    // 
    //    - an embedded result meta function: 
    //
    //          template <typename Attribute, typename Iterator, typename Context>
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
    //          template <typename Iterator, typename Context>
    //          struct apply
    //          {
    //              typedef unspecified type;
    //          };
    //
    //      which declares the given type as the expected attribute type for 
    //      the parser to create.
    ///////////////////////////////////////////////////////////////////////////
    template <typename Functor>
    inline typename result_of::as_parser<Functor>::type
    as_parser(Functor const& func)
    {
        return functor_parser<Functor>(func);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    //  The as_parser_mf generator function is equivalent to the function
    //  as_parser above except that the user has to explicitly specify a
    //  type exposing an embedded apply meta function declaring the expected
    //  parameter type for the generator to create.
    ///////////////////////////////////////////////////////////////////////////
    template <typename ParameterMF, typename Functor>
    inline typename result_of::as_parser_mf<ParameterMF, Functor>::type
    as_parser_mf(Functor const& func, ParameterMF const& mf)
    {
        return functor_parser<Functor, ParameterMF>(func, mf);
    }
    
    template <typename ParameterMF, typename Functor>
    inline typename result_of::as_parser_mf<ParameterMF, Functor>::type
    as_parser_mf(Functor const& func)
    {
        return functor_parser<Functor, ParameterMF>(func, ParameterMF());
    }
    
}}}

#endif
