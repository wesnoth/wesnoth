#include "global.hpp"

#include "thread.hpp"

#include <iostream>
#include <vector>

namespace {

int run_async_operation(void* data)
{
	threading::async_operation* const op = reinterpret_cast<threading::async_operation*>(data);
	op->run();
	op->notify_finished(); //in case the operation didn't notify of finishing

	return 0;
}

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
	const int res = SDL_CondWaitTimeout(cond_,m.m_,timeout) == 0;
	switch(res) {
	//the SDL documentation appears backward on when these results are returned
	case 0: return WAIT_TIMEOUT;
	case SDL_MUTEX_TIMEDOUT: return WAIT_OK;
	default: return WAIT_ERROR;
	}
}

void condition::notify_one()
{
	SDL_CondSignal(cond_);
}

void condition::notify_all()
{
	SDL_CondBroadcast(cond_);
}

void async_operation::notify_finished()
{
	finished_.notify_one();
}

async_operation::RESULT async_operation::execute(waiter& wait)
{
	const lock l(get_mutex());
	thread t(run_async_operation,this);

	while(wait.process() == waiter::WAIT) {
		std::cerr << "process...\n";
		const condition::WAIT_TIMEOUT_RESULT res = finished_.wait_timeout(get_mutex(),20);
		if(res == condition::WAIT_OK) {
			return COMPLETED;
		} else if(res == condition::WAIT_ERROR) {
			break;
		}
	}

	aborted_ = true;
	t.detach();
	return ABORTED;
}


}
