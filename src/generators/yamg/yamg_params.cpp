/*
   Copyright (C) 2013 - 2014 by L.Sebilleau <l.sebilleau@free.fr>
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
    Yet Another map generator
    Implementation file.
*/

#ifdef YAMG_STANDALONE

#include <ctype.h>
#include <string.h>
#include <stdio.h>

#else

#include "global.hpp"

#endif

#include "yamg_params.hpp"

//yamg_params::yamg_params(config& /*cfg*/)
//{
//	yamg_params::yamg_params();
//}


yamg_params::yamg_params()
    : seed(0)
    , width(65)
    , height(65)
    , rough(12)
    , type('t')
    , season('s')
    , snowlev(M_NUMLEVEL * 10)
    , water_ratio(100)
    , alt_mid(0)
    , alt_nw(0)
    , alt_ne(0)
    , alt_se(0)
    , alt_sw(0)
    , vill(10)
    , burgs(4)
    , forests(20)
    , players(0)
    , casthexes(6)
    , bridges(50)
    , roads(true)
    , ro_road(0)
    , thickness()
    , base_cust()
    , forest_cust()
    , houses_cust()
    , keeps_castles_cust()
    , hexes_castles_cust()
    , base_snow_cust()
    , roads_cust()
    , lilies_cust(NULL)
    , fields_cust(NULL)
    , bridges_cust(NULL)
{
    //ctor set defaults values
#ifdef YAMG_STANDALONE
    strcpy(path,"wesmap.map");           ///< pathname of the file
#endif
    //TODO

    thickness[YAMG_DEEPSEA] = 10;      ///< layers thickness
    thickness[YAMG_SHALLSEA] = 10;
    thickness[YAMG_BEACH] = 5;
    thickness[YAMG_SWAMPS] = 5;
    thickness[YAMG_GROUND] = 35;
    thickness[YAMG_HILLS] = 15;
    thickness[YAMG_MOUNTAINS] = 15;
    thickness[YAMG_IMPMOUNTS] = 10;

    int i;
    for(i=0; i < M_NUMLEVEL; i++)
        base_cust[i] = NULL;
    for(i=0; i < 12; i++)
        forest_cust[i] = NULL;
    for(i=0; i < 14; i++)
        houses_cust[i] = NULL;
    for(i=0; i < 10; i++)
        keeps_castles_cust[i] = NULL;
    for(i=0; i < 10; i++)
        hexes_castles_cust[i] = NULL;
    for(i=0; i < M_NUMLEVEL; i++)
        base_snow_cust[i] = NULL;
    for(i=0; i < 4; i++)
        roads_cust[i] = NULL;
}

yamg_params::~yamg_params()
{
    int i;
    for(i=0; i < M_NUMLEVEL; i++)
        if(base_cust[i] != NULL)
            delete base_cust[i];
    for(i=0; i < 12; i++)
        if(forest_cust[i] != NULL)
            delete forest_cust[i];
    for(i=0; i < 14; i++)
        if(houses_cust[i] != NULL)
            delete houses_cust[i];
    for(i=0; i < 10; i++)
        if(keeps_castles_cust[i] != NULL)
            delete keeps_castles_cust[i];
    for(i=0; i < 10; i++)
        if(hexes_castles_cust[i] != NULL)
            delete hexes_castles_cust[i];
    for(i=0; i < M_NUMLEVEL; i++)
        if(base_snow_cust[i] != NULL)
            delete base_snow_cust[i];
    for(i=0; i < 4; i++)
        if(roads_cust[i] != NULL)
            delete roads_cust[i];
    if(lilies_cust != NULL)
        delete lilies_cust;
    if(bridges_cust != NULL)
        delete bridges_cust;
    if(fields_cust != NULL)
        delete fields_cust;
}


/**
    Check parameters limits and consistency
    TODO: finish this
*/
unsigned int yamg_params::verify()
{

    unsigned int result = YAMG_OK;

    if((width > YAMG_MAPMAXSIZE) || (height > YAMG_MAPMAXSIZE) )
        result |= YAMG_LIMITS;

    if((rough > YAMG_ROUGHMAX) || (rough == 0))
        result |= YAMG_ROUGHOFF;

    if(!find_in_string(YAMG_TYPES,type))
        result |= YAMG_BADTYPE;

    if(!find_in_string(YAMG_SEASONS,season))
        result |= YAMG_BADSEASON;

    //TODO this line causes a warning
    if((snowlev > ((M_NUMLEVEL+1) * 10))) // || (snowlev < 0))
        result |= YAMG_SNOW;

    if((alt_mid > YAMG_ALTMAX) || (alt_mid < -YAMG_ALTMAX))
        result |= YAMG_ALTOFF;

    if(vill > YAMG_VILLMAX)
        result |= YAMG_VILLOFF;

    if(burgs > YAMG_BURGMAX)
        result |= YAMG_BURGOFF;

//    if(towns > YAMG_TOWNSMAX)
//        result |= YAMG_TOWNSOFF;

    if(forests > YAMG_FORESTMAX)
        result |= YAMG_FORESTOFF;

    if(players > YAMG_PLAYERSMAX)
        result |= YAMG_PLAYERSOFF;

    return result;
}

#ifdef YAMG_STANDALONE
/**
    This part is needed only if parameters are read from a file
*/
const char *parmlist[] =
{
    "mapname",
    "seed",
    "width",
    "height",
    "rough",
    "type",
    "season",
    "snowlevel",
    "altmiddle",
    "roads",
    "bridges",
    "castlesHexes",
    "villages",
    "burgs",
    "towns",
    "forests",
    "castles",
    "evaporation",
    "deepsea",
    "shallowsea",
    "beach",
    "swamps",
    "fields",
    "hills",
    "mountains",
    "peaks",
    "straightness",
    "altbases",
    "altforests",
    "althouses",
    "altkeeps",
    "altcastles",
    "altsnow",
    "altroads",
    "altlilies",
    "altfields",
    "altbridges",
    "altNW",
    "altNE",
    "altSE",
    "altSW",
};

/**
    Storing custom terrain codes from parameters file
*/
void yamg_params::store_terrain_codes(const char *input, const char **table, unsigned int cnt)
{
    unsigned int i,l,nt;
    const char *pt;
    char *w;

    for(i = 0; i < cnt; i++)
    {
        pt = input;
        while((*pt != ',') && (*pt != '\0'))
            pt++;
        l = pt - input;
        if((l > 1) && (l <= 6))
        {
            table[i] = new char[10];
            for(nt = 0, w = (char *)table[i]; nt < l; nt++, w++, input++)
            {
                *w = *input;
            }
            *w = '\0';
        }
        if( *pt == '\0')
            return;
        else
            input = pt + 1;
    }
}

/**
    Read a parameter file
    -> path to file
*/
unsigned int yamg_params::read_params(const char *ficnom)
{
    FILE *f;
    char buf[20000], instr[100], value[2048], *wr = NULL, *ptr, *end;
    int n,i;

    f = fopen(ficnom,"r");
    if(f == NULL)
        return YAMG_FILENOTFOUND;

    n = fread(buf,1,20000,f);
    ptr = buf;
    end = buf + n;
    n = 3;
    // ---- read loop ----
    while(ptr < end)
    {
        switch(n)
        {
        case 0:
            // catch parameter name.
            while( (*ptr != '=') && (*ptr != '#') && (ptr < end))
                *wr++ = *ptr++;
            if(*ptr == '#')
                n = 2; // skip the line
            else
            {
                *wr++ = '\0';
                wr = value;
                ptr++;
                n = 1;
            }
            break;

        case 1:
            // get the value
            while( (!isspace(*ptr)) && (*ptr != '#') && (ptr < end))
                *wr++ = *ptr++;

            *wr++ = '\0';
            for(i=0; i < 41; i++)
            {
                if(strcmp(instr,parmlist[i]) == 0)
                    break;
            }

            switch(i)
            {
            case 0: // mapname
                strcpy(path,value);
                break;
            case 1:
                sscanf(value,"%u",&seed);
                break;
            case 2:
                sscanf(value,"%u",&width);
                break;
            case 3:
                sscanf(value,"%u",&height);
                break;
            case 4:
                sscanf(value,"%u",&rough);
                break;
            case 5:
                type = value[0];
                break;
            case 6:
                season = value[0];
                break;
            case 7:
                sscanf(value,"%u",&snowlev);
                snowlev -= 10;
                break;
            case 8:
                sscanf(value,"%i",&alt_mid);
                break;
            case 9:
                roads = ((*value == 'n') || (*value == 'N')) ? false:true;
                break;
            case 10:
                sscanf(value,"%i",&bridges);
                break;
            case 11:
                sscanf(value,"%u",&casthexes);
                break;
            case 12:
                sscanf(value,"%u",&vill);
                break;
            case 13:
                sscanf(value,"%u",&burgs);
                break;
            case 14:
                //sscanf(value,"%u",&towns);
                break;
            case 15:
                sscanf(value,"%u",&forests);
                break;
            case 16:
                sscanf(value,"%u",&players);
                break;
            case 17:
                sscanf(value,"%u",&water_ratio);
                break;
            case 18:
                sscanf(value,"%u",&thickness[0]);
                break;
            case 19:
                sscanf(value,"%u",&thickness[1]);
                break;
            case 20:
                sscanf(value,"%u",&thickness[2]);
                break;
            case 21:
                sscanf(value,"%u",&thickness[3]);
                break;
            case 22:
                sscanf(value,"%u",&thickness[4]);
                break;
            case 23:
                sscanf(value,"%u",&thickness[5]);
                break;
            case 24:
                sscanf(value,"%u",&thickness[6]);
                break;
            case 25:
                sscanf(value,"%u",&thickness[7]);
                break;
            case 26:
                sscanf(value,"%u",&ro_road);
                break;
            case 27:
                store_terrain_codes(value, base_cust, M_NUMLEVEL);
                break;
            case 28:
                store_terrain_codes(value, forest_cust, 12);
                break;
            case 29:
                store_terrain_codes(value, houses_cust, 14);
                break;
            case 30:
                store_terrain_codes(value, keeps_castles_cust, 10);
                break;
            case 31:
                store_terrain_codes(value, hexes_castles_cust, 10);
                break;
            case 32:
                store_terrain_codes(value, base_snow_cust, M_NUMLEVEL);
                break;
            case 33:
                store_terrain_codes(value, roads_cust, 4);
                break;
            case 34:
                lilies_cust = new char[10];
                strncpy(lilies_cust,value,5);
                break;
            case 35:
                fields_cust = new char[10];
                strncpy(fields_cust,value,5);
                break;
            case 36:
                bridges_cust = new char[10];
                strncpy(bridges_cust,value,5);
                break;
            case 37:
                sscanf(value,"%u",&alt_nw);
                break;
            case 38:
                sscanf(value,"%u",&alt_ne);
                break;
            case 39:
                sscanf(value,"%u",&alt_se);
                break;
            case 40:
                sscanf(value,"%u",&alt_sw);
                break;

            default: // unknow parameter
                break;
            }

        case 2:
            // go to end of line
            while( (*ptr != '\n') && (ptr < end))
                ptr++;
            ptr++;
            if((ptr >= end) || (*ptr == '\n'))
                break;
            // no break if we are only at the end of a line.
        case 3:
            wr = instr;
            n = 0;
            break;
        }
    }
    fclose(f);
    return verify();
}
#endif

//----------------- Utils ----------------
bool find_in_string(const char *str, char c)
{
    const char *ptr = str;

    while( *ptr != '\0')
        if(*ptr == c)
            return true;
        else
            ptr++;
    return false;
}
