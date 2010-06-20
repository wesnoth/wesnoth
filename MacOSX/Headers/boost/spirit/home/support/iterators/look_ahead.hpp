//  Copyright (c) 2001, Daniel C. Nuffer
//  Copyright (c) 2001-2008, Hartmut Kaiser
//  http://spirit.sourceforge.net/
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_ITERATOR_LOOK_AHEAD_MAR_16_2007_1253PM)
#define BOOST_SPIRIT_ITERATOR_LOOK_AHEAD_MAR_16_2007_1253PM

#include <boost/spirit/home/support/iterators/detail/input_iterator_policy.hpp>
#include <boost/spirit/home/support/iterators/detail/first_owner_policy.hpp>
#include <boost/spirit/home/support/iterators/detail/no_check_policy.hpp>
#include <boost/spirit/home/support/iterators/detail/fixed_size_queue_policy.hpp>
#include <boost/spirit/home/support/iterators/multi_pass.hpp>

namespace boost { namespace spirit 
{
    ///////////////////////////////////////////////////////////////////////////
    //  this could be a template typedef, since such a thing doesn't
    //  exist in C++, we'll use inheritance to accomplish the same thing.
    ///////////////////////////////////////////////////////////////////////////
    template <typename T, std::size_t N>
    class look_ahead :
        public multi_pass<
            T,
            multi_pass_policies::input_iterator,
            multi_pass_policies::first_owner,
            multi_pass_policies::no_check,
            multi_pass_policies::fixed_size_queue<N> 
        >
    {
    private:
        typedef multi_pass<
            T,
            multi_pass_policies::input_iterator,
            multi_pass_policies::first_owner,
            multi_pass_policies::no_check,
            multi_pass_policies::fixed_size_queue<N> > 
        base_type;
            
    public:
        look_ahead()
          : base_type() {}

        explicit look_ahead(T x)
          : base_type(x) {}

        look_ahead(look_ahead const& x)
          : base_type(x) {}

#if BOOST_WORKAROUND(__GLIBCPP__, == 20020514)
        look_ahead(int)         // workaround for a bug in the library
          : base_type() {}      // shipped with gcc 3.1
#endif // BOOST_WORKAROUND(__GLIBCPP__, == 20020514)

    // default generated operators destructor and assignment operator are ok.
    };

}}

#endif
