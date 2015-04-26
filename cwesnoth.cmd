@echo off
rem
rem Runs Wesnoth with a Windows console attached.
rem
rem Usage:
rem   cwesnoth <command line>
rem

rem Disable stdout.txt/stderr.txt redirection in SDLmain.
set SDL_STDIO_REDIRECT=0

rem OpenMP builds need to set this variable on startup and relaunch Wesnoth in
rem order to prevent constant busy waits. Do it here so the restart doesn't
rem cause this script to return to the shell too early.
set OMP_WAIT_POLICY=PASSIVE

wesnoth --wconsole %*

rem Drop our custom environment for future runs on the same session.
set SDL_STDIO_REDIRECT=
set OMP_WAIT_POLICY=
