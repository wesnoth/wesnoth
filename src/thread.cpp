#include "global.hpp"

#include "thread.hpp"

namespace threading {

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

bool condition::wait_timeout(const mutex& m, unsigned int timeout)
{
	return SDL_CondWaitTimeout(cond_,m.m_,timeout) == 0;
}

void condition::notify_one()
{
	SDL_CondSignal(cond_);
}

void condition::notify_all()
{
	SDL_CondBroadcast(cond_);
}

}
