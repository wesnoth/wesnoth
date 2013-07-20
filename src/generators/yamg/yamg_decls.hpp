/*********************************************************************
        Yet Another Map Generator.

    This contains common definitions and compile flags

    Copyright (C) 2012  L.Sebilleau (Pyrophorus)
    Part of the Battle for Wesnoth Project http://www.wesnoth.org/
        v 0.8 -- 01/03/2012
        v 0.9 -- 27/11/2012
        v 1.0 -- 15/12/2012

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

**********************************************************************/

#ifndef YAMG_DECLS_HPP
#define YAMG_DECLS_HPP

//#include <stdio.h>

// ================ compilation flags ================
//#define YAMG_STANDALONE ///< switches to the standalone version (the only one at this time).
#define INTERN_RAND ///< uses embedded RNG instead of system RNG. This allow to make the result platform independent.

// ================ Common definitions ===============
#ifndef NULL
#define NULL 0L
#endif

//-------------- Values limits ---------------
#define YAMG_MAPMAXSIZE 500
#define YAMG_ROUGHMAX 50
#define YAMG_TYPES "ptmdec" // p(olar) t(empered) m(editerranean) d(esert) e(quatorial) c(ustom)
#define YAMG_SEASONS "seaw" // s(pring) e(aster) a(utumn) w(inter)
#define YAMG_ALTMAX 100
#define YAMG_VILLMAX 100
#define YAMG_BURGMAX 20
#define YAMG_TOWNSMAX 5
#define YAMG_FORESTMAX 100
#define YAMG_PLAYERSMAX 20

//------------- Map config error codes (bits in one word result) TODO: add new parms errors

#define YAMG_LIMITS 1    ///< map config errors
#define YAMG_ROUGHOFF YAMG_LIMITS<<1
#define YAMG_BADTYPE YAMG_ROUGHOFF<<1
#define YAMG_BADSEASON YAMG_BADTYPE<<1
#define YAMG_SNOW YAMG_BADSEASON<<1
#define YAMG_ALTOFF YAMG_SNOW<<1
#define YAMG_VILLOFF YAMG_ALTOFF<<1
#define YAMG_BURGOFF YAMG_VILLOFF<<1
#define YAMG_TOWNSOFF YAMG_BURGOFF<<1
#define YAMG_FORESTOFF YAMG_TOWNSOFF<<1
#define YAMG_PLAYERSOFF YAMG_FORESTOFF<<1

//-------------- Generator status codes ---------------
enum {
    YAMG_ERROR = -1,    ///< unknown errors (should never happen)
    YAMG_EMPTY = 0,     ///< new generator, defaults values
    YAMG_CONFIGURED,    ///< a parameter list has been set
    YAMG_LOADED,        ///< a map has been created
};

//-------------- Result codes ---------------
enum {
    YAMG_OK = 0,    ///< OK
    YAMG_NOCONFIG,  ///< missing config
    YAMG_NOMAP,     ///< No map to return
    YAMG_HAVEMAP,   ///< A map is already created
    YAMG_FILENOTFOUND, ///< no parameter file of this name
};

#define YAMG_HEXLONG 14 ///< fixed length of map elements in the result buffer

//-------------- MAPS defines --------------

#define M_BASE 100000
#define M_VARIATION 10000
#define M_RANGE 10000
#define YAMG_RIVER_ADAPTATION 300 ///< to adapt the evaporation parameter.
#define YAMG_DESERT_RIVER_OFFSET 25  ///< define a sublevel in desert maps to create oasis and sand pits.

#define MAX_HOUSESINBURG  7 ///< max houses in a burg
#define MIN_HOUSESINBURG  4 ///< min houses in a burg
#define MAX_HEXESINCASTLE 8 ///< max hexes in castles (ignored if hexcastle parameter is not 0)
#define MIN_HEXESINCASTLE 3 ///< min hexes in castles (ignored if hexcastle parameter is not 0)

#define YAMG_ALTWEIGHT_SWAMP 1000 ///< these factors tune the altitude influence of road cost. Increasing them decrease altitude influence.
#define YAMG_ALTWEIGHT_MOUNTAINS 500
#define YAMG_ALTWEIGHT_GROUND 1000
#define YAMG_ALTWEIGHT_HILLS 750

/**
    layers defines.
    The hexes are dispatched in eight distinct layers according to their altitude.
    MUST BE AN ENUM !!!
*/
#define M_NUMLEVEL 8
enum {
    YAMG_DEEPSEA = 0,
    YAMG_SHALLSEA,
    YAMG_BEACH,
    YAMG_SWAMPS,
    YAMG_GROUND,
    YAMG_HILLS,
    YAMG_MOUNTAINS,
    YAMG_IMPMOUNTS,
};

/**
    multi state locks defines
    Hexes may have a four states lock, so they can be protected from overloading by some operations only.
    Typical case is water hexes where no building can be set, but accept roads (bridges).
    MUST BE AN ENUM !!!
*/
enum {
    YAMG_UNLOCK = 0, ///< no lock
    YAMG_LIGHTLOCK,  ///< light lock
    YAMG_STRONGLOCK, ///< strong lock
    YAMG_HARDLOCK,   ///< absolute lock
};

/**
    defs for rType hex member, used in roads drawing.
    This flag identifies:
*/
enum {
    R_NONE = 0, ///< no road at this time
    R_ROAD,     ///< a road uses this hex
    R_STOP,     ///< this hex is an endpoint for the roads
    R_HIT,      ///< this endpoint have been reached by a road
};

#endif // YAMG_DECLS_HPP
