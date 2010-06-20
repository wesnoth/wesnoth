//  Copyright (c) 2001-2007 Joel de Guzman
//  Copyright (c) 2001-2008 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(SPIRIT_KARMA_OPTIONAL_MARCH_31_2007_0852AM)
#define SPIRIT_KARMA_OPTIONAL_MARCH_31_2007_0852AM

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once      // MS compatible compilers support #pragma once
#endif

#include <boost/spirit/home/support/component.hpp>
#include <boost/spirit/home/support/attribute_of.hpp>
#include <boost/spirit/home/support/attribute_transform.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/optional.hpp>

namespace boost { namespace spirit { namespace karma
{
    namespace detail
    {
        template <typename Parameter>
        struct optional_attribute
        {
            static inline bool
            is_valid(boost::optional<Parameter> const& opt)
            {
                return opt;
            }

            static inline bool
            is_valid(Parameter const&)
            {
                return true;
            }

            static inline bool
            is_valid(unused_type)
            {
                return true;
            }

            static inline Parameter const&
            get(boost::optional<Parameter> const& opt)
            {
                return boost::get(opt);
            }

            static inline Parameter const&
            get(Parameter const& p)
            {
                return p;
            }

            static inline unused_type
            get(unused_type)
            {
                return unused;
            }
        };
    }

    struct optional
    {
        template <typename T>
        struct build_attribute_container
        {
            typedef boost::optional<T> type;
        };

        template <typename Component, typename Context, typename Iterator>
        struct attribute :
            build_container<optional, Component, Iterator, Context>
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

            typedef typename traits::attribute_of<
                karma::domain, typename result_of::subject<Component>::type, 
                Context, unused_type
            >::type attribute_type;

            typedef detail::optional_attribute<attribute_type> optional_type;
            if (optional_type::is_valid(param))
            {
                director::generate(subject(component), sink, ctx, d,
                    optional_type::get(param));
            }
            return true;
        }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            std::string result = "optional[";

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
