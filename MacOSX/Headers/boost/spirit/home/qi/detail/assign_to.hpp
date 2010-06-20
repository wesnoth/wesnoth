/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman
    Copyright (c) 2001-2008 Hartmut Kaiser
    http://spirit.sourceforge.net/

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(BOOST_SPIRIT_ASSIGN_TO_APR_16_2006_0812PM)
#define BOOST_SPIRIT_ASSIGN_TO_APR_16_2006_0812PM

#include <boost/spirit/home/qi/detail/construct_fwd.hpp>
#include <boost/spirit/home/support/unused.hpp>
#include <boost/ref.hpp>

namespace boost { namespace spirit { namespace qi { namespace detail
{
    namespace construct_ 
    {
        ///////////////////////////////////////////////////////////////////////
        //  This is used to allow to overload of the attribute creation for 
        //  arbitrary types
        ///////////////////////////////////////////////////////////////////////
        template <typename Attribute, typename Iterator>
        inline void 
        construct(Attribute& attr, Iterator const& first, Iterator const& last)
        {
            attr = Attribute(first, last);
        }
        
        template <typename Attribute, typename T>
        inline void 
        construct(Attribute& attr, T const& val)
        {
            attr = val;
        }
        
        template <typename Attribute, typename T>
        inline void 
        construct(Attribute& attr, T& val)
        {
            attr = val;
        }

        template <typename Attribute, typename T>
        inline void 
        construct(reference_wrapper<Attribute> attr, T const& val)
        {
            attr = val;
        }
        
        template <typename Attribute, typename T>
        inline void 
        construct(reference_wrapper<Attribute> attr, T& val)
        {
            attr = val;
        }
    }
    
    ///////////////////////////////////////////////////////////////////////////
    //  This file contains assignment utilities. The utilities provided also
    //  accept spirit's unused_type; all no-ops. Compiler optimization will
    //  easily strip these away.
    ///////////////////////////////////////////////////////////////////////////

    template <typename Iterator, typename Attribute>
    inline void
    assign_to(Iterator const& first, Iterator const& last, Attribute& attr)
    {
        using namespace construct_;
        construct(attr, first, last);
    }

    template <typename Iterator>
    inline void
    assign_to(Iterator const& /*first*/, Iterator const& /*last*/, unused_type)
    {
    }

    template <typename T, typename Attribute>
    inline void
    assign_to(T const& val, Attribute& attr)
    {
        using namespace construct_;
        construct(attr, val);
    }

    template <typename T, typename Attribute>
    inline void
    assign_to(T& val, Attribute& attr)
    {
        using namespace construct_;
        construct(attr, val);
    }

    template <typename T, typename Attribute>
    inline void
    assign_to(T const& val, reference_wrapper<Attribute> attr)
    {
        using namespace construct_;
        construct(attr, val);
    }

    template <typename T, typename Attribute>
    inline void
    assign_to(T& val, reference_wrapper<Attribute> attr)
    {
        using namespace construct_;
        construct(attr, val);
    }

    template <typename T>
    inline void
    assign_to(T const& /*val*/, unused_type)
    {
    }

    template <typename T>
    inline void
    assign_to(T& /*val*/, unused_type)
    {
    }

}}}}

#endif
