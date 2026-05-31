/*
	Copyright (C) 2011 - 2025
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

#ifdef __EMSCRIPTEN__

#include <cstdint>

/**
 * @file
 * Shared WebSocket bridge for Emscripten builds.
 *
 * Provides EM_JS helper functions used by both network_asio_emscripten.cpp
 * (addons/campaignd) and wesnothd_connection_emscripten.cpp (multiplayer).
 *
 * All functions manage WebSocket connections keyed by integer ID on
 * Module._wsConns in JavaScript.
 */

extern "C" {

/** Returns the proxy URL from Module.wsProxyUrl (caller must free()). */
char* wa_get_ws_proxy_url();

/** Opens a WebSocket to the given URL. Returns connection ID > 0. */
int wa_ws_connect(const char* url_ptr);

/** Returns connection state: 0=connecting, 1=open, 2=closed, 3=error, -1=invalid. */
int wa_ws_state(int id);

/** Sends binary data. Returns 0 on success, -1 on error. */
int wa_ws_send(int id, const uint8_t* data, int len);

/** Returns number of bytes available to read. */
int wa_ws_recv_available(int id);

/** Copies up to maxLen bytes from JS recv buffer to C++ heap. Returns bytes copied. */
int wa_ws_recv(int id, uint8_t* buf, int maxLen);

/** Closes the WebSocket and removes the connection record. */
void wa_ws_close(int id);

} // extern "C"

#endif // __EMSCRIPTEN__
