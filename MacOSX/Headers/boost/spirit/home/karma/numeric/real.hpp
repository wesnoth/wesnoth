//  Copyright (c) 2001-2008 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_KARMA_REAL_FEB_26_2007_0512PM)
#define BOOST_SPIRIT_KARMA_REAL_FEB_26_2007_0512PM

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once      // MS compatible compilers support #pragma once
#endif

#include <boost/spirit/home/support/char_class.hpp>
#include <boost/spirit/home/support/modifier.hpp>
#include <boost/spirit/home/karma/domain.hpp>
#include <boost/spirit/home/karma/delimit.hpp>
#include <boost/spirit/home/karma/numeric/real_policies.hpp>
#include <boost/spirit/home/karma/char.hpp>
#include <boost/spirit/home/karma/numeric/int.hpp>
#include <boost/spirit/home/karma/numeric/detail/numeric_utils.hpp>
#include <boost/config/no_tr1/cmath.hpp>

namespace boost { namespace spirit { namespace karma
{
    namespace detail
    {
        template <typename RealPolicies>
        struct real_policy;
    }

    ///////////////////////////////////////////////////////////////////////////
    //  This specialization is used for real generators not having a direct
    //  initializer: float_, double_ etc. These generators must be used in
    //  conjunction with a parameter.
    ///////////////////////////////////////////////////////////////////////////
    template <typename T, typename RealPolicies, typename Tag>
    struct real_generator<false, T, RealPolicies, Tag>
    {
        template <typename Component, typename Context, typename Unused>
        struct attribute
        {
            typedef T type;
        };

        // double_/float_/etc. has a parameter attached
        template <typename Component, typename OutputIterator,
            typename Context, typename Delimiter, typename Parameter>
        static bool
        generate(Component const& component, OutputIterator& sink,
            Context& /*ctx*/, Delimiter const& d, Parameter const& param)
        {
            RealPolicies const& p = 
                detail::real_policy<RealPolicies>::get(spirit::subject(component));

            bool result = real_inserter<T, RealPolicies, Tag>::
                call(sink, param, p);

            karma::delimit(sink, d);           // always do post-delimiting
            return result;
        }

        // this double_/float_/etc. has no parameter attached, it needs to have
        // been initialized from a direct literal
        template <typename Component, typename OutputIterator,
            typename Context, typename Delimiter>
        static bool
        generate(Component const&, OutputIterator&, Context&, Delimiter const&,
            unused_type)
        {
            BOOST_MPL_ASSERT_MSG(false, real_not_usable_without_attribute,
                (Component, Context));
            return false;
        }

        template <typename Component, typename Context>
        static std::string what(Component const& /*component*/, Context const& /*ctx*/)
        {
            return "real number";
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    //  This specialization is used for real generators having a direct
    //  initializer: float_(10.), double_(20.) etc.
    ///////////////////////////////////////////////////////////////////////////
    template <typename T, typename RealPolicies, typename Tag>
    struct real_generator<true, T, RealPolicies, Tag>
    {
        template <typename Component, typename Context, typename Unused>
        struct attribute
        {
            typedef unused_type type;
        };

        template <typename Component, typename OutputIterator,
            typename Context, typename Delimiter, typename Parameter>
        static bool
        generate(Component const& component, OutputIterator& sink,
            Context& /*ctx*/, Delimiter const& d, Parameter const& /*param*/)
        {
            RealPolicies const& p = detail::real_policy<RealPolicies>::get(
                fusion::at_c<0>(component.elements));
            T n = fusion::at_c<1>(component.elements);
            bool result = real_inserter<T, RealPolicies, Tag>::call(sink, n, p);

            karma::delimit(sink, d);           // always do post-delimiting
            return result;
        }

        template <typename Component, typename Context>
        static std::string what(Component const& /*component*/, Context const& /*ctx*/)
        {
            return "real number";
        }
    };

}}}

namespace boost { namespace spirit { namespace traits
{
    ///////////////////////////////////////////////////////////////////////////
    // lower_case real generator
    ///////////////////////////////////////////////////////////////////////////
    template <typename Domain, typename Elements, typename Modifier,
      typename T, typename RealPolicies, typename Tag>
    struct make_modified_component<
        Domain, karma::real_generator<false, T, RealPolicies, Tag>, Elements, Modifier,
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
        real_policy_type;
        typedef fusion::vector<real_policy_type> vector_type;

        typedef karma::real_generator<false, T, RealPolicies, key_tag> real_type;
        typedef component<karma::domain, real_type, vector_type> type;

        static type
        call(Elements const& elements)
        {
            return type(elements);
        }
    };

    template <typename Domain, typename Elements, typename Modifier,
      typename T, typename RealPolicies, typename Tag>
    struct make_modified_component<
        Domain, karma::real_generator<true, T, RealPolicies, Tag>, Elements, Modifier,
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
        real_policy_type;
        typedef typename
            fusion::result_of::value_at_c<Elements, 1>::type
        real_data_type;
        typedef fusion::vector<real_policy_type, real_data_type> vector_type;

        typedef karma::real_generator<true, T, RealPolicies, key_tag> real_type;
        typedef component<karma::domain, real_type, vector_type> type;

        static type
        call(Elements const& elements)
        {
            return type(elements);
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    // lower_case real generator
    ///////////////////////////////////////////////////////////////////////////
    template <typename Domain, typename Elements, typename Modifier,
      typename T, typename RealPolicies, typename Tag>
    struct make_modified_component<
        Domain, karma::real_generator<false, T, RealPolicies, Tag>, Elements, Modifier,
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
        real_policy_type;
        typedef fusion::vector<real_policy_type> vector_type;

        typedef karma::real_generator<false, T, RealPolicies, key_tag> real_type;
        typedef component<karma::domain, real_type, vector_type> type;

        static type
        call(Elements const& elements)
        {
            return type(elements);
        }
    };

    template <typename Domain, typename Elements, typename Modifier,
      typename T, typename RealPolicies, typename Tag>
    struct make_modified_component<
        Domain, karma::real_generator<true, T, RealPolicies, Tag>, Elements, Modifier,
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
        real_policy_type;
        typedef typename
            fusion::result_of::value_at_c<Elements, 1>::type
        real_data_type;
        typedef fusion::vector<real_policy_type, real_data_type> vector_type;

        typedef karma::real_generator<true, T, RealPolicies, key_tag> real_type;
        typedef component<karma::domain, real_type, vector_type> type;

        static type
        call(Elements const& elements)
        {
            return type(elements);
        }
    };

}}}   // namespace boost::spirit::traits

#endif // defined(BOOST_SPIRIT_KARMA_REAL_FEB_26_2007_0512PM)
