/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying 
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(SPIRIT_ASCII_JAN_31_2006_0529PM)
#define SPIRIT_ASCII_JAN_31_2006_0529PM

#include <boost/spirit/home/support/char_class.hpp>
#include <boost/xpressive/proto/proto.hpp>

namespace boost { namespace spirit { namespace ascii
{
    typedef spirit::char_class::ascii char_set;
    namespace tag = spirit::char_class::tag;

    template <typename Class>
    struct make_tag 
      : proto::terminal<spirit::char_class::key<char_set, Class> > {};

    typedef make_tag<tag::alnum>::type alnum_type;
    typedef make_tag<tag::alpha>::type alpha_type;
    typedef make_tag<tag::blank>::type blank_type;
    typedef make_tag<tag::cntrl>::type cntrl_type;
    typedef make_tag<tag::digit>::type digit_type;
    typedef make_tag<tag::graph>::type graph_type;
    typedef make_tag<tag::print>::type print_type;
    typedef make_tag<tag::punct>::type punct_type;
    typedef make_tag<tag::space>::type space_type;
    typedef make_tag<tag::xdigit>::type xdigit_type;

    alnum_type const alnum = {{}};
    alpha_type const alpha = {{}};
    blank_type const blank = {{}};
    cntrl_type const cntrl = {{}};
    digit_type const digit = {{}};
    graph_type const graph = {{}};
    print_type const print = {{}};
    punct_type const punct = {{}};
    space_type const space = {{}};
    xdigit_type const xdigit = {{}};

    typedef proto::terminal<
        spirit::char_class::no_case_tag<char_set> >::type 
    no_case_type;

    no_case_type const no_case = no_case_type();

    typedef proto::terminal<
        spirit::char_class::lower_case_tag<char_set> >::type 
    lower_type;
    typedef proto::terminal<
        spirit::char_class::upper_case_tag<char_set> >::type 
    upper_type;

    lower_type const lower = lower_type();
    upper_type const upper = upper_type();

#if defined(__GNUC__)
    inline void silence_unused_warnings__ascii()
    {
        (void) alnum; (void) alpha; (void) blank; (void) cntrl; (void) digit; 
        (void) graph; (void) print; (void) punct; (void) space; (void) xdigit;
    }
#endif
    
}}}

#endif
