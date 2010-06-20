//  Copyright (c) 2001-2008 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_FORMAT_MANIP_MAY_05_2007_1203PM)
#define BOOST_SPIRIT_FORMAT_MANIP_MAY_05_2007_1203PM

#include <boost/spirit/home/qi/parse.hpp>
#include <boost/spirit/home/support/unused.hpp>

#include <iterator>
#include <string>

///////////////////////////////////////////////////////////////////////////////
namespace boost { namespace spirit { namespace qi { namespace detail
{
    ///////////////////////////////////////////////////////////////////////////
    template <
        typename Expr, 
        typename Attribute = unused_type const, 
        typename Skipper = unused_type
    >
    struct match_manip 
    {
        match_manip(Expr const& xpr, Attribute& a, Skipper const& s) 
          : expr(xpr), attr(a), skipper(s)
        {}

        Expr const& expr;
        Attribute& attr;
        Skipper const& skipper;
    };

    ///////////////////////////////////////////////////////////////////////////
    template<typename Char, typename Traits, typename Expr> 
    inline std::basic_istream<Char, Traits> & 
    operator>> (std::basic_istream<Char, Traits> &is, 
        match_manip<Expr> const& fm)
    {
        typedef std::istream_iterator<Char, Char, Traits> input_iterator;
        input_iterator f(is);
        input_iterator l;
        if (!qi::parse (f, l, fm.expr))
        {
            is.setstate(std::ios_base::failbit);
        }
        return is;
    }
    
    ///////////////////////////////////////////////////////////////////////////
    template<typename Char, typename Traits, typename Expr, typename Attribute> 
    inline std::basic_istream<Char, Traits> & 
    operator>> (std::basic_istream<Char, Traits> &is, 
        match_manip<Expr, Attribute> const& fm)
    {
        typedef std::istream_iterator<Char, Char, Traits> input_iterator;
        input_iterator f(is);
        input_iterator l;
        if (!qi::parse(f, l, fm.expr, fm.attr))
        {
            is.setstate(std::ios_base::failbit);
        }
        return is;
    }
    
    template<typename Char, typename Traits, typename Expr, typename Skipper> 
    inline std::basic_istream<Char, Traits> & 
    operator>> (std::basic_istream<Char, Traits> &is, 
        match_manip<Expr, unused_type, Skipper> const& fm)
    {
        typedef std::istream_iterator<Char, Char, Traits> input_iterator;
        input_iterator f(is);
        input_iterator l;
        if (!qi::phrase_parse(f, l, fm.expr, fm.skipper))
        {
            is.setstate(std::ios_base::failbit);
        }
        return is;
    }
    
    ///////////////////////////////////////////////////////////////////////////
    template<
        typename Char, typename Traits, 
        typename Expr, typename Attribute, typename Skipper
    > 
    inline std::basic_istream<Char, Traits> & 
    operator>> (
        std::basic_istream<Char, Traits> &is, 
        match_manip<Expr, Attribute, Skipper> const& fm)
    {
        typedef std::istream_iterator<Char, Char, Traits> input_iterator;
        input_iterator f(is);
        input_iterator l;
        if (!qi::phrase_parse(f, l, fm.expr, fm.attr, fm.skipper))
        {
            is.setstate(std::ios_base::failbit);
        }
        return is;
    }
    
}}}}

#endif
