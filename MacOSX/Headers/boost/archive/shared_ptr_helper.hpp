#ifndef BOOST_ARCHIVE_SHARED_PTR_HELPER_HPP
#define BOOST_ARCHIVE_SHARED_PTR_HELPER_HPP

// MS compatible compilers support #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// shared_ptr_helper.hpp: serialization for boost shared pointer

// (C) Copyright 2004 Robert Ramey and Martin Ecker
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for updates, documentation, and revision history.

#include <map>
#include <list>
#include <cstddef> // NULL

#include <boost/config.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/serialization/type_info_implementation.hpp>
#include <boost/serialization/shared_ptr_132.hpp>
#include <boost/serialization/throw_exception.hpp>

#include <boost/archive/archive_exception.hpp>

namespace boost_132 {
    template<class T> class shared_ptr;
}
namespace boost {
    template<class T> class shared_ptr;
    namespace serialization {
        class extended_type_info;
        template<class Archive, class T>
        inline void load(
            Archive & ar,
            boost::shared_ptr<T> &t,
            const unsigned int file_version
        );
    }

namespace archive{
namespace detail {

struct null_deleter {
    void operator()(void const *) const {}
};

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// a common class for holding various types of shared pointers

class shared_ptr_helper {
    typedef std::map<const void *, shared_ptr<void> > collection_type;
    typedef collection_type::const_iterator iterator_type;
    // list of shared_pointers create accessable by raw pointer. This
    // is used to "match up" shared pointers loaded at different
    // points in the archive. Note, we delay construction until
    // it is actually used since this is by default included as
    // a "mix-in" even if shared_ptr isn't used.
    collection_type * m_pointers;

#ifdef BOOST_NO_MEMBER_TEMPLATE_FRIENDS
public:
#else
    template<class Archive, class T>
    friend inline void boost::serialization::load(
        Archive & ar,
        boost::shared_ptr<T> &t,
        const unsigned int file_version
    );
#endif

    // list of loaded pointers.  This is used to be sure that the pointers
    // stay around long enough to be "matched" with other pointers loaded
    // by the same archive.  These are created with a "null_deleter" so that
    // when this list is destroyed - the underlaying raw pointers are not
    // destroyed.  This has to be done because the pointers are also held by
    // new system which is disjoint from this set.  This is implemented
    // by a change in load_construct_data below.  It makes this file suitable
    // only for loading pointers into a 1.33 or later boost system.
    std::list<boost_132::shared_ptr<void> > * m_pointers_132;

    // return a void pointer to the most derived type
    template<class T>
    const void * object_identifier(T * t) const {
        const boost::serialization::extended_type_info * true_type 
            = boost::serialization::type_info_implementation<T>::type
                ::get_const_instance().get_derived_extended_type_info(*t);
        // note:if this exception is thrown, be sure that derived pointer
        // is either registered or exported.
        if(NULL == true_type)
            boost::serialization::throw_exception(
                boost::archive::archive_exception(
                    boost::archive::archive_exception::unregistered_class
                )
            );
        const boost::serialization::extended_type_info * this_type
            = & boost::serialization::type_info_implementation<T>::type
                    ::get_const_instance();
        const void * vp = void_downcast(
            *true_type, 
            *this_type, 
            static_cast<const void *>(t)
        );
        return vp;
    }
public:
    template<class T>
    void reset(shared_ptr<T> & s, T * r){
        if(NULL == r){
            s.reset();
            return;
        }
        // get pointer to the most derived object.  This is effectively
        // the object identifer
        const void * od = object_identifier(r);

        if(NULL == m_pointers)
            m_pointers = new collection_type;

        iterator_type it = m_pointers->find(od);

        if(it == m_pointers->end()){
            s.reset(r);
            m_pointers->insert(collection_type::value_type(od,s));
        }
        else{
            s = static_pointer_cast<T>((*it).second);
        }
    }
    void append(const boost_132::shared_ptr<void> & t){
        if(NULL == m_pointers_132)
            m_pointers_132 = new std::list<boost_132::shared_ptr<void> >;
        m_pointers_132->push_back(t);
    }
public:
    shared_ptr_helper() : 
        m_pointers(NULL), 
        m_pointers_132(NULL)
    {}
    ~shared_ptr_helper(){
        if(NULL != m_pointers)
            delete m_pointers;
        if(NULL != m_pointers_132)
            delete m_pointers_132;
    }
};

} // namespace detail
} // namespace serialization
} // namespace boost

#endif // BOOST_ARCHIVE_SHARED_PTR_HELPER_HPP
