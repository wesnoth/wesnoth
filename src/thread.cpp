/* $Id$ */
/*
   Copyright (C) 2003 - 2008 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"
#include "log.hpp"
#include "thread.hpp"

#include <new>
#include <iostream>
#include <memory>
#include <vector>

#define ERR_G LOG_STREAM(err, general)

static int run_async_operation(void* data)
{
	threading::async_operation* const op = reinterpret_cast<threading::async_operation*>(data);
	op->run();

	bool should_delete = false;
	{
		const threading::lock l(op->get_mutex());
		op->notify_finished(); //in case the operation didn't notify of finishing
		if(op->is_aborted()) {
			should_delete = true;
		}
	}

	if(should_delete) {
		delete op;
	}


	return 0;
}

namespace {

std::vector<SDL_Thread*> detached_threads;

}

namespace threading {

manager::~manager()
{
	for(std::vector<SDL_Thread*>::iterator i = detached_threads.begin(); i != detached_threads.end(); ++i) {
		SDL_WaitThread(*i,NULL);
	}
}

thread::thread(int (*f)(void*), void* data) : thread_(SDL_CreateThread(f,data))
{}

thread::~thread()
{
	join();
}

void thread::kill()
{
	if(thread_ != NULL) {
		SDL_KillThread(thread_);
		thread_ = NULL;
	}
}

void thread::join()
{
	if(thread_ != NULL) {
		SDL_WaitThread(thread_,NULL);
		thread_ = NULL;
	}
}

void thread::detach()
{
	detached_threads.push_back(thread_);
	thread_ = NULL;
}

mutex::mutex() : m_(SDL_CreateMutex())
{}

mutex::~mutex()
{
	SDL_DestroyMutex(m_);
}

lock::lock(mutex& m) : m_(m)
{
	SDL_mutexP(m_.m_);
}

lock::~lock()
{
	SDL_mutexV(m_.m_);
}

condition::condition() : cond_(SDL_CreateCond())
{}

condition::~condition()
{
	SDL_DestroyCond(cond_);
}

bool condition::wait(const mutex& m)
{
	return SDL_CondWait(cond_,m.m_) == 0;
}

condition::WAIT_TIMEOUT_RESULT condition::wait_timeout(const mutex& m, unsigned int timeout)
{
	const int res = SDL_CondWaitTimeout(cond_,m.m_,timeout);
	switch(res) {
		case 0: return WAIT_OK;
		case SDL_MUTEX_TIMEDOUT: return WAIT_TIMEOUT;
		default:
			ERR_G << "SDL_CondWaitTimeout: " << SDL_GetError() << "\n";
			return WAIT_ERROR;
	}
}

bool condition::notify_one()
{
	if(SDL_CondSignal(cond_) < 0) {
		ERR_G << "SDL_CondSignal: " << SDL_GetError() << "\n";
		return false;
	}

	return true;
}

bool condition::notify_all()
{
	if(SDL_CondBroadcast(cond_) < 0) {
		ERR_G << "SDL_CondBroadcast: " << SDL_GetError() << "\n";
		return false;
	}
	return true;
}

bool async_operation::notify_finished()
{
	finishedVar_ = true;
	return finished_.notify_one();
}

async_operation::RESULT async_operation::execute(waiter& wait)
{
	//the thread must be created after the lock, and also destroyed after it.
	//this is because during the thread's execution, we must always hold the mutex
	//unless we are waiting on notification that the thread is finished, or we have
	//already received that notification.
	//
	//we cannot hold the mutex while waiting for the thread to join though, because
	//the thread needs access to the mutex before it terminates
	std::auto_ptr<thread> t(NULL);
	{
		const lock l(get_mutex());
		t = std::auto_ptr<thread>(new thread(run_async_operation,this));

		bool completed = false;
		while(wait.process() == waiter::WAIT) {
			const condition::WAIT_TIMEOUT_RESULT res = finished_.wait_timeout(get_mutex(),20);
			if(res == condition::WAIT_OK || finishedVar_) {
				completed = true;
				break;
			}
#ifndef __BEOS__
			else if(res == condition::WAIT_ERROR) {
				break;
			}
#endif
		}

		if(!completed) {
			aborted_ = true;
			t->detach();
			return ABORTED;
		}
	}

	return COMPLETED;
}


}
