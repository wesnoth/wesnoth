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
#include "config.hpp"

//! Test networking to prevent bugs there.
//! Try to create all kind of unlikely error conditions
//! Some test should lock management_mutex from worker to settup stress test
//! It is like that this will need some threading also :(


const int TEST_PORT = 15010;
const int MIN_THREADS = 1;
const int MAX_THREADS = 0;
const std::string LOCALHOST = "localhost"; 

network::manager* manager;
network::server_manager* server;

network::connection client_client1;
network::connection client_client2;

network::connection server_client1;
network::connection server_client2;

BOOST_AUTO_TEST_SUITE( network );

BOOST_AUTO_TEST_CASE( test_connect )
{
	BOOST_MESSAGE(  "Starting network test!" );
	int connections = network::nconnections();

	BOOST_WARN_MESSAGE(connections == 0, "There is open "<< connections <<" connections before test!");
	::manager = new network::manager(MIN_THREADS,MAX_THREADS);

	::server = new network::server_manager(TEST_PORT,network::server_manager::MUST_CREATE_SERVER);
	BOOST_REQUIRE_MESSAGE(server->is_running(), "Can't start server!");

	client_client1 = network::connect(LOCALHOST, TEST_PORT);

	BOOST_CHECK_MESSAGE(client_client1 > 0, "Can't connect to server");

	server_client1 = network::accept_connection();

	BOOST_CHECK_MESSAGE(server_client1 > 0, "Can't accept connection");


}

network::connection receive(config& cfg, int max_tries = 100)
{
	network::connection receive_con;
	while ((receive_con = network::receive_data(cfg)) == network::null_connection)
	{
		// loop untill data is received
		SDL_Delay(10);
		if (--max_tries <= 0)
		{
			BOOST_WARN_MESSAGE(max_tries > 0,"receiving data took too long. Preventing for ever loop");
			break;
		}
	}
	return receive_con;
}

BOOST_AUTO_TEST_CASE( test_send_client )
{
	config cfg_send;
	config& child = cfg_send.add_child("test_client_send");

	child["test"] = "yes!";
	cfg_send["test_running"] = "yes";
	network::send_data(cfg_send, client_client1, true);

	network::connection receive_from;
	config received;

	receive_from = receive(received);

	BOOST_CHECK_MESSAGE( receive_from == server_client1, "Received data is not from test client 1" );

	BOOST_CHECK_MESSAGE(cfg_send == received, "send is not same as received\n" << cfg_send.debug() << "\n" << received.debug() );

}

BOOST_AUTO_TEST_CASE( test_send_server )
{
}

BOOST_AUTO_TEST_CASE( test_multiple_connections )
{
}

BOOST_AUTO_TEST_CASE( test_cancel_transfer )
{
}

BOOST_AUTO_TEST_CASE( test_detect_errors )
{
}

BOOST_AUTO_TEST_CASE( test_binary_wml )
{
}

BOOST_AUTO_TEST_CASE( test_broken_data )
{
}

BOOST_AUTO_TEST_CASE( test_config_with_macros )
{
}

BOOST_AUTO_TEST_CASE( test_disconnect )
{
	network::disconnect(client_client1);
	network::disconnect(server_client1);
}

BOOST_AUTO_TEST_CASE( test_shutdown )
{
	delete server;
	delete ::manager;
}

BOOST_AUTO_TEST_SUITE_END();
/* vim: set ts=4 sw=4: */

