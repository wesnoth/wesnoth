/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(BOOST_SPIRIT_VIRTUAL_COMPONENT_BASE_FEB_12_2007_0440PM)
#define BOOST_SPIRIT_VIRTUAL_COMPONENT_BASE_FEB_12_2007_0440PM

#include <boost/spirit/home/support/unused.hpp>
#include <boost/spirit/home/support/component.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/detail/atomic_count.hpp>
#include <boost/mpl/eval_if.hpp>
#include <boost/mpl/identity.hpp>
#include <boost/type_traits/is_same.hpp>

namespace boost { namespace spirit { namespace qi
{
    struct no_skipper
    {
        // this struct accepts only unused types and
        // nothing else. This is used by the second
        // pure virtual parse member function of
        // virtual_component_base below.

        no_skipper(unused_type) {}
    };

    template <typename Iterator, typename Context, typename Skipper>
    struct virtual_component_base
    {
        struct take_no_skipper {};

        typedef typename
            mpl::eval_if<
                is_same<Skipper, unused_type>
              , mpl::identity<take_no_skipper>
              , result_of::as_component<qi::domain, Skipper>
            >::type
        skipper_type;

        virtual_component_base()
          : use_count(0)
        {
        }

        virtual ~virtual_component_base()
        {
        }

        virtual bool
        parse(
            Iterator& first
          , Iterator const& last
          , Context& context
          , skipper_type const& skipper) = 0;

        virtual bool
        parse(
            Iterator& first
          , Iterator const& last
          , Context& context
          , no_skipper) = 0;

        boost::detail::atomic_count use_count;
    };

    template <typename Iterator, typename Context, typename Skipper>
    inline void
    intrusive_ptr_add_ref(virtual_component_base<Iterator, Context, Skipper>* p)
    {
        ++p->use_count;
    }

    template <typename Iterator, typename Context, typename Skipper>
    inline void
    intrusive_ptr_release(virtual_component_base<Iterator, Context, Skipper>* p)
    {
        if (--p->use_count == 0)
            delete p;
    }
}}}

#endif
