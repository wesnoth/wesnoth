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

#ifndef XZIPIOS_XCOLL_H
#define XZIPIOS_XCOLL_H

#include <zipios++/collcoll.h>

#define stubmethod(rtype,name,proto) rtype name
namespace xzipios {

	class XCColl: public zipios::CColl {
	public:
		// explicit XCColl
		bool hasSubdir(const std::string) const ;
		void childrenOf(std::string path,
		                std::vector<std::string>* files,
		                std::vector<std::string>* dirs) const;
	};
}

#endif // XZIPIOS_XCOLL_H
