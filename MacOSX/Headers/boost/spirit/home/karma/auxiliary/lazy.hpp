//  Copyright (c) 2001-2008 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_KARMA_LAZY_MARCH_27_2007_1231PM)
#define BOOST_SPIRIT_KARMA_LAZY_MARCH_27_2007_1231PM

#include <boost/spirit/home/karma/domain.hpp>
#include <boost/spirit/home/karma/delimit.hpp>
#include <boost/spirit/home/support/attribute_of.hpp>
#include <boost/spirit/home/support/unused.hpp>
#include <boost/fusion/include/at.hpp>
#include <boost/utility/result_of.hpp>
#include <boost/type_traits/remove_reference.hpp>

namespace boost { namespace spirit { namespace karma
{
    struct lazy_generator
    {
        template <typename Component, typename Context, typename Unused>
        struct attribute
        {
            typedef typename
                result_of::subject<Component>::type
            subject_type;

            typedef typename
                remove_reference<
                    typename boost::result_of<
                        subject_type(unused_type, Context)
                    >::type
                >::type
            expr_type;

            typedef typename
                result_of::as_component<karma::domain, expr_type>::type
            component_type;

            typedef typename
                traits::attribute_of<
                    karma::domain, component_type, Context>::type
            type;
        };

        template <typename Component, typename OutputIterator, 
            typename Context, typename Delimiter, typename Parameter>
        static bool 
        generate(Component const& component, OutputIterator& sink, 
            Context& ctx, Delimiter const& d, Parameter const& param) 
        {
            typedef typename
                result_of::subject<Component>::type
            subject_type;

            typedef typename
                remove_reference<
                    typename boost::result_of<
                        subject_type(unused_type, Context)
                    >::type
                >::type
            expr_type;

            typedef typename
                result_of::as_component<karma::domain, expr_type>::type
            component_type;

            component_type subject
                = spirit::as_component(
                    karma::domain(), 
                    fusion::at_c<0>(component.elements)(unused, ctx));

            return component_type::director::
                generate(subject, sink, ctx, d, param);
        }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            std::string result = "lazy[";
            // FIXME: need to get at the what of the embedded component
            result += "...";
            result += "]";
            return result;
        }
    };
}}}

#endif
