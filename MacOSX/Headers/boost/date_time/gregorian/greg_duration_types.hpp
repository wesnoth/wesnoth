#ifndef GREG_DURATION_TYPES_HPP___
#define GREG_DURATION_TYPES_HPP___
                                                                                
/* Copyright (c) 2004 CrystalClear Software, Inc.
 * Subject to Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or http://www.boost.org/LICENSE_1_0.txt)
 * Author: Jeff Garland, Bart Garst
 * $Date: 2008-02-27 15:00:24 -0500 (Wed, 27 Feb 2008) $
 */


#include "boost/date_time/gregorian/greg_date.hpp"
#include "boost/date_time/int_adapter.hpp"
#include "boost/date_time/adjust_functors.hpp"
#include "boost/date_time/date_duration.hpp"
#include "boost/date_time/date_duration_types.hpp"

namespace boost {
namespace gregorian {

  //! config struct for additional duration types (ie months_duration<> & years_duration<>)
  struct greg_durations_config {
    typedef date date_type;
    typedef date_time::int_adapter<int> int_rep;
    typedef date_time::month_functor<date_type> month_adjustor_type; 
  };

  typedef date_time::months_duration<greg_durations_config> months;
  typedef date_time::years_duration<greg_durations_config> years;
  typedef date_time::weeks_duration<date_time::duration_traits_adapted> weeks;

}} // namespace boost::gregorian

#endif // GREG_DURATION_TYPES_HPP___
