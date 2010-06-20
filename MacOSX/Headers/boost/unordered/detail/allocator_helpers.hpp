
// Copyright 2005-2008 Daniel James.
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_UNORDERED_DETAIL_ALLOCATOR_UTILITIES_HPP_INCLUDED
#define BOOST_UNORDERED_DETAIL_ALLOCATOR_UTILITIES_HPP_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include <boost/config.hpp>

#if (defined(BOOST_NO_STD_ALLOCATOR) || defined(BOOST_DINKUMWARE_STDLIB)) \
    && !defined(__BORLANDC__)
#  define BOOST_UNORDERED_USE_ALLOCATOR_UTILITIES
#endif

#if defined(BOOST_UNORDERED_USE_ALLOCATOR_UTILITIES)
#  include <boost/detail/allocator_utilities.hpp>
#endif

#include <boost/mpl/aux_/config/eti.hpp>

namespace boost {
    namespace unordered_detail {

#if defined(BOOST_UNORDERED_USE_ALLOCATOR_UTILITIES)
        template <class Alloc, class T>
        struct rebind_wrap : ::boost::detail::allocator::rebind_to<Alloc, T> {};
#else
        template <class Alloc, class T>
        struct rebind_wrap
        {
            typedef BOOST_DEDUCED_TYPENAME
                Alloc::BOOST_NESTED_TEMPLATE rebind<T>::other
                type;
        };
#endif

#if !BOOST_WORKAROUND(BOOST_MSVC, < 1300)
        template <class T>
        inline void reset(T& x) { x  = T(); }

        template <class Ptr>
        inline Ptr null_ptr() { return Ptr(); }
#else
        template <class T>
        inline void reset_impl(T& x, ...) { x  = T(); }
        template <class T>
        inline void reset_impl(T*& x, int) { x  = 0; }
        template <class T>
        inline void reset(T& x) { reset_impl(x); }

        template <class Ptr>
        inline Ptr null_ptr() { Ptr x; reset(x); return x; }
#endif

        // Work around for Microsoft's ETI bug.
        
        template <class Allocator> struct allocator_value_type
        {
            typedef BOOST_DEDUCED_TYPENAME Allocator::value_type type;
        };

        template <class Allocator> struct allocator_pointer
        {
            typedef BOOST_DEDUCED_TYPENAME Allocator::pointer type;
        };
        
        template <class Allocator> struct allocator_const_pointer
        {
            typedef BOOST_DEDUCED_TYPENAME Allocator::const_pointer type;
        };
        
        template <class Allocator> struct allocator_reference
        {
            typedef BOOST_DEDUCED_TYPENAME Allocator::reference type;
        };
        
        template <class Allocator> struct allocator_const_reference
        {
            typedef BOOST_DEDUCED_TYPENAME Allocator::const_reference type;
        };
        
#if defined(BOOST_MPL_CFG_MSVC_ETI_BUG)

        template <>
        struct allocator_value_type<int>
        {
            typedef int type;
        };

        template <>
        struct allocator_pointer<int>
        {
            typedef int type;
        };

        template <>
        struct allocator_const_pointer<int>
        {
            typedef int type;
        };

        template <>
        struct allocator_reference<int>
        {
            typedef int type;
        };

        template <>
        struct allocator_const_reference<int>
        {
            typedef int type;
        };

#endif

        template <class Allocator>
        struct allocator_constructor
        {
            typedef BOOST_DEDUCED_TYPENAME allocator_value_type<Allocator>::type value_type;
            typedef BOOST_DEDUCED_TYPENAME allocator_pointer<Allocator>::type pointer;

            Allocator& alloc_;
            pointer ptr_;
            bool constructed_;

            allocator_constructor(Allocator& a)
                : alloc_(a), ptr_(), constructed_(false)
            {
#if BOOST_WORKAROUND(BOOST_MSVC, < 1300)
                    unordered_detail::reset(ptr_);
#endif
            }

            ~allocator_constructor() {
                if(ptr_) {
                    if(constructed_) alloc_.destroy(ptr_);
                    alloc_.deallocate(ptr_, 1);
                }
            }

            template <class V>
            void construct(V const& v) {
                BOOST_ASSERT(!ptr_ && !constructed_);
                ptr_ = alloc_.allocate(1);
                alloc_.construct(ptr_, value_type(v));
                constructed_  = true;
            }

            void construct(value_type const& v) {
                BOOST_ASSERT(!ptr_ && !constructed_);
                ptr_ = alloc_.allocate(1);
                alloc_.construct(ptr_, v);
                constructed_  = true;
            }

            pointer get() const
            {
                return ptr_;
            }

            // no throw
            pointer release()
            {
                pointer p = ptr_;
                constructed_ = false;
                unordered_detail::reset(ptr_);
                return p;
            }
        };

        template <class Allocator>
        struct allocator_array_constructor
        {
            typedef BOOST_DEDUCED_TYPENAME allocator_pointer<Allocator>::type pointer;

            Allocator& alloc_;
            pointer ptr_;
            pointer constructed_;
            std::size_t length_;

            allocator_array_constructor(Allocator& a)
                : alloc_(a), ptr_(), constructed_(), length_(0)
            {
#if BOOST_WORKAROUND(BOOST_MSVC, < 1300)
                unordered_detail::reset(constructed_);
                unordered_detail::reset(ptr_);
#endif
            }

            ~allocator_array_constructor() {
                if (ptr_) {
                    for(pointer p = ptr_; p != constructed_; ++p)
                        alloc_.destroy(p);

                    alloc_.deallocate(ptr_, length_);
                }
            }

            template <class V>
            void construct(V const& v, std::size_t l)
            {
                BOOST_ASSERT(!ptr_);
                length_ = l;
                ptr_ = alloc_.allocate(length_);
                pointer end = ptr_ + static_cast<std::ptrdiff_t>(length_);
                for(constructed_ = ptr_; constructed_ != end; ++constructed_)
                    alloc_.construct(constructed_, v);
            }

            pointer get() const
            {
                return ptr_;
            }

            pointer release()
            {
                pointer p(ptr_);
                unordered_detail::reset(ptr_);
                return p;
            }
        private:
            allocator_array_constructor(allocator_array_constructor const&);
            allocator_array_constructor& operator=(allocator_array_constructor const&);
        };
    }
}

#if defined(BOOST_UNORDERED_USE_ALLOCATOR_UTILITIES)
#  undef BOOST_UNORDERED_USE_ALLOCATOR_UTILITIES
#endif

#endif
