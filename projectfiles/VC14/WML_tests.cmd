:: WML_tests.cmd : Run the WML unit tests specified in wml_test_schedule

:: preamble: don't spam stdout with commands, don't expand ERRORLEVEL to 0
@echo off
setlocal enabledelayedexpansion

if "%SKIP_WMLTESTS%" == "TRUE" exit 0
echo running WML tests:

:: if OMP_WAIT_POLICY isn't set, the process restarts itself and !ERRORLEVEL!=0
set OMP_WAIT_POLICY=PASSIVE

:: Argument 1 (accessed via %~f1%) is the build's OutDir, but it might be
:: empty, in which case use run_wml_tests built-in logic for working out where
:: the binary should be.
cd ..\..\
if "%~f1%" == "" (
	py -3 .\run_wml_tests -c -v
) else (
	py -3 .\run_wml_tests -p "%~f1%" -c -v
)

:: restore the state before execution, but return the result of run_wml_tests
set RESULT=%ERRORCODE%
cd %~p0
echo on
@exit /b RESULT
