#ifndef THREAD_HPP_INCLUDED
#define THREAD_HPP_INCLUDED

#include "SDL.h"
#include "SDL_thread.h"

namespace threading
{

class thread
{
public:
	explicit thread(int (*f)(void*), void* data=NULL);
	~thread();

	void kill();
	void join();
private:
	thread(const thread&);
	void operator=(const thread&);

	SDL_Thread* thread_;
};

class mutex
{
public:
	mutex();
	~mutex();

	friend class lock;
	friend class condition;

private:
	mutex(const mutex&);
	void operator=(const mutex&);

	SDL_mutex* const m_;
};

class lock
{
public:
	explicit lock(mutex&);
	~lock();
private:
	lock(const lock&);
	void operator=(const lock&);

	mutex& m_;
};

class condition
{
public:
	condition();
	~condition();

	bool wait(const mutex& m);
	bool wait_timeout(const mutex& m, unsigned int timeout);
	void notify_one();
	void notify_all();

private:
	condition(const condition&);
	void operator=(const condition&);

	SDL_cond* const cond_;
};

}

#endif
