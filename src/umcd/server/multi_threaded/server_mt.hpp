/*
	Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)

	Distributed under the Boost Software License, Version 1.0.
	(See http://www.boost.org/LICENSE_1_0.txt)
*/
/*
	Copyright (C) 2013 by Pierre Talbot <ptalbot@mopong.net>
	Part of the Battle for Wesnoth Project http://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#ifndef SERVER_MULTI_THREADED_SERVER_HPP
#define SERVER_MULTI_THREADED_SERVER_HPP

#include "umcd/server/basic_server.hpp"
#include "umcd/boost/thread/workaround.hpp"
#include <boost/thread/thread.hpp>
#include "umcd/umcd_logger.hpp"

template <class Protocol, class ProtocolFactory>
class server_mt : public basic_server<Protocol, ProtocolFactory>
{
private:
	typedef basic_server<Protocol, ProtocolFactory> base;

public:
	explicit server_mt(const config& cfg, typename base::protocol_factory_type protocol_factory);
	void run();

private:
	std::size_t thread_pool_size_;
};

template <class Protocol, class ProtocolFactory>
server_mt<Protocol, ProtocolFactory>::server_mt(const config &cfg, typename base::protocol_factory_type protocol_factory)
: base(cfg, protocol_factory)
{
	thread_pool_size_ = cfg["threads"];
	if(thread_pool_size_ == 0)
	{
		thread_pool_size_ = boost::thread::hardware_concurrency();
		UMCD_LOG(info) << thread_pool_size_ << " cores found.";
	}
}

template <class Protocol, class ProtocolFactory>
void server_mt<Protocol, ProtocolFactory>::run()
{
	// Create a pool of threads to run all of the io_services.
	std::vector<boost::shared_ptr<boost::thread> > threads;
	for (std::size_t i = 0; i < thread_pool_size_-1; ++i)
	{
		boost::shared_ptr<boost::thread> thread = boost::make_shared<boost::thread>(
					boost::bind(&base::run, this));
		threads.push_back(thread);
	}

	// This thread is also used.
	base::run();

	// Wait for all threads in the pool to exit.
	for (std::size_t i = 0; i < threads.size(); ++i)
		threads[i]->join();
}

#endif // SERVER_MULTI_THREADED_SERVER_HPP
