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
#define BOOST_TEST_MODULE umcd_test

#include <fstream>

#include <boost/test/unit_test.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

#include "serialization/parser.hpp"
#include "umcd/wml_request.hpp"
#include "umcd/wml_reply.hpp"

using namespace boost::unit_test;
using boost::asio::ip::tcp;

struct umcd_stream
{
  static tcp::iostream tcp_iostream;

  static const std::string& port()
  {
    static const std::string p = "12523";
    return p;
  }

  static const std::string& host()
  {
    static const std::string h = "localhost";
    return h;
  }

  static tcp::iostream& stream()
  {
    static tcp::iostream tcp_iostream(host(), port());
    return tcp_iostream;
  }
};

BOOST_AUTO_TEST_SUITE(umcd_common_test_suite)

BOOST_AUTO_TEST_CASE(umcd_bad_request_name)
{
  const std::string filename = "../data/umcd/tests/common/bad_request_name.cfg";
  const std::string error_reply_cfg_path = "../data/umcd/protocol_schema/error_reply.cfg";

  std::ifstream request_file(filename.c_str());
  config request_conf;
  read(request_conf, request_file);
  wml_reply reply = make_reply(request_conf);
  reply.send(umcd_stream::stream());
  wml_request request = make_request(umcd_stream::stream(), error_reply_cfg_path);
  BOOST_REQUIRE(true);
}

BOOST_AUTO_TEST_SUITE_END() // umcd_common_test_suite