/*
   Copyright (C) 2013 by L.Sebilleau <l.sebilleau@free.fr>
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
    unsigned int larg;      ///< map width
    unsigned int haut;      ///< map height
    unsigned int rough;     ///< map roughness
    char type;              ///< landscape type
    char season;            ///< season to use
    unsigned int snowlev;   ///< snow level
    unsigned int wRatio;    ///< water ratio
    int altMid;             ///< map altitude center
    int altNW;              ///< map altitude NW
    int altNE;              ///< map altitude NE
    int altSE;              ///< map altitude SE
    int altSW;              ///< map altitude SW

    unsigned int vill;      ///< number of isolated houses
    unsigned int burgs;     ///< number of villages
    //unsigned int towns;     ///< number of towns
    unsigned int forests;   ///< forests rate of terrain
    unsigned int players;   ///< number of players
    unsigned int casthexes; ///< number of hex in castles
    int bridges;            ///< bridge creation cost
    bool roads;             ///< roads generation
    unsigned int roRoad;    ///< tune the paths windiness

    int thickness[M_NUMLEVEL];       ///< layers thickness
    const char *baseCust[M_NUMLEVEL];   ///< custom terrains
    const char *forestCust[12];
    const char *housesCust[14];
    const char *keepsCastlesC[10];
    const char *hexesCastlesC[10];
    const char *baseSnowC[M_NUMLEVEL];
    const char *roadsC[4];
    char *liliesC;
    char *fieldsC;
    char *bridgesC;


    unsigned int verify();  ///< verify parameters
#ifdef YAMG_STANDALONE
    unsigned int readParams(const char *ficnom);    ///< read the params file
#endif

    protected:
#ifdef YAMG_STANDALONE
    void storeTerrainCodes(const char *input, const char **table, unsigned int cnt); ///< read the terrain code parameters
#endif

    private:
};

bool findInString(const char *str, char c);

#endif // YAMG_PARAMS_HPP
