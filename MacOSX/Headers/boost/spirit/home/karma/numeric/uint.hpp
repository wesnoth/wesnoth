//  Copyright (c) 2001-2008 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_KARMA_UINT_FEB_23_2007_0840PM)
#define BOOST_SPIRIT_KARMA_UINT_FEB_23_2007_0840PM

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once      // MS compatible compilers support #pragma once
#endif

#include <limits>

#include <boost/spirit/home/support/modifier.hpp>
#include <boost/spirit/home/karma/domain.hpp>
#include <boost/spirit/home/karma/delimit.hpp>
#include <boost/spirit/home/karma/numeric/numeric_fwd.hpp>
#include <boost/spirit/home/karma/numeric/detail/numeric_utils.hpp>
#include <boost/fusion/include/at.hpp>
#include <boost/fusion/include/value_at.hpp>
#include <boost/mpl/assert.hpp>

namespace boost { namespace spirit { namespace karma
{
    template <typename T, unsigned Radix, bool ForceSign, typename Tag>
    struct uint_generator<false, T, Radix, ForceSign, Tag>
    {
        template <typename Component, typename Context, typename Unused>
        struct attribute
        {
            typedef T type;
        };

        // check template parameter 'Radix' for validity
        BOOST_MPL_ASSERT_MSG(
            Radix == 2 || Radix == 8 || Radix == 10 || Radix == 16,
            not_supported_radix, ());

        BOOST_MPL_ASSERT_MSG(!std::numeric_limits<T>::is_signed,
            signed_unsigned_mismatch, ());

        // uint has a parameter attached
        template <typename Component, typename OutputIterator,
            typename Context, typename Delimiter, typename Parameter>
        static bool
        generate(Component const& /*component*/, OutputIterator& sink,
            Context& /*ctx*/, Delimiter const& d, Parameter const& param)
        {
            bool result = int_inserter<Radix, Tag>::call(sink, param);
            karma::delimit(sink, d);           // always do post-delimiting
            return result;
        }

        // this uint has no parameter attached, it needs to have been
        // initialized from a direct literal
        template <typename Component, typename OutputIterator,
            typename Context, typename Delimiter>
        static bool
        generate(Component const&, OutputIterator&, Context&, Delimiter const&,
            unused_type)
        {
            BOOST_MPL_ASSERT_MSG(false, uint_not_usable_without_attribute, ());
            return false;
        }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            return "unsigned integer";
        }
    };

    template <typename T, unsigned Radix, bool ForceSign, typename Tag>
    struct uint_generator<true, T, Radix, ForceSign, Tag>
    {
        template <typename Component, typename Context, typename Unused>
        struct attribute
        {
            typedef unused_type type;
        };

        // check template parameter 'Radix' for validity
        BOOST_MPL_ASSERT_MSG(
            Radix == 2 || Radix == 8 || Radix == 10 || Radix == 16,
            not_supported_radix, ());

        BOOST_MPL_ASSERT_MSG(!std::numeric_limits<T>::is_signed,
            signed_unsigned_mismatch, ());

        template <typename Component, typename OutputIterator,
            typename Context, typename Delimiter, typename Parameter>
        static bool
        generate(Component const& component, OutputIterator& sink,
            Context& /*ctx*/, Delimiter const& d, Parameter const& /*param*/)
        {
            T n = fusion::at_c<0>(component.elements);
            bool result = int_inserter<Radix, Tag>::call(sink, n);
            karma::delimit(sink, d);           // always do post-delimiting
            return result;
        }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            return "unsigned integer";
        }
    };

}}}

namespace boost { namespace spirit { namespace traits
{
    ///////////////////////////////////////////////////////////////////////////
    // lower_case int_ generator
    ///////////////////////////////////////////////////////////////////////////
    template <typename Domain, typename Elements, typename Modifier,
      typename T, unsigned Radix, bool ForceSign, typename Tag>
    struct make_modified_component<
        Domain, karma::uint_generator<false, T, Radix, ForceSign, Tag>, Elements, Modifier,
        typename enable_if<
            is_member_of_modifier<Modifier, spirit::char_class::lower_case_base_tag>
        >::type
    >
    {
        typedef typename Modifier::char_set char_set;
        typedef spirit::char_class::tag::lower char_class_;
        typedef spirit::char_class::key<char_set, char_class_> key_tag;

        typedef karma::uint_generator<false, T, Radix, ForceSign, key_tag> int_type;
        typedef component<karma::domain, int_type, fusion::nil> type;

        static type
        call(Elements const&)
        {
            return type(fusion::nil());
        }
    };

    template <typename Domain, typename Elements, typename Modifier,
      typename T, unsigned Radix, bool ForceSign, typename Tag>
    struct make_modified_component<
        Domain, karma::uint_generator<true, T, Radix, ForceSign, Tag>, Elements, Modifier,
        typename enable_if<
            is_member_of_modifier<Modifier, spirit::char_class::lower_case_base_tag>
        >::type
    >
    {
        typedef typename Modifier::char_set char_set;
        typedef spirit::char_class::tag::lower char_class_;
        typedef spirit::char_class::key<char_set, char_class_> key_tag;

        typedef typename
            fusion::result_of::value_at_c<Elements, 0>::type
        int_data_type;
        typedef fusion::vector<int_data_type> vector_type;

        typedef karma::uint_generator<true, T, Radix, ForceSign, key_tag> int_type;
        typedef component<karma::domain, int_type, vector_type> type;

        static type
        call(Elements const& elements)
        {
            return type(elements);
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    // upper_case int_ generator
    ///////////////////////////////////////////////////////////////////////////
    template <typename Domain, typename Elements, typename Modifier,
      typename T, unsigned Radix, bool ForceSign, typename Tag>
    struct make_modified_component<
        Domain, karma::uint_generator<false, T, Radix, ForceSign, Tag>, Elements, Modifier,
        typename enable_if<
            is_member_of_modifier<Modifier, spirit::char_class::upper_case_base_tag>
        >::type
    >
    {
        typedef typename Modifier::char_set char_set;
        typedef spirit::char_class::tag::upper char_class_;
        typedef spirit::char_class::key<char_set, char_class_> key_tag;

        typedef karma::uint_generator<false, T, Radix, ForceSign, key_tag> int_type;
        typedef component<karma::domain, int_type, fusion::nil> type;

        static type
        call(Elements const&)
        {
            return type(fusion::nil());
        }
    };

    template <typename Domain, typename Elements, typename Modifier,
      typename T, unsigned Radix, bool ForceSign, typename Tag>
    struct make_modified_component<
        Domain, karma::uint_generator<true, T, Radix, ForceSign, Tag>, Elements, Modifier,
        typename enable_if<
            is_member_of_modifier<Modifier, spirit::char_class::upper_case_base_tag>
        >::type
    >
    {
        typedef typename Modifier::char_set char_set;
        typedef spirit::char_class::tag::upper char_class_;
        typedef spirit::char_class::key<char_set, char_class_> key_tag;

        typedef typename
            fusion::result_of::value_at_c<Elements, 0>::type
        int_data_type;
        typedef fusion::vector<int_data_type> vector_type;

        typedef karma::uint_generator<true, T, Radix, ForceSign, key_tag> int_type;
        typedef component<karma::domain, int_type, vector_type> type;

        static type
        call(Elements const& elements)
        {
            return type(elements);
        }
    };

}}}   // namespace boost::spirit::traits

#endif
