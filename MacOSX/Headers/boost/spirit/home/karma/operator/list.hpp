//  Copyright (c) 2001-2007 Joel de Guzman
//  Copyright (c) 2001-2008 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(SPIRIT_KARMA_LIST_MAY_01_2007_0229PM)
#define SPIRIT_KARMA_LIST_MAY_01_2007_0229PM

#include <boost/spirit/home/support/component.hpp>
#include <boost/spirit/home/support/detail/container.hpp>
#include <boost/spirit/home/support/unused.hpp>
#include <boost/spirit/home/support/attribute_transform.hpp>

#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace boost { namespace spirit { namespace karma
{
    struct list
    {
        template <typename T>
        struct build_attribute_container
        {
            typedef std::vector<T> type;
        };

        template <typename Component, typename Context, typename Iterator>
        struct attribute :
            build_container<list, Component, Iterator, Context>
        {
        };

        template <typename Component, typename OutputIterator,
            typename Context, typename Delimiter, typename Parameter>
        static bool
        generate(Component const& component, OutputIterator& sink,
            Context& ctx, Delimiter const& d, Parameter const& param)
        {
            typedef typename
                spirit::result_of::left<Component>::type::director
            ldirector;

            typedef typename
                spirit::result_of::right<Component>::type::director
            rdirector;

            typedef typename
                container::result_of::iterator<Parameter const>::type
            iterator_type;

            iterator_type it = container::begin(param);
            iterator_type end = container::end(param);

            bool result = !container::compare(it, end);
            if (result && ldirector::generate(
                  spirit::left(component), sink, ctx, d, container::deref(it)))
            {
                for (container::next(it); result && !container::compare(it, end);
                     container::next(it))
                {
                    result =
                        rdirector::generate(
                            spirit::right(component), sink, ctx, d, unused) &&
                        ldirector::generate(
                            spirit::left(component), sink, ctx, d, container::deref(it));
                }
                return result;
            }
            return false;
        }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            std::string result = "list[";

            typedef typename
                spirit::result_of::left<Component>::type::director
            ldirector;

            typedef typename
                spirit::result_of::right<Component>::type::director
            rdirector;

            result += ldirector::what(spirit::left(component), ctx);
            result += ", ";
            result += rdirector::what(spirit::right(component), ctx);
            result += "]";
            return result;
        }
    };

}}}

#endif
