/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(BOOST_SPIRIT_RULE_FEB_12_2007_0440PM)
#define BOOST_SPIRIT_RULE_FEB_12_2007_0440PM

#include <boost/spirit/home/qi/nonterminal/virtual_component_base.hpp>
#include <boost/assert.hpp>

namespace boost { namespace spirit { namespace qi { namespace detail
{
    template <
        typename Iterator, typename Component
      , typename Context, typename Skipper
      , typename Auto
    >
    struct virtual_component : virtual_component_base<Iterator, Context, Skipper>
    {
        typedef virtual_component_base<Iterator, Context, Skipper> base_type;
        typedef typename base_type::skipper_type skipper_type;
        typedef typename base_type::take_no_skipper take_no_skipper;

        virtual_component(Component const& component)
          : component(component)
        {
        }

        virtual ~virtual_component()
        {
        }

        template <typename T>
        static void clear(T& attr)
        {
            attr = T();
        }

        template <typename Skipper_>
        bool parse_main(
            Iterator& first
          , Iterator const& last
          , Context& context
          , Skipper_ const& skipper
          , mpl::false_)
        {
            // If Auto is false, the component's attribute is unused.

            typedef typename Component::director director;
            return director::parse(
                component, first, last, context, skipper, unused);
        }

        template <typename Skipper_>
        bool parse_main(
            Iterator& first
          , Iterator const& last
          , Context& context
          , Skipper_ const& skipper
          , mpl::true_)
        {
            // If Auto is true, we synthesize the rule's attribute and pass
            // it on to the component. On successful parse, this attribute
            // is swapped back to the the rule's attribute.

            typename
                remove_reference<
                    typename fusion::result_of::value_at_c<
                        typename fusion::result_of::value_at_c<Context, 0>::type
                      , 0
                    >::type
                >::type
            attribute; // default constructed
            typedef typename Component::director director;
            if (director::parse(
                component, first, last, context, skipper, attribute))
            {
                // $$$ need to optimize this for fusion sequences $$$
                std::swap(fusion::at_c<0>(fusion::at_c<0>(context)), attribute);
                return true;
            }
            return false;
        }

        bool parse_main(
            Iterator& /*first*/
          , Iterator const& /*last*/
          , Context&
          , take_no_skipper
          , mpl::false_)
        {
            BOOST_ASSERT(false); // this should never be called
            return false;
        }

        bool parse_main(
            Iterator& /*first*/
          , Iterator const& /*last*/
          , Context& /*context*/
          , take_no_skipper
          , mpl::true_)
        {
            BOOST_ASSERT(false); // this should never be called
            return false;
        }

        virtual bool
        parse(
            Iterator& first
          , Iterator const& last
          , Context& context
          , skipper_type const& skipper)
        {
            return parse_main(first, last, context, skipper, Auto());
        }

        virtual bool
        parse(
            Iterator& first
          , Iterator const& last
          , Context& context
          , no_skipper)
        {
            return parse_main(first, last, context, unused, Auto());
        }

        Component component;
    };
}}}}

#endif
