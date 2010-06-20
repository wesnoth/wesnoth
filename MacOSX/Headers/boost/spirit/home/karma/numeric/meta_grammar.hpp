//  Copyright (c) 2001-2008 Hartmut Kaiser
//  Copyright (c) 2001-2007 Joel de Guzman
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_KARMA_META_GRAMMAR_FEB_23_2007_0505PM)
#define BOOST_SPIRIT_KARMA_META_GRAMMAR_FEB_23_2007_0505PM

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once      // MS compatible compilers support #pragma once
#endif

#include <boost/spirit/home/karma/domain.hpp>
#include <boost/spirit/home/karma/numeric/numeric_fwd.hpp>
#include <boost/spirit/home/support/placeholders.hpp>
#include <boost/spirit/home/support/meta_grammar.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/mpl/identity.hpp>
#include <boost/mpl/bool.hpp>

namespace boost { namespace spirit { namespace karma
{
    template <typename Expr, typename Enable>
    struct is_valid_expr;

    template <typename Expr, typename Enable>
    struct expr_transform;

    ///////////////////////////////////////////////////////////////////////////
    // numeric tags
    ///////////////////////////////////////////////////////////////////////////
    template <typename T, unsigned Radix, bool ForceSign>
    struct int_tag
    {};

    template <typename T, unsigned Radix, bool ForceSign>
    struct uint_tag
    {};

    template <typename T, typename RealPolicies>
    struct real_tag 
    {
        RealPolicies policies;
    };

    ///////////////////////////////////////////////////////////////////////////
    // numeric specs
    ///////////////////////////////////////////////////////////////////////////
    template <typename T, unsigned Radix, bool ForceSign>
    struct int_spec
      : proto::terminal<int_tag<T, Radix, ForceSign> >::type
    {};

    template <typename T, unsigned Radix, bool ForceSign>
    struct uint_spec
      : proto::terminal<uint_tag<T, Radix, ForceSign> >::type 
    {};

    ///////////////////////////////////////////////////////////////////////////
    template <typename T, typename RealPolicies>
    struct real_spec
      : proto::terminal<real_tag<T, RealPolicies> >::type 
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
    
}}}   // namespace boost::spirit::karma

namespace boost { namespace spirit 
{
    ///////////////////////////////////////////////////////////////////////////
    //  test if a tag is an int tag (the basic specializations are defined in
    //  the file support/placeholders.hpp)
    ///////////////////////////////////////////////////////////////////////////
    template <typename T, unsigned Radix, bool ForceSign>
    struct is_int_tag<karma::int_tag<T, Radix, ForceSign>, karma::domain> : 
        mpl::true_ {};

    template <typename T, unsigned Radix, bool ForceSign>
    struct is_int_tag<karma::uint_tag<T, Radix, ForceSign>, karma::domain> : 
        mpl::true_ {};

    ///////////////////////////////////////////////////////////////////////////
    //  test if a tag is a real tag (the basic specializations are defined in
    //  the file support/placeholders.hpp)
    ///////////////////////////////////////////////////////////////////////////
    template <typename T, typename RealPolicies>
    struct is_real_tag<karma::real_tag<T, RealPolicies>, karma::domain> : 
        mpl::true_ {};

}}  // namespace boost::spirit

namespace boost { namespace spirit { namespace karma
{
    ///////////////////////////////////////////////////////////////////////////
    // get the director of an int tag
    ///////////////////////////////////////////////////////////////////////////
    template <typename T, bool IsLiteral>
    struct extract_int_director;

    template <bool IsLiteral>
    struct extract_int_director<tag::bin, IsLiteral>    
    {
        typedef uint_generator<IsLiteral, unsigned, 2, false> type;
    };

    template <bool IsLiteral>
    struct extract_int_director<tag::oct, IsLiteral>    
    {
        typedef uint_generator<IsLiteral, unsigned, 8, false> type;
    };
        
    template <bool IsLiteral>
    struct extract_int_director<tag::hex, IsLiteral>    
    {
        typedef uint_generator<IsLiteral, unsigned, 16, false> type;
    };
        
    template <bool IsLiteral>
    struct extract_int_director<tag::ushort, IsLiteral>    
    {
        typedef uint_generator<IsLiteral, unsigned short, 10, false> type;
    };
        
    template <bool IsLiteral>
    struct extract_int_director<tag::ulong, IsLiteral>    
    {
        typedef uint_generator<IsLiteral, unsigned long, 10, false> type;
    };
        
    template <bool IsLiteral>
    struct extract_int_director<tag::uint, IsLiteral>    
    {
        typedef uint_generator<IsLiteral, unsigned int, 10, false> type;
    };
    
    template <bool IsLiteral>
    struct extract_int_director<tag::short_, IsLiteral>    
    {
        typedef int_generator<IsLiteral, short, 10, false> type;
    };

    template <bool IsLiteral>
    struct extract_int_director<tag::long_, IsLiteral>    
    {
        typedef int_generator<IsLiteral, long, 10, false> type;
    };

    template <bool IsLiteral>
    struct extract_int_director<tag::int_, IsLiteral>    
    {
        typedef int_generator<IsLiteral, int, 10, false> type;
    };

#ifdef BOOST_HAS_LONG_LONG
    template <bool IsLiteral>
    struct extract_int_director<tag::ulong_long, IsLiteral>    
    {
        typedef uint_generator<IsLiteral, boost::ulong_long_type, 10, false> type;
    };
        
    template <bool IsLiteral>
    struct extract_int_director<tag::long_long, IsLiteral>    
    {
        typedef int_generator<IsLiteral, boost::long_long_type, 10, false> type;
    };
#endif

    template <typename T, unsigned Radix, bool ForceSign, bool IsLiteral>
    struct extract_int_director<int_tag<T, Radix, ForceSign>, IsLiteral>
    {
        typedef int_generator<IsLiteral, T, Radix, ForceSign> type;
    };

    template <typename T, unsigned Radix, bool ForceSign, bool IsLiteral>
    struct extract_int_director<uint_tag<T, Radix, ForceSign>, IsLiteral>
    {
        typedef uint_generator<IsLiteral, T, Radix, ForceSign> type;
    };

    template <typename T, typename Unused>
    struct extract_int_director_lit
      : extract_int_director<T, true> {};
    
    template <typename T>
    struct extract_int_director_plain
      : extract_int_director<T, false> {};

    ///////////////////////////////////////////////////////////////////////////
    // get the director of a floating point literal type
    ///////////////////////////////////////////////////////////////////////////
    template <typename Tag>
    struct extract_literal_real_director;

    template <>
    struct extract_literal_real_director<float>    
    {
        typedef 
            real_generator<true, float, real_generator_policies<float> > 
        type;
    };

    template <>
    struct extract_literal_real_director<double>    
    {
        typedef 
            real_generator<true, double, real_generator_policies<double> > 
        type;
    };

    template <>
    struct extract_literal_real_director<long double>    
    {
        typedef 
            real_generator<
                true, long double, real_generator_policies<long double> 
            > 
        type;
    };

    ///////////////////////////////////////////////////////////////////////////
    // get the director of a floating point tag
    ///////////////////////////////////////////////////////////////////////////
    template <typename Tag, bool IsLiteral>
    struct extract_real_director;
    
    template <bool IsLiteral>
    struct extract_real_director<tag::float_, IsLiteral>    
    {
        typedef 
            real_generator<IsLiteral, float, real_generator_policies<float> > 
        type;
    };

    template <bool IsLiteral>
    struct extract_real_director<tag::double_, IsLiteral>    
    {
        typedef 
            real_generator<IsLiteral, double, real_generator_policies<double> > 
        type;
    };

    template <bool IsLiteral>
    struct extract_real_director<tag::long_double, IsLiteral>    
    {
        typedef 
            real_generator<
                IsLiteral, long double, real_generator_policies<long double> 
            > 
        type;
    };

    template <typename T, typename RealPolicies, bool IsLiteral>
    struct extract_real_director<real_tag<T, RealPolicies>, IsLiteral>
    {
        typedef real_generator<IsLiteral, T, RealPolicies> type;
    };

    ///////////////////////////////////////////////////////////////////////////
    template <typename Tag, typename Unused>
    struct extract_real_director_lit
      : extract_real_director<Tag, true> {};
    
    template <typename Tag>
    struct extract_real_director_plain
      : extract_real_director<Tag, false> {};

    ///////////////////////////////////////////////////////////////////////////
    // get the director of an integer literal type
    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    struct extract_literal_int_director;

    template <>
    struct extract_literal_int_director<short>    
    {
        typedef int_generator<true, short, 10, false> type;
    };

    template <>
    struct extract_literal_int_director<unsigned short>    
    {
        typedef uint_generator<true, unsigned short, 10, false> type;
    };
        
    template <>
    struct extract_literal_int_director<int>
    {
        typedef int_generator<true, int, 10, false> type;
    };
        
    template <>
    struct extract_literal_int_director<unsigned int>    
    {
        typedef uint_generator<true, unsigned int, 10, false> type;
    };
        
    template <>
    struct extract_literal_int_director<long>
    {
        typedef int_generator<true, long, 10, false> type;
    };
        
    template <>
    struct extract_literal_int_director<unsigned long>    
    {
        typedef uint_generator<true, unsigned long, 10, false> type;
    };
    
#ifdef BOOST_HAS_LONG_LONG
    template <>
    struct extract_literal_int_director<boost::ulong_long_type>    
    {
        typedef int_generator<true, boost::ulong_long_type, 10, false> type;
    };

    template <>
    struct extract_literal_int_director<boost::long_long_type>    
    {
        typedef uint_generator<true, boost::long_long_type, 10, false> type;
    };
#endif

    ///////////////////////////////////////////////////////////////////////////
    // numeric parser meta-grammar
    ///////////////////////////////////////////////////////////////////////////

    // literals: 10, 10L, 10LL
    struct int_literal_meta_grammar
      : meta_grammar::compose_empty<
            proto::if_<
                is_int_lit_tag<proto::_arg, karma::domain>()
            >,
            karma::domain,
            mpl::identity<extract_literal_int_director<mpl::_> >
        >
    {};

    // all the different integer's as int_, uint, bin, oct, dec, hex, etc.
    // and the corresponding int_(10), uint(10), etc.
    struct int_meta_grammar
      : proto::or_<
            meta_grammar::compose_empty<
                proto::if_<
                    is_int_tag<proto::_arg, karma::domain>()
                >,
                karma::domain,
                mpl::identity<extract_int_director_plain<mpl::_> >
            >,
            meta_grammar::compose_function1_eval<
                proto::function<
                    proto::if_<
                        is_int_tag<proto::_arg, karma::domain>()
                    >, 
                    int_literal_meta_grammar
                >, 
                karma::domain, 
                mpl::identity<extract_int_director_lit<mpl::_, mpl::_> >
            >
        >
    {};

    // floating point literals: 1.0, 1.0f, 10.1e2 etc.
    struct real_literal_meta_grammar
      : meta_grammar::compose_empty<
            proto::if_<
                is_real_lit_tag<proto::_arg, karma::domain>()
            >,
            karma::domain,
            mpl::identity<extract_literal_real_director<mpl::_> >
        >
    {};
    
    struct real_meta_grammar
      : proto::or_<
            meta_grammar::compose_single<
                proto::if_<
                    is_real_tag<proto::_arg, karma::domain>()
                >,
                karma::domain,
                mpl::identity<extract_real_director_plain<mpl::_> >
            >,
            meta_grammar::compose_function1_full<
                proto::function<
                    proto::if_<
                        is_real_tag<proto::_arg, karma::domain>()
                    >, 
                    real_literal_meta_grammar
                >, 
                karma::domain, 
                mpl::identity<extract_real_director_lit<mpl::_, mpl::_> >
            >
        >
    {};
    
    ///////////////////////////////////////////////////////////////////////////
    struct numeric_meta_grammar
      : proto::or_<
            int_meta_grammar,
            real_meta_grammar
        >
    {};

    ///////////////////////////////////////////////////////////////////////////
    //  These specializations non-intrusively hooks into the RD meta-grammar.
    //  (see qi/meta_grammar.hpp)
    ///////////////////////////////////////////////////////////////////////////  
    template <typename Expr>
    struct is_valid_expr<Expr
      , typename enable_if<proto::matches<Expr, numeric_meta_grammar> >::type>
      : mpl::true_
    {};

    template <typename Expr>
    struct expr_transform<Expr
      , typename enable_if<proto::matches<Expr, numeric_meta_grammar> >::type>
      : mpl::identity<numeric_meta_grammar>
    {};
    
}}}

#endif
