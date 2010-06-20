/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(BOOST_SPIRIT_LAZY_MARCH_27_2007_1002AM)
#define BOOST_SPIRIT_LAZY_MARCH_27_2007_1002AM

#include <boost/spirit/home/qi/domain.hpp>
#include <boost/spirit/home/qi/skip.hpp>
#include <boost/spirit/home/support/attribute_of.hpp>
#include <boost/spirit/home/support/unused.hpp>
#include <boost/fusion/include/at.hpp>
#include <boost/utility/result_of.hpp>
#include <boost/type_traits/remove_reference.hpp>

namespace boost { namespace spirit { namespace qi
{
    struct lazy_parser
    {
        template <typename Component, typename Context, typename Iterator>
        struct attribute
        {
            typedef typename
                result_of::subject<Component>::type
            subject_type;

            typedef typename
                remove_reference<
                    typename boost::result_of<subject_type(unused_type, Context)>::type
                >::type
            expr_type;

            typedef typename
                result_of::as_component<qi::domain, expr_type>::type
            component_type;

            typedef typename
                traits::attribute_of<
                    qi::domain, component_type, Context, Iterator>::type
            type;
        };

        template <
            typename Component
          , typename Iterator, typename Context
          , typename Skipper, typename Attribute>
        static bool parse(
            Component const& component
          , Iterator& first, Iterator const& last
          , Context& context, Skipper const& skipper
          , Attribute& attr)
        {
            typedef typename
                result_of::subject<Component>::type
            subject_type;

            typedef typename
                remove_reference<
                    typename boost::result_of<subject_type(unused_type, Context)>::type
                >::type
            expr_type;

            typedef typename
                result_of::as_component<qi::domain, expr_type>::type
            component_type;

            component_type subject
                = spirit::as_component(
                    qi::domain(), fusion::at_c<0>(component.elements)(unused, context));

            return component_type::director::
                parse(subject, first, last, context, skipper, attr);
        }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            //~ typedef typename
                //~ result_of::subject<Component>::type
            //~ subject_type;

            //~ typedef typename
                //~ remove_reference<
                    //~ typename boost::result_of<subject_type(unused_type, unused_type)>::type
                //~ >::type
            //~ expr_type;

            //~ typedef typename
                //~ result_of::as_component<qi::domain, expr_type>::type
            //~ component_type;

            //~ component_type subject
                //~ = spirit::as_component(
                    //~ qi::domain(), fusion::at_c<0>(component.elements)(unused, unused));

            std::string result = "lazy[";
            //~ result += component_type::director::what(subject, ctx);
            result += "]";
            return result;
        }
    };
}}}

#endif
