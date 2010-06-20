/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(BOOST_SPIRIT_META_GRAMMAR_FEB_05_2007_0951AM)
#define BOOST_SPIRIT_META_GRAMMAR_FEB_05_2007_0951AM

#include <boost/spirit/home/qi/domain.hpp>
#include <boost/spirit/home/support/placeholders.hpp>
#include <boost/spirit/home/support/meta_grammar.hpp>
#include <boost/spirit/home/qi/numeric/real_policies.hpp>
#include <boost/utility/enable_if.hpp>

namespace boost { namespace spirit
{
    namespace qi
    {
        template <typename T, unsigned Radix, unsigned MinDigits, int MaxDigits>
        struct int_tag;

        template <typename T, unsigned Radix, unsigned MinDigits, int MaxDigits>
        struct uint_tag;

        template <typename T, typename RealPolicies>
        struct real_tag;
    }

    template <typename T, unsigned Radix, unsigned MinDigits, int MaxDigits>
    struct is_int_tag<qi::int_tag<T, Radix, MinDigits, MaxDigits>, qi::domain> 
      : mpl::true_ {};

    template <typename T, unsigned Radix, unsigned MinDigits, int MaxDigits>
    struct is_int_tag<qi::uint_tag<T, Radix, MinDigits, MaxDigits>, qi::domain> 
      : mpl::true_ {};

    template <typename T, typename RealPolicies>
    struct is_real_tag<qi::real_tag<T, RealPolicies>, qi::domain> 
      : mpl::true_ {};
}}

namespace boost { namespace spirit { namespace qi
{
    ///////////////////////////////////////////////////////////////////////////
    // forwards
    ///////////////////////////////////////////////////////////////////////////
    template <typename T, unsigned Radix, unsigned MinDigits, int MaxDigits>
    struct int_parser;

    template <typename T, unsigned Radix, unsigned MinDigits, int MaxDigits>
    struct uint_parser;

    template <typename T, typename RealPolicies>
    struct real_parser;

    template <typename Expr, typename Enable>
    struct is_valid_expr;

    template <typename Expr, typename Enable>
    struct expr_transform;

    ///////////////////////////////////////////////////////////////////////////
    // numeric tags
    ///////////////////////////////////////////////////////////////////////////
    template <typename T, unsigned Radix, unsigned MinDigits, int MaxDigits>
    struct int_tag
    {
    };

    template <typename T, unsigned Radix, unsigned MinDigits, int MaxDigits>
    struct uint_tag
    {
    };

    template <typename T, typename RealPolicies>
    struct real_tag
    {
        RealPolicies policies;
    };

    ///////////////////////////////////////////////////////////////////////////
    // numeric specs
    ///////////////////////////////////////////////////////////////////////////
    template <
        typename T = int
      , unsigned Radix = 10
      , unsigned MinDigits = 1
      , int MaxDigits = -1
    >
    struct int_spec
      : proto::terminal<
            int_tag<T, Radix, MinDigits, MaxDigits>
        >::type
    {
    };

    template <
        typename T = int
      , unsigned Radix = 10
      , unsigned MinDigits = 1
      , int MaxDigits = -1
    >
    struct uint_spec
      : proto::terminal<
            uint_tag<T, Radix, MinDigits, MaxDigits>
        >::type
    {
    };

    ///////////////////////////////////////////////////////////////////////////
    template <
        typename T = double,
        typename RealPolicies = real_policies<T>
    >
    struct real_spec
      : proto::terminal<
            real_tag<T, RealPolicies>
        >::type
    {
    private:
        typedef typename 
            proto::terminal<real_tag<T, RealPolicies> >::type
        base_type;

        base_type make_tag(RealPolicies const& p) const
        {
            base_type xpr = {{p}};
            return xpr;
        }

    public:
        real_spec(RealPolicies const& p = RealPolicies())
          : base_type(make_tag(p))
        {}
    };

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        template <typename RealPolicies>
        struct real_policy
        {
            template <typename Tag>
            static RealPolicies get(Tag) { return RealPolicies(); }

            template <typename T>
            static RealPolicies const& get(real_tag<T, RealPolicies> const& p) 
            { return p.policies; }
        };
    }
    
    ///////////////////////////////////////////////////////////////////////////
    // get the director of an int tag
    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    struct extract_int_director;

    template <>
    struct extract_int_director<tag::bin>
    {
        typedef uint_parser<unsigned, 2, 1, -1> type;
    };

    template <>
    struct extract_int_director<tag::oct>
    {
        typedef uint_parser<unsigned, 8, 1, -1> type;
    };

    template <>
    struct extract_int_director<tag::hex>
    {
        typedef uint_parser<unsigned, 16, 1, -1> type;
    };

    template <>
    struct extract_int_director<tag::ushort>
    {
        typedef uint_parser<unsigned short, 10, 1, -1> type;
    };

    template <>
    struct extract_int_director<tag::ulong>
    {
        typedef uint_parser<unsigned long, 10, 1, -1> type;
    };

    template <>
    struct extract_int_director<tag::uint>
    {
        typedef uint_parser<unsigned int, 10, 1, -1> type;
    };

    template <>
    struct extract_int_director<tag::short_>
    {
        typedef int_parser<short, 10, 1, -1> type;
    };

    template <>
    struct extract_int_director<tag::long_>
    {
        typedef int_parser<long, 10, 1, -1> type;
    };

    template <>
    struct extract_int_director<tag::int_>
    {
        typedef int_parser<int, 10, 1, -1> type;
    };

#ifdef BOOST_HAS_LONG_LONG
    template <>
    struct extract_int_director<tag::ulong_long>
    {
        typedef uint_parser<unsigned long long, 10, 1, -1> type;
    };

    template <>
    struct extract_int_director<tag::long_long>
    {
        typedef int_parser<long long, 10, 1, -1> type;
    };
#endif

    template <typename T, unsigned Radix, unsigned MinDigits, int MaxDigits>
    struct extract_int_director<int_tag<T, Radix, MinDigits, MaxDigits> >
    {
        typedef int_parser<T, Radix, MinDigits, MaxDigits> type;
    };

    template <typename T, unsigned Radix, unsigned MinDigits, int MaxDigits>
    struct extract_int_director<uint_tag<T, Radix, MinDigits, MaxDigits> >
    {
        typedef uint_parser<T, Radix, MinDigits, MaxDigits> type;
    };

    ///////////////////////////////////////////////////////////////////////////
    // get the director of a real tag
    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    struct extract_real_director;

    template <>
    struct extract_real_director<tag::float_>
    {
        typedef real_parser<float, real_policies<float> > type;
    };

    template <>
    struct extract_real_director<tag::double_>
    {
        typedef real_parser<double, real_policies<double> > type;
    };

    template <>
    struct extract_real_director<tag::long_double>
    {
        typedef real_parser<long double, real_policies<long double> > type;
    };

    template <typename T, typename RealPolicies>
    struct extract_real_director<real_tag<T, RealPolicies> >
    {
        typedef real_parser<T, RealPolicies> type;
    };

    ///////////////////////////////////////////////////////////////////////////
    // numeric parser meta-grammar
    ///////////////////////////////////////////////////////////////////////////
    struct int_meta_grammar
      : meta_grammar::compose_empty<
            proto::if_<is_int_tag<proto::_arg, qi::domain>()>
          , qi::domain
          , mpl::identity<extract_int_director<mpl::_> >
        >
    {};

    struct real_meta_grammar
      : meta_grammar::compose_single<
            proto::if_<is_real_tag<proto::_arg, qi::domain>()>
          , qi::domain
          , mpl::identity<extract_real_director<mpl::_> >
        >
    {};

    struct numeric_meta_grammar
      : proto::or_<int_meta_grammar, real_meta_grammar>
    {
    };

    ///////////////////////////////////////////////////////////////////////////
    //  These specializations non-intrusively hooks into the RD meta-grammar.
    //  (see qi/meta_grammar.hpp)
    ///////////////////////////////////////////////////////////////////////////
    template <typename Expr>
    struct is_valid_expr<Expr
      , typename enable_if<proto::matches<Expr, numeric_meta_grammar> >::type>
      : mpl::true_
    {
    };

    template <typename Expr>
    struct expr_transform<Expr
      , typename enable_if<proto::matches<Expr, numeric_meta_grammar> >::type>
      : mpl::identity<numeric_meta_grammar>
    {
    };
}}}

#endif
