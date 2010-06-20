// (C) Copyright Jeremy Siek 2000.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// This header replaces the implementation of ct_if that preceded the
// introduction of Boost.MPL with a facade that defers to that reviewed and
// accepted library.

// Author: Ronald Garcia
// Date: 20 October, 2006


#ifndef BOOST_CT_IF_HPP
#define BOOST_CT_IF_HPP


// A stub implementation in terms of Boost.MPL

#include <boost/mpl/if.hpp>
#include <boost/mpl/not.hpp>
#include <boost/mpl/and.hpp>
// true_type and false_type are used by applications of ct_if
#include <boost/type_traits/integral_constant.hpp> 

namespace boost {

  template <class A, class B>
  struct ct_and : boost::mpl::and_<A,B> {};

  template <class A>
  struct ct_not : mpl::not_<A> {};

  template <bool cond, class A, class B>
  struct ct_if : mpl::if_c<cond,A,B> {};

  template <class cond, class A, class B>
  struct ct_if_t : mpl::if_<cond,A,B> {};

} // namespace boost

#endif // BOOST_CT_IF_HPP

