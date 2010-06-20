
// Copyright 2005-2008 Daniel James.
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  Based on Peter Dimov's proposal
//  http://www.open-std.org/JTC1/SC22/WG21/docs/papers/2005/n1756.pdf
//  issue 6.18. 

#if !defined(BOOST_FUNCTIONAL_HASH_EXTENSIONS_HPP)
#define BOOST_FUNCTIONAL_HASH_EXTENSIONS_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

namespace boost
{

#if defined(BOOST_NO_FUNCTION_TEMPLATE_ORDERING)
    namespace hash_detail
    {
        template <bool IsArray>
        struct call_hash_impl
        {
            template <class T>
            struct inner
            {
                static std::size_t call(T const& v)
                {
                    using namespace boost;
                    return hash_value(v);
                }
            };
        };

        template <>
        struct call_hash_impl<true>
        {
            template <class Array>
            struct inner
            {
#if !BOOST_WORKAROUND(BOOST_MSVC, < 1300)
                static std::size_t call(Array const& v)
#else
                static std::size_t call(Array& v)
#endif
                {
                    const int size = sizeof(v) / sizeof(*v);
                    return boost::hash_range(v, v + size);
                }
            };
        };

        template <class T>
        struct call_hash
            : public call_hash_impl<boost::is_array<T>::value>
                ::BOOST_NESTED_TEMPLATE inner<T>
        {
        };
    }
#endif // BOOST_NO_FUNCTION_TEMPLATE_ORDERING

#if !defined(BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION)

    template <class T> struct hash
        : std::unary_function<T, std::size_t>
    {
#if !defined(BOOST_NO_FUNCTION_TEMPLATE_ORDERING)
        std::size_t operator()(T const& val) const
        {
            return hash_value(val);
        }
#else
        std::size_t operator()(T const& val) const
        {
            return hash_detail::call_hash<T>::call(val);
        }
#endif
    };

#if BOOST_WORKAROUND(__DMC__, <= 0x848)
    template <class T, unsigned int n> struct hash<T[n]>
        : std::unary_function<T[n], std::size_t>
    {
        std::size_t operator()(const T* val) const
        {
            return boost::hash_range(val, val+n);
        }
    };
#endif

#else // BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION

    // On compilers without partial specialization, boost::hash<T>
    // has already been declared to deal with pointers, so just
    // need to supply the non-pointer version.

    namespace hash_detail
    {
        template <bool IsPointer>
        struct hash_impl;

#if !BOOST_WORKAROUND(BOOST_MSVC, < 1300)

        template <>
        struct hash_impl<false>
        {
            template <class T>
            struct inner
                : std::unary_function<T, std::size_t>
            {
#if !defined(BOOST_NO_FUNCTION_TEMPLATE_ORDERING)
                std::size_t operator()(T const& val) const
                {
                    return hash_value(val);
                }
#else
                std::size_t operator()(T const& val) const
                {
                    return hash_detail::call_hash<T>::call(val);
                }
#endif
            };
        };

#else // Visual C++ 6.5

    // There's probably a more elegant way to Visual C++ 6.5 to work
    // but I don't know what it is.

        template <bool IsConst>
        struct hash_impl_msvc
        {
            template <class T>
            struct inner
                : public std::unary_function<T, std::size_t>
            {
                std::size_t operator()(T const& val) const
                {
                    return hash_detail::call_hash<T const>::call(val);
                }

                std::size_t operator()(T& val) const
                {
                    return hash_detail::call_hash<T>::call(val);
                }
            };
        };

        template <>
        struct hash_impl_msvc<true>
        {
            template <class T>
            struct inner
                : public std::unary_function<T, std::size_t>
            {
                std::size_t operator()(T& val) const
                {
                    return hash_detail::call_hash<T>::call(val);
                }
            };
        };
        
        template <class T>
        struct hash_impl_msvc2
            : public hash_impl_msvc<boost::is_const<T>::value>
                    ::BOOST_NESTED_TEMPLATE inner<T> {};
        
        template <>
        struct hash_impl<false>
        {
            template <class T>
            struct inner : public hash_impl_msvc2<T> {};
        };

#endif // Visual C++ 6.5
    }
#endif  // BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION
}

#endif
