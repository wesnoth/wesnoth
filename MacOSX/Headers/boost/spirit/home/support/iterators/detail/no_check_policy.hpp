//  Copyright (c) 2001, Daniel C. Nuffer
//  Copyright (c) 2001-2008, Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_ITERATOR_NO_CHECK_POLICY_MAR_16_2007_1121AM)
#define BOOST_SPIRIT_ITERATOR_NO_CHECK_POLICY_MAR_16_2007_1121AM

#include <boost/spirit/home/support/iterators/multi_pass_fwd.hpp>
#include <boost/spirit/home/support/iterators/detail/multi_pass.hpp>

namespace boost { namespace spirit { namespace multi_pass_policies
{
    ///////////////////////////////////////////////////////////////////////////
    //  class no_check
    //  Implementation of the CheckingPolicy used by multi_pass
    //  It does not do anything :-)
    ///////////////////////////////////////////////////////////////////////////
    struct no_check
    {
        ///////////////////////////////////////////////////////////////////////
        struct unique // : public detail::default_checking_policy
        {
            void swap(unique&) {}

            template <typename MultiPass>
            static void check(MultiPass const&) {}

            template <typename MultiPass>
            static void clear_queue(MultiPass&) {}

            template <typename MultiPass>
            static void destroy(MultiPass&) {}
        };

        ///////////////////////////////////////////////////////////////////////
        struct shared 
        {};
    };

}}}

#endif
