@echo off
rem
rem Runs Wesnoth with a Windows console attached.
rem
rem Usage:
rem   cwesnoth <command line>
rem

set SDL_STDIO_REDIRECT=0
set OMP_WAIT_POLICY=PASSIVE
@wesnoth --wconsole %*

rem Drop this from environment for future runs on the same session.
set SDL_STDIO_REDIRECT=
