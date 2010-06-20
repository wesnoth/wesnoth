/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(SPIRIT_EXPECT_APRIL_29_2007_0445PM)
#define SPIRIT_EXPECT_APRIL_29_2007_0445PM

#include <boost/spirit/home/qi/operator/sequence_base.hpp>
#include <boost/spirit/home/qi/detail/expect_function.hpp>

namespace boost { namespace spirit { namespace qi
{
    template <typename Iterator>
    struct expectation_failure
    {
        Iterator first;
        Iterator last;
        std::string what;
    };

    struct expect : sequence_base<expect>
    {
        friend struct sequence_base<expect>;

    private:

        template <typename Iterator, typename Context, typename Skipper>
        static detail::expect_function<
            Iterator, Context, Skipper
          , expectation_failure<Iterator> >
        fail_function(
            Iterator& first, Iterator const& last
          , Context& context, Skipper const& skipper)
        {
            return detail::expect_function<
                Iterator, Context, Skipper, expectation_failure<Iterator> >
                (first, last, context, skipper);
        }

        static std::string what_()
        {
            return "expect[";
        }
    };
}}}

#endif
