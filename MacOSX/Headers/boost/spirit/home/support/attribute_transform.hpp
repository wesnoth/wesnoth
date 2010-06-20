/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman
    http://spirit.sourceforge.net/

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(BOOST_SPIRIT_ATTRIBUTE_TRANSFORM_DEC_17_2007_0718AM)
#define BOOST_SPIRIT_ATTRIBUTE_TRANSFORM_DEC_17_2007_0718AM

#include <boost/spirit/home/support/unused.hpp>
#include <boost/spirit/home/support/component.hpp>
#include <boost/spirit/home/support/attribute_of.hpp>
#include <boost/spirit/home/support/detail/values.hpp>
#include <boost/fusion/include/vector.hpp>
#include <boost/fusion/include/is_sequence.hpp>
#include <boost/variant/variant_fwd.hpp>
#include <boost/fusion/include/transform.hpp>
#include <boost/fusion/include/filter_if.hpp>
#include <boost/mpl/if.hpp>
#include <boost/type_traits/is_same.hpp>

namespace boost { namespace spirit
{
    // Generalized attribute transformation utilities for Qi parsers

    namespace traits
    {
        using boost::spirit::detail::not_is_variant;

        // Here, we provide policies for stripping single element fusion
        // sequences. Add more specializations as needed.
        template <typename T, typename IsSequence, typename Enable = void>
        struct strip_single_element_sequence
        {
            typedef T type;
        };

        template <typename T>
        struct strip_single_element_sequence<
            fusion::vector<T>, mpl::false_,
            typename boost::enable_if<not_is_variant<T> >::type
        >
        {
            //  Strips single element fusion vectors into its 'naked'
            //  form: vector<T> --> T
            typedef T type;
        };

        template <typename T>
        struct strip_single_element_sequence<
            fusion::vector<T>, mpl::true_,
            typename boost::enable_if<not_is_variant<T> >::type
        >
        {
            //  Strips single element fusion vectors into its 'naked'
            //  form: vector<T> --> T, but does so only if T is not a fusion 
            //  sequence itself
            typedef typename 
                mpl::if_<
                    fusion::traits::is_sequence<T>,
                    fusion::vector<T>,
                    T
                >::type 
            type;
        };

        template <BOOST_VARIANT_ENUM_PARAMS(typename T), typename IsSequence>
        struct strip_single_element_sequence<
                fusion::vector<boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)> > 
              , IsSequence
            >
        {
            //  Exception: Single element variants are not stripped!
            typedef fusion::vector<boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)> > type;
        };
    }

    // Use this when building heterogeneous fusion sequences
    // Note:
    //
    //      Director should have these nested metafunctions
    //
    //      1:  build_container<All, Filtered>
    //
    //          All: all child attributes
    //          Filtered: all child attributes except unused
    //
    //      2:  transform_child<T>
    //
    //          T: child attribute
    //
    template <
        typename Director, typename Component
      , typename Iterator, typename Context
      , typename IsSequence = mpl::false_>
    struct build_fusion_sequence
    {
        template <
            typename Domain, typename Director_
          , typename Iterator_, typename Context_>
        struct child_attribute
        {
            template <typename T>
            struct result;

            template <typename F, typename ChildComponent>
            struct result<F(ChildComponent)>
            {
                typedef typename
                    Director_::template transform_child<
                        typename traits::attribute_of<
                            Domain, ChildComponent, Context_, Iterator_>::type
                    >::type
                type;
            };
        };

        // Compute the list of attributes of all sub-parsers
        typedef
            typename fusion::result_of::transform<
                typename Component::elements_type
              , child_attribute<
                    typename Component::domain, Director, Iterator, Context>
            >::type
        all;

        // Compute the list of all *used* attributes of sub-parsers
        // (filter all unused parsers from the list)
        typedef
            typename fusion::result_of::filter_if<
                all
              , spirit::traits::is_not_unused<mpl::_>
            >::type
        filtered;

        // Ask the director to build the actual fusion sequence.
        // But *only if* the filtered sequence is not empty. i.e.
        // if the sequence has all unused elements, our result
        // will also be unused.
        typedef
            typename mpl::eval_if<
                fusion::result_of::empty<filtered>
              , mpl::identity<unused_type>
              , typename Director::template build_container<all, filtered>
            >::type
        attribute_sequence;

        // Finally, strip single element sequences into its
        // naked form (e.g. vector<T> --> T)
        typedef typename
            traits::strip_single_element_sequence<attribute_sequence, IsSequence>::type
        type;
    };

    // Use this when building homogeneous containers. Component
    // is assumed to be a unary. Note:
    //
    //      Director should have this nested metafunction
    //
    //      1:  build_attribute_container<T>
    //
    //          T: the data-type for the container
    //
    template <
        typename Director, typename Component
      , typename Iterator, typename Context>
    struct build_container
    {
        // Get the component's subject.
        typedef typename
            result_of::subject<Component>::type
        subject_type;

        // Get the subject's attribute
        typedef typename
            traits::attribute_of<
                typename Component::domain, subject_type, Context, Iterator>::type
        attr_type;

        // If attribute is unused_type, return it as it is.
        // If not, then ask the director to build the actual
        // container for the attribute type.
        typedef typename
            mpl::if_<
                is_same<unused_type, attr_type>
              , unused_type
              , typename Director::template
                    build_attribute_container<attr_type>::type
            >::type
        type;
    };
}}

#endif
