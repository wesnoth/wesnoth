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

#include "thread_pool.hpp"

#include <algorithm>
#include <cassert>
#include <iterator>

thread_pool::thread_pool()
	: exiting_(false)
{
	init();

	std::generate_n(std::back_inserter(threads_), NUM_THREADS,
		[this]() { return std::thread(&thread_pool::thread_proc, this); });
}

thread_pool::~thread_pool()
{
	{
		std::unique_lock<std::mutex> lock(mutex_);

		// A thread pool can't exit if work is in progress.
		assert(!work_);

		exiting_ = true;
		work_cond_.notify_all();
	}

	for(std::thread& t : threads_) {
		t.join();
	}
}

void thread_pool::init()
{
	num_waiting_threads_ = 0u;
	num_finished_threads_ = 0u;
	ready_for_work_ = false;
	work_ = nullptr;
	done_ = false;
}

void thread_pool::thread_proc()
{
	std::unique_lock<std::mutex> lock(mutex_);

	++num_waiting_threads_;
	if(num_waiting_threads_ == NUM_THREADS) {
		ready_for_work_ = true;
		ready_for_work_cond_.notify_one();
	}

	while(!work_ && !exiting_) {
		work_cond_.wait(lock);
	}

	num_finished_threads_ = 0u;
	lock.unlock();

	while(!exiting_) {
		work_();

		lock.lock();
		++num_finished_threads_;
		if(num_finished_threads_ == NUM_THREADS) {
			done_ = true;
			done_promise_.set_value();
			work_ = nullptr;
		}

		while(!work_ && !exiting_) {
			work_cond_.wait(lock);
		}

		num_finished_threads_ = 0u;
		lock.unlock();
	}
}
