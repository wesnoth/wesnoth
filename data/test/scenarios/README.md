# Test scenarios

This directory contains both the scenarios used by C++ unit tests and those which are
WML unit tests.

## C++ unit tests

For the C++ unit tests, it is recommended to reuse the same scenario file as much as possible
and just inject WML into it.

Injection can be done by adding a config object containing event code and then registering that
manually for `game_events`.

## Manual tests

The `manual_tests` subdirectory contains scenarios that expect to be run interactively, either by
binding a hotkey for the main menu's "Choose Test Scenario" option, or with the command line
argument `-t <testname>`.

Many of these are closer to workbenches than tests, allowing developers to do some action that isn't
automated, and then to find out whether the result matched the expectation.

## Automated WML unit tests

WML unit tests are self-contained scenario files to test a specific area of WML.

The test result is a status code from the `unit_test_result` enum found in `game_launcher.hpp`, or
in rare cases tests expect to be timed out by the test runner. They can be run individually with
Wesnoth's `-u <testname>` command line argument, but are usually run by the `run_wml_tests script`
based on the list in `wml_test_schedule`.

They are unlikely to return the same status if run with `-t <testname>`. Running them with `-t` can
still be helpful for debugging.

The tests aren't meant to prevent intentional changes to behavior; they're for checking that
bugfixes or unrelated changes don't accidentally change the behavior. They make intentional changes
easier to notice and review, and you should pay attention to any comments in tests that you change,
but the mere existence of a test shouldn't be taken as saying that the current behavior is good.

### Guidelines for writing automated new tests

Tests are generally implemented with the `GENERIC_UNIT_TEST` macro, with two leaders called Alice
and Bob on separate keeps. If your test needs them to be adjacent to each other, consider using
`COMMON_KEEP_A_B_UNIT_TEST` instead, which puts their starting locations next to each other instead
of needing to move them.

Most will expect the result `PASS`, and new tests should generally be written to result in a `PASS`.
The testing mechanism supports other expectations too, however the optimisation to run a batch of
tests in a single instance of Wesnoth currently only supports batching for tests that return `PASS`.

Tests that shouldn't `PASS` should have a name that makes that expectation obvious. However, the
existing tests don't conform to this goal yet.

Names containing `_fail_` or ending `_fail` are reserved for tests that should not `PASS`. However,
they may expect a status that is neither `PASS` nor `FAIL`.

Names containing `_bug_` or ending `_bug` are reserved for tests where consensus is that the
behavior should change in the development branch, however that it's better to accept the current
behavior in the stable branch than to risk Out Of Sync errors.

The words `break` and `error` aren't reserved, and can be used even for tests expected to `PASS`.
Some of these are testing loops, or testing error-handling that is expected to handle the error.
