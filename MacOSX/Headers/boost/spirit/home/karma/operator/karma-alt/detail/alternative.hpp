//  Copyright (c) 2001-2008 Hartmut Kaiser
//  Copyright (c) 2001-2007 Joel de Guzman
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(SPIRIT_KARMA_ALTERNATIVE_MAR_01_2007_1124AM)
#define SPIRIT_KARMA_ALTERNATIVE_MAR_01_2007_1124AM

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once      // MS compatible compilers support #pragma once
#endif

#include <boost/spirit/home/support/unused.hpp>
#include <boost/spirit/home/support/attribute_of.hpp>
#include <boost/spirit/home/karma/domain.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/mpl/find_if.hpp>
#include <boost/mpl/deref.hpp>
#include <boost/mpl/distance.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/is_convertible.hpp>
#include <boost/variant.hpp>

namespace boost { namespace spirit { namespace karma { namespace detail
{
    ///////////////////////////////////////////////////////////////////////////
    //  A component is compatible to a given parameter type if the parameter
    //  is the same as the expected type of the component
    ///////////////////////////////////////////////////////////////////////////
    template <typename Expected, typename Parameter>
    struct is_compatible_component
    {
        typedef typename Parameter::types types;
        typedef typename mpl::end<types>::type end;
        typedef typename mpl::begin<types>::type begin;

        typedef typename
            mpl::find_if<
                types,
                is_same<mpl::_1, Expected>
            >::type
        iter;

        typedef typename mpl::not_<is_same<iter, end> >::type type;
        enum { value = type::value };
    };

    template <typename Expected>
    struct is_compatible_component<Expected, unused_type>
      : mpl::false_
    {};

    ///////////////////////////////////////////////////////////////////////////
    //  execute a generator if the given parameter type is compatible
    ///////////////////////////////////////////////////////////////////////////

    //  this get's instantiated if the parameter type is _not_ compatible with
    //  the generator
    template <typename Component, typename Parameter, typename Expected,
        typename Enable = void>
    struct alternative_generate
    {
        template <typename OutputIterator, typename Context, typename Delimiter>
        static bool
        call(Component const&, OutputIterator&, Context&, Delimiter const&, 
            Parameter const&)
        {
            return false;
        }
    };

    template <typename Component>
    struct alternative_generate<Component, unused_type, unused_type>
    {
        template <typename OutputIterator, typename Context, typename Delimiter>
        static bool
        call(Component const& component, OutputIterator& sink,
            Context& ctx, Delimiter const& delim, unused_type const&)
        {
            // return true if any of the generators succeed
            typedef typename Component::director director;
            return director::generate(component, sink, ctx, delim, unused);
        }
    };

    //  this get's instantiated if there is no parameter given for the
    //  alternative generator
    template <typename Component, typename Expected>
    struct alternative_generate<Component, unused_type, Expected>
      : alternative_generate<Component, unused_type, unused_type>
    {};

    //  this get's instantiated if the generator does not expect to receive a
    //  parameter (the generator is self contained).
    template <typename Component, typename Parameter>
    struct alternative_generate<Component, Parameter, unused_type>
      : alternative_generate<Component, unused_type, unused_type>
    {};

    //  this get's instantiated if the parameter type is compatible to the
    //  generator
    template <typename Component, typename Parameter, typename Expected>
    struct alternative_generate<
        Component, Parameter, Expected,
        typename enable_if<
            is_compatible_component<Expected, Parameter>
        >::type
    >
    {
        template <typename OutputIterator, typename Context, typename Delimiter>
        static bool
        call(Component const& component, OutputIterator& sink,
            Context& ctx, Delimiter const& delim, Parameter const& param)
        {
            typedef
                is_compatible_component<Expected, Parameter>
            component_type;

            typedef typename
                mpl::distance<
                    typename component_type::begin,
                    typename component_type::iter
                >::type
            distance_type;

            // make sure, the content of the passed variant matches our
            // expectations
            if (param.which() != distance_type::value)
                return false;

            // returns true if any of the generators succeed
            typedef
                typename mpl::deref<
                    typename component_type::iter
                >::type
            compatible_type;

            typedef typename Component::director director;
            return director::generate(component, sink, ctx, delim,
                get<compatible_type>(param));
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    //  alternative_generate_functor: a functor supplied to spirit::any which
    //  will be executed for every generator in a given alternative generator
    //  expression
    ///////////////////////////////////////////////////////////////////////////
    template <typename OutputIterator, typename Context, typename Delimiter,
        typename Parameter>
    struct alternative_generate_functor
    {
        alternative_generate_functor(OutputIterator& sink_, Context& ctx_,
              Delimiter const& d, Parameter const& p)
          : sink(sink_), ctx(ctx_), delim(d), param(p)
        {
        }

        template <typename Component>
        bool operator()(Component const& component)
        {
            typedef
                typename traits::attribute_of<
                    karma::domain, Component, Context>::type
            expected;
            typedef
                alternative_generate<Component, Parameter, expected>
            generate;

            return generate::call(component, sink, ctx, delim, param);
        }

        OutputIterator& sink;
        Context& ctx;
        Delimiter const& delim;
        Parameter const& param;
    };

}}}}

#endif
