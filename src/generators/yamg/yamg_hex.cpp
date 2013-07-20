/*******************************************
    Yet Another map generator
    Implementation file.
********************************************/

#include "yamg_hex.hpp"
#include <stdio.h>

char defaultHexCode[] = "Gll"; ///< terrain default value for the constructor
char defaultOver[] = "";

/**
    This constructor initialize the hexes so createTerrainCode() will never produce invalid values.
*/
yamg_hex::yamg_hex(unsigned int xcoor, unsigned int ycoor)
{
    //ctor
    x = xcoor;
    y = ycoor;
    layer = YAMG_GROUND;
    next = NULL;
    road = NULL;
    list = NULL;
    alt = 0;
    player = 0;
    base = defaultHexCode;
    over = defaultOver;
    water = 0;
    key = 0;
    rType = R_NONE;
    done = false;
    lock = YAMG_UNLOCK;
    wFlag = false;
}

/**
    Write the terrain code of the hex in a buffer.
    The format is:
    [player numberSPACE]{base terrain}[^overlay]{SPACE padding UP to 12 chars},SPACE

    -> pointer where to write.
    <- number of bytes written (always YAMG_HEXLONG)
*/
unsigned int yamg_hex::createTerrainCode(char *ptrwr) {

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
