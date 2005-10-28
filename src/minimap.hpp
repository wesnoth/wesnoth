#ifndef MINIMAP_HPP_INCLUDED
#define MINIMAP_HPP_INCLUDED

#include "map.hpp"
#include "sdl_utils.hpp"


class team;

namespace image {
	///function to create the minimap for a given map
	///the surface returned must be freed by the user
	surface getMinimap(int w, int h, const gamemap& map_, const team* tm=NULL);
}

#endif
