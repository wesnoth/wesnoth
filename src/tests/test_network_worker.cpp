/* $Id: test_util.cpp 23124 2008-01-21 21:56:39Z noyga $ */
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
#include <boost/test/auto_unit_test.hpp>
#include <string>
#include "network.hpp"

//! Initial implementation. I will wokr more on this shortly
//! This sohuld be complit test for networking that there won't
//! be any crashes even in rarely used code paths.
//! It is like that this will need some threading also :(

const int TEST_PORT = 15010;
const int MIN_THREADS = 1;
const int MAX_THREADS = 0;
const std::string LOCALHOST = "localhost"; 

network::manager* manager;
network::server_manager* server;

BOOST_AUTO_TEST_CASE( test_network_connect )
{
	int connections = network::nconnections();
	network::connection client_client;
	network::connection server_client;
	BOOST_WARN_MESSAGE(connections == 0, "There is open conenctions before test!");
	manager = new network::manager(MIN_THREADS,MAX_THREADS);

	server = new network::server_manager(TEST_PORT,network::server_manager::MUST_CREATE_SERVER);
	BOOST_REQUIRE_MESSAGE(server->is_running(), "Can't start server!");

	client_client = network::connect(LOCALHOST, TEST_PORT);

	BOOST_CHECK_MESSAGE(client_client > 0, "Can't connect to server");

	server_client = network::accept_connection();

	BOOST_CHECK_MESSAGE(server_client > 0, "Can't accept connection");

	


}

BOOST_AUTO_TEST_CASE( test_network_shutdown )
{
	delete server;
	delete manager;
}

/* vim: set ts=4 sw=4: */

