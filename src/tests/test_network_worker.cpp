/*
   Copyright (C) 2008 - 2014 by Pauli Nieminen <paniemin@cc.hut.fi>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-test"

#include <boost/test/unit_test.hpp>


#include "utils/auto_parameterized.hpp"
#include "utils/predicate.hpp"

#include "network_worker.hpp"
#include "thread.hpp"
#include "filesystem.hpp"

#include "config.hpp"
#include "game_config.hpp"

/**
 * Test networking to prevent bugs there.
 * Try to create all kind of unlikely error conditions
 * Some test should lock management_mutex from worker to settup stress test
 * It is like that this will need some threading also :(
 */

BOOST_AUTO_TEST_SUITE( test_network )

const int TEST_PORT = 15010;
const int MIN_THREADS = 1;
const int MAX_THREADS = 5;
const std::string LOCALHOST = "localhost";


network::manager* wes_manager;
network::server_manager* wes_server;

network::connection client_client1;
network::connection client_client2;

network::connection server_client1;
network::connection server_client2;


BOOST_AUTO_TEST_CASE( test_connect )
{
	int connections = network::nconnections();

	BOOST_WARN_MESSAGE(connections == 0, "There is open "<< connections <<" connections before test!");
	BOOST_CHECK_MESSAGE(wes_manager = new network::manager(MIN_THREADS,MAX_THREADS), "network::manager failed to initialize");

	BOOST_CHECK_MESSAGE(wes_server = new network::server_manager(TEST_PORT,network::server_manager::MUST_CREATE_SERVER),
			"network::server_manager failed to initialize");
	BOOST_REQUIRE_MESSAGE(wes_server->is_running(), "Can't start server!");

	client_client1 = network::connect(LOCALHOST, TEST_PORT);

	BOOST_CHECK_MESSAGE(client_client1 > 0, "Can't connect to server");

	server_client1 = network::accept_connection();

	BOOST_CHECK_MESSAGE(server_client1 > 0, "Can't accept connection");


}

template<class T>
static network::connection receive(T& cfg, int max_tries = 100)
{
	network::connection receive_con;
	while ((receive_con = network::receive_data(cfg)) == network::null_connection)
	{
		// loop untill data is received
		SDL_Delay(50);
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
	cfg_send["test_running"] = true;
	network::send_data(cfg_send, client_client1);

	network::connection receive_from;
	config received;

	receive_from = receive(received);

	BOOST_CHECK_MESSAGE( receive_from == server_client1, "Received data is not from test client 1" );

	BOOST_CHECK_EQUAL(cfg_send, received);

}

void try_send_random_seed ( const std::string seed_str, const unsigned int random_calls)
{
	config cfg_send;
	config& child = cfg_send.add_child("command");

	child["random_seed"] = seed_str;
	child["random_calls"] = random_calls;

	network::send_data(cfg_send, client_client1);

	network::connection receive_from;
	config received;

	receive_from = receive(received);

	BOOST_CHECK_MESSAGE( receive_from == server_client1, "Received data is not from test client 1" );

	BOOST_CHECK_EQUAL(cfg_send, received);

	config rec_command = received.child("command");

	std::string rec_seed_str = rec_command["random_seed"].str();
	unsigned int rec_calls = rec_command["random_calls"];

	BOOST_CHECK_EQUAL(seed_str, rec_seed_str);
	BOOST_CHECK_EQUAL(random_calls, rec_calls);
}

BOOST_AUTO_TEST_CASE( test_send_random_seed )
{
	try_send_random_seed("0000badd",0);
	try_send_random_seed("00001234",1);
	try_send_random_seed("deadbeef",2);
	try_send_random_seed("12345678",3);
	try_send_random_seed("00009999",4);
	try_send_random_seed("ffffaaaa",5);
	try_send_random_seed("11110000",6);
	try_send_random_seed("10101010",7);
	try_send_random_seed("aaaa0000",8);
}

class connect_aborter : public threading::waiter
{
public:
	connect_aborter() : start_(SDL_GetTicks())
	{}
	ACTION process();

private:
	size_t start_;
};

connect_aborter::ACTION connect_aborter::process()
{
	// Abort connection after 5 ms
	if(SDL_GetTicks() - start_ >= 5) {
		return ABORT;
	} else {
		return WAIT;
	}
}

#if 0
BOOST_AUTO_TEST_CASE( test_sdl_thread_wait_crash )
{

	delete wes_server;
	wes_server = 0;
	delete wes_manager;
	wes_manager = 0;
	BOOST_CHECK_MESSAGE(wes_manager == 0, "network::manager nono zero after delete");

	BOOST_CHECK_MESSAGE(wes_manager = new network::manager(MIN_THREADS,MAX_THREADS), "network::manager failed to initialize");
	BOOST_CHECK_THROW(client_client1 = network::connect(LOCALHOST, TEST_PORT), network::error);
	BOOST_CHECK_MESSAGE(client_client1 > 0, "Can't connect to server");
	delete wes_manager;
	wes_manager = 0;
	BOOST_CHECK_MESSAGE(wes_manager = new network::manager(MIN_THREADS,MAX_THREADS), "network::manager failed to initialize");

	connect_aborter aborter;
	BOOST_CHECK_MESSAGE((client_client1 = network::connect("server.wesnoth.org", 15000, aborter)) == 0, "Connection create but not shoul");
	delete wes_manager;

	BOOST_CHECK_MESSAGE(wes_manager = new network::manager(MIN_THREADS,MAX_THREADS), "network::manager failed to initialize");
	BOOST_CHECK_MESSAGE(wes_server = new network::server_manager(TEST_PORT,network::server_manager::MUST_CREATE_SERVER), "");
	BOOST_CHECK_MESSAGE((client_client1 = network::connect(LOCALHOST, TEST_PORT, aborter)) == 0, "Connection create but not shoul");
	delete wes_manager;

	wes_manager = new network::manager(MIN_THREADS,MAX_THREADS);
	client_client1 = network::connect(LOCALHOST, TEST_PORT);
	BOOST_CHECK_MESSAGE(client_client1 > 0, "Can't connect to server");
	delete wes_server;
	delete wes_manager;
	wes_manager = new network::manager(MIN_THREADS,MAX_THREADS);
	wes_server = new network::server_manager(TEST_PORT,network::server_manager::MUST_CREATE_SERVER);
}
#endif

// Use 1kb, 500kb and 10Mb files for testing
struct sendfile_param {
	sendfile_param(size_t size, bool system) : size_(size), system_(system) {}
	size_t size_;
	bool system_;
};

sendfile_param sendfile_sizes[] = {sendfile_param(1*1024,true),
								   sendfile_param(5*1024*1024,true),
								   sendfile_param(1*1024,false),
								   sendfile_param(5*1024*1024,false)};

static std::string create_random_sendfile(size_t size)
{
	char buffer[1024];
	const int buffer_size = sizeof(buffer)/sizeof(buffer[0]);
	int *begin = reinterpret_cast<int*>(&buffer[0]);
	int *end = begin + sizeof(buffer)/sizeof(int);
	std::string filename = "sendfile.tmp";
	filesystem::scoped_ostream file = filesystem::ostream_file(filename);
	std::generate(begin,end,std::rand);
	while( size > 0
		&& !file->bad())
	{
		file->write(buffer, buffer_size);
		size -= buffer_size;
	}
	return filename;
}

static void delete_random_sendfile(const std::string& file)
{
	filesystem::delete_file(file);
}

template<class T>
class auto_resetter {
	T& value_to_change_;
	T old_val_;
	public:
	auto_resetter(const T& new_value, T& value_to_change) : value_to_change_(value_to_change), old_val_(value_to_change)
	{
		value_to_change_ = new_value;
	}
	~auto_resetter()
	{
		value_to_change_ = old_val_;
	}
};

WESNOTH_PARAMETERIZED_TEST_CASE( test_multi_sendfile, sendfile_param, sendfile_sizes, size )
{
	auto_resetter<std::string> path("", game_config::path);
	network::set_raw_data_only();
	std::string file = create_random_sendfile(size.size_);
	network_worker_pool::set_use_system_sendfile(size.system_);

	network::connection cl_client1, se_client1;
	network::connection cl_client2, se_client2;
	network::connection cl_client3, se_client3;

	BOOST_CHECK_MESSAGE((cl_client1 = network::connect(LOCALHOST, TEST_PORT)) > 0, "Can't connect to server!");
	BOOST_CHECK_MESSAGE((se_client1 = network::accept_connection()) > 0, "Coulnd't accept new connection");
	BOOST_CHECK_MESSAGE((cl_client2 = network::connect(LOCALHOST, TEST_PORT)) > 0, "Can't connect to server!");
	BOOST_CHECK_MESSAGE((se_client2 = network::accept_connection()) > 0, "Coulnd't accept new connection");
	BOOST_CHECK_MESSAGE((cl_client3 = network::connect(LOCALHOST, TEST_PORT)) > 0, "Can't connect to server!");
	BOOST_CHECK_MESSAGE((se_client3 = network::accept_connection()) > 0, "Coulnd't accept new connection");

	network::send_file(file, cl_client1);
	network::send_file(file, cl_client2);
	network::send_file(file, cl_client3);

	std::vector<char> data;

	BOOST_CHECK_PREDICATE(test_utils::one_of<network::connection> , (receive(data,500))(3)(se_client1)(se_client2)(se_client3));
	BOOST_CHECK_EQUAL(data.size(), static_cast<size_t>(filesystem::file_size(file)));
	BOOST_CHECK_PREDICATE(test_utils::one_of<network::connection> , (receive(data,500))(3)(se_client1)(se_client2)(se_client3));
	BOOST_CHECK_EQUAL(data.size(), static_cast<size_t>(filesystem::file_size(file)));
	BOOST_CHECK_PREDICATE(test_utils::one_of<network::connection> , (receive(data,500))(3)(se_client1)(se_client2)(se_client3));

	BOOST_CHECK_EQUAL(data.size(), static_cast<size_t>(filesystem::file_size(file)));

	network::disconnect(cl_client1);
	network::disconnect(cl_client2);
	network::disconnect(cl_client3);

	BOOST_CHECK_THROW(receive(data),network::error);
	BOOST_CHECK_THROW(receive(data),network::error);
	BOOST_CHECK_THROW(receive(data),network::error);

	delete_random_sendfile(file);
}

#if 0
BOOST_AUTO_TEST_CASE( test_multiple_connections )
{
}

BOOST_AUTO_TEST_CASE( test_cancel_transfer )
{
}

BOOST_AUTO_TEST_CASE( test_detect_errors )
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
#endif
BOOST_AUTO_TEST_CASE( test_shutdown )
{
	delete wes_server;
	BOOST_CHECK_MESSAGE(true,"Not true");
	delete wes_manager;
}

BOOST_AUTO_TEST_SUITE_END()
/* vim: set ts=4 sw=4: */
