//  Copyright (c) 2001-2007 Joel de Guzman
//  Copyright (c) 2001-2008 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_KARMA_RULE_MAR_05_2007_0519PM)
#define BOOST_SPIRIT_KARMA_RULE_MAR_05_2007_0519PM

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once      // MS compatible compilers support #pragma once
#endif

#include <boost/spirit/home/support/unused.hpp>
#include <boost/spirit/home/support/attribute_of.hpp>
#include <boost/spirit/home/karma/domain.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/detail/atomic_count.hpp>
#include <boost/mpl/eval_if.hpp>
#include <boost/mpl/identity.hpp>
#include <boost/mpl/size.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/mpl/not.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/equal_to.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/add_const.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <boost/function_types/is_function.hpp>
#include <boost/assert.hpp>
#include <boost/fusion/include/pop_front.hpp>
#include <boost/fusion/include/is_sequence.hpp>

namespace boost { namespace spirit { namespace karma { namespace detail
{
    struct no_delimiter
    {
        // this struct accepts only unused types and
        // nothing else. This is used by the second
        // pure virtual parse member function of
        // virtual_component_base below.

        no_delimiter(unused_type) {}
    };

    template <typename OutputIterator, typename Context, typename Delimiter>
    struct virtual_component_base
    {
        struct take_no_delimiter {};

        typedef typename
            mpl::eval_if<
                is_same<Delimiter, unused_type>,
                mpl::identity<take_no_delimiter>,
                result_of::as_component<karma::domain, Delimiter>
            >::type
        delimiter_type;

        virtual_component_base()
          : use_count(0)
        {
        }

        virtual ~virtual_component_base()
        {
        }

        virtual bool
        generate(OutputIterator& sink, Context& context,
            delimiter_type const& delim) = 0;

        virtual bool
        generate(OutputIterator& sink, Context& context, no_delimiter) = 0;

        boost::detail::atomic_count use_count;
    };

    template <typename OutputIterator, typename Context, typename Delimiter>
    inline void
    intrusive_ptr_add_ref(
        virtual_component_base<OutputIterator, Context, Delimiter>* p)
    {
        ++p->use_count;
    }

    template <typename OutputIterator, typename Context, typename Delimiter>
    inline void
    intrusive_ptr_release(
        virtual_component_base<OutputIterator, Context, Delimiter>* p)
    {
        if (--p->use_count == 0)
            delete p;
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename OutputIterator, typename Component, typename Context,
        typename Delimiter, typename Auto>
    struct virtual_component
      : virtual_component_base<OutputIterator, Context, Delimiter>
    {
        typedef
            virtual_component_base<OutputIterator, Context, Delimiter>
        base_type;
        typedef typename base_type::delimiter_type delimiter_type;
        typedef typename base_type::take_no_delimiter take_no_delimiter;

        virtual_component(Component const& component)
          : component(component)
        {
        }

        virtual ~virtual_component()
        {
        }

        template <typename Delimiter_>
        bool generate_main(OutputIterator& sink, Context& context,
            Delimiter_ const& delim, mpl::false_)
        {
            // If Auto is false, we synthesize a new (default constructed) 
            // attribute instance based on the attributes of the embedded 
            // generator.
            typename traits::attribute_of<
                    karma::domain, Component, Context
                >::type param;
                
            typedef typename Component::director director;
            return director::generate(component, sink, context, delim, param);
        }

        template <typename Delimiter_>
        bool generate_main(OutputIterator& sink, Context& context,
            Delimiter_ const& delim, mpl::true_)
        {
            //  If Auto is true, we pass the rule's attribute on to the
            //  component.
            typedef typename Component::director director;
            return director::generate(component, sink, context, delim,
                fusion::at_c<0>(fusion::at_c<0>(context)));
        }

        bool
        generate_main(OutputIterator&, Context&, take_no_delimiter, mpl::false_)
        {
            BOOST_ASSERT(false); // this should never be called
            return false;
        }

        bool
        generate_main(OutputIterator&, Context&, take_no_delimiter, mpl::true_)
        {
            BOOST_ASSERT(false); // this should never be called
            return false;
        }

        virtual bool
        generate(OutputIterator& sink, Context& context,
            delimiter_type const& delim)
        {
            return generate_main(sink, context, delim, Auto());
        }

        virtual bool
        generate(OutputIterator& sink, Context& context, no_delimiter)
        {
            return generate_main(sink, context, unused, Auto());
        }

        Component component;
    };

}}}}

#endif
