/*
   Copyright (C) 2014 by Sebastian Koelle <sk.aquileia@gmail.com>
   Part of the Battle for Wesnoth Project https://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file
 * Execute another executable:
 * - wait for the process to quit, then pass through the error code
 * - terminate it after a timeout of "t" microseconds, then return 2
 */

#if _WIN32_WINNT < 0x0602 //Windows version before Win8
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
#else
	#include <Processthreadsapi.h>
#endif
#include <stdlib.h>
#include <iostream>

int main(int argc, char* argv[]) {

	if (argc != 3) {
		std::cout << "WindowsTimeout: Error USAGE1: " << argv[0] << " \"command [--options]\" t";
		return -1;
	}

	PROCESS_INFORMATION pi;
	STARTUPINFO info = { sizeof(info) };
	HANDLE hStdErr = GetStdHandle(STD_ERROR_HANDLE);
	info.hStdError = hStdErr;

	if (!CreateProcess(NULL, argv[1], NULL, NULL, TRUE, 0, NULL, NULL, &info, &pi))
		return -1;

	DWORD dwExitCode;
	if (::WaitForSingleObject(pi.hProcess, atoi(argv[2])) == WAIT_OBJECT_0) {
		GetExitCodeProcess(pi.hProcess, &dwExitCode);
	}
	else {
		TerminateProcess(pi.hProcess, 2);
		dwExitCode = 2;
	}
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	CloseHandle(hStdErr);
	return dwExitCode;
}
