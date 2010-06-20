/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(BOOST_SPIRIT_ERROR_HANDLER_APR_29_2007_1042PM)
#define BOOST_SPIRIT_ERROR_HANDLER_APR_29_2007_1042PM

#include <boost/spirit/home/qi/nonterminal/virtual_component_base.hpp>
#include <boost/spirit/home/qi/nonterminal/error_handler_result.hpp>
#include <boost/spirit/home/qi/operator/expect.hpp>
#include <boost/fusion/include/vector.hpp>

namespace boost { namespace spirit { namespace qi { namespace detail
{
    template <
        typename Iterator, typename Context
      , typename Skipper, typename F, error_handler_result action
    >
    struct error_handler : virtual_component_base<Iterator, Context, Skipper>
    {
        typedef virtual_component_base<Iterator, Context, Skipper> base_type;
        typedef intrusive_ptr<base_type> pointer_type;
        typedef typename base_type::skipper_type skipper_type;

        error_handler(pointer_type subject, F f)
          : subject(subject)
          , f(f)
        {
        }

        template <typename Skipper_>
        bool parse_main(
            Iterator& first
          , Iterator const& last
          , Context& context
          , Skipper_ const& skipper)
        {
            while (true)
            {
                try
                {
                    Iterator i = first;
                    bool r = subject->parse(i, last, context, skipper);
                    if (r)
                        first = i;
                    return r;
                }
                catch (expectation_failure<Iterator> const& x)
                {
                    typedef
                        fusion::vector<
                            Iterator&
                          , Iterator const&
                          , Iterator const&
                          , std::string>
                    params;
                    error_handler_result r = action;
                    params args(first, last, x.first, x.what);
                    f(args, context, r);

                    switch (r)
                    {
                        case fail: return false;
                        case retry: continue;
                        case accept: return true;
                        case rethrow: throw x;
                    }
                }
            }
            return false;
        }

        virtual bool
        parse(
            Iterator& first
          , Iterator const& last
          , Context& context
          , skipper_type const& skipper)
        {
            return parse_main(first, last, context, skipper);
        }

        virtual bool
        parse(
            Iterator& first
          , Iterator const& last
          , Context& context
          , no_skipper)
        {
            return parse_main(first, last, context, unused);
        }

        pointer_type subject;
        F f;
    };
}}}}

#endif
