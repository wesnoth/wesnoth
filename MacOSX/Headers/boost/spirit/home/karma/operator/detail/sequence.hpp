//  Copyright (c) 2001-2008 Hartmut Kaiser
//  Copyright (c) 2001-2007 Joel de Guzman
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(SPIRIT_KARMA_SEQUENCE_FEB_28_2007_0249PM)
#define SPIRIT_KARMA_SEQUENCE_FEB_28_2007_0249PM

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once      // MS compatible compilers support #pragma once
#endif

#include <boost/spirit/home/support/unused.hpp>

namespace boost { namespace spirit { namespace karma { namespace detail
{
    template <typename OutputIterator, typename Context, typename Delimiter>
    struct sequence_generate
    {
        sequence_generate(OutputIterator& sink_, Context& context_, 
              Delimiter const& delim_)
          : sink(sink_), ctx(context_), delim(delim_) 
        {
        }
        
        template <typename Component, typename Parameter>
        bool operator()(Component const& component, Parameter& p)
        {
            // return true if any of the generators fail
            typedef typename Component::director director;
            return !director::generate(component, sink, ctx, delim, p);
        }

        template <typename Component>
        bool operator()(Component const& component)
        {
            // return true if any of the generators fail
            typedef typename Component::director director;
            return !director::generate(component, sink, ctx, delim, unused);
        }

        OutputIterator& sink;
        Context& ctx;
        Delimiter const& delim;
    };
    
}}}}  // namespace boost::spirit::karma::detail

#endif
