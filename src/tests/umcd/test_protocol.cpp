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
#include "serialization/one_hierarchy_validator.hpp"
#include "umcd/protocol/wml/umcd_protocol.hpp"

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
};

void test_stream_state(const tcp::iostream& stream)
{
  if(!stream.good())
  {
    BOOST_FAIL(stream.error().message());
  }
}

std::string zero_padding(const std::string& value, std::size_t size)
{
  std::string res;
  int missing_zero = size - value.size();
  for(int i=0; i < missing_zero; ++i)
    res.push_back('0');
  return res + value;
}

void test_exchange(const std::string& request_path, const std::string& reply_validator_path)
{
  std::ifstream request_file(request_path.c_str());
  config request_conf;
  read(request_conf, request_file);

  tcp::iostream stream(umcd_stream::host(), umcd_stream::port());
  test_stream_state(stream);
  std::string request_conf_string = request_conf.to_string();
  std::string request_size = boost::lexical_cast<std::string>(request_conf_string.size());
  BOOST_TEST_MESSAGE("max digits = " << umcd_protocol::MAX_NUMBER_OF_DIGITS << " | " <<  zero_padding(request_size, umcd_protocol::MAX_NUMBER_OF_DIGITS));
  stream << zero_padding(request_size, umcd_protocol::MAX_NUMBER_OF_DIGITS);
  stream << request_conf_string;
  test_stream_state(stream);
  config response;
  boost::shared_ptr<schema_validation::schema_validator> validator(new schema_validation::schema_validator(reply_validator_path));
  // Should not throw! we don't use BOOST_CHECK_NO_THROW because it doesn't print the message.
  ::read(response, stream, validator.get());
  test_stream_state(stream);
}

BOOST_AUTO_TEST_SUITE(umcd_common_test_suite)

BOOST_AUTO_TEST_CASE(umcd_bad_request_name)
{
  const std::string request_path = "../data/umcd/tests/common/bad_request_name.cfg";
  const std::string error_reply_cfg_path = "../data/umcd/protocol_schema/error_reply.cfg";
  test_exchange(request_path, error_reply_cfg_path);
}

BOOST_AUTO_TEST_SUITE_END() // umcd_common_test_suite

BOOST_AUTO_TEST_SUITE(umcd_request_license_test_suite)

BOOST_AUTO_TEST_CASE(umcd_request_license_empty_lang)
{
  const std::string request_path = "../data/umcd/tests/request_license/request_license_empty_lang.cfg";
  const std::string license_path = "../data/umcd/protocol_schema/request_license_reply.cfg";
  test_exchange(request_path, license_path);
}

BOOST_AUTO_TEST_CASE(umcd_request_license_en_GB)
{
  const std::string request_path = "../data/umcd/tests/request_license/request_license_english.cfg";
  const std::string license_path = "../data/umcd/protocol_schema/request_license_reply.cfg";
  test_exchange(request_path, license_path);
}

BOOST_AUTO_TEST_SUITE_END() // umcd_request_license_test_suite

BOOST_AUTO_TEST_SUITE(umcd_metaprogramming_tools_suite)

BOOST_AUTO_TEST_CASE(umcd_num_digits)
{
  BOOST_CHECK_EQUAL(num_digits<0>::value, 1);
  BOOST_CHECK_EQUAL(num_digits<9>::value, 1);
  BOOST_CHECK_EQUAL(num_digits<10>::value, 2);
  BOOST_CHECK_EQUAL(num_digits<11>::value, 2);
  BOOST_CHECK_EQUAL(num_digits<100>::value, 3);
  BOOST_CHECK_EQUAL(num_digits<1000>::value, 4);
  BOOST_CHECK_EQUAL(num_digits<99999>::value, 5);
}

BOOST_AUTO_TEST_SUITE_END() // umcd_request_license_test_suite