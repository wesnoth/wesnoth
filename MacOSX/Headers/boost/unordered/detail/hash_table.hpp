
// Copyright (C) 2003-2004 Jeremy B. Maitin-Shepard.
// Copyright (C) 2005-2008 Daniel James
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_UNORDERED_DETAIL_HASH_TABLE_HPP_INCLUDED
#define BOOST_UNORDERED_DETAIL_HASH_TABLE_HPP_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include <boost/config.hpp>

#include <cstddef>
#include <boost/config/no_tr1/cmath.hpp>
#include <algorithm>
#include <utility>
#include <stdexcept>

#include <boost/iterator.hpp>
#include <boost/iterator/iterator_categories.hpp>
#include <boost/limits.hpp>
#include <boost/assert.hpp>
#include <boost/static_assert.hpp>
#include <boost/unordered/detail/allocator_helpers.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/and.hpp>
#include <boost/detail/workaround.hpp>

#include <boost/mpl/aux_/config/eti.hpp>

#if defined(BOOST_HAS_RVALUE_REFS) && defined(BOOST_HAS_VARIADIC_TMPL)
#include <boost/type_traits/remove_reference.hpp>
#include <boost/type_traits/remove_const.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/mpl/not.hpp>
#endif

#if BOOST_WORKAROUND(__BORLANDC__, <= 0x0582)
#define BOOST_UNORDERED_BORLAND_BOOL(x) (bool)(x)
#else
#define BOOST_UNORDERED_BORLAND_BOOL(x) x
#endif

#if BOOST_WORKAROUND(BOOST_MSVC, < 1300)
#define BOOST_UNORDERED_MSVC_RESET_PTR(x) unordered_detail::reset(x)
#else
#define BOOST_UNORDERED_MSVC_RESET_PTR(x)
#endif

namespace boost {
    namespace unordered_detail {
        template <class T> struct type_wrapper {};

        static const std::size_t default_initial_bucket_count = 50;
        static const float minimum_max_load_factor = 1e-3f;

        template <class T>
        inline void hash_swap(T& x, T& y)
        {
#if defined(BOOST_NO_ARGUMENT_DEPENDENT_LOOKUP)
            std::swap(x,y);
#else
            using std::swap;
            swap(x, y);
#endif
        }

        inline std::size_t double_to_size_t(double f)
        {
            return f >= static_cast<double>((std::numeric_limits<std::size_t>::max)()) ?
                (std::numeric_limits<std::size_t>::max)() :
                static_cast<std::size_t>(f);
        }

        // prime number list, accessor

        template<typename T> struct prime_list_template
        {
            static std::size_t const value[];
            static std::ptrdiff_t const length;
        };

        template<typename T>
        std::size_t const prime_list_template<T>::value[] = {
            53ul, 97ul, 193ul, 389ul, 769ul,
            1543ul, 3079ul, 6151ul, 12289ul, 24593ul,
            49157ul, 98317ul, 196613ul, 393241ul, 786433ul,
            1572869ul, 3145739ul, 6291469ul, 12582917ul, 25165843ul,
            50331653ul, 100663319ul, 201326611ul, 402653189ul, 805306457ul,
            1610612741ul, 3221225473ul, 4294967291ul };

        template<typename T>
        std::ptrdiff_t const prime_list_template<T>::length = 28;

        typedef prime_list_template<std::size_t> prime_list;

        // no throw
        inline std::size_t next_prime(std::size_t n) {
            std::size_t const* const prime_list_begin = prime_list::value;
            std::size_t const* const prime_list_end = prime_list_begin +
                prime_list::length;
            std::size_t const* bound =
                std::lower_bound(prime_list_begin, prime_list_end, n);
            if(bound == prime_list_end)
                bound--;
            return *bound;
        }

        // no throw
        inline std::size_t prev_prime(std::size_t n) {
            std::size_t const* const prime_list_begin = prime_list::value;
            std::size_t const* const prime_list_end = prime_list_begin +
                prime_list::length;
            std::size_t const* bound =
                std::upper_bound(prime_list_begin,prime_list_end, n);
            if(bound != prime_list_begin)
                bound--;
            return *bound;
        }
        
        // Controls how many buckets are allocated and which buckets hash
        // values map to. Does not contain the buckets themselves, or ever
        // deal with them directly.

        struct bucket_manager {
            std::size_t bucket_count_;
            
            bucket_manager()
                : bucket_count_(0) {}
            
            explicit bucket_manager(std::size_t n)
                : bucket_count_(next_prime(n)) {}
            
            std::size_t bucket_count() const {
                return bucket_count_;
            }
            
            std::size_t bucket_from_hash(std::size_t hashed) const {
                return hashed % bucket_count_;
            }
            
            std::size_t max_bucket_count(std::size_t max_size) const {
                return prev_prime(max_size);
            }
        };

        // pair_cast - used to convert between pair types.

        template <class Dst1, class Dst2, class Src1, class Src2>
        inline std::pair<Dst1, Dst2> pair_cast(std::pair<Src1, Src2> const& x)
        {
            return std::pair<Dst1, Dst2>(Dst1(x.first), Dst2(x.second));
        }

#if !defined(BOOST_NO_STD_DISTANCE)
        using ::std::distance;
#else
        template <class ForwardIterator>
        inline std::size_t distance(ForwardIterator i, ForwardIterator j) {
            std::size_t x;
            std::distance(i, j, x);
            return x;
        }
#endif

        struct move_tag {};

        // Both hasher and key_equal's copy/assign can throw so double
        // buffering is used to copy them.

        template <typename Hash, typename Pred>
        struct buffered_functions
        {
            typedef Hash hasher;
            typedef Pred key_equal;

            class functions
            {
                std::pair<hasher, key_equal> functions_;

            public:

                functions(hasher const& h, key_equal const& k)
                    : functions_(h, k) {}

                hasher const& hash_function() const
                {
                    return functions_.first;
                }

                key_equal const& key_eq() const
                {
                    return functions_.second;
                }
            };

            typedef functions buffered_functions::*functions_ptr;

            buffered_functions(hasher const& h, key_equal const& k)
                : func1_(h, k), func2_(h, k), func_(&buffered_functions::func1_) {}

            // This copies the given function objects into the currently unused
            // function objects and returns a pointer, that func_ can later be
            // set to, to commit the change.
            //
            // Strong exception safety (since only usued function objects are
            // changed).
            functions_ptr buffer(buffered_functions const& x) {
                functions_ptr ptr = func_ == &buffered_functions::func1_
                    ? &buffered_functions::func2_ : &buffered_functions::func1_;
                this->*ptr = x.current();
                return ptr;
            }

            void set(functions_ptr ptr) {
                BOOST_ASSERT(ptr != func_);
                func_ = ptr;
            }

            functions const& current() const {
                return this->*func_;
            }

        private:
            functions func1_;
            functions func2_;
            functions_ptr func_; // The currently active functions.
        };

    }
}

#define BOOST_UNORDERED_EQUIVALENT_KEYS 1
#include <boost/unordered/detail/hash_table_impl.hpp>
#undef BOOST_UNORDERED_EQUIVALENT_KEYS

#define BOOST_UNORDERED_EQUIVALENT_KEYS 0
#include <boost/unordered/detail/hash_table_impl.hpp>
#undef BOOST_UNORDERED_EQUIVALENT_KEYS

namespace boost {
    namespace unordered_detail {
        class iterator_access
        {
        public:
            template <class Iterator>
            static BOOST_DEDUCED_TYPENAME Iterator::base const& get(Iterator const& it) {
                return it.base_;
            }
        };

        template <class ValueType, class KeyType,
            class Hash, class Pred, class Alloc>
        class hash_types_unique_keys
        {
        public:
            typedef BOOST_DEDUCED_TYPENAME
                boost::unordered_detail::rebind_wrap<Alloc, ValueType>::type
                value_allocator;

            typedef hash_table_unique_keys<ValueType, KeyType, Hash, Pred,
                    value_allocator> hash_table;
            typedef hash_table_data_unique_keys<value_allocator> data;
            typedef BOOST_DEDUCED_TYPENAME data::iterator_base iterator_base;

            typedef hash_const_local_iterator_unique_keys<value_allocator> const_local_iterator;
            typedef hash_local_iterator_unique_keys<value_allocator> local_iterator;
            typedef hash_const_iterator_unique_keys<value_allocator> const_iterator;
            typedef hash_iterator_unique_keys<value_allocator> iterator;

            typedef BOOST_DEDUCED_TYPENAME data::size_type size_type;
            typedef std::ptrdiff_t difference_type;
        };

        template <class ValueType, class KeyType,
            class Hash, class Pred, class Alloc>
        class hash_types_equivalent_keys
        {
        public:
            typedef BOOST_DEDUCED_TYPENAME
                boost::unordered_detail::rebind_wrap<Alloc, ValueType>::type
                value_allocator;

            typedef hash_table_equivalent_keys<ValueType, KeyType, Hash, Pred,
                    value_allocator> hash_table;
            typedef hash_table_data_equivalent_keys<value_allocator> data;
            typedef BOOST_DEDUCED_TYPENAME data::iterator_base iterator_base;

            typedef hash_const_local_iterator_equivalent_keys<value_allocator> const_local_iterator;
            typedef hash_local_iterator_equivalent_keys<value_allocator> local_iterator;
            typedef hash_const_iterator_equivalent_keys<value_allocator> const_iterator;
            typedef hash_iterator_equivalent_keys<value_allocator> iterator;

            typedef BOOST_DEDUCED_TYPENAME data::size_type size_type;
            typedef std::ptrdiff_t difference_type;
        };
    } // namespace boost::unordered_detail
} // namespace boost

#undef BOOST_UNORDERED_BORLAND_BOOL
#undef BOOST_UNORDERED_MSVC_RESET_PTR

#endif // BOOST_UNORDERED_DETAIL_HASH_TABLE_HPP_INCLUDED
