/*******************************************
    Yet Another map generator
    Implementation file.
********************************************/

#include "yamg_hex.hpp"
#include <stdio.h>

char default_hex_code[] = "Gll"; ///< terrain default value for the constructor
char default_over[] = "";

/**
    This constructor initialize the hexes so createTerrainCode() will never produce invalid values.
*/
yamg_hex::yamg_hex(unsigned int xcoor, unsigned int ycoor)
    : next(NULL)
    , list(NULL)
    , road(NULL)
    , x(xcoor)
    , y(ycoor)
    , alt(0)
    , layer(YAMG_GROUND)
    , base(default_hex_code)
    , over(default_over)
    , player(0)
    , water(0)
    , key(0)
    , done(false)
    , lock(YAMG_UNLOCK)
    , road_type(R_NONE)
    , water_flag(false)
{}

/**
    Write the terrain code of the hex in a buffer.
    The format is:
    [player numberSPACE]{base terrain}[^overlay]{SPACE padding UP to 12 chars},SPACE

    -> pointer where to write.
    <- number of bytes written (always YAMG_HEXLONG)
*/
unsigned int yamg_hex::create_terrain_code(char *ptrwr) {

	int n = 0;

	//TODO
//    n = sprintf(ptrwr,"%6u, ",alt);
//    return n;

    if(base == NULL)
        return 0; // TODO: this skips bad hexes, but the map will be incorrect. Raise an exception instead
    if(player != 0)
        n = sprintf(ptrwr,"%u ",player);

    if(*over == '\0')
        n += sprintf((ptrwr+n),"%s",base);
    else
        n += sprintf((ptrwr+n),"%s^%s",base,over);
    ptrwr += n;
    n = 12 - n;
    for(;n > 0; n--)
        *ptrwr++ = ' '; // padd with spaces
    *ptrwr++ = ',';
    *ptrwr++ = ' ';

    return YAMG_HEXLONG;

}
