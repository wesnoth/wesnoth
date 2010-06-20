/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(BOOST_SPIRIT_RANGE_RUN_MAY_16_2006_0807_PM)
#define BOOST_SPIRIT_RANGE_RUN_MAY_16_2006_0807_PM

#include <boost/spirit/home/qi/char/detail/range_functions.hpp>
#include <boost/assert.hpp>
#include <boost/integer_traits.hpp>
#include <algorithm>

namespace boost { namespace spirit { namespace qi { namespace detail
{
    template <typename Run, typename Iterator, typename Range>
    inline bool
    try_merge(Run& run, Iterator iter, Range const& range)
    {
        // if *iter intersects with, or is adjacent to, 'range'...
        if (can_merge(*iter, range))
        {
            typedef typename Range::value_type value_type;
            typedef integer_traits<value_type> integer_traits;

            // merge range and *iter
            merge(*iter, range);

            // collapse all subsequent ranges that can merge with *iter
            Iterator i;
            value_type last =
                iter->last == integer_traits::const_max
                ? iter->last : iter->last+1;

            for (i = iter+1; i != run.end() && last >= i->first; ++i)
            {
                iter->last = i->last;
            }
            // erase all ranges that were collapsed
            run.erase(iter+1, i);
            return true;
        }
        return false;
    }

    template <typename Char>
    inline bool
    range_run<Char>::test(Char val) const
    {
        if (run.empty())
            return false;

        // search the ranges for one that potentially includes val
        typename storage_type::const_iterator iter =
            std::upper_bound(
                run.begin(), run.end(), val,
                range_compare<range_type>()
            );

        // return true if *(iter-1) includes val
        return iter != run.begin() && includes(*(--iter), val);
    }

    template <typename Char>
    inline void
    range_run<Char>::swap(range_run& other)
    {
        run.swap(other.run);
    }

    template <typename Char>
    void
    range_run<Char>::set(range_type const& range)
    {
        BOOST_ASSERT(is_valid(range));
        if (run.empty())
        {
            // the vector is empty, insert 'range'
            run.push_back(range);
            return;
        }

        // search the ranges for one that potentially includes 'range'
        typename storage_type::iterator iter =
            std::upper_bound(
                run.begin(), run.end(), range,
                range_compare<range_type>()
            );

        if (iter != run.begin())
        {
            // if *(iter-1) includes 'range', return early
            if (includes(*(iter-1), range))
            {
                return;
            }

            // if *(iter-1) can merge with 'range', merge them and return
            if (try_merge(run, iter-1, range))
            {
                return;
            }
        }

        // if *iter can merge with with 'range', merge them
        if (iter == run.end() || !try_merge(run, iter, range))
        {
            // no overlap, insert 'range'
            run.insert(iter, range);
        }
    }

    template <typename Char>
    void
    range_run<Char>::clear(range_type const& range)
    {
        BOOST_ASSERT(is_valid(range));
        if (!run.empty())
        {
            // search the ranges for one that potentially includes 'range'
            typename storage_type::iterator iter =
                std::upper_bound(
                    run.begin(), run.end(), range,
                    range_compare<range_type>()
                );

            typename storage_type::iterator left_iter;

            // if *(iter-1) includes the 'range.first',
            if ((iter != run.begin()) &&
                includes(*(left_iter = (iter-1)), range.first))
            {
                // if the 'range' is in the middle,
                if (left_iter->last > range.last)
                {
                    // break it apart into two ranges (punch a hole)
                    Char save_last = left_iter->last;
                    left_iter->last = range.first-1;
                    run.insert(iter, range_type(range.last+1, save_last));
                    return;
                }
                else // if it is not in the middle,
                {
                    // truncate it (clip its right)
                    left_iter->last = range.first-1;
                }
            }

            // position i to the first range that 'range'
            // does not intersect with
            typename storage_type::iterator i = iter;
            while (i != run.end() && includes(range, *i))
            {
                i++;
            }

            // if *i includes 'range.last', truncate it (clip its left)
            if (i != run.end() && includes(*i, range.last))
            {
                i->first = range.last+1;
            }

            // cleanup... erase all subsequent ranges that the
            // 'range' includes
            run.erase(iter, i);
        }
    }

    template <typename Char>
    inline void
    range_run<Char>::clear()
    {
        run.clear();
    }
}}}}

#endif
