/*=============================================================================
    Copyright (c) 2001-2006 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying 
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(FUSION_STD_PAIR_ITERATOR_09262005_0934)
#define FUSION_STD_PAIR_ITERATOR_09262005_0934

#include <boost/fusion/iterator/iterator_facade.hpp>
#include <boost/type_traits/is_const.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/mpl/identity.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/int.hpp>
#include <boost/mpl/minus.hpp>
#include <boost/config/no_tr1/utility.hpp>

namespace boost { namespace fusion
{
    struct random_access_traversal_tag;

    template <typename Pair_, int N_>
    struct std_pair_iterator
        : iterator_facade<std_pair_iterator<Pair_, N_>, random_access_traversal_tag>
    {
        BOOST_MPL_ASSERT_RELATION(N_, >=, 0);
        BOOST_MPL_ASSERT_RELATION(N_, <=, 2);

        typedef mpl::int_<N_> index;
        typedef Pair_ pair_type;

        std_pair_iterator(Pair_& pair)
            : pair(pair) {}
        Pair_& pair;

        template <typename Iterator>
        struct value_of;

        template <typename Pair>
        struct value_of<std_pair_iterator<Pair, 0> >
            : mpl::identity<typename Pair::first_type> {};

        template <typename Pair>
        struct value_of<std_pair_iterator<Pair, 1> >
            : mpl::identity<typename Pair::second_type> {};

        template <typename Iterator>
        struct deref;

        template <typename Pair>
        struct deref<std_pair_iterator<Pair, 0> >
        {
            typedef typename
                mpl::if_<
                    is_const<Pair>
                  , typename Pair::first_type const&
                  , typename Pair::first_type&
                >::type
            type;

            static type
            call(std_pair_iterator<Pair, 0> const& iter)
            {
                return iter.pair.first;
            }
        };

        template <typename Pair>
        struct deref<std_pair_iterator<Pair, 1> >
        {
            typedef typename
                mpl::if_<
                    is_const<Pair>
                  , typename Pair::second_type const&
                  , typename Pair::second_type&
                >::type
            type;

            static type
            call(std_pair_iterator<Pair, 1> const& iter)
            {
                return iter.pair.second;
            }
        };

        template <typename Iterator, typename N>
        struct advance
        {
            typedef typename Iterator::index index;
            typedef typename Iterator::pair_type pair_type;
            typedef std_pair_iterator<pair_type, index::value + N::value> type;

            static type
            call(Iterator const& iter)
            {
                return type(iter.pair);
            }
        };

        template <typename Iterator>
        struct next : advance<Iterator, mpl::int_<1> > {};

        template <typename Iterator>
        struct prior : advance<Iterator, mpl::int_<-1> > {};

        template <typename I1, typename I2>
        struct distance : mpl::minus<typename I2::index, typename I1::index>
        {
            typedef typename
                mpl::minus<
                    typename I2::index, typename I1::index
                >::type 
            type;

            static type
            call(I1 const&, I2 const&)
            {
                return type();
            }
        };
    };
}}

#endif


