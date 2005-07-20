/* $Id$ */
/*
   Copyright (C) 2003-5 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "wesconfig.h"

#ifdef USE_ZIPIOS

#include <vector>
#include "xcoll.hpp"
#include "wassert.hpp"
#include <zipios++/fileentry.h>

bool xzipios::XCColl::hasSubdir(const std::string path) const {
	zipios::ConstEntries e = entries();
	for (zipios::ConstEntries::iterator i = e.begin(); i != e.end(); ++i) {
		std::string name = (**i).getName();
		//std::cerr << " considering " << name << "\n";
		if ((0 == name.compare(0, path.size(), path)) &&
		    ((path[path.size()-1] == '/') || (name[path.size()] == '/')))
			return true;
	}
	return false;
}

void xzipios::XCColl::childrenOf(std::string path,
				 std::vector<std::string>* files,
				 std::vector<std::string>* dirs) const {
	// be sure the dirname ends with a slash
	if (path[path.size()-1] != '/') path += '/';

	zipios::ConstEntries e = entries();
	for (zipios::ConstEntries::iterator i = e.begin(); i != e.end(); ++i) {
		std::string name = (**i).getName();
		//std::cerr << " considering " << name << "\n";
		if (0 == name.compare(0, path.size(), path)) {
			size_t pos = 0; // cannot be returned by find
			if ((files!=NULL) &&
			    ((pos = name.find('/', path.size())) == std::string::npos) &&
			    (name != path)) {
				files->push_back(name);
			} else if ((dirs!=NULL) &&
				   ((pos && (pos!=std::string::npos)) ? pos : (name.find('/', path.size()) != std::string::npos))) {
				wassert(pos!=0);
				std::string s = name.substr(0, pos);
				// FIXME: find more efficient ? (eg. look at last entry added first)
				std::vector<std::string>::iterator j;
				for (j = dirs->begin(); j != dirs->end(); ++j)
					if (*j == s) break;
				if (j == dirs->end())
					dirs->push_back(s);
			}
		}
	}
}

#endif // defined USE_ZIPIOS
