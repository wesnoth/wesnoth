/*
   Copyright (C) 2013 - 2015 by L.Sebilleau <l.sebilleau@free.fr>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/*

        Yet Another Map Generator. Class YAMG_Params

    This class groups generation parameters.
    It has mainly a constructor to initialize it and a checking method.

*/

#ifndef YAMG_PARAMS_HPP
#define YAMG_PARAMS_HPP

#include "yamg_decls.hpp"


class yamg_params
{
public:

	//	yamg_params(config& cfg);
	yamg_params();
	~yamg_params();

#ifdef YAMG_STANDALONE
    char path[2048];        ///< dest filepath
#endif
    unsigned int seed;      ///< random seed
    unsigned int width;      ///< map width
    unsigned int height;      ///< map height
    unsigned int rough;     ///< map roughness
    char type;              ///< landscape type
    char season;            ///< season to use
    unsigned int snowlev;   ///< snow level
    unsigned int water_ratio;    ///< water ratio
    int alt_mid;             ///< map altitude center
    int alt_nw;              ///< map altitude NW
    int alt_ne;              ///< map altitude NE
    int alt_se;              ///< map altitude SE
    int alt_sw;              ///< map altitude SW

    unsigned int vill;      ///< number of isolated houses
    unsigned int burgs;     ///< number of villages
    //unsigned int towns;     ///< number of towns
    unsigned int forests;   ///< forests rate of terrain
    unsigned int players;   ///< number of players
    unsigned int casthexes; ///< number of hex in castles
    int bridges;            ///< bridge creation cost
    bool roads;             ///< roads generation
    unsigned int ro_road;    ///< tune the paths windiness

    int thickness[M_NUMLEVEL];       ///< layers thickness
    const char *base_cust[M_NUMLEVEL];   ///< custom terrains
    const char *forest_cust[12];
    const char *houses_cust[14];
    const char *keeps_castles_cust[10];
    const char *hexes_castles_cust[10];
    const char *base_snow_cust[M_NUMLEVEL];
    const char *roads_cust[4];
    char *lilies_cust;
    char *fields_cust;
    char *bridges_cust;


    unsigned int verify();  ///< verify parameters
#ifdef YAMG_STANDALONE
    unsigned int read_params(const char *ficnom);    ///< read the params file
#endif

    protected:
#ifdef YAMG_STANDALONE
    void store_terrain_codes(const char *input, const char **table, unsigned int cnt); ///< read the terrain code parameters
#endif

    private:
};

bool find_in_string(const char *str, char c);

#endif // YAMG_PARAMS_HPP
