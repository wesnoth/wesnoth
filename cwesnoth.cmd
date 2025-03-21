@echo off
rem
rem Runs Wesnoth with a Windows console attached.
rem
rem Usage:
rem   cwesnoth <command line>
rem

wesnoth --no-log-to-file %*
pause
