/*
   Copyright (C) 2018 by Jyrki Vesterinen <sandgtx@gmail.com>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <thread>
#include <vector>

class thread_pool
{
public:
	thread_pool();
	~thread_pool();

	/** Runs the given workload.
	@param work Container that contains objects to modify.
	Can be a std::vector, std::array or std::deque.
	@param func Function to call for every element in the container.
	The function is assumed to edit the object in-place.
	@return Future that becomes available when the task has finished. */
	template<typename W, typename F>
	std::future<void> run(W& work, F func);

private:
	const unsigned int NUM_THREADS = 16u;

	void thread_proc();

	template<typename W, typename F>
	void worker(W& work, F func);

	void init();

	std::vector<std::thread> threads_;
	std::function<void()> work_;
	std::mutex mutex_;
	unsigned int num_waiting_threads_;
	unsigned int num_finished_threads_;
	bool ready_for_work_;
	std::condition_variable ready_for_work_cond_;
	std::condition_variable work_cond_;
	std::promise<void> done_promise_;
	bool exiting_;
	std::atomic<unsigned int> counter_;
};

template<typename W, typename F>
std::future<void> thread_pool::run(W& work, F func)
{
	std::unique_lock<std::mutex> lock(mutex_);

	// There must not be existing ongoing work.
	assert(!work_);

	while(!ready_for_work_) {
		ready_for_work_cond_.wait(lock);
	}

	work_ = std::bind(&thread_pool::worker<W,F>, this, std::ref(work), func);
	counter_ = 0u;
	done_promise_ = std::promise<void>();
	work_cond_.notify_all();

	return done_promise_.get_future();
}

template<typename W, typename F>
void thread_pool::worker(W& work, F func)
{
	// Note that fetch_add() returns the previous value.
	// Thus, this returns zero for the first worker like it should.
	std::atomic<unsigned int> index = counter_.fetch_add(1u,
		std::memory_order::memory_order_relaxed);

	while(index < work.size()) {
		func(work[index]);
		index = counter_.fetch_add(1u, std::memory_order::memory_order_relaxed);
	}
}
