///////////////////////////////////////////////////////////////////////////////
/// \file literal.hpp
/// The literal\<\> terminal wrapper, and the proto::lit() function for
/// creating literal\<\> wrappers.
//
//  Copyright 2008 Eric Niebler. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PROTO_LITERAL_HPP_EAN_01_03_2007
#define BOOST_PROTO_LITERAL_HPP_EAN_01_03_2007

#include <boost/xpressive/proto/detail/prefix.hpp>
#include <boost/config.hpp>
#include <boost/xpressive/proto/proto_fwd.hpp>
#include <boost/xpressive/proto/expr.hpp>
#include <boost/xpressive/proto/traits.hpp>
#include <boost/xpressive/proto/extends.hpp>
#include <boost/xpressive/proto/detail/suffix.hpp>

namespace boost { namespace proto
{
    namespace utility
    {
        /// \brief A simple wrapper for a terminal, provided for
        /// ease of use.
        ///
        /// A simple wrapper for a terminal, provided for
        /// ease of use. In all cases, <tt>literal\<X\> l(x);</tt>
        /// is equivalent to <tt>terminal\<X\>::::type l = {x};</tt>.
        ///
        /// The \c Domain template parameter defaults to
        /// \c proto::default_domain.
        template<
            typename T
          , typename Domain BOOST_PROTO_FOR_DOXYGEN_ONLY(= default_domain)
        >
        struct literal
          : extends<typename terminal<T>::type, literal<T, Domain>, Domain>
        {
        private:
            typedef typename terminal<T>::type terminal_type;
            typedef extends<terminal_type, literal<T, Domain>, Domain> base_type;

        public:
            typedef typename proto::result_of::arg<terminal_type>::type value_type;
            typedef typename proto::result_of::arg<terminal_type>::reference reference;
            typedef typename proto::result_of::arg<terminal_type>::const_reference const_reference;

            template<typename U>
            literal(U &u)
              : base_type(terminal_type::make(u))
            {}

            template<typename U>
            literal(U const &u)
              : base_type(terminal_type::make(u))
            {}

            template<typename U>
            literal(literal<U, Domain> const &u)
              : base_type(terminal_type::make(proto::arg(u)))
            {}

            using base_type::operator =;

            reference get()
            {
                return proto::arg(*this);
            }

            const_reference get() const
            {
                return proto::arg(*this);
            }
        };
    }

    /// \brief A helper function for creating a \c literal\<\> wrapper.
    /// \param t The object to wrap.
    /// \return literal\<T &\>(t)
    /// \attention The returned value holds the argument by reference.
    /// \throw nothrow
    template<typename T>
    inline literal<T &> lit(T &t)
    {
        return literal<T &>(t);
    }

    /// \overload
    ///
    template<typename T>
    inline literal<T const &> lit(T const &t)
    {
        #ifdef BOOST_MSVC
        #pragma warning(push)
        #pragma warning(disable: 4180) // warning C4180: qualifier applied to function type has no meaning; ignored
        #endif

        return literal<T const &>(t);

        #ifdef BOOST_MSVC
        #pragma warning(pop)
        #endif
    }

}}

#endif
