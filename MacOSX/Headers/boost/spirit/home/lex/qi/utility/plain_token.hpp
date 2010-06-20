//  Copyright (c) 2001-2008 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_LEX_PLAIN_TOKEN_NOV_11_2007_0451PM)
#define BOOST_SPIRIT_LEX_PLAIN_TOKEN_NOV_11_2007_0451PM

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once      // MS compatible compilers support #pragma once
#endif

#include <boost/lexical_cast.hpp>
#include <boost/xpressive/proto/proto.hpp>
#include <boost/range/iterator_range.hpp>

namespace boost { namespace spirit { namespace qi
{
    ///////////////////////////////////////////////////////////////////////////
    struct plain_token
    {
        template <typename Component, typename Context, typename Iterator>
        struct attribute
        {
            typedef typename Iterator::base_iterator_type iterator_type;
            typedef iterator_range<iterator_type> type;
        };

        template <
            typename Component
          , typename Iterator, typename Context
          , typename Skipper, typename Attribute>
        static bool parse(
            Component const& component
          , Iterator& first, Iterator const& last
          , Context& context, Skipper const& skipper
          , Attribute& attr)
        {
            qi::skip(first, last, skipper);   // always do a pre-skip
            
            if (first != last) {
                // simply match the token id with the id this component has 
                // been initialized with
                
                typedef typename 
                    boost::detail::iterator_traits<Iterator>::value_type 
                token_type;

                token_type &t = *first;
                if (fusion::at_c<0>(component.elements) == t.id()) {
                    qi::detail::assign_to(t, attr);
                    ++first;
                    return true;
                }
            }
            return false;
        }
        
        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            std::string result("token(\"");
            result += lexical_cast<std::string>(
                fusion::at_c<0>(component.elements));
            result += "\")";
            return result;
        }
    };

}}}

#endif
