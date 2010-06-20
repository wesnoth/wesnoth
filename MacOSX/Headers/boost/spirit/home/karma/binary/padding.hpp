//  Copyright (c) 2001-2008 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_KARMA_PADDING_MAY_06_2008_0436PM)
#define BOOST_SPIRIT_KARMA_PADDING_MAY_06_2008_0436PM

#include <boost/lexical_cast.hpp>

#include <boost/spirit/home/karma/domain.hpp>
#include <boost/spirit/home/karma/delimit.hpp>
#include <boost/spirit/home/support/unused.hpp>
#include <boost/fusion/include/at.hpp>
#include <boost/spirit/home/karma/detail/generate_to.hpp>

namespace boost { namespace spirit { namespace karma
{
    struct binary_padding_director
    {
        template <typename Component, typename Context, typename Unused>
        struct attribute
        {
            typedef unused_type type;
        };

        template <typename Component, typename OutputIterator, 
            typename Context, typename Delimiter, typename Parameter>
        static bool 
        generate(Component const& component, OutputIterator& sink, 
            Context& ctx, Delimiter const& d, Parameter const&) 
        {
            std::size_t padbytes = fusion::at_c<0>(component.elements);
            std::size_t count = sink.get_out_count() % padbytes;
            
            if (count)
                count = padbytes - count;
                
            bool result = true;
            while (result && count-- != 0)
                result = detail::generate_to(sink, 0);

            karma::delimit(sink, d);      // always do post-delimiting
            return result;
        }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            return std::string("pad(") +
                boost::lexical_cast<std::string>(
                    fusion::at_c<0>(component.elements)) +
                ")";
        }
    };

}}}

#endif
