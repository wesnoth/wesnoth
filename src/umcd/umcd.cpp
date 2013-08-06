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
#include <boost/function.hpp>

#include "config.hpp"
#include "game_config.hpp"

#include "umcd/server_options.hpp"
#include "umcd/server/multi_threaded/server_mt.hpp"
#include "umcd/protocol/wml/umcd_protocol.hpp"
#include "umcd/umcd_logger.hpp"
#include "umcd/daemon.hpp"

void init_game_path(const server_options& opt, const config& cfg)
{
	boost::optional<std::string> wesdir = opt.wesnoth_dir(cfg);
	if(wesdir)
	{
		// Many classes are tightly coupled with the game path, and in particular with this global variable, so we need to set it.
		game_config::path = cfg.child("server_core")["wesnoth_dir"].str();
	}
	else
	{
		throw std::runtime_error("The field wesnoth_dir is missing in the configuration file.");
	}
}

int main(int argc, char *argv[])
{
	try
	{
		server_options options(argc, argv);
		if(!options.is_info())
		{
			config cfg = options.read_config();
			init_game_path(options, cfg);
			options.validate(cfg);
			umcd_logger::get().set_config(cfg.child("logging"));

			if(options.is_daemon())
			{
				boost::optional<std::string> err = launch_daemon();
				if(err)
				{
					UMCD_LOG(error) << *err;
					UMCD_LOG(warning) << "The server has been launched in frontend mode.";
				}
			}
			boost::thread logger_thread(boost::bind(&umcd_logger::run, boost::ref(umcd_logger::get())));

			UMCD_LOG(info) << "Configuration requested:\n" << cfg;

			server_info serverinfo(cfg);
			typedef boost::function<boost::shared_ptr<umcd_protocol> (umcd_protocol::io_service_type&)> umcd_protocol_factory;
			server_mt<umcd_protocol, umcd_protocol_factory> addon_server(
				cfg.child("server_core"),
				boost::bind(&make_umcd_protocol, _1, boost::cref(serverinfo))
			);
			addon_server.run();
		}
	}
	catch(const twml_exception& e)
	{
		UMCD_LOG(fatal) << " (user message=" << e.user_message << " ; dev message=" << e.dev_message << ")";
	}
	catch(std::exception &e)
	{
		UMCD_LOG(fatal) << e.what();
	}
	umcd_logger::get().run_once();
	return 0;
}
