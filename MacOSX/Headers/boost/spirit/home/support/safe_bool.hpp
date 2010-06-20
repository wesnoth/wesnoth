/*=============================================================================
    Copyright (c) 2003 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying 
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(BOOST_SPIRIT_SAFE_BOOL_HPP)
#define BOOST_SPIRIT_SAFE_BOOL_HPP

#include <boost/config.hpp>
#include <boost/detail/workaround.hpp>

namespace boost { namespace spirit
{
    namespace detail
    {
        template <typename T>
        struct no_base {};

        template <typename T>
        struct safe_bool_impl
        {
#if BOOST_WORKAROUND(__MWERKS__, BOOST_TESTED_AT(0x3003))
            void stub(T*) {}
            typedef void (safe_bool_impl::*type)(T*);
#else
            typedef T* TP; // workaround to make parsing easier
            TP stub;
            typedef TP safe_bool_impl::*type;
#endif
        };
    }

    template <typename Derived, typename Base = detail::no_base<Derived> >
    struct safe_bool : Base
    {
    private:
        typedef detail::safe_bool_impl<Derived> impl_type;
        typedef typename impl_type::type bool_type;

    public:
        operator bool_type() const
        {
            return static_cast<const Derived*>(this)->operator_bool() ?
                &impl_type::stub : 0;
        }

        operator bool_type()
        {
            return static_cast<Derived*>(this)->operator_bool() ?
                &impl_type::stub : 0;
        }
    };
}}

#endif

