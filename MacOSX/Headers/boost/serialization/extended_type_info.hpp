#ifndef BOOST_SERIALIZATION_EXTENDED_TYPE_INFO_HPP
#define BOOST_SERIALIZATION_EXTENDED_TYPE_INFO_HPP

// MS compatible compilers support #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// extended_type_info.hpp: interface for portable version of type_info

// (C) Copyright 2002 Robert Ramey - http://www.rrsd.com . 
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for updates, documentation, and revision history.

// for now, extended type info is part of the serialization libraries
// this could change in the future.
#include <cstdarg>
#include <cassert>
#include <cstddef> // NULL
#include <boost/config.hpp>
#include <boost/serialization/config.hpp>

#include <boost/config/abi_prefix.hpp> // must be the last header
#ifdef BOOST_MSVC
#  pragma warning(push)
#  pragma warning(disable : 4251 4231 4660 4275)
#endif

#define BOOST_SERIALIZATION_MAX_KEY_SIZE 128

namespace boost { 
namespace serialization {

class BOOST_SERIALIZATION_DECL(BOOST_PP_EMPTY()) extended_type_info
{
private: 
    // used to uniquely identify the type of class derived from this one
    // so that different derivations of this class can be simultaneously
    // included in implementation of sets and maps.
    const unsigned int m_type_info_key;
    virtual bool
    is_less_than(const extended_type_info & /*rhs*/) const {
        assert(false);
        return false;
    };
    virtual bool
    is_equal(const extended_type_info & /*rhs*/) const {
        assert(false);
        return false;
    };
    void key_unregister();
protected:
    const char * m_key;
    // this class can't be used as is. It's just the 
    // common functionality for all type_info replacement
    // systems.  Hence, make these protected
    extended_type_info(const unsigned int type_info_key = 0);
    // account for bogus gcc warning
    #if defined(__GNUC__)
    virtual
    #endif
    ~extended_type_info();
public:
    const char * get_key() const {
        return m_key;
    }
    void key_register(const char *key);
    bool operator<(const extended_type_info &rhs) const;
    bool operator==(const extended_type_info &rhs) const;
    bool operator!=(const extended_type_info &rhs) const {
        return !(operator==(rhs));
    }
    static const extended_type_info * find(const char *key);
    // for plugins
    virtual void * construct(unsigned int /*count*/ = 0, ...) const {
        assert(false); // must be implemented if used
        return NULL;
    };
    virtual void destroy(void const * const /*p*/) const {
        assert(false); // must be implemented if used
    }
};

} // namespace serialization 
} // namespace boost

#ifdef BOOST_MSVC
#pragma warning(pop)
#endif

#include <boost/config/abi_suffix.hpp> // pops abi_suffix.hpp pragmas

#endif // BOOST_SERIALIZATION_EXTENDED_TYPE_INFO_HPP

