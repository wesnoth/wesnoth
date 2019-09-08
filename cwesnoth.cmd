@echo off
rem
rem Runs Wesnoth with a Windows console attached.
rem
rem Usage:
rem   cwesnoth <command line>
rem

setlocal

rem Disable stdout.txt/stderr.txt redirection in SDLmain.
set SDL_STDIO_REDIRECT=0

wesnoth --wconsole %*

endlocal
