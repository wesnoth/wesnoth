/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman
    Copyright (c) 2001-2008 Hartmut Kaiser
    http://spirit.sourceforge.net/

    Distributed under the Boost Software License, Version 1.0. (See accompanying 
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(BOOST_SPIRIT_CONTAINER_FEB_06_2007_1001AM)
#define BOOST_SPIRIT_CONTAINER_FEB_06_2007_1001AM

#include <boost/spirit/home/support/unused.hpp>
#include <boost/detail/iterator.hpp> // for boost::detail::iterator_traits

namespace boost { namespace spirit { namespace container
{
    ///////////////////////////////////////////////////////////////////////////
    //  This file contains some container utils for stl containers. The
    //  utilities provided also accept spirit's unused_type; all no-ops.
    //  Compiler optimization will easily strip these away.
    ///////////////////////////////////////////////////////////////////////////

    namespace result_of 
    {
        template <typename Container>
        struct value
        {
            typedef typename Container::value_type type;
        };

        template <>
        struct value<unused_type>
        {
            typedef unused_type type;
        };

        template <>
        struct value<unused_type const>
        {
            typedef unused_type type;
        };

        template <typename Container>
        struct iterator
        {
            typedef typename Container::iterator type;
        };
        
        template <typename Container>
        struct iterator<Container const>
        {
            typedef typename Container::const_iterator type;
        };
        
        template <>
        struct iterator<unused_type>
        {
            typedef unused_type const* type;
        };

        template <>
        struct iterator<unused_type const>
        {
            typedef unused_type const* type;
        };
    }
    
    ///////////////////////////////////////////////////////////////////////////
    template <typename Container, typename T>
    inline void push_back(Container& c, T const& val)
    {
        c.push_back(val);
    }

    template <typename Container>
    inline void push_back(Container&, unused_type)
    {
    }
    
    template <typename T>
    inline void push_back(unused_type, T const&)
    {
    }
    
    inline void push_back(unused_type, unused_type)
    {
    }
    
    ///////////////////////////////////////////////////////////////////////////
    template <typename Container>
    inline typename result_of::iterator<Container>::type
    begin(Container& c)
    {
        return c.begin();
    }
    
    template <typename Container>
    inline typename result_of::iterator<Container const>::type
    begin(Container const& c)
    {
        return c.begin();
    }
    
    inline unused_type const*
    begin(unused_type)
    {
        return &unused;
    }
    
    template <typename Container>
    inline typename result_of::iterator<Container>::type
    end(Container& c)
    {
        return c.end();
    }

    template <typename Container>
    inline typename result_of::iterator<Container const>::type
    end(Container const& c)
    {
        return c.end();
    }

    inline unused_type const*
    end(unused_type)
    {
        return &unused;
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename Iterator>
    inline typename boost::detail::iterator_traits<Iterator>::value_type
    deref(Iterator& it)
    {
        return *it;
    }
    
    inline unused_type
    deref(unused_type*)
    {
        return unused;
    }
    
    inline unused_type
    deref(unused_type const*)
    {
        return unused;
    }
    
    ///////////////////////////////////////////////////////////////////////////
    template <typename Iterator>
    inline Iterator
    next(Iterator& it)
    {
        return ++it;
    }
    
    inline unused_type
    next(unused_type*)
    {
        return &unused;
    }
        
    inline unused_type
    next(unused_type const*)
    {
        return &unused;
    }
        
    ///////////////////////////////////////////////////////////////////////////
    template <typename Iterator>
    inline bool
    compare(Iterator const& it1, Iterator const& it2)
    {
        return it1 == it2;
    }
    
    inline bool
    compare(unused_type*, unused_type*)
    {
        return true;
    }
        
}}}

#endif
