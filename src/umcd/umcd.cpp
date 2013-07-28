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

#include <stdexcept>

#include "umcd/boost/thread/workaround.hpp"
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

#include "config.hpp"
#include "game_config.hpp"

#include "umcd/server_options.hpp"
#include "umcd/server/multi_threaded/server_mt.hpp"
#include "umcd/protocol/wml/umcd_protocol.hpp"
#include "umcd/umcd_logger.hpp"

int main(int argc, char *argv[])
{
	try
	{
		server_options options(argc, argv);
		if(!options.print_info())
		{
			boost::thread logger_thread(boost::bind(&umcd_logger::run, boost::ref(umcd_logger::get())));
			config cfg = options.build_config();
			UMCD_LOG(info) << "Configuration requested:\n" << cfg;
			// Many classes are tightly coupled with the game path, and in particular with this global variable, so we need to set it.
			game_config::path = cfg["wesnoth_dir"].str();
			boost::shared_ptr<umcd_protocol> protocol = boost::make_shared<umcd_protocol>(cfg);
			server_mt<umcd_protocol> addon_server(cfg, protocol);
			addon_server.run();
		}
	}
	catch(std::exception &e)
	{
		std::cerr << "[Critical]: " << e.what() << std::endl;
		umcd_logger::get().run_once();
	}
	return 0;
}
