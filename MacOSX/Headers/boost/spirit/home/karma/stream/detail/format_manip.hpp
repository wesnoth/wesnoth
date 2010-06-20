//  Copyright (c) 2001-2008 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_KARMA_FORMAT_MANIP_MAY_03_2007_1424PM)
#define BOOST_SPIRIT_KARMA_FORMAT_MANIP_MAY_03_2007_1424PM

#include <iterator>
#include <string>
#include <boost/spirit/home/karma/detail/ostream_iterator.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace boost { namespace spirit { namespace karma { namespace detail
{
    ///////////////////////////////////////////////////////////////////////////
    template <
        typename Expr, 
        typename Parameter = unused_type, 
        typename Delimiter = unused_type
    >
    struct format_manip 
    {
        format_manip(Expr const& xpr, Parameter const& p, Delimiter const& d) 
          : expr(xpr), param(p), delim(d)
        {}

        Expr const& expr;
        Parameter const& param;
        Delimiter const& delim;
    };

    ///////////////////////////////////////////////////////////////////////////
    template<typename Char, typename Traits, typename Expr> 
    inline std::basic_ostream<Char, Traits> & 
    operator<< (std::basic_ostream<Char, Traits> &os, 
        format_manip<Expr> const& fm)
    {
        ostream_iterator<Char, Char, Traits> sink(os);
        if (!karma::generate (sink, fm.expr))
        {
            os.setstate(std::ios_base::failbit);
        }
        return os;
    }
    
    ///////////////////////////////////////////////////////////////////////////
    template<typename Char, typename Traits, typename Expr, typename Parameter> 
    inline std::basic_ostream<Char, Traits> & 
    operator<< (std::basic_ostream<Char, Traits> &os, 
        format_manip<Expr, Parameter> const& fm)
    {
        ostream_iterator<Char, Char, Traits> sink(os);
        if (!karma::generate(sink, fm.expr, fm.param))
        {
            os.setstate(std::ios_base::failbit);
        }
        return os;
    }
    
    template<typename Char, typename Traits, typename Expr, typename Delimiter> 
    inline std::basic_ostream<Char, Traits> & 
    operator<< (std::basic_ostream<Char, Traits> &os, 
        format_manip<Expr, unused_type, Delimiter> const& fm)
    {
        ostream_iterator<Char, Char, Traits> sink(os);
        if (!karma::generate_delimited(sink, fm.expr, fm.delim))
        {
            os.setstate(std::ios_base::failbit);
        }
        return os;
    }
    
    ///////////////////////////////////////////////////////////////////////////
    template<
        typename Char, typename Traits, 
        typename Expr, typename Parameter, typename Delimiter
    > 
    inline std::basic_ostream<Char, Traits> & 
    operator<< (
        std::basic_ostream<Char, Traits> &os, 
        format_manip<Expr, Parameter, Delimiter> const& fm)
    {
        ostream_iterator<Char, Char, Traits> sink(os);
        if (!karma::generate_delimited(sink, fm.expr, fm.param, fm.delim))
        {
            os.setstate(std::ios_base::failbit);
        }
        return os;
    }
    
}}}}

#endif
