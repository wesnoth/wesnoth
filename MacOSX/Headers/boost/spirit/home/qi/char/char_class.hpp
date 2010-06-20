/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(BOOST_SPIRIT_CHAR_CLASS_APR_16_2006_1051AM)
#define BOOST_SPIRIT_CHAR_CLASS_APR_16_2006_1051AM

#include <boost/spirit/home/qi/char/char_parser.hpp>
#include <boost/spirit/home/qi/domain.hpp>
#include <boost/spirit/home/support/modifier.hpp>
#include <boost/spirit/home/support/iso8859_1.hpp>
#include <boost/spirit/home/support/ascii.hpp>
#include <boost/spirit/home/support/standard.hpp>
#include <boost/spirit/home/support/standard_wide.hpp>
#include <boost/fusion/include/cons.hpp>

namespace boost { namespace spirit { namespace qi
{
    ///////////////////////////////////////////////////////////////////////////
    // generic isxxx parser (for alnum, alpha, graph, etc.)
    ///////////////////////////////////////////////////////////////////////////
    template <typename Tag>
    struct char_class
      : char_parser<char_class<Tag>, typename Tag::char_set::char_type>
    {
        typedef typename Tag::char_set char_set;
        typedef typename Tag::char_class char_class_;

        template <typename Component, typename CharParam, typename Context>
        static bool test(Component const&, CharParam ch, Context&)
        {
            using spirit::char_class::classify;
            return classify<char_set>::is(char_class_(), ch);
        }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            typedef spirit::char_class::what<char_set> what_;
            return what_::is(char_class_());
        }
    };
}}}

namespace boost { namespace spirit { namespace traits
{
    ///////////////////////////////////////////////////////////////////////////
    // no_case char_class conversions
    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        using spirit::char_class::key;
        using spirit::char_class::lower_case_tag;
        using spirit::char_class::upper_case_tag;
        using spirit::char_class::tag::alpha;

        template <typename Tag>
        struct make_no_case_char_class :
            mpl::identity<qi::char_class<Tag> > {};

        template <typename CharSet>
        struct make_no_case_char_class<lower_case_tag<CharSet> >
          : mpl::identity<qi::char_class<key<CharSet, alpha> > > {};

        template <typename CharSet>
        struct make_no_case_char_class<upper_case_tag<CharSet> >
          : mpl::identity<qi::char_class<key<CharSet, alpha> > > {};
    }

    template <
        typename Domain, typename Elements, typename Modifier, typename Tag
    >
    struct make_modified_component<
        Domain, qi::char_class<Tag>, Elements, Modifier
      , typename enable_if<
            is_member_of_modifier<Modifier, spirit::char_class::no_case_base_tag>
        >::type
    >
    {
        typedef typename detail::make_no_case_char_class<Tag>::type director;
        typedef component<qi::domain, director, fusion::nil> type;

        static type
        call(Elements const&)
        {
            return type(fusion::nil());
        }
    };
}}}

#endif
