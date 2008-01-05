/* $Id$ */
/*
   Copyright (C) 2003 - 2007 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef DISABLE_4786_HPP_INCLUDED
#define DISABLE_4786_HPP_INCLUDED

//for windows compilers
#ifdef __MSVCRT__
#ifndef __GNUC__
	#undef snprintf
	#define snprintf _snprintf
#endif
#endif

#ifdef _MSC_VER

#undef snprintf
#define snprintf _snprintf

//disable the warning to let us know about 'this' being used in
//initializer list, since it's a common thing to want to do
//for callbacks, and there is no other way to easily workaround the warning
#pragma warning(disable:4355)

//disable the warnings for long template names
#pragma warning(disable:4503)
#pragma warning(disable:4786)

//the following code causes the incredibly irritating warning 4786 to really
//be muted in Visual C++ 6. No-one seems to know *why* it works (possibly not even Microsoft)
//but it does. So don't ask, and just leave it there.

class warning4786WorkAround {
public:
warning4786WorkAround() {}
};

static warning4786WorkAround VariableThatHacksWarning4786IntoBeingMutedForSomeUnknownReason;

//put the mathematical functions where they belong: in the std namespace
//it is necessary for VC6 at least
#include <cmath>
namespace std {
  using ::floor;
  using ::sqrt;
  using ::ceil;
  using ::fmod;
  using ::pow;
}

//put the FILE where it belongs: in the std namespace
#include <cstdio>
namespace std {
  using ::FILE;
  using ::fclose;
}

//put the Locale functions where they belong: in the std namespace
#include <clocale>
namespace std {
 using ::setlocale;
}

//put the string functions where they belong: in the std namespace
#include <cstring>
namespace std {
  using ::strrchr;
}

#endif
#endif
