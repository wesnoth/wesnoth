/*********************************************************************
        Yet Another Map Generator. Class YAMG_Hex

    This class represents an individual hex on the map.
    The map itself is a two dimension array of (pointers to) objects of this class.

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

#ifndef YAMG_HEX_HPP
#define YAMG_HEX_HPP

#include "yamg_decls.hpp"

class yamg_hex
{
    public:
        yamg_hex(unsigned int xcoor, unsigned int ycoor);

        yamg_hex* next;         ///< to organize stacks and lists
        yamg_hex* list;         ///< to organize stacks and lists
        yamg_hex* road;         ///< to draw the roads

        unsigned int x;         ///< x,y coordinates
        unsigned int y;

        int alt;                ///< computed altitude
        unsigned int layer;     ///< computed layer

        const char *base;             ///< base terrain
        const char *over;             ///< overlay terrain
        unsigned int player;          ///< player number if any
        unsigned int water;           ///< temporary value for water computing
        int key;                      ///< temporary value for heap sort
        bool done;                    ///< temporary lock flag
        int lock;                     ///< the multi state lock
        int road_type;                    ///< a flag for use with roads drawing
        bool water_flag;                   ///< marking water terrains

//----------- methods ------------
        unsigned int create_terrain_code(char *ptr);   ///< build the final output map code

    protected:
    private:
};

#endif // YAMG_HEX_HPP
