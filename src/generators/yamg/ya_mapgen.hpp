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
 Yet Another Map Generator.

 This class is the main generator class. Once instanced and configured, it can create a random map and can produce a buffer which BfW can use.
 Each generator owns a single map, but multiple instances can be used. Generator can be reused too.

 Public functions are used in this order in the standalone tool:
 - Constructor
 - Configuration creation and modification.
 - SetUp : install and check configuration.
 - CreateMap : create the map in memory.
 - GetMap : write the map data to a buffer.
 - ResetMap : free the old map and reset the generator to default values.

 Provisions have been made to make this class a child of map_generator, but the implementation is not finished yet.
 */

#ifndef YA_MAPGEN_HPP
#define YA_MAPGEN_HPP

#include "yamg_params.hpp"
#include "yamg_hex.hpp"
#include "yamg_hexheap.hpp"

#ifndef YAMG_STANDALONE
#include "config.hpp"
#include "generators/map_generator.hpp"
#endif

/**
 This structure is used to store one hex and it's neighbors
 */
struct neighbors {
	yamg_hex *center;
	yamg_hex *no;
	yamg_hex *nw;
	yamg_hex *sw;
	yamg_hex *so;
	yamg_hex *se;
	yamg_hex *ne;
};

/**
 This structure is used to store the burgs
 */
struct burg {
    burg *next;
	yamg_hex *center;
	yamg_hex *road1;
	yamg_hex *road2;
};

//============================== Class definition =========================

#ifdef YAMG_STANDALONE
class ya_mapgen
{
public:

#else

class ya_mapgen: public map_generator
{
public:
	ya_mapgen(const config& cfg);
#endif
	ya_mapgen();
	virtual ~ya_mapgen();

	//----------- Inherited methods from map_generator -----------------

#ifndef YAMG_STANDALONE
	std::string name() const; // {return "yamg";};
	std::string config_name() const; // {return "generator";};
	std::string create_map(boost::optional<boost::uint32_t> randomseed);
#endif

//----------------- Methods -------------

    unsigned int set_up(yamg_params *); ///< uses parameter list object to configure
    int do_create_map();           ///< do the job, return OK if everything is OK
    int get_map(char *buf);     ///< write the map to a buffer
    void reset_map();           ///< reset the generator to new use

protected:
    //----------------- Functions -------------

    unsigned int create_empty_map(); ///< creates an empty map according parameters height and width
    unsigned int free_map();         ///< frees all memory used

    void create_altitudes(unsigned int x, unsigned int xm, unsigned int y,
			unsigned int ym, unsigned int rough); ///< compute hexes altitudes
    int normalize_map();             ///< normalize altitudes
    void set_base_terrains(int range);    ///< set hexes base terrains
    void custom_terrains();          ///< terrain customization

    void make_rivers();              ///< create river and lakes
    int calc_water_contribs(yamg_hex *h); ///< calculate rain flowing
    void erode_terrains(yamg_hex *h, int report); ///< erode terrains to make rivers

    void make_burgs();       ///< creates the burgs (some agglomerated villages)
    void make_castles();     ///< creates the castles
    void make_forests();     ///< set forests overlays
    void make_houses();      ///< creates some houses (villages)
    void make_roads();       ///< creates roads

    void get_neighbors(yamg_hex *h, neighbors* p);               ///< get neighbours of some hex
    int fill_with(const char *over[], yamg_hex *h, int num); ///< utility to fill overlays
    void clear_done_flag();                                   ///< reset done flag on all hexes
    yamg_hex *sel_neigh(yamg_hex *it);                       ///< lists the available hexes for roads
    void store_neighbors(yamg_hex *it, unsigned int layMin, unsigned int layMax); ///< get neighbours and store them in the heap

private:
    unsigned int status_;    ///< this is the state of the generator.
    unsigned int siz_; ///< the 'real' size of the map (used allocation and computing altitudes) 2^n + 1
    yamg_params *parms_;     ///< its parameter list.
    yamg_hex ***map_;        ///< the generated map
    yamg_hex *summits_;      ///< a list of hexes, various purposes
    yamg_hex *endpoints_;    ///< something to store a list of road startpoints
    yamg_hex *castles_;      ///< a list of castle keeps

    int table_[M_NUMLEVEL];  ///< array defining layers boundaries.
    int snow_limit_;               ///< the snow limit floor.
    unsigned int riv_;       ///< the reference water level
    const char *terrains_[M_NUMLEVEL]; ///< terrains to use for each layer (overloaded with snow)
    yamg_hexheap *heap_;          ///< an utility heap to sort hexes
};

int m_rand(int limit);                 ///< returns a random number 0 < n < limit
void init_rand(unsigned int seed);    ///< init random number generator

#ifdef INTERN_RAND
void init_genrand(unsigned long s); ///< embedded RNG initialization
unsigned long genrand(void);        ///< RNG production
#endif

#endif // YA_MAPGEN_HPP
