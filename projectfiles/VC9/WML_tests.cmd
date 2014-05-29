:: WML_tests.cmd : Run the WML unit tests specified in wml_test_schedule

:: preamble: don't spam stdout with commands, don't expand ERRORLEVEL to 0
@echo off
setlocal enabledelayedexpansion

set LoadFile=wml_test_schedule
set binary=wesnoth.exe
set opt=--log-strict=warning

cd ..\..\

:: ignore lines beginning with #
:: %%G contains whether the test should pass (0), timeout (2) or fail (1,4)
:: %%H is the name of the WML unit test to be executed
for /f "eol=# tokens=1,2 delims= " %%G in (%LoadFile%) do (
    :: WindowsTimeout is more reliable than the --timeout option
    WindowsTimeout.exe "%binary% %opt% -u %%H" 20000
    if !ERRORLEVEL! neq %%G (
        echo WML_tests.cmd: Error WML1: Test '%%H' returned !ERRORLEVEL!, expected %%G
    )
)

:: restore the state before execution
cd %~p0
echo on
