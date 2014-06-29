/*
   Copyright (C) 2003 - 2014 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef THREAD_HPP_INCLUDED
#define THREAD_HPP_INCLUDED

#include <list>

#include <boost/cstdint.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/smart_ptr.hpp>

struct SDL_Thread;

#if defined(_MSC_VER) && _MSC_VER <= 1600
/*
	This is needed because msvc up to 2010 fails to correcty forward declare this struct as a return value this case.
	And will create corrupt binaries without giving a warning / error.
*/
#include <SDL_mutex.h>
#else
struct SDL_mutex;
struct SDL_cond;
#endif

// Threading primitives wrapper for SDL_Thread.
//
// This module defines primitives for wrapping C++ around SDL's threading
// interface
namespace threading
{

struct manager
{
	~manager();
};

// Threading object.
//
// This class defines threading objects. One such object represents a
// thread and admits killing and joining on threads. Intended to be
// used for manipulating threads instead of poking around with SDL_Thread
// calls.
class thread
	: private boost::noncopyable
{
public:
	// Construct a new thread to start executing the function
	// pointed to by f. The void* data will be passed to f, to
	// facilitate passing of parameters to f.
	//
	// \param f the function at which the thread should start executing
	// \param data passed to f
	//
	// \pre f != NULL
	explicit thread(int (*f)(void*), void* data=NULL);

	// Destroy the thread object. This is done by waiting on the
	// thread with the join() operation, thus blocking until the
	// thread object has finished its operation.
	~thread();

	// Join (wait) on the thread to finish. When the thread finishes,
	// the function will return. calling wait() on an already killed
	// thread is a no-op.
	void join();

	void detach();

	boost::uint32_t get_id();
private:

	SDL_Thread* thread_;
};

boost::uint32_t get_current_thread_id();
// Binary mutexes.
//
// Implements an interface to binary mutexes. This class only defines the
// mutex itself. Locking is handled through the friend class lock,
// and monitor interfacing through condition variables is handled through
// the friend class condition.
class mutex
	: private boost::noncopyable
{
public:
	mutex();
	~mutex();

	friend class lock;
	friend class condition;

private:

	SDL_mutex* const m_;
};

// Binary mutex locking.
//
// Implements a locking object for mutexes. The creation of a lock
// object on a mutex will lock the mutex as long as this object is
// not deleted.
class lock
	: private boost::noncopyable
{
public:
	// Create a lock object on the mutex given as a parameter to
	// the constructor. The lock will be held for the duration
	// of the object existence.
	// If the mutex is already locked, the constructor will
	// block until the mutex lock can be acquired.
	//
	// \param m the mutex on which we should try to lock.
	explicit lock(mutex& m);
	// Delete the lock object, thus releasing the lock acquired
	// on the mutex which the lock object was created with.
	~lock();
private:

	mutex& m_;
};

// Condition variable locking.
//
// Implements condition variables for mutexes. A condition variable
// allows you to free up a lock inside a critical section
// of the code and regain it later. Condition classes only make
// sense to do operations on, if one already acquired a mutex.
class condition
	: private boost::noncopyable
{
public:
	condition();
	~condition();

	// Wait on the condition. When the condition is met, you
	// have a lock on the mutex and can do work in the critical
	// section. When the condition is not met, wait blocks until
	// the condition is met and atomically frees up the lock on
	// the mutex. One will automatically regain the lock when the
	// thread unblocks.
	//
	// If wait returns false we have an error. In this case one cannot
	// assume that he has a lock on the mutex anymore.
	//
	// \param m the mutex you wish to free the lock for
	// \returns true: the wait was successful, false: an error occurred
	//
	// \pre You have already acquired a lock on mutex m
	//
	bool wait(const mutex& m);

	enum WAIT_TIMEOUT_RESULT { WAIT_OK, WAIT_TIMED_OUT, WAIT_ERROR };

	// wait on the condition with a timeout. Basically the same as the
	// wait() function, but if the lock is not acquired before the
	// timeout, the function returns with an error.
	//
	// \param m the mutex you wish free the lock for.
	// \param timeout the allowed timeout in milliseconds (ms)
	// \returns result based on whether condition was met, it timed out,
	// or there was an error
	WAIT_TIMEOUT_RESULT wait_timeout(const mutex& m, unsigned int timeout);
	// signal the condition and wake up one thread waiting on the
	// condition. If no thread is waiting, notify_one() is a no-op.
	// Does not unlock the mutex.
	bool notify_one();

	// signal all threads waiting on the condition and let them contend
	// for the lock. This is often used when varying resource amounts are
	// involved and you do not know how many processes might continue.
	// The function should be used with care, especially if many threads are
	// waiting on the condition variable.
	bool notify_all();

private:

	SDL_cond* const cond_;
};

//class which defines an interface for waiting on an asynchronous operation
class waiter {
public:
	enum ACTION { WAIT, ABORT };

	virtual ~waiter() {}
	virtual ACTION process() = 0;
};

class async_operation;

typedef boost::shared_ptr<async_operation> async_operation_ptr;

typedef std::list<async_operation_ptr> active_operation_list;

//class which defines an asynchronous operation. Objects of this class are accessed from
//both the worker thread and the calling thread, and so it has 'strange' allocation semantics.
//It is allocated by the caller, and generally deleted by the caller. However, in some cases
//the asynchronous operation is aborted, and the caller abandons it. The caller cannot still
//delete the operation, since the worker thread might still access it, so in the case when the
//operation is aborted, the worker thread will delete it.
//
//The caller should hold these objects using the async_operation_holder class below, which will
//handle the delete semantics
class async_operation
{
public:

	enum RESULT { COMPLETED, ABORTED };

	async_operation() :
		thread_(), aborted_(false), finished_(), finishedVar_(false), mutex_()
	{
		while (!active_.empty() && active_.front().unique())
			active_.pop_front();
	}
	virtual ~async_operation() {}

	RESULT execute(async_operation_ptr this_ptr, waiter& wait);

	mutex& get_mutex() { return mutex_; }

	virtual void run() = 0;

	//notify that the operation is finished. Can be called from within the thread
	//while holding the mutex and after checking is_aborted()
	//if we want to be sure that if the operation is completed, the caller is notified.
	//will be called in any case after the operation returns
	bool notify_finished();

	//must hold the mutex before calling this function from the worker thread
	bool is_aborted() const { return aborted_; }

private:
	boost::scoped_ptr<thread> thread_;
	bool aborted_;
	condition finished_;
	bool finishedVar_;
	mutex mutex_;

	static active_operation_list active_;
};

}

#endif
