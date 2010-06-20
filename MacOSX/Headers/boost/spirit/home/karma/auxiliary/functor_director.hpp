//  Copyright (c) 2001-2008 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_KARMA_FUNCTOR_DIRECTOR_APR_01_2007_1041AM)
#define BOOST_SPIRIT_KARMA_FUNCTOR_DIRECTOR_APR_01_2007_1041AM

#include <boost/spirit/home/support/auxiliary/functor_holder.hpp>
#include <boost/spirit/home/support/component.hpp>
#include <boost/spirit/home/karma/domain.hpp>

namespace boost { namespace spirit { namespace karma
{
    // this is the director for all functor generators 
    struct functor_director
    {
        // expected value type of the generator
        template <typename Component, typename Context, typename Unused>
        struct attribute
        {
            typedef typename 
                result_of::subject<Component>::type::functor_holder
            functor_holder;
            typedef typename functor_holder::functor_type functor_type;
            
            typedef typename
                functor_type::template result<Context>::type 
            type;
        };

        // generate functionality, delegates back to the corresponding functor
        template <typename Component, typename OutputIterator, 
            typename Context, typename Delimiter, typename Parameter>
        static bool 
        generate(Component const& component, OutputIterator& sink, 
            Context& ctx, Delimiter const& d, Parameter const& param) 
        {
            bool result = subject(component).held->generate(sink, ctx, param);
            karma::delimit(sink, d);           // always do post-delimiting 
            return result;
        }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            return "functor";
        }
    };
    
}}}

#endif
