#ifndef FOREACH_HPP
#define FOREACH_HPP

#include <boost/foreach.hpp>

#ifdef __CDT_PARSER__
     #define foreach(a, b) for(a : b)
#else
     #define foreach BOOST_FOREACH
#endif

#endif
