/* $Id$ */
/*
   Copyright (C) 2008 by Pauli Nieminen <paniemin@cc.hut.fi>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef SERVER_MONITOR_HPP_INCLUDED
#define SERVER_MONITOR_HPP_INCLUDED
#include "../thread.hpp"

namespace nserver  {

class server_monitor {
public:
	server_monitor(size_t delay = 1000);
	~server_monitor();
#ifdef SERVER_MONITOR
	void check();
	threading::mutex& get_mutex() const {return *mutex_;};
	bool is_ending() const {return ending_;};
	size_t get_delay() const {return delay_;};
private:
	bool ending_;
	int delay_;
	threading::mutex* mutex_;
	threading::thread* thread_;
#endif
private:
	server_monitor(const server_monitor&);
	const server_monitor& operator=(const server_monitor&);

};
}
#endif // SERVER_MONITOR_HPP_INCLUDED
