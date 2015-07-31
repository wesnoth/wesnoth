This directory contains both the scenarios used by C++ unit tests and those which are
WML unit tests.

For the C++ unit tests, it is recommended to reuse the same scenario file as much as possible
and just inject WML into it.
Injection can be done by adding a config object containing event code and then registering that
manually for game_events.

WML unit tests are self-contained scenario files to test a specific area of WML. They can be
implemented with the GENERIC_UNIT_TEST macro.
