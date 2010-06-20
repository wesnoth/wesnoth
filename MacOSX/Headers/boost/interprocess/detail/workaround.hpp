//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2005-2008. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/interprocess for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef BOOST_INTERPROCESS_PTR_WRKRND_HPP
#define BOOST_INTERPROCESS_PTR_WRKRND_HPP

#include <boost/interprocess/detail/config_begin.hpp>

#undef BOOST_DISABLE_WIN32

#if !(defined BOOST_WINDOWS) || (defined BOOST_DISABLE_WIN32)

   #include <unistd.h>

   #if defined(_POSIX_THREAD_PROCESS_SHARED)
   # if !((_XOPEN_VERSION >= 600) && (_POSIX_THREAD_PROCESS_SHARED - 0 <= 0))
   //Cygwin defines _POSIX_THREAD_PROCESS_SHARED but does not implement it.
   //Mac Os X >= Leopard defines _POSIX_THREAD_PROCESS_SHARED but does not seems to work.
   #  if !defined(__CYGWIN__) && !defined(__APPLE__)
   #  define BOOST_INTERPROCESS_POSIX_PROCESS_SHARED
   #  endif
   # endif
   #endif 

   #if defined(_POSIX_BARRIERS)
   # if !((_XOPEN_VERSION >= 600) && (_POSIX_BARRIERS - 0 <= 0))
   # define BOOST_INTERPROCESS_POSIX_BARRIERS
   # endif
   #endif 

   #if defined(_POSIX_SEMAPHORES)
   # if !((_XOPEN_VERSION >= 600) && (_POSIX_SEMAPHORES - 0 <= 0))
   # define BOOST_INTERPROCESS_POSIX_SEMAPHORES
   #  if defined(__CYGWIN__)
      #define BOOST_INTERPROCESS_POSIX_SEMAPHORES_NO_UNLINK
   #  endif
   # endif
   #endif 

   #if ((defined _V6_ILP32_OFFBIG)  &&(_V6_ILP32_OFFBIG   - 0 > 0)) ||\
       ((defined _V6_LP64_OFF64)    &&(_V6_LP64_OFF64     - 0 > 0)) ||\
       ((defined _V6_LPBIG_OFFBIG)  &&(_V6_LPBIG_OFFBIG   - 0 > 0)) ||\
       ((defined _XBS5_ILP32_OFFBIG)&&(_XBS5_ILP32_OFFBIG - 0 > 0)) ||\
       ((defined _XBS5_LP64_OFF64)  &&(_XBS5_LP64_OFF64   - 0 > 0)) ||\
       ((defined _XBS5_LPBIG_OFFBIG)&&(_XBS5_LPBIG_OFFBIG - 0 > 0)) ||\
       ((defined _FILE_OFFSET_BITS) &&(_FILE_OFFSET_BITS  - 0 >= 64))||\
       ((defined _FILE_OFFSET_BITS) &&(_FILE_OFFSET_BITS  - 0 >= 64))
      #define BOOST_INTERPROCESS_UNIX_64_BIT_OR_BIGGER_OFF_T
   #else
   #endif

   #if defined(_POSIX_SHARED_MEMORY_OBJECTS)
   # if !((_XOPEN_VERSION >= 600) && (_POSIX_SHARED_MEMORY_OBJECTS - 0 <= 0))
   # define BOOST_INTERPROCESS_POSIX_SHARED_MEMORY_OBJECTS
   # endif
   #else
   # if defined(__vms)
   #  if __CRTL_VER >= 70200000
   #  define BOOST_INTERPROCESS_POSIX_SHARED_MEMORY_OBJECTS
   #  endif
   # endif 
   #endif

   #if defined(_POSIX_TIMEOUTS)
   # if !((_XOPEN_VERSION >= 600) && (_POSIX_TIMEOUTS - 0 <= 0))
   # define BOOST_INTERPROCESS_POSIX_TIMEOUTS
   # endif
   #endif 

   #ifdef BOOST_INTERPROCESS_POSIX_SHARED_MEMORY_OBJECTS
      //Some systems have filesystem-based shared memory, so the
      //portable "/shmname" format does not work due to permission issues
      //For those systems we need to form a path to a temporary directory:
      //          hp-ux               tru64               vms
      #if defined(__hpux) || defined(__osf__) || defined(__vms)
      #define BOOST_INTERPROCESS_FILESYSTEM_BASED_POSIX_SHARED_MEMORY
      #endif
   #endif

   #ifdef BOOST_INTERPROCESS_POSIX_SEMAPHORES
      //Some systems have filesystem-based shared memory, so the
      //portable "/semname" format does not work due to permission issues
      //For those systems we need to form a path to a temporary directory:
      //          hp-ux               tru64               vms
      #if defined(__hpux) || defined(__osf__) || defined(__vms)
      #define BOOST_INTERPROCESS_FILESYSTEM_BASED_POSIX_SEMAPHORES
      #endif
   #endif

   #if ((_POSIX_VERSION + 0)>= 200112L || (_XOPEN_VERSION + 0)>= 500)
   #define BOOST_INTERPROCESS_POSIX_RECURSIVE_MUTEXES
   #endif

#endif

#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ > 2)
// C++0x features are only enabled when -std=c++0x or -std=gnu++0x are
// passed on the command line, which in turn defines
// __GXX_EXPERIMENTAL_CXX0X__. Note: __GXX_EXPERIMENTAL_CPP0X__ is
// defined by some very early development versions of GCC 4.3; we will
// remove this part of the check in the near future.
#  if defined(__GXX_EXPERIMENTAL_CPP0X__) || defined(__GXX_EXPERIMENTAL_CXX0X__)
#     define BOOST_INTERPROCESS_RVALUE_REFERENCE
#     define BOOST_INTERPROCESS_VARIADIC_TEMPLATES
#     if defined(__GLIBCPP__) || defined(__GLIBCXX__)
#        define BOOST_INTERPROCESS_RVALUE_PAIR
#     endif
#  endif
#endif

#if defined(BOOST_INTERPROCESS_RVALUE_REFERENCE) && defined(BOOST_INTERPROCESS_VARIADIC_TEMPLATES)
#define BOOST_INTERPROCESS_PERFECT_FORWARDING
#endif

//Now declare some Boost.Interprocess features depending on the implementation

#if defined(BOOST_INTERPROCESS_POSIX_SEMAPHORES) && !defined(BOOST_INTERPROCESS_POSIX_SEMAPHORES_NO_UNLINK)

#define BOOST_INTERPROCESS_NAMED_MUTEX_USES_POSIX_SEMAPHORES

#endif

#if defined(BOOST_INTERPROCESS_POSIX_SEMAPHORES) && !defined(BOOST_INTERPROCESS_POSIX_SEMAPHORES_NO_UNLINK)

#define BOOST_INTERPROCESS_NAMED_MUTEX_USES_POSIX_SEMAPHORES
#define BOOST_INTERPROCESS_NAMED_SEMAPHORE_USES_POSIX_SEMAPHORES

#endif

#include <boost/interprocess/detail/config_end.hpp>

#endif   //#ifndef BOOST_INTERPROCESS_PTR_WRKRND_HPP
