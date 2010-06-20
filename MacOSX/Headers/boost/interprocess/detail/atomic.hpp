//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2006-2008
// (C) Copyright Markus Schoepflin 2007
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/interprocess for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef BOOST_INTERPROCESS_DETAIL_ATOMIC_HPP
#define BOOST_INTERPROCESS_DETAIL_ATOMIC_HPP

#include <boost/interprocess/detail/config_begin.hpp>
#include <boost/interprocess/detail/workaround.hpp>
#include <boost/cstdint.hpp>

namespace boost{
namespace interprocess{
namespace detail{

//! Atomically increment an apr_uint32_t by 1
//! "mem": pointer to the object
//! Returns the old value pointed to by mem
inline boost::uint32_t atomic_inc32(volatile boost::uint32_t *mem);

//! Atomically read an boost::uint32_t from memory
inline boost::uint32_t atomic_read32(volatile boost::uint32_t *mem);

//! Atomically set an boost::uint32_t in memory
//! "mem": pointer to the object
//! "param": val value that the object will assume
inline void atomic_write32(volatile boost::uint32_t *mem, boost::uint32_t val);

//! Compare an boost::uint32_t's value with "cmp".
//! If they are the same swap the value with "with"
//! "mem": pointer to the value
//! "with": what to swap it with
//! "cmp": the value to compare it to
//! Returns the old value of *mem
inline boost::uint32_t atomic_cas32
   (volatile boost::uint32_t *mem, boost::uint32_t with, boost::uint32_t cmp);

}  //namespace detail{
}  //namespace interprocess{
}  //namespace boost{

#if (defined BOOST_WINDOWS) && !(defined BOOST_DISABLE_WIN32)

#include <boost/interprocess/detail/win32_api.hpp>

namespace boost{
namespace interprocess{
namespace detail{

//! Atomically decrement an boost::uint32_t by 1
//! "mem": pointer to the atomic value
//! Returns the old value pointed to by mem
inline boost::uint32_t atomic_dec32(volatile boost::uint32_t *mem)
{  return winapi::interlocked_decrement(reinterpret_cast<volatile long*>(mem)) + 1;  }

//! Atomically increment an apr_uint32_t by 1
//! "mem": pointer to the object
//! Returns the old value pointed to by mem
inline boost::uint32_t atomic_inc32(volatile boost::uint32_t *mem)
{  return winapi::interlocked_increment(reinterpret_cast<volatile long*>(mem))-1;  }

//! Atomically read an boost::uint32_t from memory
inline boost::uint32_t atomic_read32(volatile boost::uint32_t *mem)
{  return *mem;   }

//! Atomically set an boost::uint32_t in memory
//! "mem": pointer to the object
//! "param": val value that the object will assume
inline void atomic_write32(volatile boost::uint32_t *mem, boost::uint32_t val)
{  winapi::interlocked_exchange(reinterpret_cast<volatile long*>(mem), val);  }

//! Compare an boost::uint32_t's value with "cmp".
//! If they are the same swap the value with "with"
//! "mem": pointer to the value
//! "with": what to swap it with
//! "cmp": the value to compare it to
//! Returns the old value of *mem
inline boost::uint32_t atomic_cas32
   (volatile boost::uint32_t *mem, boost::uint32_t with, boost::uint32_t cmp)
{  return winapi::interlocked_compare_exchange(reinterpret_cast<volatile long*>(mem), with, cmp);  }

}  //namespace detail{
}  //namespace interprocess{
}  //namespace boost{

#elif defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))

namespace boost {
namespace interprocess {
namespace detail{

//! Compare an boost::uint32_t's value with "cmp".
//! If they are the same swap the value with "with"
//! "mem": pointer to the value
//! "with" what to swap it with
//! "cmp": the value to compare it to
//! Returns the old value of *mem
inline boost::uint32_t atomic_cas32
   (volatile boost::uint32_t *mem, boost::uint32_t with, boost::uint32_t cmp)
{
   boost::uint32_t prev = cmp;
   asm volatile( "lock\n\t"
                 "cmpxchg %3,%1"
               : "=a" (prev), "=m" (*(mem))
               : "0" (prev), "r" (with)
               : "memory", "cc");
   return prev;
/*
   boost::uint32_t prev;

   asm volatile ("lock; cmpxchgl %1, %2"             
               : "=a" (prev)               
               : "r" (with), "m" (*(mem)), "0"(cmp));
   asm volatile("" : : : "memory");

   return prev;
*/
}

//! Atomically add 'val' to an boost::uint32_t
//! "mem": pointer to the object
//! "val": amount to add
//! Returns the old value pointed to by mem
inline boost::uint32_t atomic_add32
   (volatile boost::uint32_t *mem, boost::uint32_t val)
{
   // int r = *pw;
   // *mem += val;
   // return r;
   int r;

   asm volatile
   (
      "lock\n\t"
      "xadd %1, %0":
      "+m"( *mem ), "=r"( r ): // outputs (%0, %1)
      "1"( val ): // inputs (%2 == %1)
      "memory", "cc" // clobbers
   );

   return r;
/*
   asm volatile( "lock\n\t; xaddl %0,%1"
               : "=r"(val), "=m"(*mem)
               : "0"(val), "m"(*mem));
   asm volatile("" : : : "memory");

   return val;
*/
}

//! Atomically increment an apr_uint32_t by 1
//! "mem": pointer to the object
//! Returns the old value pointed to by mem
inline boost::uint32_t atomic_inc32(volatile boost::uint32_t *mem)
{  return atomic_add32(mem, 1);  }

//! Atomically decrement an boost::uint32_t by 1
//! "mem": pointer to the atomic value
//! Returns the old value pointed to by mem
inline boost::uint32_t atomic_dec32(volatile boost::uint32_t *mem)
{  return atomic_add32(mem, (boost::uint32_t)-1);  }

//! Atomically read an boost::uint32_t from memory
inline boost::uint32_t atomic_read32(volatile boost::uint32_t *mem)
{  return *mem;   }

//! Atomically set an boost::uint32_t in memory
//! "mem": pointer to the object
//! "param": val value that the object will assume
inline void atomic_write32(volatile boost::uint32_t *mem, boost::uint32_t val)
{  *mem = val; }

}  //namespace detail{
}  //namespace interprocess{
}  //namespace boost{

#elif defined(__GNUC__) && (defined(__PPC__) || defined(__ppc__))

namespace boost {
namespace interprocess {
namespace detail{

//! Atomically add 'val' to an boost::uint32_t
//! "mem": pointer to the object
//! "val": amount to add
//! Returns the old value pointed to by mem
inline boost::uint32_t atomic_add32(volatile boost::uint32_t *mem, boost::uint32_t val)
{
   boost::uint32_t prev, temp;

   asm volatile ("0:\n\t"                 // retry local label     
               "lwarx  %0,0,%2\n\t"       // load prev and reserve 
               "add    %1,%0,%3\n\t"      // temp = prev + val   
               "stwcx. %1,0,%2\n\t"       // conditionally store   
               "bne-   0b"                // start over if we lost
                                          // the reservation
               //XXX find a cleaner way to define the temp         
               //it's not an output
               : "=&r" (prev), "=&r" (temp)        // output, temp 
               : "b" (mem), "r" (val)              // inputs       
               : "memory", "cc");                  // clobbered    
   return prev;
}

//! Compare an boost::uint32_t's value with "cmp".
//! If they are the same swap the value with "with"
//! "mem": pointer to the value
//! "with" what to swap it with
//! "cmp": the value to compare it to
//! Returns the old value of *mem
inline boost::uint32_t atomic_cas32
   (volatile boost::uint32_t *mem, boost::uint32_t with, boost::uint32_t cmp)
{
   boost::uint32_t prev;

   asm volatile ("0:\n\t"                 // retry local label     
               "lwarx  %0,0,%1\n\t"       // load prev and reserve 
               "cmpw   %0,%3\n\t"         // does it match cmp?    
               "bne-   1f\n\t"            // ...no, bail out       
               "stwcx. %2,0,%1\n\t"       // ...yes, conditionally
                                          //   store with            
               "bne-   0b\n\t"            // start over if we lost
                                          //   the reservation       
               "1:"                       // exit local label      

               : "=&r"(prev)                        // output      
               : "b" (mem), "r" (with), "r"(cmp)    // inputs      
               : "memory", "cc");                   // clobbered   
   return prev;
}

//! Atomically increment an apr_uint32_t by 1
//! "mem": pointer to the object
//! Returns the old value pointed to by mem
inline boost::uint32_t atomic_inc32(volatile boost::uint32_t *mem)
{  return atomic_add32(mem, 1);  }

//! Atomically decrement an boost::uint32_t by 1
//! "mem": pointer to the atomic value
//! Returns the old value pointed to by mem
inline boost::uint32_t atomic_dec32(volatile boost::uint32_t *mem)
{  return atomic_add32(mem, boost::uint32_t(-1u));  }

//! Atomically read an boost::uint32_t from memory
inline boost::uint32_t atomic_read32(volatile boost::uint32_t *mem)
{  return *mem;   }

//! Atomically set an boost::uint32_t in memory
//! "mem": pointer to the object
//! "param": val value that the object will assume
inline void atomic_write32(volatile boost::uint32_t *mem, boost::uint32_t val)
{  *mem = val; }

}  //namespace detail{
}  //namespace interprocess{
}  //namespace boost{

#elif defined(__GNUC__) && ( __GNUC__ * 100 + __GNUC_MINOR__ >= 401 )

namespace boost {
namespace interprocess {
namespace detail{

//! Atomically add 'val' to an boost::uint32_t
//! "mem": pointer to the object
//! "val": amount to add
//! Returns the old value pointed to by mem
inline boost::uint32_t atomic_add32
   (volatile boost::uint32_t *mem, boost::uint32_t val)
{  return __sync_fetch_and_add(const_cast<boost::uint32_t *>(mem), val);   }

//! Atomically increment an apr_uint32_t by 1
//! "mem": pointer to the object
//! Returns the old value pointed to by mem
inline boost::uint32_t atomic_inc32(volatile boost::uint32_t *mem)
{  return atomic_add32(mem, 1);  }

//! Atomically decrement an boost::uint32_t by 1
//! "mem": pointer to the atomic value
//! Returns the old value pointed to by mem
inline boost::uint32_t atomic_dec32(volatile boost::uint32_t *mem)
{  return atomic_add32(mem, (boost::uint32_t)-1);   }

//! Atomically read an boost::uint32_t from memory
inline boost::uint32_t atomic_read32(volatile boost::uint32_t *mem)
{  return *mem;   }

//! Compare an boost::uint32_t's value with "cmp".
//! If they are the same swap the value with "with"
//! "mem": pointer to the value
//! "with" what to swap it with
//! "cmp": the value to compare it to
//! Returns the old value of *mem
inline boost::uint32_t atomic_cas32
   (volatile boost::uint32_t *mem, boost::uint32_t with, boost::uint32_t cmp)
{  return __sync_val_compare_and_swap(const_cast<boost::uint32_t *>(mem), with, cmp);   }

//! Atomically set an boost::uint32_t in memory
//! "mem": pointer to the object
//! "param": val value that the object will assume
inline void atomic_write32(volatile boost::uint32_t *mem, boost::uint32_t val)
{  *mem = val; }

}  //namespace detail{
}  //namespace interprocess{
}  //namespace boost{

#elif (defined(sun) || defined(__sun))

#include <atomic.h>

namespace boost{
namespace interprocess{
namespace detail{

//! Atomically add 'val' to an boost::uint32_t
//! "mem": pointer to the object
//! "val": amount to add
//! Returns the old value pointed to by mem
inline boost::uint32_t atomic_add32(volatile boost::uint32_t *mem, boost::uint32_t val)
{   return atomic_add_32_nv(reinterpret_cast<volatile ::uint32_t*>(mem), (int32_t)val) - val;   }

//! Compare an boost::uint32_t's value with "cmp".
//! If they are the same swap the value with "with"
//! "mem": pointer to the value
//! "with" what to swap it with
//! "cmp": the value to compare it to
//! Returns the old value of *mem
inline boost::uint32_t atomic_cas32
   (volatile boost::uint32_t *mem, boost::uint32_t with, boost::uint32_t cmp)
{  return atomic_cas_32(reinterpret_cast<volatile ::uint32_t*>(mem), cmp, with);  }

//! Atomically increment an apr_uint32_t by 1
//! "mem": pointer to the object
//! Returns the old value pointed to by mem
inline boost::uint32_t atomic_inc32(volatile boost::uint32_t *mem)
{  return atomic_add_32_nv(reinterpret_cast<volatile ::uint32_t*>(mem), 1) - 1; }

//! Atomically decrement an boost::uint32_t by 1
//! "mem": pointer to the atomic value
//! Returns the old value pointed to by mem
inline boost::uint32_t atomic_dec32(volatile boost::uint32_t *mem)
{  return atomic_add_32_nv(reinterpret_cast<volatile ::uint32_t*>(mem), (boost::uint32_t)-1) + 1; }

//! Atomically read an boost::uint32_t from memory
inline boost::uint32_t atomic_read32(volatile boost::uint32_t *mem)
{  return *mem;   }

//! Atomically set an boost::uint32_t in memory
//! "mem": pointer to the object
//! "param": val value that the object will assume
inline void atomic_write32(volatile boost::uint32_t *mem, boost::uint32_t val)
{  *mem = val; }

}  //namespace detail{
}  //namespace interprocess{
}  //namespace boost{

#elif defined(__osf__) && defined(__DECCXX)

#include <machine/builtins.h>
#include <c_asm.h>

namespace boost{
namespace interprocess{
namespace detail{

//! Atomically decrement a uint32_t by 1
//! "mem": pointer to the atomic value
//! Returns the old value pointed to by mem
//! Acquire, memory barrier after decrement.
inline boost::uint32_t atomic_dec32(volatile boost::uint32_t *mem)
{  boost::uint32_t old_val = __ATOMIC_DECREMENT_LONG(mem); __MB(); return old_val; }

//! Atomically increment a uint32_t by 1
//! "mem": pointer to the object
//! Returns the old value pointed to by mem
//! Release, memory barrier before increment.
inline boost::uint32_t atomic_inc32(volatile boost::uint32_t *mem)
{  __MB(); return __ATOMIC_INCREMENT_LONG(mem); }

// Rational for the implementation of the atomic read and write functions.
//
// 1. The Alpha Architecture Handbook requires that access to a byte,
// an aligned word, an aligned longword, or an aligned quadword is
// atomic. (See 'Alpha Architecture Handbook', version 4, chapter 5.2.2.)
//
// 2. The CXX User's Guide states that volatile quantities are accessed
// with single assembler instructions, and that a compilation error
// occurs when declaring a quantity as volatile which is not properly
// aligned.

//! Atomically read an boost::uint32_t from memory
//! Acquire, memory barrier after load.
inline boost::uint32_t atomic_read32(volatile boost::uint32_t *mem)
{  boost::uint32_t old_val = *mem; __MB(); return old_val;  }

//! Atomically set an boost::uint32_t in memory
//! "mem": pointer to the object
//! "param": val value that the object will assume
//! Release, memory barrier before store.
inline void atomic_write32(volatile boost::uint32_t *mem, boost::uint32_t val)
{  __MB(); *mem = val; }

//! Compare an boost::uint32_t's value with "cmp".
//! If they are the same swap the value with "with"
//! "mem": pointer to the value
//! "with" what to swap it with
//! "cmp": the value to compare it to
//! Returns the old value of *mem
//! Memory barrier between load and store.
inline boost::uint32_t atomic_cas32(
  volatile boost::uint32_t *mem, boost::uint32_t with, boost::uint32_t cmp)
{
  // Note:
  //
  // Branch prediction prefers backward branches, and the Alpha Architecture
  // Handbook explicitely states that the loop should not be implemented like
  // it is below. (See chapter 4.2.5.) Therefore the code should probably look
  // like this:
  //
  // return asm(
  //   "10: ldl_l %v0,(%a0) ;"
  //   "    cmpeq %v0,%a2,%t0 ;"
  //   "    beq %t0,20f ;"
  //   "    mb ;"
  //   "    mov %a1,%t0 ;"
  //   "    stl_c %t0,(%a0) ;"
  //   "    beq %t0,30f ;"
  //   "20: ret ;"
  //   "30: br 10b;",
  //   mem, with, cmp);
  //
  // But as the compiler always transforms this into the form where a backward
  // branch is taken on failure, we can as well implement it in the straight
  // forward form, as this is what it will end up in anyway.

  return asm(
    "10: ldl_l %v0,(%a0) ;"    // load prev value from mem and lock mem
    "    cmpeq %v0,%a2,%t0 ;"  // compare with given value
    "    beq %t0,20f ;"        // if not equal, we're done
    "    mb ;"                 // memory barrier
    "    mov %a1,%t0 ;"        // load new value into scratch register
    "    stl_c %t0,(%a0) ;"    // store new value to locked mem (overwriting scratch)
    "    beq %t0,10b ;"        // store failed because lock has been stolen, retry
    "20: ",
    mem, with, cmp);
}

}  //namespace detail{
}  //namespace interprocess{
}  //namespace boost{

#else

#error No atomic operations implemented for this platform, sorry!

#endif

#include <boost/interprocess/detail/config_end.hpp>

#endif   //BOOST_INTERPROCESS_DETAIL_ATOMIC_HPP
