//  Copyright (c) 2001, Daniel C. Nuffer
//  Copyright (c) 2001-2008, Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_ITERATOR_INPUT_ITERATOR_POLICY_MAR_16_2007_1156AM)
#define BOOST_SPIRIT_ITERATOR_INPUT_ITERATOR_POLICY_MAR_16_2007_1156AM

#include <boost/spirit/home/support/iterators/multi_pass_fwd.hpp>
#include <boost/spirit/home/support/iterators/detail/multi_pass.hpp>
#include <boost/detail/iterator.hpp> // for boost::detail::iterator_traits
#include <boost/assert.hpp>

namespace boost { namespace spirit { namespace multi_pass_policies
{
    namespace input_iterator_is_valid_test_
    {
        template <typename Token>
        inline bool token_is_valid(Token const&)
        {
            return true;
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    //  class input_iterator
    //  Implementation of the InputPolicy used by multi_pass
    // 
    //  The input_iterator encapsulates an input iterator of type T
    ///////////////////////////////////////////////////////////////////////////
    struct input_iterator
    {
        ///////////////////////////////////////////////////////////////////////
        template <typename T>
        class unique : public detail::default_input_policy
        {
        private:
            typedef
                typename boost::detail::iterator_traits<T>::value_type
            result_type;

        public:
            typedef
                typename boost::detail::iterator_traits<T>::difference_type
            difference_type;
            typedef
                typename boost::detail::iterator_traits<T>::distance_type
            distance_type;
            typedef
                typename boost::detail::iterator_traits<T>::pointer
            pointer;
            typedef
                typename boost::detail::iterator_traits<T>::reference
            reference;
            typedef result_type value_type;

        protected:
            unique() {}
            explicit unique(T x) : input(x) {}

            void swap(unique& x)
            {
                spirit::detail::swap(input, x.input);
            }

        public:
            template <typename MultiPass>
            static void advance_input(MultiPass& mp, value_type& t)
            {
                // if mp.shared is NULL then this instance of the multi_pass 
                // represents a end iterator, so no advance functionality is 
                // needed
                if (0 != mp.shared) 
                    t = *++mp.input;
            }

            // test, whether we reached the end of the underlying stream
            template <typename MultiPass>
            static bool input_at_eof(MultiPass const& mp, value_type const&) 
            {
                return mp.input == T();
            }

            template <typename MultiPass>
            static bool input_is_valid(MultiPass const& mp, value_type const& t) 
            {
                using namespace input_iterator_is_valid_test_;
                return token_is_valid(t);
            }

        protected:
            T input;
        };

        ///////////////////////////////////////////////////////////////////////
        template <typename T>
        struct shared
        {
            explicit shared(T) {}

            // no shared data elements
        };
    };

}}}

#endif
