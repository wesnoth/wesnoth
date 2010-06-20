//  Copyright (c) 2001-2007 Joel de Guzman
//  Copyright (c) 2001-2008 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_KARMA_KLEENE_MAR_03_2007_0337AM)
#define BOOST_SPIRIT_KARMA_KLEENE_MAR_03_2007_0337AM

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once      // MS compatible compilers support #pragma once
#endif

#include <boost/spirit/home/support/component.hpp>
#include <boost/spirit/home/support/detail/container.hpp>
#include <boost/spirit/home/support/unused.hpp>
#include <boost/spirit/home/support/attribute_transform.hpp>

#include <vector>

namespace boost { namespace spirit { namespace karma
{
    struct kleene
    {
        template <typename T>
        struct build_attribute_container
        {
            typedef std::vector<T> type;
        };

        template <typename Component, typename Context, typename Iterator>
        struct attribute :
            build_container<kleene, Component, Iterator, Context>
        {
        };

        template <typename Component, typename OutputIterator,
            typename Context, typename Delimiter, typename Parameter>
        static bool
        generate(Component const& component, OutputIterator& sink,
            Context& ctx, Delimiter const& d, Parameter const& param)
        {
            typedef typename
                result_of::subject<Component>::type::director
            director;
            typedef typename
                container::result_of::iterator<Parameter const>::type
            iterator_type;

            iterator_type it = container::begin(param);
            iterator_type end = container::end(param);

            // kleene fails only if the embedded parser fails
            bool result = true;
            for (/**/; result && !container::compare(it, end);
                 container::next(it))
            {
                result = director::generate(subject(component), sink, ctx, d,
                    container::deref(it));
            }
            return result;
        }

        // this kleene has no parameter attached
//         template <typename Component, typename OutputIterator,
//             typename Context, typename Delimiter>
//         static bool
//         generate(Component const&, OutputIterator&, Context&, Delimiter const&,
//             unused_type)
//         {
//             BOOST_MPL_ASSERT_MSG(false, kleene_not_usable_without_attribute, ());
//             return false;
//         }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            std::string result = "kleene[";

            typedef typename
                spirit::result_of::subject<Component>::type::director
            director;

            result += director::what(spirit::subject(component), ctx);
            result += "]";
            return result;
        }
    };

}}}

#endif
