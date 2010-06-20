/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(SPIRIT_PERMUTATION_OR_MARCH_13_2007_1145PM)
#define SPIRIT_PERMUTATION_OR_MARCH_13_2007_1145PM

#include <boost/spirit/home/qi/detail/permute_function.hpp>
#include <boost/spirit/home/support/attribute_transform.hpp>
#include <boost/spirit/home/support/algorithm/any.hpp>
#include <boost/spirit/home/support/detail/what_function.hpp>
#include <boost/fusion/include/vector.hpp>
#include <boost/fusion/include/as_vector.hpp>
#include <boost/fusion/include/size.hpp>
#include <boost/optional.hpp>
#include <boost/foreach.hpp>
#include <boost/array.hpp>

namespace boost { namespace spirit { namespace qi
{
    struct permutation
    {
        template <typename T>
        struct transform_child
        {
            typedef boost::optional<T> type;
        };

        template <typename All, typename Filtered>
        struct build_container
        {
            typedef
                typename fusion::result_of::as_vector<Filtered>::type
            type;
        };

        template <typename Component, typename Context, typename Iterator>
        struct attribute :
            build_fusion_sequence<permutation, Component, Iterator, Context>
        {
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
            detail::permute_function<Iterator, Context, Skipper>
                f(first, last, context, skipper);

            boost::array<
                bool
              , fusion::result_of::size<typename Component::elements_type>::value
            >
            slots;

            BOOST_FOREACH(bool& taken, slots)
            {
                taken = false;
            }

            // We have a bool array 'slots' with one flag for each parser.
            // permute_function sets the slot to true when the corresponding
            // parser successful matches. We loop until there are no more
            // successful parsers.

            bool result = false;
            f.taken = slots.begin();
            while (spirit::any_ns(component.elements, attr, f))
            {
                f.taken = slots.begin();
                result = true;
            }
            return result;
        }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            std::string result = "permutation[";
            fusion::for_each(component.elements,
                spirit::detail::what_function<Context>(result, ctx));
            result += "]";
            return result;
        }
    };
}}}

#endif
