/*
Copyright (C) 2017 by the Battle for Wesnoth Project http://www.wesnoth.org/

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY.

See the COPYING file for more details.
*/

#pragma once

#include <string>

namespace preferences {
	std::string login();
	void set_login(const std::string& login);

	std::string password(const std::string& server, const std::string& login);
	void set_password(const std::string& server, const std::string& login, const std::string& key);

	bool remember_password();
	void set_remember_password(bool remember);

	void load_credentials();
	void save_credentials();
}
