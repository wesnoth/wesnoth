/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(BOOST_SPIRIT_TST_MARCH_09_2007_0905AM)
#define BOOST_SPIRIT_TST_MARCH_09_2007_0905AM

#include <boost/call_traits.hpp>
#include <boost/detail/iterator.hpp>
#include <boost/foreach.hpp>
#include <boost/assert.hpp>

namespace boost { namespace spirit { namespace qi { namespace detail
{
    // This file contains low level TST routines, not for
    // public consumption.

    template <typename Char, typename T>
    struct tst_node
    {
        tst_node(Char id)
          : id(id), data(0), lt(0), eq(0), gt(0)
        {
        }

        template <typename Alloc>
        static void
        destruct_node(tst_node* p, Alloc* alloc)
        {
            if (p)
            {
                if (p->data)
                    alloc->delete_data(p->data);
                destruct_node(p->lt, alloc);
                destruct_node(p->eq, alloc);
                destruct_node(p->gt, alloc);
                alloc->delete_node(p);
            }
        }

        template <typename Alloc>
        static tst_node*
        clone_node(tst_node* p, Alloc* alloc)
        {
            if (p)
            {
                tst_node* clone = alloc->new_node(p->id);
                if (p->data)
                    clone->data = alloc->new_data(*p->data);
                clone->lt = clone_node(p->lt, alloc);
                clone->eq = clone_node(p->eq, alloc);
                clone->gt = clone_node(p->gt, alloc);
                return clone;
            }
            return 0;
        }

        template <typename Iterator, typename Filter>
        static T*
        find(tst_node* start, Iterator& first, Iterator last, Filter filter)
        {
            if (first == last)
                return false;

            Iterator i = first;
            Iterator latest = first;
            tst_node* p = start;
            T* found = 0;

            while (p && i != last)
            {
                typename
                    boost::detail::iterator_traits<Iterator>::value_type
                c = filter(*i); // filter only the input

                if (c == p->id)
                {
                    if (p->data)
                    {
                        found = p->data;
                        latest = i;
                    }
                    p = p->eq;
                    i++;
                }
                else if (c < p->id)
                {
                    p = p->lt;
                }
                else
                {
                    p = p->gt;
                }
            }

            if (found)
                first = ++latest; // one past the last matching char
            return found;
        }

        template <typename Iterator, typename Alloc>
        static bool
        add(
            tst_node*& start
          , Iterator first
          , Iterator last
          , typename boost::call_traits<T>::param_type val
          , Alloc* alloc)
        {
            if (first == last)
                return false;

            tst_node** pp = &start;
            while (true)
            {
                typename
                    boost::detail::iterator_traits<Iterator>::value_type
                c = *first;

                if (*pp == 0)
                    *pp = alloc->new_node(c);
                tst_node* p = *pp;

                if (c == p->id)
                {
                    if (++first == last)
                    {
                        if (p->data == 0)
                        {
                            p->data = alloc->new_data(val);
                            return true;
                        }
                        return false;
                    }
                    pp = &p->eq;
                }
                else if (c < p->id)
                {
                    pp = &p->lt;
                }
                else
                {
                    pp = &p->gt;
                }
            }
        }

        template <typename Iterator, typename Alloc>
        static void
        remove(tst_node*& p, Iterator first, Iterator last, Alloc* alloc)
        {
            if (p == 0 || first == last)
                return;

            typename
                boost::detail::iterator_traits<Iterator>::value_type
            c = *first;

            if (c == p->id)
            {
                if (++first == last)
                {
                    if (p->data)
                    {
                        alloc->delete_data(p->data);
                        p->data = 0;
                    }
                }
                remove(p->eq, first, last, alloc);
            }
            else if (c < p->id)
            {
                remove(p->lt, first, last, alloc);
            }
            else
            {
                remove(p->gt, first, last, alloc);
            }

            if (p->lt == 0 && p->eq == 0 && p->gt == 0)
            {
                alloc->delete_node(p);
                p = 0;
            }
        }

        template <typename F>
        static void
        for_each(tst_node* p, std::basic_string<Char> prefix, F f)
        {
            if (p)
            {
                for_each(p->lt, prefix, f);
                std::basic_string<Char> s = prefix + p->id;
                for_each(p->eq, s, f);
                if (p->data)
                    f(s, *p->data);
                for_each(p->gt, prefix, f);
            }
        }

        Char id;        // the node's identity character
        T* data;        // optional data
        tst_node* lt;   // left pointer
        tst_node* eq;   // middle pointer
        tst_node* gt;   // right pointer
    };

/*
    template <typename Char, typename T>
    struct tst
    {
        typedef Char char_type; // the character type
        typedef T value_type; // the value associated with each entry
        typedef tst_node<Char, T> tst_node;

        tst()
        {
        }

        ~tst()
        {
            // Nothing to do here.
            // The pools do the right thing for us
        }

        tst(tst const& rhs)
        {
            copy(rhs);
        }

        tst& operator=(tst const& rhs)
        {
            return assign(rhs);
        }

        template <typename Iterator, typename Filter>
        T* find(Iterator& first, Iterator last, Filter filter) const
        {
            if (first != last)
            {
                Iterator save = first;
                typename map_type::const_iterator
                    i = map.find(filter(*first++));
                if (i == map.end())
                {
                    first = save;
                    return 0;
                }
                if (T* p = detail::find(i->second.root, first, last, filter))
                {
                    return p;
                }
                return i->second.data;
            }
            return 0;
        }

        template <typename Iterator>
        T* find(Iterator& first, Iterator last) const
        {
            return find(first, last, tst_pass_through());
        }

        template <typename Iterator>
        bool add(
            Iterator first
          , Iterator last
          , typename boost::call_traits<T>::param_type val)
        {
            if (first != last)
            {
                map_data x = {0, 0};
                std::pair<typename map_type::iterator, bool>
                    r = map.insert(std::pair<Char, map_data>(*first++, x));

                if (first != last)
                {
                    return detail::add(r.first->second.root, first, last, val, this);
                }
                else
                {
                    if (r.first->second.data)
                        return false;
                    r.first->second.data = this->new_data(val);
                }
                return true;
            }
            return false;
        }

        template <typename Iterator>
        void remove(Iterator first, Iterator last)
        {
            if (first != last)
            {
                typename map_type::iterator i = map.find(*first++);
                if (i != map.end())
                {
                    if (first != last)
                    {
                        detail::remove(i->second.root, first, last, this);
                    }
                    else if (i->second.data)
                    {
                        this->delete_data(i->second.data);
                        i->second.data = 0;
                    }
                    if (i->second.data == 0 && i->second.root == 0)
                    {
                        map.erase(i);
                    }
                }
            }
        }

        void clear()
        {
            BOOST_FOREACH(typename map_type::value_type& x, map)
            {
                destruct_node(x.second.root, this);
                if (x.second.data)
                    this->delete_data(x.second.data);
            }
            map.clear();
        }

        template <typename F>
        void for_each(F f) const
        {
            BOOST_FOREACH(typename map_type::value_type const& x, map)
            {
                std::basic_string<Char> s(1, x.first);
                detail::for_each(x.second.root, s, f);
                if (x.second.data)
                    f(s, *x.second.data);
            }
        }

        tst_node* new_node(Char id)
        {
            return node_pool.construct(id);
        }

        T* new_data(typename boost::call_traits<T>::param_type val)
        {
            return data_pool.construct(val);
        }

        void delete_node(tst_node* p)
        {
            node_pool.destroy(p);
        }

        void delete_data(T* p)
        {
            data_pool.destroy(p);
        }

    private:

        struct map_data
        {
            tst_node* root;
            T* data;
        };

        typedef unordered_map<Char, map_data> map_type;

        void copy(tst const& rhs)
        {
            BOOST_FOREACH(typename map_type::value_type const& x, rhs.map)
            {
                map_data xx = {clone_node(x.second.root, this), 0};
                if (x.second.data)
                    xx.data = data_pool.construct(*x.second.data);
                map[x.first] = xx;
            }
        }

        tst& assign(tst const& rhs)
        {
            if (this != &rhs)
            {
                BOOST_FOREACH(typename map_type::value_type& x, map)
                {
                    destruct_node(x.second.root, this);
                }
                map.clear();
                copy(rhs);
            }
            return *this;
        }

        map_type map;
        object_pool<tst_node> node_pool;
        object_pool<T> data_pool;
    };
*/
}}}}

#endif
