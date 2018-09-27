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

#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <thread>
#include <vector>

class thread_pool
{
	thread_pool();
	~thread_pool();

private:
	const unsigned int NUM_THREADS = 16u;

	void thread_proc();

	void init();

	std::vector<std::thread> threads_;
	std::function<void()> work_;
	std::mutex mutex_;
	unsigned int num_waiting_threads_;
	unsigned int num_finished_threads_;
	bool ready_for_work_;
	std::condition_variable ready_for_work_cond_;
	std::condition_variable work_cond_;
	bool done_;
	std::promise<void> done_promise_;
	bool exiting_;
};
