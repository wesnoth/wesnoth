/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(SPIRIT_SEQUENCE_APR_22_2006_0811AM)
#define SPIRIT_SEQUENCE_APR_22_2006_0811AM

#include <boost/spirit/home/qi/operator/sequence_base.hpp>
#include <boost/spirit/home/qi/detail/fail_function.hpp>

namespace boost { namespace spirit { namespace qi
{
    struct sequence : sequence_base<sequence>
    {
        friend struct sequence_base<sequence>;

    private:

        template <typename Iterator, typename Context, typename Skipper>
        static detail::fail_function<Iterator, Context, Skipper>
        fail_function(
            Iterator& first, Iterator const& last
          , Context& context, Skipper const& skipper)
        {
            return detail::fail_function<Iterator, Context, Skipper>
                (first, last, context, skipper);
        }

        static std::string what_()
        {
            return "sequence[";
        }
    };
}}}

#endif
