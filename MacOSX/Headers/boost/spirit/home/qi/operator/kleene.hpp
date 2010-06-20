/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(SPIRIT_KLEENE_JAN_07_2007_0818AM)
#define SPIRIT_KLEENE_JAN_07_2007_0818AM

#include <boost/spirit/home/qi/domain.hpp>
#include <boost/spirit/home/support/component.hpp>
#include <boost/spirit/home/support/detail/container.hpp>
#include <boost/spirit/home/support/attribute_transform.hpp>
#include <vector>

namespace boost { namespace spirit { namespace qi
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
                traits::attribute_of<
                    qi::domain, subject_type, Context, Iterator>::type
            attr_type;
            typedef typename subject_type::director director;

            // create a value if Attribute is not unused_type
            typename mpl::if_<
                is_same<typename remove_const<Attribute>::type, unused_type>
              , unused_type
              , attr_type>::type
            val;

            while(
                director::parse(
                    subject(component)
                  , first, last, context, skipper, val)
                )
            {
                container::push_back(attr, val);
            }
            return true;
        }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            std::string result = "kleene[";

            typedef typename
                result_of::subject<Component>::type::director
            director;

            result += director::what(subject(component), ctx);
            result += "]";
            return result;
        }
    };
}}}

#endif
