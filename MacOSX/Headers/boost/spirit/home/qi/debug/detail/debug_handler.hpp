/*=============================================================================
    Copyright (c) 2001-2008 Hartmut Kaiser
    Copyright (c) 2001-2007 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(BOOST_SPIRIT_DEBUG_HANDLER_NOV_12_2007_0926AM)
#define BOOST_SPIRIT_DEBUG_HANDLER_NOV_12_2007_0926AM

#include <boost/spirit/home/qi/nonterminal/virtual_component_base.hpp>

namespace boost { namespace spirit { namespace qi { namespace debug
{

namespace detail
{
    ///////////////////////////////////////////////////////////////////////////
    //  This class is to avoid linker problems and to ensure a real singleton
    //  'level' variable
    static int& get_trace_level()
    {
        static int level = 0;
        return level;
    }

    struct trace_level
    {
        trace_level(int &level)
          : level(level)
        {
            ++level;
        }
        ~trace_level()
        {
            --level;
        }

        int& level;
    };

    ///////////////////////////////////////////////////////////////////////////
    template <
        typename Iterator, typename Context, typename Skipper,
        typename PreParseF, typename PostParseF
    >
    struct debug_handler
      : virtual_component_base<Iterator, Context, Skipper>
    {
        typedef
            virtual_component_base<Iterator, Context, Skipper>
        base_type;
        typedef intrusive_ptr<base_type> pointer_type;
        typedef typename base_type::skipper_type skipper_type;

        debug_handler(pointer_type subject, std::string const& name,
                bool trace, PreParseF preF, PostParseF postF)
          : subject(subject), name(name), trace(trace),
            preF(preF), postF(postF)
        {
        }

        template <typename Skipper_>
        bool parse_main(
            Iterator& first
          , Iterator const& last
          , Context& context
          , Skipper_ const& skipper)
        {
            // execute embedded parser if tracing is disabled or if the
            // pre-parse hook returns true
            bool r = false;
            if (!trace || preF(name, subject, get_trace_level(), first, last))
            {
                {
                    trace_level level(get_trace_level());

                    // do the actual parsing
                    Iterator i = first;
                    r = subject->parse(i, last, context, skipper);
                    if (r)
                        first = i;
                }

                // the post-parse hook gets executed only if tracing is enabled
                if (trace)
                    postF(r, name, subject, get_trace_level(), first, last);
            }
            return r;
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
        std::string const& name;
        bool trace;
        PreParseF preF;
        PostParseF postF;
    };

}
}}}}

#endif
