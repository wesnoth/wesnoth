//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2005-2008. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/interprocess for documentation.
//
//////////////////////////////////////////////////////////////////////////////
//
// Parts of the pthread code come from Boost Threads code:
//
//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2001-2003
// William E. Kempf
//
// Permission to use, copy, modify, distribute and sell this software
// and its documentation for any purpose is hereby granted without fee,
// provided that the above copyright notice appear in all copies and
// that both that copyright notice and this permission notice appear
// in supporting documentation.  William E. Kempf makes no representations
// about the suitability of this software for any purpose.
// It is provided "as is" without express or implied warranty.
//////////////////////////////////////////////////////////////////////////////

#include <boost/interprocess/detail/posix_time_types_wrk.hpp>
#include <boost/interprocess/detail/os_thread_functions.hpp>
#include <boost/interprocess/exceptions.hpp>

namespace boost {

namespace interprocess {

inline interprocess_recursive_mutex::interprocess_recursive_mutex() 
   : m_nLockCount(0), m_nOwner(detail::get_invalid_thread_id()){}

inline interprocess_recursive_mutex::~interprocess_recursive_mutex(){}

inline void interprocess_recursive_mutex::lock()
{
   detail::OS_thread_id_t th_id = detail::get_current_thread_id();
   if(detail::equal_thread_id(th_id, m_nOwner)){
      if((unsigned int)(m_nLockCount+1) == 0){
         //Overflow, throw an exception
         throw interprocess_exception();
      } 
      ++m_nLockCount;
   }
   else{
      m_mutex.lock();
      m_nOwner = th_id;
      m_nLockCount = 1;
   }
}

inline bool interprocess_recursive_mutex::try_lock()
{
   detail::OS_thread_id_t th_id = detail::get_current_thread_id();
   if(detail::equal_thread_id(th_id, m_nOwner)) {  // we own it
      if((unsigned int)(m_nLockCount+1) == 0){
         //Overflow, throw an exception
         throw interprocess_exception();
      } 
      ++m_nLockCount;
      return true;
   }
   if(m_mutex.try_lock()){
      m_nOwner = th_id;
      m_nLockCount = 1;
      return true;
   }
   return false;
}

inline bool interprocess_recursive_mutex::timed_lock(const boost::posix_time::ptime &abs_time)
{
   if(abs_time == boost::posix_time::pos_infin){
      this->lock();
      return true;
   }
   detail::OS_thread_id_t th_id = detail::get_current_thread_id();
   if(detail::equal_thread_id(th_id, m_nOwner)) {  // we own it
      if((unsigned int)(m_nLockCount+1) == 0){
         //Overflow, throw an exception
         throw interprocess_exception();
      } 
      ++m_nLockCount;
      return true;
   }
   if(m_mutex.timed_lock(abs_time)){
      m_nOwner = th_id;
      m_nLockCount = 1;
      return true;
   }
   return false;
}

inline void interprocess_recursive_mutex::unlock()
{
   assert(detail::equal_thread_id(detail::get_current_thread_id(), m_nOwner));
   --m_nLockCount;
   if(!m_nLockCount){
      m_nOwner = detail::get_invalid_thread_id();
      m_mutex.unlock();
   }
}

}  //namespace interprocess {

}  //namespace boost {

