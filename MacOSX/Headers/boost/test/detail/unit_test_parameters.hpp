//  (C) Copyright Gennadiy Rozental 2001-2008.
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at 
//  http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org/libs/test for the library home page.
//
//  File        : $RCSfile$
//
//  Version     : $Revision: 49312 $
//
//  Description : storage for unit test framework parameters information
// ***************************************************************************

#ifndef BOOST_TEST_UNIT_TEST_PARAMETERS_HPP_071894GER
#define BOOST_TEST_UNIT_TEST_PARAMETERS_HPP_071894GER

#include <boost/test/detail/global_typedef.hpp>
#include <boost/test/detail/log_level.hpp>

#include <boost/test/detail/suppress_warnings.hpp>

//____________________________________________________________________________//

namespace boost {

namespace unit_test {

// ************************************************************************** //
// **************                 runtime_config               ************** //
// ************************************************************************** //

namespace runtime_config {

void                    BOOST_TEST_DECL init( int* argc, char** argv );

unit_test::log_level    BOOST_TEST_DECL log_level();
bool                    BOOST_TEST_DECL no_result_code();
unit_test::report_level BOOST_TEST_DECL report_level();
const_string            BOOST_TEST_DECL test_to_run();
const_string            BOOST_TEST_DECL break_exec_path();
bool                    BOOST_TEST_DECL save_pattern();
bool                    BOOST_TEST_DECL show_build_info();
bool                    BOOST_TEST_DECL show_progress();
bool                    BOOST_TEST_DECL catch_sys_errors();
bool                    BOOST_TEST_DECL auto_start_dbg();
bool                    BOOST_TEST_DECL use_alt_stack();
bool                    BOOST_TEST_DECL detect_fp_exceptions();
output_format           BOOST_TEST_DECL report_format();
output_format           BOOST_TEST_DECL log_format();
long                    BOOST_TEST_DECL detect_memory_leaks();
int                     BOOST_TEST_DECL random_seed();

} // namespace runtime_config

} // namespace unit_test

} // namespace boost

//____________________________________________________________________________//

#include <boost/test/detail/enable_warnings.hpp>

#endif // BOOST_TEST_UNIT_TEST_PARAMETERS_HPP_071894GER
