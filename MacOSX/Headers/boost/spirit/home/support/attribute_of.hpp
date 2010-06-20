/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(BOOST_SPIRIT_ATTRIBUTE_OF_JAN_29_2007_0954AM)
#define BOOST_SPIRIT_ATTRIBUTE_OF_JAN_29_2007_0954AM

#include <boost/spirit/home/support/component.hpp>
#include <boost/spirit/home/support/unused.hpp>

namespace boost { namespace spirit { namespace traits
{
    template <
        typename Domain, typename T
      , typename Context, typename Iterator = unused_type>
    struct attribute_of :
        attribute_of<
            Domain
          , typename result_of::as_component<Domain, T>::type
          , Context
          , Iterator
        >
    {
    };

    template <
        typename Domain, typename Director, typename Elements
      , typename Context, typename Iterator>
    struct attribute_of<
            Domain
          , component<Domain, Director, Elements>
          , Context
          , Iterator
        >
    {
        typedef
            component<Domain, Director, Elements>
        component_type;

        typedef typename Director::template
            attribute<component_type, Context, Iterator>::type
        type;
    };

}}}

#endif
