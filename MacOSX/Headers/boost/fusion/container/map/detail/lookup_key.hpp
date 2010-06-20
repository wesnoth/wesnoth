/*=============================================================================
    Copyright (c) 2001-2006 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying 
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(FUSION_LOOKUP_KEY_07222005_1248)
#define FUSION_LOOKUP_KEY_07222005_1248

#include <boost/mpl/int.hpp>
#include <boost/type_traits/add_const.hpp>
#include <boost/type_traits/add_reference.hpp>
#include <boost/fusion/support/detail/unknown_key.hpp>

namespace boost { namespace fusion 
{
    struct void_;    
}}

namespace boost { namespace fusion { namespace detail 
{
    template <typename T>
    struct map_data_type
    {
        typedef typename 
            add_reference<
                typename T::second_type
            >::type 
        type;
    };

    template <>
    struct map_data_type<void_>
    {
        typedef void_& type;
    };

    template <typename T>
    struct map_const_data_type
    {
        typedef typename 
            add_reference<
                typename add_const<
                    typename T::second_type
                >::type
            >::type 
        type;
    };

    template <>
    struct map_const_data_type<void_>
    {
        typedef void_ const& type;
    };

    template <typename T>
    struct map_value_type
    {
        typedef typename T::second_type type;
    };

    template <>
    struct map_value_type<void_>
    {
        typedef void_ type;
    };

    template <typename T, int index>
    struct map_key_type
    {
        typedef typename T::first_type type;
    };

    template <int index>
    struct map_key_type<void_, index>
    {
        typedef unknown_key<index> type;
    };

    template <int index, typename RT, typename Key, typename Vector>
    struct map_lookup_key
    {
        static RT
        call(Vector& vec)
        {
            return vec.at_impl(mpl::int_<index>()).second;
        }
    };

    template <int index, typename Vector>
    struct map_lookup_key<index, void_&, unknown_key<index>, Vector>
    {
        static void_&
        call(Vector& vec); // intentionally undefined
    };
}}} 

#endif

