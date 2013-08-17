/*
	Copyright (C) 2012-2013 by Pierre Talbot <ptalbot@mopong.net>
	Part of the Battle for Wesnoth Project http://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#ifndef UMCD_DAEMON_HPP
#define UMCD_DAEMON_HPP

#include <boost/optional.hpp>
#include <cstdlib>

#ifndef _WIN32
#include <unistd.h>
#endif

/**
	@post The father has exited. Only the child returns.
	@return Return an error message if a problem occured.
*/
inline boost::optional<std::string> launch_daemon()
{
#ifdef _WIN32
	return std::string("Running as a daemon is not supported on this plateform (Windows)\n");
#else
	const pid_t pid = fork();
	if (pid < 0) 
	{
		return std::string("Could not fork and run as a daemon\n");
	}
	// For the father processus.
	else if(pid > 0)
	{
		exit(0);
	}

	setsid();
	return boost::optional<std::string>();
#endif
}

#endif // UMCD_DAEMON_HPP
