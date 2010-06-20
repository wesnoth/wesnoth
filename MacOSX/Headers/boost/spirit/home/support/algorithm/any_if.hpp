//  Copyright (c) 2001-2008 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_ANY_IF_MAR_30_2007_1220PM)
#define BOOST_SPIRIT_ANY_IF_MAR_30_2007_1220PM

#include <boost/fusion/include/equal_to.hpp>
#include <boost/fusion/include/next.hpp>
#include <boost/fusion/include/deref.hpp>
#include <boost/fusion/include/value_of.hpp>
#include <boost/fusion/include/begin.hpp>
#include <boost/fusion/include/end.hpp>
#include <boost/fusion/include/is_sequence.hpp>
#include <boost/fusion/include/any.hpp>
#include <boost/fusion/include/make_cons.hpp>
#include <boost/spirit/home/support/unused.hpp>

#include <boost/mpl/bool.hpp>
#include <boost/mpl/print.hpp>
#include <boost/mpl/apply.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/identity.hpp>
#include <boost/utility/enable_if.hpp>

namespace boost { namespace spirit
{
    ///////////////////////////////////////////////////////////////////////////
    //  This is a special version for a binary fusion::any. The predicate is
    //  used to decide, whether to advance the second iterator or not.
    //  This is needed for sequences containing components with unused
    //  attributes.
    //  The second iterator is advanced only if the attribute of the 
    //  corresponding component iterator is not unused.
    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        ///////////////////////////////////////////////////////////////////////
        template <typename Iterator, typename Pred>
        struct apply_predicate
        {
            typedef typename
                mpl::apply1<
                    Pred,
                    typename fusion::result_of::value_of<Iterator>::type
                >::type
            type;
        };

        ///////////////////////////////////////////////////////////////////////
        //  if the predicate is true, attribute_next returns next(Iterator2),
        //  otherwise Iterator2
        namespace result_of
        {
            template <typename Iterator1, typename Iterator2, typename Pred>
            struct attribute_next
            {
                typedef typename apply_predicate<Iterator1, Pred>::type pred;
                typedef typename
                    mpl::eval_if<
                        pred,
                        fusion::result_of::next<Iterator2>,
                        mpl::identity<Iterator2>
                    >::type
                type;

                template <typename Iterator>
                static type
                call(Iterator const& i, mpl::true_)
                {
                    return fusion::next(i);
                }

                template <typename Iterator>
                static type
                call(Iterator const& i, mpl::false_)
                {
                    return i;
                }

                template <typename Iterator>
                static type
                call(Iterator const& i)
                {
                    return call(i, pred());
                }
            };
        }

        template <typename Pred, typename Iterator1, typename Iterator2>
        inline typename
            result_of::attribute_next<Iterator1, Iterator2, Pred
        >::type const
        attribute_next(Iterator2 const& i)
        {
            return result_of::attribute_next<Iterator1, Iterator2, Pred>::call(i);
        }

        ///////////////////////////////////////////////////////////////////////
        //  if the predicate is true, attribute_value returns deref(Iterator2),
        //  otherwise unused
        namespace result_of
        {
            template <typename Iterator1, typename Iterator2, typename Pred>
            struct attribute_value
            {
                typedef typename apply_predicate<Iterator1, Pred>::type pred;
                typedef typename
                    mpl::eval_if<
                        pred,
                        fusion::result_of::deref<Iterator2>,
                        mpl::identity<unused_type const>
                    >::type
                type;

                template <typename Iterator>
                static type
                call(Iterator const& i, mpl::true_)
                {
                    return fusion::deref(i);
                }

                template <typename Iterator>
                static type
                call(Iterator const&, mpl::false_)
                {
                    return unused;
                }

                template <typename Iterator>
                static type
                call(Iterator const& i)
                {
                    return call(i, pred());
                }
            };
        }

        template <typename Pred, typename Iterator1, typename Iterator2>
        inline typename
            result_of::attribute_value<Iterator1, Iterator2, Pred
        >::type
        attribute_value(Iterator2 const& i)
        {
            return result_of::attribute_value<Iterator1, Iterator2, Pred>::call(i);
        }

        ///////////////////////////////////////////////////////////////////////
        template <
            typename Pred, typename First1, typename Last, typename First2,
            typename F
        >
        inline bool
        any_if (First1 const&, First2 const&, Last const&, F const&, mpl::true_)
        {
            return false;
        }

        template <
            typename Pred, typename First1, typename Last, typename First2,
            typename F
        >
        inline bool
        any_if (First1 const& first1, First2 const& first2, Last const& last,
            F& f, mpl::false_)
        {
            return f(*first1, attribute_value<Pred, First1>(first2)) ||
                detail::any_if<Pred>(
                    fusion::next(first1)
                  , attribute_next<Pred, First1>(first2)
                  , last
                  , f
                  , fusion::result_of::equal_to<
                        typename fusion::result_of::next<First1>::type, Last>());
        }
    }

    template <typename Pred, typename Sequence1, typename Sequence2, typename F>
    inline typename enable_if<fusion::traits::is_sequence<Sequence2>, bool>::type
    any_if(Sequence1 const& seq1, Sequence2& seq2, F f, Pred)
    {
        return detail::any_if<Pred>(
                fusion::begin(seq1)
              , fusion::begin(seq2)
              , fusion::end(seq1)
              , f
              , fusion::result_of::equal_to<
                    typename fusion::result_of::begin<Sequence1>::type
                  , typename fusion::result_of::end<Sequence1>::type>());
    }

    template <typename Pred, typename Sequence1, typename Attribute, typename F>
    inline typename disable_if<fusion::traits::is_sequence<Attribute>, bool>::type
    any_if(Sequence1 const& seq1, Attribute& attr, F f, Pred /*p*/)
    {
        typename
            fusion::result_of::make_cons<Attribute&>::type
        seq2(attr); // wrap attribute in a single element tuple

        return detail::any_if<Pred>(
                fusion::begin(seq1)
              , fusion::begin(seq2)
              , fusion::end(seq1)
              , f
              , fusion::result_of::equal_to<
                    typename fusion::result_of::begin<Sequence1>::type
                  , typename fusion::result_of::end<Sequence1>::type>());
    }

    template <typename Pred, typename Sequence, typename F>
    inline bool
    any_if(Sequence const& seq, unused_type const, F f, Pred)
    {
        return fusion::any(seq, f);
    }

}}

#endif

