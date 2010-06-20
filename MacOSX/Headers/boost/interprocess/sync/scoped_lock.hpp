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
// This interface is inspired by Howard Hinnant's lock proposal.
// http://home.twcny.rr.com/hinnant/cpp_extensions/threads_move.html
//
//////////////////////////////////////////////////////////////////////////////

#ifndef BOOST_INTERPROCESS_SCOPED_LOCK_HPP
#define BOOST_INTERPROCESS_SCOPED_LOCK_HPP

#if (defined _MSC_VER) && (_MSC_VER >= 1200)
#  pragma once
#endif

#include <boost/interprocess/detail/config_begin.hpp>
#include <boost/interprocess/detail/workaround.hpp>
#include <boost/interprocess/sync/lock_options.hpp>
#include <boost/interprocess/exceptions.hpp>
#include <boost/interprocess/detail/move.hpp>
#include <boost/interprocess/detail/posix_time_types_wrk.hpp>

//!\file
//!Describes the scoped_lock class.

namespace boost {
namespace interprocess {

template<class M>
class sharable_lock;

template<class M>
class upgradable_lock;

//!scoped_lock is meant to carry out the tasks for locking, unlocking, try-locking
//!and timed-locking (recursive or not) for the Mutex. The Mutex need not supply all
//!of this functionality. If the client of scoped_lock<Mutex> does not use
//!functionality which the Mutex does not supply, no harm is done. Mutex ownership
//!transfer is supported through the syntax of move semantics. Ownership transfer
//!is allowed both by construction and assignment. The scoped_lock does not support
//!copy semantics. A compile time error results if copy construction or copy
//!assignment is attempted. Mutex ownership can also be moved from an
//!upgradable_lock and sharable_lock via constructor. In this role, scoped_lock
//!shares the same functionality as a write_lock.
template <class Mutex>
class scoped_lock
{
   /// @cond
   private:
   typedef scoped_lock<Mutex> this_type;
   scoped_lock(scoped_lock const&);
   scoped_lock& operator=  (scoped_lock const&);
   typedef bool this_type::*unspecified_bool_type;
   /// @endcond
   public:
   typedef Mutex mutex_type;

   //!Effects: Default constructs a scoped_lock.
   //!Postconditions: owns() == false and mutex() == 0.
   scoped_lock()
      : mp_mutex(0), m_locked(false)
   {}

   //!Effects: m.lock().
   //!Postconditions: owns() == true and mutex() == &m.
   //!Notes: The constructor will take ownership of the mutex. If another thread
   //!   already owns the mutex, this thread will block until the mutex is released.
   //!   Whether or not this constructor handles recursive locking depends upon the mutex.
   explicit scoped_lock(mutex_type& m)
      : mp_mutex(&m), m_locked(false)
   {  mp_mutex->lock();   m_locked = true;  }

   //!Postconditions: owns() == false, and mutex() == &m.
   //!Notes: The constructor will not take ownership of the mutex. There is no effect
   //!   required on the referenced mutex.
   scoped_lock(mutex_type& m, detail::defer_lock_type)
      : mp_mutex(&m), m_locked(false)
   {}

   //!Postconditions: owns() == true, and mutex() == &m.
   //!Notes: The constructor will suppose that the mutex is already locked. There
   //!   is no effect required on the referenced mutex.
   scoped_lock(mutex_type& m, detail::accept_ownership_type)
      : mp_mutex(&m), m_locked(true)
   {}

   //!Effects: m.try_lock(). 
   //!Postconditions: mutex() == &m. owns() == the return value of the
   //!   m.try_lock() executed within the constructor.
   //!Notes: The constructor will take ownership of the mutex if it can do
   //!   so without waiting. Whether or not this constructor handles recursive
   //!   locking depends upon the mutex. If the mutex_type does not support try_lock,
   //!   this constructor will fail at compile time if instantiated, but otherwise
   //!   have no effect.
   scoped_lock(mutex_type& m, detail::try_to_lock_type)
      : mp_mutex(&m), m_locked(mp_mutex->try_lock())
   {}

   //!Effects: m.timed_lock(abs_time). 
   //!Postconditions: mutex() == &m. owns() == the return value of the
   //!   m.timed_lock(abs_time) executed within the constructor.
   //!Notes: The constructor will take ownership of the mutex if it can do
   //!   it until abs_time is reached. Whether or not this constructor
   //!   handles recursive locking depends upon the mutex. If the mutex_type
   //!   does not support try_lock, this constructor will fail at compile
   //!   time if instantiated, but otherwise have no effect.
   scoped_lock(mutex_type& m, const boost::posix_time::ptime& abs_time)
      : mp_mutex(&m), m_locked(mp_mutex->timed_lock(abs_time))
   {}

   //!Postconditions: mutex() == the value scop.mutex() had before the
   //!   constructor executes. s1.mutex() == 0. owns() == the value of
   //!   scop.owns() before the constructor executes. scop.owns().
   //!Notes: If the scop scoped_lock owns the mutex, ownership is moved
   //!   to thisscoped_lock with no blocking. If the scop scoped_lock does not
   //!   own the mutex, then neither will this scoped_lock. Only a moved
   //!   scoped_lock's will match this signature. An non-moved scoped_lock
   //!   can be moved with the expression: "detail::move_impl(lock);". This
   //!   constructor does not alter the state of the mutex, only potentially
   //!   who owns it.
   #ifndef BOOST_INTERPROCESS_RVALUE_REFERENCE
   scoped_lock(detail::moved_object<scoped_lock<Mutex> > scop)
      : mp_mutex(0), m_locked(scop.get().owns())
   {  mp_mutex = scop.get().release(); }
   #else

   scoped_lock(scoped_lock &&scop)
      : mp_mutex(0), m_locked(scop.owns())
   {  mp_mutex = scop.release(); }
   #endif

   //!Effects: If upgr.owns() then calls unlock_upgradable_and_lock() on the
   //!   referenced mutex. upgr.release() is called. 
   //!Postconditions: mutex() == the value upgr.mutex() had before the construction.
   //!   upgr.mutex() == 0. owns() == upgr.owns() before the construction.
   //!   upgr.owns() == false after the construction.
   //!Notes: If upgr is locked, this constructor will lock this scoped_lock while
   //!   unlocking upgr. If upgr is unlocked, then this scoped_lock will be
   //!   unlocked as well. Only a moved upgradable_lock's will match this
   //!   signature. An non-moved upgradable_lock can be moved with
   //!   the expression: "detail::move_impl(lock);" This constructor may block if
   //!   other threads hold a sharable_lock on this mutex (sharable_lock's can
   //!   share ownership with an upgradable_lock).
   #ifndef BOOST_INTERPROCESS_RVALUE_REFERENCE
   scoped_lock(detail::moved_object<upgradable_lock<Mutex> > upgr)
      : mp_mutex(0), m_locked(false)
   {
      upgradable_lock<mutex_type> &u_lock = upgr.get();
      if(u_lock.owns()){
         u_lock.mutex()->unlock_upgradable_and_lock();
         m_locked = true;
      }
      mp_mutex = u_lock.release();
   }
   #else
   scoped_lock(upgradable_lock<Mutex> &&upgr)
      : mp_mutex(0), m_locked(false)
   {
      upgradable_lock<mutex_type> &u_lock = upgr;
      if(u_lock.owns()){
         u_lock.mutex()->unlock_upgradable_and_lock();
         m_locked = true;
      }
      mp_mutex = u_lock.release();
   }
   #endif

   //!Effects: If upgr.owns() then calls try_unlock_upgradable_and_lock() on the
   //!referenced mutex:
   //!   a)if try_unlock_upgradable_and_lock() returns true then mutex() obtains
   //!      the value from upgr.release() and owns() is set to true. 
   //!   b)if try_unlock_upgradable_and_lock() returns false then upgr is
   //!      unaffected and this scoped_lock construction as the same effects as  
   //!      a default construction. 
   //!   c)Else upgr.owns() is false. mutex() obtains the value from upgr.release()
   //!      and owns() is set to false 
   //!Notes: This construction will not block. It will try to obtain mutex
   //!   ownership from upgr immediately, while changing the lock type from a
   //!   "read lock" to a "write lock". If the "read lock" isn't held in the
   //!   first place, the mutex merely changes type to an unlocked "write lock".
   //!   If the "read lock" is held, then mutex transfer occurs only if it can
   //!   do so in a non-blocking manner.*/
   #ifndef BOOST_INTERPROCESS_RVALUE_REFERENCE
   scoped_lock(detail::moved_object<upgradable_lock<Mutex> > upgr
              ,detail::try_to_lock_type)
      : mp_mutex(0), m_locked(false)
   {
      upgradable_lock<mutex_type> &u_lock = upgr.get();
      if(u_lock.owns()){
         if((m_locked = u_lock.mutex()->try_unlock_upgradable_and_lock()) == true){
            mp_mutex = u_lock.release();
         }
      }
      else{
         u_lock.release();
      }
   }
   #else
   scoped_lock(upgradable_lock<Mutex> &&upgr
              ,detail::try_to_lock_type)
      : mp_mutex(0), m_locked(false)
   {
      upgradable_lock<mutex_type> &u_lock = upgr;
      if(u_lock.owns()){
         if((m_locked = u_lock.mutex()->try_unlock_upgradable_and_lock()) == true){
            mp_mutex = u_lock.release();
         }
      }
      else{
         u_lock.release();
      }
   }
   #endif

   //!Effects: If upgr.owns() then calls timed_unlock_upgradable_and_lock(abs_time)
   //!   on the referenced mutex:
   //!   a)if timed_unlock_upgradable_and_lock(abs_time) returns true then mutex()
   //!      obtains the value from upgr.release() and owns() is set to true. 
   //!   b)if timed_unlock_upgradable_and_lock(abs_time) returns false then upgr
   //!      is unaffected and this scoped_lock construction as the same effects
   //!      as a default construction.
   //!   c)Else upgr.owns() is false. mutex() obtains the value from upgr.release()
   //!      and owns() is set to false 
   //!Notes: This construction will not block. It will try to obtain mutex ownership
   //!   from upgr immediately, while changing the lock type from a "read lock" to a
   //!   "write lock". If the "read lock" isn't held in the first place, the mutex
   //!   merely changes type to an unlocked "write lock". If the "read lock" is held,
   //!   then mutex transfer occurs only if it can do so in a non-blocking manner.
   #ifndef BOOST_INTERPROCESS_RVALUE_REFERENCE
   scoped_lock(detail::moved_object<upgradable_lock<Mutex> > upgr
              ,boost::posix_time::ptime &abs_time)
      : mp_mutex(0), m_locked(false)
   {
      upgradable_lock<mutex_type> &u_lock = upgr.get();
      if(u_lock.owns()){
         if((m_locked = u_lock.mutex()->timed_unlock_upgradable_and_lock(abs_time)) == true){
            mp_mutex = u_lock.release();
         }
      }
      else{
         u_lock.release();
      }
   }
   #else
   scoped_lock(upgradable_lock<Mutex> &&upgr
              ,boost::posix_time::ptime &abs_time)
      : mp_mutex(0), m_locked(false)
   {
      upgradable_lock<mutex_type> &u_lock = upgr;
      if(u_lock.owns()){
         if((m_locked = u_lock.mutex()->timed_unlock_upgradable_and_lock(abs_time)) == true){
            mp_mutex = u_lock.release();
         }
      }
      else{
         u_lock.release();
      }
   }
   #endif

   //!Effects: If shar.owns() then calls try_unlock_sharable_and_lock() on the
   //!referenced mutex. 
   //!   a)if try_unlock_sharable_and_lock() returns true then mutex() obtains
   //!      the value from shar.release() and owns() is set to true. 
   //!   b)if try_unlock_sharable_and_lock() returns false then shar is
   //!      unaffected and this scoped_lock construction has the same
   //!      effects as a default construction. 
   //!   c)Else shar.owns() is false. mutex() obtains the value from
   //!      shar.release() and owns() is set to false 
   //!Notes: This construction will not block. It will try to obtain mutex
   //!   ownership from shar immediately, while changing the lock type from a
   //!   "read lock" to a "write lock". If the "read lock" isn't held in the
   //!   first place, the mutex merely changes type to an unlocked "write lock".
   //!   If the "read lock" is held, then mutex transfer occurs only if it can
   //!   do so in a non-blocking manner.
   #ifndef BOOST_INTERPROCESS_RVALUE_REFERENCE
   scoped_lock(detail::moved_object<sharable_lock<Mutex> > shar
              ,detail::try_to_lock_type)
      : mp_mutex(0), m_locked(false)
   {
      sharable_lock<mutex_type> &s_lock = shar.get();
      if(s_lock.owns()){
         if((m_locked = s_lock.mutex()->try_unlock_sharable_and_lock()) == true){
            mp_mutex = s_lock.release();
         }
      }
      else{
         s_lock.release();
      }
   }
   #else
   scoped_lock(sharable_lock<Mutex> &&shar
              ,detail::try_to_lock_type)
      : mp_mutex(0), m_locked(false)
   {
      sharable_lock<mutex_type> &s_lock = shar;
      if(s_lock.owns()){
         if((m_locked = s_lock.mutex()->try_unlock_sharable_and_lock()) == true){
            mp_mutex = s_lock.release();
         }
      }
      else{
         s_lock.release();
      }
   }
   #endif

   //!Effects: if (owns()) mp_mutex->unlock().
   //!Notes: The destructor behavior ensures that the mutex lock is not leaked.*/
   ~scoped_lock()
   {
      try{  if(m_locked && mp_mutex)   mp_mutex->unlock();  }
      catch(...){}
   }

   //!Effects: If owns() before the call, then unlock() is called on mutex().
   //!   *this gets the state of scop and scop gets set to a default constructed state. 
   //!Notes: With a recursive mutex it is possible that both this and scop own
   //!   the same mutex before the assignment. In this case, this will own the
   //!   mutex after the assignment (and scop will not), but the mutex's lock
   //!   count will be decremented by one.
   #ifndef BOOST_INTERPROCESS_RVALUE_REFERENCE
   scoped_lock &operator=(detail::moved_object<scoped_lock> scop)
   {  
      if(this->owns())
         this->unlock();
      m_locked = scop.get().owns();
      mp_mutex = scop.get().release();
      return *this;
   }
   #else
   scoped_lock &operator=(scoped_lock &&scop)
   {  
      if(this->owns())
         this->unlock();
      m_locked = scop.owns();
      mp_mutex = scop.release();
      return *this;
   }
   #endif

   //!Effects: If mutex() == 0 or if already locked, throws a lock_exception()
   //!   exception. Calls lock() on the referenced mutex.
   //!Postconditions: owns() == true.
   //!Notes: The scoped_lock changes from a state of not owning the mutex, to
   //!   owning the mutex, blocking if necessary.
   void lock()
   {
      if(!mp_mutex || m_locked)
         throw lock_exception();
      mp_mutex->lock();
      m_locked = true;
   }

   //!Effects: If mutex() == 0 or if already locked, throws a lock_exception()
   //!   exception. Calls try_lock() on the referenced mutex. 
   //!Postconditions: owns() == the value returned from mutex()->try_lock().
   //!Notes: The scoped_lock changes from a state of not owning the mutex, to
   //!   owning the mutex, but only if blocking was not required. If the
   //!   mutex_type does not support try_lock(), this function will fail at
   //!   compile time if instantiated, but otherwise have no effect.*/
   bool try_lock()
   {
      if(!mp_mutex || m_locked)
         throw lock_exception();
      m_locked = mp_mutex->try_lock();
      return m_locked;
   }

   //!Effects: If mutex() == 0 or if already locked, throws a lock_exception()
   //!   exception. Calls timed_lock(abs_time) on the referenced mutex.
   //!Postconditions: owns() == the value returned from mutex()-> timed_lock(abs_time).
   //!Notes: The scoped_lock changes from a state of not owning the mutex, to
   //!   owning the mutex, but only if it can obtain ownership by the specified
   //!   time. If the mutex_type does not support timed_lock (), this function
   //!   will fail at compile time if instantiated, but otherwise have no effect.*/
   bool timed_lock(const boost::posix_time::ptime& abs_time)
   {
      if(!mp_mutex || m_locked)
         throw lock_exception();
      m_locked = mp_mutex->timed_lock(abs_time);
      return m_locked;
   }

   //!Effects: If mutex() == 0 or if not locked, throws a lock_exception()
   //!   exception. Calls unlock() on the referenced mutex.
   //!Postconditions: owns() == false.
   //!Notes: The scoped_lock changes from a state of owning the mutex, to not
   //!   owning the mutex.*/
   void unlock()
   {
      if(!mp_mutex || !m_locked)
         throw lock_exception();
      mp_mutex->unlock();
      m_locked = false;
   }

   //!Effects: Returns true if this scoped_lock has acquired
   //!the referenced mutex.
   bool owns() const
   {  return m_locked && mp_mutex;  }

   //!Conversion to bool.
   //!Returns owns().
   operator unspecified_bool_type() const
   {  return m_locked? &this_type::m_locked : 0;   }

   //!Effects: Returns a pointer to the referenced mutex, or 0 if
   //!there is no mutex to reference.
   mutex_type* mutex() const
   {  return  mp_mutex;  }

   //!Effects: Returns a pointer to the referenced mutex, or 0 if there is no
   //!   mutex to reference.
   //!Postconditions: mutex() == 0 and owns() == false.
   mutex_type* release()
   {
      mutex_type *mut = mp_mutex;
      mp_mutex = 0;
      m_locked = false;
      return mut;
   }
 
   //!Effects: Swaps state with moved lock. 
   //!Throws: Nothing.
   #ifndef BOOST_INTERPROCESS_RVALUE_REFERENCE
   void swap(detail::moved_object<scoped_lock<mutex_type> > other)
   {
      std::swap(mp_mutex, other.get().mp_mutex);
      std::swap(m_locked, other.get().m_locked);
   }
   #else
   void swap(scoped_lock<mutex_type> &&other)
   {
      std::swap(mp_mutex, other.mp_mutex);
      std::swap(m_locked, other.m_locked);
   }
   #endif

   /// @cond
   private:
   mutex_type *mp_mutex; 
   bool        m_locked;
   /// @endcond
};

/// @cond

//!This class is movable
template <class M>
struct is_movable<scoped_lock<M> >
{
   enum {   value = true };
};
/// @endcond

} // namespace interprocess
} // namespace boost

#include <boost/interprocess/detail/config_end.hpp>

#endif // BOOST_INTERPROCESS_SCOPED_LOCK_HPP
