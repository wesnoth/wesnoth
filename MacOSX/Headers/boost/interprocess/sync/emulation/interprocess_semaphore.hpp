//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2005-2008. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/interprocess for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#include<boost/interprocess/exceptions.hpp>
#include <boost/interprocess/detail/posix_time_types_wrk.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>

namespace boost {
namespace interprocess {

inline interprocess_semaphore::~interprocess_semaphore()
{}

inline interprocess_semaphore::interprocess_semaphore(int initialCount)
   :  m_mut(), m_cond(), m_count(initialCount)
{}

inline void interprocess_semaphore::post()
{
   scoped_lock<interprocess_mutex> lock(m_mut);
   if(m_count == 0){
      m_cond.notify_one();
   }
   ++m_count;
}

inline void interprocess_semaphore::wait()
{
   scoped_lock<interprocess_mutex> lock(m_mut);
   while(m_count == 0){
      m_cond.wait(lock);
   }
   --m_count;
}

inline bool interprocess_semaphore::try_wait()
{
   scoped_lock<interprocess_mutex> lock(m_mut);
   if(m_count == 0){
      return false;
   }
   --m_count;
   return true;
}

inline bool interprocess_semaphore::timed_wait(const boost::posix_time::ptime &abs_time)
{
   if(abs_time == boost::posix_time::pos_infin){
      this->wait();
      return true;
   }
   scoped_lock<interprocess_mutex> lock(m_mut);
   while(m_count == 0){
      if(!m_cond.timed_wait(lock, abs_time))
         return m_count != 0;
   }
   --m_count;
   return true;
}
/*
inline int interprocess_semaphore::get_count() const
{
   scoped_lock<interprocess_mutex> lock(m_mut);
   return count;   
}*/

}  //namespace interprocess {
}  //namespace boost {
