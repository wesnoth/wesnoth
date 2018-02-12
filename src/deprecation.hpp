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

#include <string>

// Note: When using version (for level 2 or 3 deprecation), specify the first version
// in which the feature could be removed... NOT the version at which it was deprecated.
// For level 1 or 4 deprecation, it's fine to just pass an empty string, as the parameter will not be used.
void deprecated_message(const std::string& elem_name, int level, const class version_info& version, const std::string& detail = "");
