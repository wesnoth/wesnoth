/*=============================================================================
    Copyright (c) 2001-2008 Hartmut Kaiser

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(BOOST_SPIRIT_SIMPLE_DEBUG_NOV_12_2007_1155AM)
#define BOOST_SPIRIT_SIMPLE_DEBUG_NOV_12_2007_1155AM

#include <boost/spirit/home/qi/debug/detail/debug_handler.hpp>
#include <boost/spirit/home/qi/debug/detail/print_node_info.hpp>
#include <boost/spirit/home/qi/nonterminal/rule.hpp>
#include <string>

///////////////////////////////////////////////////////////////////////////////
namespace boost { namespace spirit { namespace qi { namespace debug
{
    ///////////////////////////////////////////////////////////////////////////
    //  Simple pre-parse hook allowing to print the context before a rule is
    //  parsed.
    template <typename Subject, typename Iterator>
    inline bool
    simple_pre_parse(std::string const& name, Subject subject,
        unsigned level, Iterator first, Iterator const& last)
    {
        detail::print_node_info(false, level, false, name, first, last);
        return true;
    }

    ///////////////////////////////////////////////////////////////////////////
    //  Simple post-parse hook allowing to print the context after a rule is
    //  parsed.
    template <typename Subject, typename Iterator>
    inline void
    simple_post_parse(bool hit, std::string const& name, Subject subject,
        unsigned level, Iterator first, Iterator const& last)
    {
        detail::print_node_info(hit, level, true, name, first, last);
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename Nonterminal>
    inline void
    enable_simple_debug_support(Nonterminal& r, bool trace)
    {
        typedef typename Nonterminal::iterator_type iterator_type;
        typedef typename Nonterminal::pointer_type pointer_type;

        typedef bool (*pre_parse_functor_type)(std::string const&,
            pointer_type, unsigned, iterator_type, iterator_type const&);
        typedef void (*post_parse_functor_type)(bool, std::string const&,
            pointer_type, unsigned, iterator_type, iterator_type const&);

        typedef
            detail::debug_handler<
                iterator_type,
                typename Nonterminal::base_type::context_type,
                typename Nonterminal::skipper_type,
                pre_parse_functor_type,
                post_parse_functor_type>
        simple_debug_handler;

        pre_parse_functor_type pre = 
            &simple_pre_parse<pointer_type, iterator_type>;
        post_parse_functor_type post = 
            &simple_post_parse<pointer_type, iterator_type>;
        decorate<simple_debug_handler>(r, r.name(), trace, pre, post);
    }

}}}}

#endif
