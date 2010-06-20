/*=============================================================================
    Copyright (c) 2001-2006 Joel de Guzman
    Copyright (c) 2005-2006 Dan Marsden

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(BOOST_FUSION_STD_PAIR_24122005_1744)
#define BOOST_FUSION_STD_PAIR_24122005_1744

#include <boost/fusion/support/tag_of_fwd.hpp>
#include <boost/fusion/adapted/struct.hpp>
#include <boost/mpl/int.hpp>
#include <boost/config/no_tr1/utility.hpp>

namespace boost { namespace fusion
{
    struct struct_tag;

    namespace traits
    {
        template <typename T1, typename T2>
#if defined(BOOST_NO_PARTIAL_SPECIALIZATION_IMPLICIT_DEFAULT_ARGS)
        struct tag_of<std::pair<T1, T2>, void >
#else
        struct tag_of<std::pair<T1, T2> >
#endif
        {
            typedef struct_tag type;
        };
    }

    namespace extension
    {
        template <typename Struct, int N>
        struct struct_member;

        template <typename Struct>
        struct struct_size;

        template <typename T1, typename T2>
        struct struct_member<std::pair<T1, T2>, 0>
        {
            typedef T1 type;

            static type& call(std::pair<T1, T2>& pair)
            {
                return pair.first;
            }
        };

        template <typename T1, typename T2>
        struct struct_member<std::pair<T1, T2>, 1>
        {
            typedef T2 type;

            static type& call(std::pair<T1, T2>& pair)
            {
                return pair.second;
            }
        };

        template <typename T1, typename T2>
        struct struct_size<std::pair<T1, T2> > : mpl::int_<2>
        {
        };
    }
}}

#endif
