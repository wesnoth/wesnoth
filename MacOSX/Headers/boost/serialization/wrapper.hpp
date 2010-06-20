#ifndef BOOST_SERIALIZATION_WRAPPER_HPP
#define BOOST_SERIALIZATION_WRAPPER_HPP

// (C) Copyright 2005-2006 Matthias Troyer
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <boost/serialization/traits.hpp>
#include <boost/type_traits/is_base_and_derived.hpp>
#include <boost/mpl/eval_if.hpp>

namespace boost { namespace serialization {

/// the base class for serialization wrappers
///
/// wrappers need to be treated differently at various places in the serialization library,
/// e.g. saving of non-const wrappers has to be possible. Since partial specialization
// is not supported by all compilers, we derive all wrappers from wrapper_traits. 

template<
    class T, 
    int Level = object_serializable, 
    int Tracking = track_never,
    unsigned int Version = 0,
    class ETII = extended_type_info_impl< T >
>
struct wrapper_traits : public traits<T,Level,Tracking,Version,ETII,mpl::true_> 
{};

/// the is_wrapper type traits class. 

namespace detail {
template <class T>
struct is_wrapper_member
{
  typedef BOOST_DEDUCED_TYPENAME T::is_wrapper type;
};

}


template<class T>
struct is_wrapper
 : mpl::eval_if<
      is_base_and_derived<basic_traits,T>,
      detail::is_wrapper_member<T>,
      mpl::false_
    >::type
{};
 
} } // end namespace boost::serialization

// A macro to define that a class is a wrapper
#define BOOST_CLASS_IS_WRAPPER(T)            \
namespace boost {                            \
namespace serialization {                    \
template<>                                   \
struct is_wrapper< T > : mpl::true_ {};      \
}}                                           


#endif //BOOST_SERIALIZATION_WRAPPER_HPP
