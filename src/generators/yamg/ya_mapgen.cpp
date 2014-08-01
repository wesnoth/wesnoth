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

#include "ya_mapgen.hpp"
#include "yamg_hexheap.hpp"
#include <cstdio>
#include <cstring>

#ifdef YAMG_STANDALONE
#include <stdlib.h>
#endif

//========================== Static parameters =========================
/*
 Here are the various terrain tables used to define base terrains. Each one is supposed to contain M_NUMLEVEL 'layers' exactly
 Each landscape type has it's own. One more is used for 'snowing' the landscape.
 A custom land type has been added to allow user to declare a custom set of terrain bases, forests and houses.
 */

// ------------- default = (t)empered ----------------
const char *base_land[M_NUMLEVEL] = { ///< terrain value for standard lands
		"Wo",    // deep water
				"Ww",    // shallow water
				"Ds",    // beach
				"Ss",    // swamps
				"Gg",    // flat
				"Hh",    // hills
				"Mm",    // mountains
				"Mm^Xm", // high mountains
		};
// forests table
const char *forest_land[] = { "Fds", "Fdf", "Fdw", "Fda", // level 1 spring easter, fall, winter, snow
		"Fms", "Fmf", "Fmw", "Fma", // level 2
		"Fp", "Fp", "Fp", "Fpa",    // level 3
		};
// houses tables
const char *houses_land[] = { "Vm", "Vaa",   // water
		"Vhs", "Vca",  // sand
		"Vhs", "Vca",  // swamps
		"Vh", "Vha",   // flat ground
		"Ve", "Vea",   // flat forests
		"Vhh", "Vhha", // hills naked
		"Vl", "Vla",   // hills forested
		};

const char *house_burg[] = { "Vm", "Vaa",   // water
		"Vhs", "Vca",  // sand
		"Vhs", "Vca",  // swamps
		"Vhc", "Vhca", "Ve", "Vea",   // flat forests
		"Vhh", "Vhha", "Vl", "Vla",   // hills forested
		};

const char *keeps_castles[] = { "Khw", "Khw", // water
		"Khs", "Khs", // swamp
		"Ke", "Kea",  // land
		"Ko", "Koa",  // hills
		"Kh", "Kha",  // mountains
		};
const char *hexes_castles[] = { "Chw", "Chw", // water
		"Chs", "Chs", // swamp
		"Ce", "Cea",  // land
		"Co", "Coa",  // hills
		"Ch", "Cha",  // mountains
		};

// ------------- (p)olar landscape ----------------

const char *base_polar[M_NUMLEVEL] = { ///< terrain value for polar lands
		"Wog", "Wwg", "Rb", "Sm", "Aa", "Ha", "Ms", "Ms^Xm", };
const char *forest_polar[] = { "Fmw", "Fmw", "Fmw", "Fma", "Fp", "Fp", "Fp",
		"Fpa", "Fmw", "Fmw", "Fmw", "Fma", };
// houses tables
const char *houses_polar[] = { "Vm", "Vaa", // water
		"Vc", "Vca", // sands
		"Vc", "Vca", // swamps
		"Vo", "Voa", // flat ground
		"Vl", "Vla", // flat forests
		"Vu", "Vaa", // hills naked
		"Vl", "Vla", // hills forested
		};

const char *keeps_castles_polar[] = { // polar
		"Khw", "Khw", // water
				"Khs", "Khs", // swamp
				"Ke", "Kea",  // land
				"Ko", "Koa",  // hills
				"Kh", "Kha",  // mountains
		};

const char *hexes_castles_polar[] = { "Chw", "Chw", // water
		"Chs", "Chs", // swamp
		"Ce", "Cea",  // land
		"Co", "Coa",  // hills
		"Ch", "Cha",  // mountains
		};

// ------------- (m)id desertic landscape ----------------

const char *base_medit[M_NUMLEVEL] = { ///< terrain value for mid desert lands
		"Wo", "Ww", "Ds", "Re", "Gd", "Hhd", "Md", "Md^Xm", };
const char *forest_medit[] = { "Ft", "Ft", "Ft", "Fda", "Fms", "Fmf", "Fmw",
		"Fma", "Fp", "Fp", "Fp", "Fpa", };
// houses tables
const char *houses_medit[] = { "Vm", "Vm",    // water
		"Vhs", "Vca",  // sands
		"Vhs", "Vca",  // swamps
		"Vc", "Vca",   // flat ground
		"Ve", "Vea",   // flat forests
		"Vhh", "Vhha", // hills naked
		"Vl", "Vla",   // hills forested
		};

const char *keeps_castles_medit[] = { "Khw", "Khw", // water
		"Khs", "Khs", // swamp
		"Ke", "Kea",  // land
		"Kd", "Kd",   // hills
		"Kud", "Kud", // mountains
		};

const char *hexes_castles_medit[] = { "Chw", "Chw", // water
		"Chs", "Chs", // swamp
		"Ce", "Cea",  // land
		"Cd", "Cd",   // hills
		"Cud", "Cud", // mountains
		};

// ------------- (d)esertic landscape ----------------

const char *base_desert[M_NUMLEVEL] = { ///< terrain value for desert lands
		"Wot", "Wwt", "Ds", "Rd", "Dd", "Hd", "Md", "Md^Xm", };
const char *forest_desert[] = { "Edp", "Edp", "Edp", "Edp", "Edpp", "Edpp",
		"Edpp", "Edpp", "Esd", "Esd", "Esd", "Esd", };
// houses tables
const char *houses_desert[] = { "Vm", "Vaa",  // water
		"Vhs", "Vca", // sands
		"Vhs", "Vca", // swamps
		"Vda", "Vda", // flat ground
		"Vda", "Vda", // flat forests
		"Vdt", "Vdt", // hills naked
		"Vc", "Vca",  // hills forested
		};

const char *keeps_castles_desert[] = { // desert
		"Khw", "Khw", // water
				"Kdr", "Kdr", // swamp
				"Ke", "Kea",  // land
				"Kd", "Kd",   // hills
				"Kdr", "Kdr", // mountains
		};
const char *hexes_castles_desert[] = { "Chw", "Chw", // water
		"Cdr", "Cdr", // swamp
		"Ce", "Cea",  // land
		"Cd", "Cd",   // hills
		"Cdr", "Cdr", // mountains
		};

// ------------- (e)quatorial landscape ----------------

const char *base_trop[M_NUMLEVEL] = { ///< terrain value for tropical lands
		"Wot", "Wwt", "Ds", "Ss", "Gg", "Hh", "Mm", "Mm^Xm", };
const char *forest_trop[] = { "Ft", "Ft", "Ft", "Ft", "Ft", "Ft", "Ft", "Ft",
		"Edp", "Edp", "Edp", "Edp", };
// houses tables
const char *houses_trop[] = { "Vm", "Vaa",  // water
		"Vhs", "Vca", // sands
		"Vhs", "Vca", // swamps
		"Vht", "Vca", // flat ground
		"Vht", "Vea", // flat forests
		"Vo", "Voa",  // hills naked
		"Vl", "Vla",  // hills forested
		};

const char *keeps_castles_trop[] = { "Khw", "Khw", // water
		"Khs", "Khs", // swamp
		"Ke", "Kea",  // land
		"Kv", "Kv",   // hills
		"Kd", "Kd",   // mountains
		};

const char *hexes_castles_trop[] = { "Chw", "Chw", // water
		"Chs", "Chs", // swamp
		"Ce", "Cea",  // land
		"Cv", "Cv",   // hills
		"Cd", "Cd",   // mountains
		};

// ------------- snowy terrains table ----------------

const char *base_snow[M_NUMLEVEL] = { ///< terrain value for snow management
		"Wo", "Ai", "Ds", "Sm", "Aa", "Ha", "Ms", "Ms^Xm", };

// ---------- misc no substitution here ----------

const char default_over[] = "";
const char oasis_base[] = "Dd";
const char oasis_water[] = "Do";
const char oasis_dry[] = "Dc";
const char fords[] = "Wwf";

// ------------ default decorations (can be customized) ------------

const char road_burg_dec[] = "Rr";
const char place_burg_dec[] = "Rr";
const char open_road_dec[] = "Rr";
const char forest_road_dec[] = "Rp";
const char field_over_dec[] = "Gvs";
const char *bridges_dec[] = { "Bsb|", "Bsb/", "Bsb\\" };

// ------------- custom terrains set ----------------

const char *base_cust[M_NUMLEVEL];
const char *forest_cust[12];
const char *houses_cust[14];
const char *keeps_castles_cust[10];
const char *hexes_castles_cust[10];
const char *base_snow_cust[M_NUMLEVEL];

const char *road_burg;
const char *place_burg;
const char *open_road;
const char *forest_road;
const char *lilies;
const char *field_over;
const char *bridges[3];

//===================================== Constructors & Destructors ===================================

ya_mapgen::ya_mapgen() :
        siz_(0), summits_(NULL), castles_(NULL), snow_limit_(0), riv_(0) {
	status_ = YAMG_EMPTY;
	parms_ = NULL;
	map_ = NULL;
	heap_ = NULL;
	endpoints_ = NULL;
}

ya_mapgen::~ya_mapgen() {
	//dtor
	if (parms_ != NULL) {
		try {
			delete parms_;
		} catch (...) {
		};
	}
	if (map_ != NULL)
		free_map();
	if (heap_ != NULL)
		delete heap_;
}

//================================= map_generator class implementation ==================================

#ifndef YAMG_STANDALONE

const char gen_nom[] = "YetAnotherMapGenerator";
const char def_nom[] = "tempered";
const char pol_nom[] = "polar";
const char medi_nom[] = "mediterranean";
const char equ_nom[] = "equatorial";
const char desert_nom[] = "desert";
const char cust_nom[] = "custom";

ya_mapgen::ya_mapgen(const config& /*cfg*/) :
        siz_(0), summits_(NULL), castles_(NULL), snow_limit_(0), riv_(0) {
	status_ = YAMG_EMPTY;
	parms_ = NULL;
	map_ = NULL;
	heap_ = NULL;
	endpoints_ = NULL;
}

/**
 *** allow_user_config

 No UI at this point.
 */
bool ya_mapgen::allow_user_config() const {
	return false;
}
void ya_mapgen::user_config(display& /*disp*/) {
	return;
}

/**
 *** name
 Returns generator's name
 */
std::string ya_mapgen::name() const {
	return gen_nom;
}

/**
 *** config_name
 Returns config name if any.
 */
std::string ya_mapgen::config_name() const {
	if (parms_ == NULL)
		return def_nom;
	else
		switch (parms_->type) {
		case 'p':
			return pol_nom;
		case 'm':
			return medi_nom;
		case 'd':
			return desert_nom;
		case 'e':
			return equ_nom;
		case 'c':
			return cust_nom;
		case 't':
		default:
			return def_nom;
		}
}

/**
 *** create_map
 Parameters and errors reporting are not defined yet, so this is a mere skeleton.
 */
std::string ya_mapgen::create_map() {

	yamg_params *par = new yamg_params();

	//TODO
	unsigned int n;  // = par->readParams(args); // fills the parms object

	n = set_up(par); // clear the old map if any and check parameters

	//TODO
//	if(n > 0) {
//		printf(stderr,"Errors in parameters. Execution aborted.");
//		return NULL;
//	}
	n = do_create_map(); // do the job
	n = (par->height * par->width * (YAMG_HEXLONG + 2)) + 100; // allocate buffer for result
	char* buf = new char[n];

	n = get_map(buf); //creates the map in Wesnoth format
	const std::string result(buf);
	delete[] buf;
	buf = NULL;
	return result;
}
#endif

//================================ Member functions =================================

/**
 *** reset_map

 Reset generator for reuse.
 Note: this erase all previous configuration including delete parm list if needed.
 */
void ya_mapgen::reset_map() {
	if (parms_ != NULL) {
		try {
			delete parms_;
		} catch (...) {
		};
		parms_ = NULL;
	}
	if (map_ != NULL)
		free_map();
	endpoints_ = NULL;
	status_ = YAMG_EMPTY;
}

/**
 *** set_up

 Setup: set a new parameters list to the generator.

 -> pointer to a parm list object.
 <- return OK or parameters errors.

 Note: this calls ResetMap() (if no errors).
 */
unsigned int ya_mapgen::set_up(yamg_params *pars) {

	//---- We verify all parameters first ----
	int n = pars->verify();
	if (n != YAMG_OK)
		return n;

	if (pars == parms_)
		parms_ = NULL; // this to avoid deleting reused parms objects
	reset_map();
	parms_ = pars;
	status_ = YAMG_CONFIGURED;
	return YAMG_OK;
}

/**
 *** create_map

 Do the actual map creation. All parameters should have been set previously: if not, an error is returned.

 Operation order is very important here. Don't change it.

 <- returns OK or error code.
 */
int ya_mapgen::do_create_map() {
	unsigned int err;

	switch (status_) {
	case YAMG_EMPTY:
		return YAMG_NOCONFIG;
	case YAMG_CONFIGURED:
		if ((err = parms_->verify()) != YAMG_OK)
			return err;
		break;
	case YAMG_LOADED:
		return YAMG_HAVEMAP;
	default:
		return YAMG_ERROR;
	}

	//------- customize terrains if needed
	custom_terrains();

	//------- first create an empty map and an empty heap according map dimension.

	create_empty_map();
	heap_ = new yamg_hexheap(parms_->height * parms_->width); // this allocate a heap for various uses, we expect it not containing more hexes than the map itself.

	//------- compute altitudes --------
	unsigned int dim = siz_ - 1;
	init_rand(parms_->seed);
	if (parms_->alt_mid >= 0) {
		map_[0][0]->alt = M_BASE + (parms_->alt_nw * M_VARIATION);
		map_[0][dim]->alt = M_BASE + (parms_->alt_ne * M_VARIATION);
		map_[dim][0]->alt = M_BASE + (parms_->alt_sw * M_VARIATION);
		map_[dim][dim]->alt = M_BASE + (parms_->alt_se * M_VARIATION);
		map_[dim / 2][dim / 2]->alt = (parms_->alt_mid * M_VARIATION);
	} else {
		unsigned int z = -(parms_->alt_mid * M_VARIATION);
		map_[0][0]->alt = M_BASE + z + (parms_->alt_nw * M_VARIATION);
		map_[0][dim]->alt = M_BASE + z + (parms_->alt_ne * M_VARIATION);
		map_[dim][0]->alt = M_BASE + z + (parms_->alt_sw * M_VARIATION);
		map_[dim][dim]->alt = M_BASE + z + (parms_->alt_se * M_VARIATION);
		map_[dim / 2][dim / 2]->alt = M_BASE;
	}
	create_altitudes(0, dim, 0, dim, M_VARIATION);

	// some altitudes calculations giving back the full range of them
	unsigned int range = normalize_map();

	//------- make rivers and lakes. This modify slightly the existing map
	if (parms_->water_ratio != 0) {
		riv_ = ((parms_->water_ratio * (parms_->width + parms_->height))
				/ YAMG_RIVER_ADAPTATION) + 3;
		make_rivers(); // since this change slightly the mesh, we have to do this before setting base terrains.
	} else
		riv_ = 0;

	//------- set base terrains, i.e. according to their altitude, land option, snow limit and so on.
	set_base_terrains(range);

	//------- creates burgs
	init_rand(parms_->seed + 2); // this reinitialization (and further ones) help to maintain reproducibility when changing parameters.
	if (parms_->burgs > 0)
		make_burgs();

	//------- creates castles
	init_rand(parms_->seed + 3);
	if (parms_->players > 0)
		make_castles();

	//------- creates forests
	init_rand(parms_->seed + 4);
	make_forests();

	//------- creates houses (villages)
	init_rand(parms_->seed + 6);
	make_houses();

	//-------- creates roads
	if(parms_->roads)
	   make_roads();

	//------- finish map
	delete heap_;
	heap_ = NULL;
	status_ = YAMG_LOADED;
	return YAMG_OK;
}

/**
 *** get_map

 Writes the map into a buffer in Wesnoth format. The map must obviously have been created first.
 -> *buffer to fill
 <- OK or error code
 */
int ya_mapgen::get_map(char *buf) {
	char *ptr = buf;

	switch (status_) {
	case YAMG_EMPTY:
		return YAMG_NOCONFIG;
	case YAMG_CONFIGURED:
		return YAMG_NOMAP;
	case YAMG_LOADED:
		break;
	default:
		return YAMG_ERROR;
	}

	//---------- create the header
	ptr += sprintf(ptr, "border_size=1\nusage=map\n\n");

	//---------- iterate on hexes
	for (unsigned int y = 0; y < parms_->height; y++) {
		for (unsigned int x = 0; x < parms_->width; x++)
			ptr += map_[y][x]->create_terrain_code(ptr);
		ptr -= 2; // because at the end of line the previous instruction created a ', '
		ptr += sprintf(ptr, "\n");
	}
	*ptr = '\0';

	return YAMG_OK;
}

//---------------------- Create and delete maps ------------------------

/**
 *** create_empty_map

 Allocate a new map, i.e. a bi-dimensional array of hexes.
 Because the recursive computing algorithm, full maps are always square and their size is a power of 2.
 They are clipped to their requested size later, but we need here all the memory.

 -> nothing
 <- result code.
 TODO: set up something to catch 'out of memory' errors.
 */

unsigned int ya_mapgen::create_empty_map() {
	unsigned int n, m;

	if (parms_->height > parms_->width)
		n = parms_->height;
	else
		n = parms_->width;

	m = 32;
	while (m < n)
		m *= 2;
	n = siz_ = m + 1;

	map_ = new yamg_hex **[n * sizeof(yamg_hex **)]; //(yamg_hex ***)malloc(n * sizeof(yamg_hex **));
	for (unsigned int i = 0; i < n; i++) {
		map_[i] = new yamg_hex *[n * sizeof(yamg_hex **)]; //(yamg_hex **)malloc(n * sizeof(yamg_hex **));
		for (unsigned int j = 0; j < n; j++)
			map_[i][j] = new yamg_hex(j + 1, i + 1);
	}
	return YAMG_OK;
}

/**
 *** free_map

 Clears the current map.
 Call the destructor of each hex and next free memory.
 */
unsigned int ya_mapgen::free_map() {

	for (unsigned int i = 0; i < siz_; i++) {
		for (unsigned int j = 0; j < siz_; j++)
			delete map_[i][j];
		delete map_[i];
	}
	delete map_;
	map_ = NULL;
	return YAMG_OK;
}

/**
 *** Terrain Customization.

 Get the custom terrains from parameter object and build the needed terrain tables.

 To avoid errors, the function first fills every table with a default builtin values.
 Some terrains tables are used only when terrain type = c(ustom)
 */

void ya_mapgen::custom_terrains() {
	int i;
	char *tmp;

	// applying defaults to avoid problems
	for (i = 0; i < M_NUMLEVEL; i++)
		base_cust[i] = base_land[i];
	for (i = 0; i < 12; i++)
		forest_cust[i] = forest_land[i];
	for (i = 0; i < 14; i++)
		houses_cust[i] = houses_land[i];
	for (i = 0; i < 10; i++)
		keeps_castles_cust[i] = keeps_castles[i];
	for (i = 0; i < 10; i++)
		hexes_castles_cust[i] = hexes_castles[i];
	for (i = 0; i < M_NUMLEVEL; i++)
		base_snow_cust[i] = base_snow[i];

	road_burg = road_burg_dec;
	place_burg = place_burg_dec;
	open_road = open_road_dec;
	forest_road = forest_road_dec;
	lilies = default_over;
	field_over = field_over_dec;
	bridges[0] = bridges_dec[0];
	bridges[1] = bridges_dec[1];
	bridges[2] = bridges_dec[2];

	if (parms_->roads_cust[0] != NULL)
		open_road = parms_->roads_cust[0];
	if (parms_->roads_cust[1] != NULL)
		forest_road = parms_->roads_cust[1];
	if (parms_->roads_cust[2] != NULL)
		road_burg = parms_->roads_cust[2];
	if (parms_->roads_cust[3] != NULL)
		place_burg = parms_->roads_cust[3];

	if (parms_->lilies_cust != NULL)
		lilies = parms_->lilies_cust;
	if (parms_->fields_cust != NULL)
		field_over = parms_->fields_cust;
	if (parms_->bridges_cust != NULL) {
		tmp = new char[10];
		strncpy(tmp, parms_->bridges_cust, 5);
		strncat(tmp, "|", 6);
		bridges[0] = tmp;
		tmp = new char[10];
		strncpy(tmp, parms_->bridges_cust, 5);
		strncat(tmp, "/", 6);
		bridges[1] = tmp;
		tmp = new char[10];
		strncpy(tmp, parms_->bridges_cust, 5);
		strncat(tmp, "\\", 6);
		bridges[2] = tmp;
	}

	if (parms_->type != 'c')
		return;

	for (i = 0; i < M_NUMLEVEL; i++)
		if (parms_->base_cust[i] != NULL)
			base_cust[i] = parms_->base_cust[i];
	for (i = 0; i < 12; i++)
		if (parms_->forest_cust[i] != NULL)
			forest_cust[i] = parms_->forest_cust[i];
	for (i = 0; i < 14; i++)
		if (parms_->houses_cust[i] != NULL)
			houses_cust[i] = parms_->houses_cust[i];
	for (i = 0; i < 10; i++)
		if (parms_->keeps_castles_cust[i] != NULL)
			keeps_castles_cust[i] = parms_->keeps_castles_cust[i];
	for (i = 0; i < 10; i++)
		if (parms_->hexes_castles_cust[i] != NULL)
			hexes_castles_cust[i] = parms_->hexes_castles_cust[i];
	for (i = 0; i < M_NUMLEVEL; i++)
		if (parms_->base_snow_cust[i] != NULL)
			base_snow_cust[i] = parms_->base_snow_cust[i];

}

//------------------ Computing map content -------------------

/*
 *** create_altitudes:

 This method creates altitudes using a simplistic fractal recursive algorithm.
 It works on a square grid (x,y) and adds a new point in the middle of every edge of the grid, plus one in the middle of the square.
 Altitudes of the created points are interpolated from original ones with some randomization. The random factor is tuned by the map parameter rough and the grid size.
 Adding a new point in the middle creates four squares. The method is recursively called on all these new squares.
 Stop condition is when edges size is 1. The initial square size must then be a power of two to avoid irregularities.
 It's possible to call the method on separate pieces of the map, even when previously calculated, least the altitudes have been zeroed first.

 -> x,y coordinates of a map square.
 <- none (altitudes are directly written in the map).
 */

void ya_mapgen::create_altitudes(unsigned int x, unsigned int xm, unsigned int y,
		unsigned int ym, unsigned int variation) {
	int off, o2, px = (xm - x) / 2;

	// calc altitudes of middle points x+px,y x,y+px x+px,y+px xm,y+px x+px,ym
	off = px * variation; // this is the random factor limit
	o2 = off / 2;

	if (map_[y][x + px]->alt == 0)
		map_[y][x + px]->alt = map_[y][x]->alt
		        + (map_[y][xm]->alt - map_[y][x]->alt) / 2 + m_rand(off) - o2;

	if (map_[y + px][x]->alt == 0)
		map_[y + px][x]->alt = map_[ym][x]->alt
		        + (map_[y][x]->alt - map_[ym][x]->alt) / 2 + m_rand(off) - o2;

	if (map_[y + px][xm]->alt == 0)
		map_[y + px][xm]->alt = map_[y][xm]->alt
		        + (map_[ym][xm]->alt - map_[y][xm]->alt) / 2 + m_rand(off) - o2;

	if (map_[ym][x + px]->alt == 0)
		map_[ym][x + px]->alt = map_[ym][xm]->alt
		        + (map_[ym][x]->alt - map_[ym][xm]->alt) / 2 + m_rand(off) - o2;

	if (map_[y + px][x + px]->alt == 0)
		map_[y + px][x + px]->alt = map_[y][x]->alt
		        + (map_[ym][xm]->alt - map_[y][x]->alt) / 2 + m_rand(off) - o2;

	// call on four squares if needed
	variation = (variation * parms_->rough) / 10;
	if (px > 1) {
		create_altitudes(x, x + px, y, y + px, variation);
		create_altitudes(x + px, xm, y, y + px, variation);
		create_altitudes(x, x + px, y + px, ym, variation);
		create_altitudes(x + px, xm, y + px, ym, variation);
	}
	return;
}

/**
 *** normalize_map:

 Recalc altitudes to make them fit in a range 0 - return value.
 */
int ya_mapgen::normalize_map() {
	int minA, maxA;
	unsigned int i, j, range;
	maxA = 0;
	minA = RAND_MAX / 2;

	for (i = 0; i < parms_->height; i++) {
		for (j = 0; j < parms_->width; j++) {
			if (map_[i][j]->alt > maxA)
				maxA = map_[i][j]->alt;
			if (map_[i][j]->alt < minA)
				minA = map_[i][j]->alt;
		}
	}

	for (i = 0; i < parms_->height; i++) {
		for (j = 0; j < parms_->width; j++) {
			map_[i][j]->alt -= minA;
		}
	}

	//--- next we compute the upper limit of each layer according to thickness parameters
	j = 0;
	range = maxA - minA;
	unsigned fact = 0;
	for (i = 0; i < M_NUMLEVEL; i++)
		fact += parms_->thickness[i];
	for (i = 0; i < M_NUMLEVEL; i++) {
		j = (range * parms_->thickness[i]) / fact + j;
		table_[i] = j;
	}
	return (range);
}

/**
 *** set_base_terrains:

 Assign base terrains and layers according to altitudes and various options.
 The map is ready to be used on exit and all further computing can be disabled
 -> range of possible altitudes.
 <- none.
 */

void ya_mapgen::set_base_terrains(int /*range*/)
{
	unsigned int lev;
	const char **base;
	unsigned int i, j, k;
	yamg_hex *h;

	//--- we get the landscape type to build the terrain table and ratios
	// p(olar) t(empered) m(editerranean) d(esert) e(quatorial)
	switch (parms_->type) {
	case 'p':
		base = base_polar;
		break;
	case 'm':
		base = base_medit;
		break;
	case 'd':
		base = base_desert;
		break;
	case 'e':
		base = base_trop;
		break;
	case 'c':
		base = base_cust;
		break;
	case 't':
	default:
		base = base_land;
		break;
	}

	//--- the snow limit altitude is defined here: not exactly a layer limit to blur the snow limit on the map and give a better look.
	lev = parms_->snowlev / 10;
	if (lev < M_NUMLEVEL - 1) {
		snow_limit_ = table_[lev]
				+ (((table_[lev + 1] - table_[lev]) * (parms_->snowlev % 10))
						/ 10);
	} else
		snow_limit_ = table_[M_NUMLEVEL - 1] + 100; // this ensure there will be no snow

	//--- we melt the terrain table with the snow table, i.e. terrains higher than snowlevel are replaced with their snowy counterpart
	for (i = 0; i < M_NUMLEVEL; i++) {
		if (i <= (lev + 1))
			terrains_[i] = base[i];
		else
			terrains_[i] = base_snow[i];
	}

	//--- we assign base terrain using terrain table, layers limits and snow limit.
	for (i = 0; i < siz_; i++) {
		for (j = 0; j < siz_; j++) {
			map_[i][j]->base = terrains_[M_NUMLEVEL - 1];
			map_[i][j]->layer = M_NUMLEVEL - 1;
			for (k = 0; k < M_NUMLEVEL; k++) {
				if (map_[i][j]->alt < table_[k]) {
					map_[i][j]->layer = k;
					if (map_[i][j]->alt > snow_limit_)
						map_[i][j]->base = base_snow[k];
					else
						map_[i][j]->base = terrains_[k];
					if (k <= YAMG_SHALLSEA)
						map_[i][j]->water_flag = true;
					break;
				}
			}
		}
	}

	if (riv_ != 0) {
		//--- we overload with water when asked. Only hexes with water > riv shall be changed. Slightly less values are converted to swamps.
		// Note water here has nothing to do with 'sea', i.e. layers 0 and 1, those are drawn in loop 5. Water substitution apply only on higher layers.
		lev++;
		bool lFlag = (parms_->type != 'd');
		for (i = 0; i < parms_->height; i++) {
			for (j = 0; j < parms_->width; j++) {
				if (map_[i][j]->water > riv_) {
					map_[i][j]->lock = YAMG_LIGHTLOCK; // this will allow terrain reuse for roads, but not for forests
					map_[i][j]->water_flag = true;
					if (map_[i][j]->alt > snow_limit_) {
						map_[i][j]->base = base_snow[YAMG_SHALLSEA];
					} else {
						map_[i][j]->base = terrains_[YAMG_SHALLSEA];
						if (lFlag && (map_[i][j]->water < riv_ + 2)
								&& (map_[i][j]->layer < lev))
							map_[i][j]->over = lilies; // makes lilyponds on really shallow waters.
					}
				} else {
					if (map_[i][j]->water > riv_ - 2) {
						map_[i][j]->lock = YAMG_LIGHTLOCK;

						if (map_[i][j]->alt > snow_limit_)
							map_[i][j]->base = base_snow[YAMG_SWAMPS];
						else
							map_[i][j]->base = terrains_[YAMG_SWAMPS];
					}
				}
			}
		}
		if (parms_->type == 'd') { // the special case of deserts. Rivers computation creates oasis, sand holes and dry brooks
			while (summits_ != NULL) {
				h = summits_;
				summits_ = h->next;
				h->next = NULL;
				if (h->water > riv_) {
					h->base = oasis_base;
					h->over = oasis_water;
					h->lock = YAMG_STRONGLOCK;
				} else if (h->water > riv_ - YAMG_DESERT_RIVER_OFFSET) {
					h->base = oasis_base;
					h->over = oasis_dry;
					h->lock = YAMG_LIGHTLOCK;
				}
			}
		}
	}
}

/**
 *** make_rivers:

 The algorithm simulate water collection from higher terrains.
 Starts with all hexes 'water' member to 0.
 Parse the list. If some terrain has 0 value, then stack all higher hexes.
 for each hex in the stack: if its value is set to 0 stack all higher hexes as previous
 if not, check to see if it has another connected hex lower than it. If so, it contribute for nothing, if not, add the water value to pointed hex.
 Add 1 to water value, and go to next hex.

 When all hexes have been parsed, terrain replacement will occur. Only terrain which water is greater than water limit shall be taken in account, this to allow more or less
 wet terrains.
 */

void ya_mapgen::make_rivers() {
	unsigned int i, j;
	yamg_hex *h;

	// first, calculate the water flood.
	summits_ = NULL;

	for (i = 0; i < parms_->height; i++) {
		for (j = 0; j < parms_->width; j++) {
			h = map_[i][j];
			if (!h->done && (h->water == 0)
					&& (h->alt > table_[YAMG_SHALLSEA])) {
				calc_water_contribs(h);
			}
		}
	}

	if (parms_->type != 'd') {
		clear_done_flag();
		while ((h = heap_->pick_hex()) != NULL) {
			h->next = summits_;
			summits_ = h;
		}
		while (summits_ != NULL) {
			h = summits_;
			summits_ = h->next;
			h->next = NULL;
			erode_terrains(h, h->water);
		}
	}
	clear_done_flag();
}

/**
 *** calc_water_contribs:

 This recursive function compute the water flood from higher hexes
 */

int ya_mapgen::calc_water_contribs(yamg_hex *h) {
	neighbors vo;
	yamg_hex *dest;

	if (h->done)
		return 0;
	get_neighbors(h, &vo);
	dest = h;

	// find dest of our water
	if ((vo.ne != NULL) && (vo.ne->alt < dest->alt))
		dest = vo.ne;
	if ((vo.nw != NULL) && (vo.nw->alt < dest->alt))
		dest = vo.nw;
	if ((vo.no != NULL) && (vo.no->alt < dest->alt))
		dest = vo.no;
	if ((vo.se != NULL) && (vo.se->alt < dest->alt))
		dest = vo.se;
	if ((vo.sw != NULL) && (vo.sw->alt < dest->alt))
		dest = vo.sw;
	if ((vo.so != NULL) && (vo.so->alt < dest->alt))
		dest = vo.so;

	if (dest == h) {
		h->key = h->alt;
		heap_->add_hex(h);
		h->list = NULL; // if we can't give this water to another hex, add this hex to the 'summits' list which will be managed in erode()
	} else {
		h->list = dest;
	}

	// calc our water value
	h->water = 1;

	if ((vo.ne != NULL) && (vo.ne->alt > h->alt)) {
		dest = vo.ne;
		if (!dest->done) {
			if (dest->water == 0)
				calc_water_contribs(dest);
			if (dest->list == h) {
				h->water += dest->water;
				dest->done = true;
			}
		}
	}

	if ((vo.nw != NULL) && (vo.nw->alt > h->alt)) {
		dest = vo.nw;
		if (!dest->done) {
			if (dest->water == 0)
				calc_water_contribs(dest);
			if (dest->list == h) {
				h->water += dest->water;
				dest->done = true;
			}
		}
	}

	if ((vo.no != NULL) && (vo.no->alt > h->alt)) {
		dest = vo.no;
		if (!dest->done) {
			if (dest->water == 0)
				calc_water_contribs(dest);
			if (dest->list == h) {
				h->water += dest->water;
				dest->done = true;
			}
		}
	}

	if ((vo.se != NULL) && (vo.se->alt > h->alt)) {
		dest = vo.se;
		if (!dest->done) {
			if (dest->water == 0)
				calc_water_contribs(dest);
			if (dest->list == h) {
				h->water += dest->water;
				dest->done = true;
			}
		}
	}

	if ((vo.sw != NULL) && (vo.sw->alt > h->alt)) {
		dest = vo.sw;
		if (!dest->done) {
			if (dest->water == 0)
				calc_water_contribs(dest);
			if (dest->list == h) {
				h->water += dest->water;
				dest->done = true;
			}
		}
	}

	if ((vo.so != NULL) && (vo.so->alt > h->alt)) {
		dest = vo.so;
		if (!dest->done) {
			if (dest->water == 0)
				calc_water_contribs(dest);
			if (dest->list == h) {
				h->water += dest->water;
				dest->done = true;
			}
		}
	}
	return h->water;
}

/**
 *** erode_terrains:

 Hexes from the 'summits' list must be turned into lakes or rivers, letting the water run to the sea (or at least some other low point).
 */
void ya_mapgen::erode_terrains(yamg_hex *h, int /*report*/)
{
	neighbors vo;
	int lAlt;
	yamg_hex *nex, *stack;
	unsigned int cumul, n, val, oldval;

	if (h->done)
		return;

	h->list = NULL;
	stack = h;
	lAlt = h->alt;
	cumul = h->water;
	n = 1;
	oldval = val = cumul;
	heap_->clear_heap();

	while (val > riv_) {
		// we seek the lower neighbour here, or an existing lake, or the map border
		get_neighbors(h, &vo);
		if (vo.no != NULL) {
			vo.no->key = vo.no->alt;
			heap_->add_hex(vo.no);
		}
		if (vo.nw != NULL) {
			vo.nw->key = vo.nw->alt;
			heap_->add_hex(vo.nw);
		}
		if (vo.ne != NULL) {
			vo.ne->key = vo.ne->alt;
			heap_->add_hex(vo.ne);
		}
		if (vo.sw != NULL) {
			vo.sw->key = vo.sw->alt;
			heap_->add_hex(vo.sw);
		}
		if (vo.se != NULL) {
			vo.se->key = vo.se->alt;
			heap_->add_hex(vo.se);
		}
		if (vo.so != NULL) {
			vo.so->key = vo.so->alt;
			heap_->add_hex(vo.so);
		}

		nex = heap_->pick_hex(); // get the lower adjacent hex
		if ((nex == NULL) || (nex->water == 0)) // water == 0 means we are on the lower layers of the map, i.e. the sea
			break;

		nex->list = stack; // this stack store all the hexes of the future lake
		stack = nex;
		lAlt = nex->alt;
		oldval = val;
		cumul += nex->water;
		n++;
		val = cumul / (n / 2); // which share the total water amount. Stop condition is when there is not enough water to draw a lake
		h = nex;
	}

	// When done, update all hexes from the stack to their new altitude and water value (and marking them as done to avoid their reuse)
	h = stack;
	while (h != NULL) {
		h->water = oldval;
		h->alt = lAlt;
		h->done = true;
		nex = h->list;
		h->list = NULL;
		h = nex;
	}
}

/**
 *** make_burgs:

 Burgs are little agglomerations with a crossing road and, maybe, some embellishments.
 Burgs and castles are start points for roads. So their position is kept in the 'endpoints' list.
 */

void ya_mapgen::make_burgs() {
	yamg_hex *it = NULL;
	int num, n, essais, i, j;
	burg b;
	const char **treeTable;

	switch (parms_->type) {
	case 'd':
		treeTable = houses_desert;
		break;
	case 'p':
		treeTable = houses_polar;
		break;
	case 'm':
		treeTable = houses_medit;
		break;
	case 'e':
		treeTable = houses_trop;
		break;
	case 'c':
		treeTable = houses_cust;
		break;
	case 't':
	default:
		treeTable = house_burg;
		break;
	}

	essais = (parms_->height * parms_->width * parms_->burgs); // since the random hex selection can result in an endless loop, we set an arbitrary limit
	num = parms_->burgs;
	i = 0;
	j = 0;

	while ((essais-- > 0) && (num > 0)) {
		i = ((i + m_rand(parms_->height)) % (parms_->height - 3));
		j = ((j + m_rand(parms_->width)) % (parms_->width - 3));
		if ((i < 2) || (j < 2))
			continue;
		it = map_[i][j];

		// select only layer ground and hills hexes, not already covered with something else
		if ((it->layer < YAMG_MOUNTAINS) && (it->layer >= YAMG_GROUND)
				&& (it->lock == YAMG_UNLOCK)) {
			heap_->clear_heap();

			// first, we try to find more hexes to setup a bit of road
			store_neighbors(it, YAMG_GROUND, YAMG_MOUNTAINS);
			if (heap_->last_ < 3)
				continue; // we're not in a good place here !

			b.center = it;
			it->done = true;
			b.road1 = heap_->pick_hex();
			b.road2 = heap_->pick_hex();
			store_neighbors(b.road1, YAMG_GROUND, YAMG_MOUNTAINS);
			store_neighbors(b.road2, YAMG_GROUND, YAMG_MOUNTAINS);

			if (heap_->last_ < 3) // is the place large enough ?
				continue; // resolutely not !

			// here, we can start building the burg.
			n = m_rand(MAX_HOUSESINBURG - MIN_HOUSESINBURG) + MIN_HOUSESINBURG;
			while ((n-- > 0) && (heap_->last_ > 0)) { // these shall be the houses
				it->next = heap_->pick_hex();
				it = it->next;
				it->next = NULL;
			}
			b.center->base = place_burg;
			b.center->lock = YAMG_STRONGLOCK;
			b.road1->base = road_burg;
			b.road1->lock = YAMG_STRONGLOCK;
			b.road2->base = road_burg;
			b.road2->lock = YAMG_STRONGLOCK;

			it = b.center->next;
			b.road2->next = b.center->next; // this to create the complete list and avoid further overloads
			b.road1->next = b.road2;
			b.center->next = b.road1;

			b.road2->list = endpoints_; // this to create the complete list and avoid further overloads
			b.road1->list = b.road2;
			b.center->list = b.road1; // add this one to the road endpoints list
			endpoints_ = b.center;

			// use the list to setup the houses
			while (it != NULL) {
				it->lock = YAMG_LIGHTLOCK; //YAMG_HARDLOCK; // YAMG_LIGHTLOCK;
				n = (it->alt > snow_limit_) ? 1 : 0;
				if (it->layer == 4)
					it->over = treeTable[n + 6];
				else
					it->over = treeTable[n + 10];
				it = it->next;
			}
			while (heap_->last_ > 0) { // extract some more hexes to put field embellishments if possible
				it = heap_->pick_hex();
				it->lock = YAMG_LIGHTLOCK; // we lock it modified or not to ensure reproductibility with or without snow
				if (*it->base == 'G') {
					it->over = field_over;
				}
			}
			num--;
		}
	}
}

/**
 *** make_castles:

 Castles are keeps with some castle hexes. They must be adjacent. They should be not too close other castles.
 */
void ya_mapgen::make_castles() {
	yamg_hex *ca, *it = NULL;
	int num, n, k, essais, i, j, miniDist;

	const char **treeTableK;
	const char **treeTableH;

	switch (parms_->type) {
	case 'd':
		treeTableK = keeps_castles_desert;
		treeTableH = hexes_castles_desert;
		break;
	case 'p':
		treeTableK = keeps_castles_polar;
		treeTableH = hexes_castles_polar;
		break;
	case 'm':
		treeTableK = keeps_castles_medit;
		treeTableH = hexes_castles_medit;
		break;
	case 'e':
		treeTableK = keeps_castles_trop;
		treeTableH = hexes_castles_trop;
		break;
	case 'c':
		treeTableK = keeps_castles_cust;
		treeTableH = hexes_castles_cust;
		break;
	case 't':
	default:
		treeTableK = keeps_castles;
		treeTableH = hexes_castles;
		break;
	}

	castles_ = NULL;
	essais = (parms_->height * parms_->width * parms_->players); // since the random hex selection can result in an endless loop, we set an arbitrary limit
	num = parms_->players;
	miniDist = (parms_->width + parms_->height) / (parms_->players + 1);
	i = 0;
	j = 0;

	while ((essais-- > 0) && (num > 0)) {
		i = ((i + m_rand(parms_->height)) % (parms_->height - 5)) + 2;
		j = ((j + m_rand(parms_->width)) % (parms_->width - 5)) + 2;
		it = map_[i][j];

		// select only if not too close to another castle
		ca = castles_;
		while (ca != NULL) {
			const int dx = ca->x - it->x;
			const int dy = ca->y - it->y;
			if ((abs(dx) + abs(dy)) > miniDist) {
				ca = ca->next;
			} else
				break;
		}
		if (ca != NULL)
			continue;

		// select only layer beach - mountains hexes, not already covered with something else
		if ((it->layer < YAMG_IMPMOUNTS) && (it->layer >= YAMG_BEACH)
				&& (it->lock < YAMG_LIGHTLOCK)) {
			heap_->clear_heap();
			// first, we try to find more hexes to setup the keep
			store_neighbors(it, it->layer, it->layer + 1);
			n =
					(parms_->casthexes == 0) ?
			                m_rand(
									MAX_HEXESINCASTLE - MIN_HEXESINCASTLE) + MIN_HEXESINCASTLE :
							parms_->casthexes;

			if (heap_->last_ < 3)
				continue; // we're not in a good place here !

			it->done = true;
			k = ((it->layer - 2) * 2);
			k += (it->alt > snow_limit_) ? 1 : 0;
			it->base = treeTableK[k];
			it->over = default_over;
			if (num < 10) // because Wesnoth editor don't accept more than 9 players
				it->player = num;
			it->lock = YAMG_STRONGLOCK;
			it->next = castles_; // add it to castles list
			castles_ = it;
			it->list = endpoints_;   // add it to the road startpoints
			endpoints_ = it;

			while (n > 0) {
				it = heap_->pick_hex();
				if (it != NULL) {
					if (heap_->last_ < (n - 1)) { // we need some more hexes here
						store_neighbors(it, it->layer, it->layer + 1);
					}
					it->base = treeTableH[k];
					it->over = default_over;
					it->lock = YAMG_STRONGLOCK;
				} else
					break;
				n--;
			}
			num--;
		}
	}
}

/**
 *** store_neighbors:

 Store neighbours of it in the heap, filtering on layers
 */
void ya_mapgen::store_neighbors(yamg_hex *it, unsigned int layMin,
		unsigned int layMax) {
	neighbors p;
	unsigned int x, y, m, z;

	p.center = it;
	x = it->x - 1;
	y = it->y - 1;
	m = x % 2;
	z = parms_->height;

	if (y < 3) {
		p.no = NULL;
	} else {
		p.no = map_[y - 1][x];
	}
	if (y >= parms_->height - 2) {
		p.so = NULL;
	} else {
		p.so = map_[y + 1][x];
	}

	if (x < 3) {
		p.nw = p.sw = NULL;
	} else {
		if(y < m)
			p.nw =NULL;
		else
			p.nw = map_[y - m][x - 1];
		if (y >= (z + m))
			p.sw = NULL;
		else
			p.sw = map_[y - m + 1][x - 1];
	}
	if (x >= parms_->width - 2) {
		p.ne = p.se = NULL;
	} else {
		if(y < m)
			p.ne = NULL;
		else
			p.ne = map_[y - m][x + 1];
		if ( y >= (z + m) )
			p.se = NULL;
		else
			p.se = map_[y - m + 1][x + 1];
	}

	if ((p.no != NULL) && (p.no->lock < YAMG_LIGHTLOCK)
			&& (p.no->layer >= layMin) && (p.no->layer < layMax)) {
		p.no->key = abs(it->alt - p.no->alt);
		heap_->add_hex(p.no);
	}

	if ((p.ne != NULL) && (p.ne->lock < YAMG_LIGHTLOCK)
			&& (p.ne->layer >= layMin) && (p.ne->layer < layMax)) {
		p.ne->key = abs(it->alt - p.ne->alt);
		heap_->add_hex(p.ne);
	}

	if ((p.nw != NULL) && (p.nw->lock < YAMG_LIGHTLOCK)
			&& (p.nw->layer >= layMin) && (p.nw->layer < layMax)) {
		p.nw->key = abs(it->alt - p.nw->alt);
		heap_->add_hex(p.nw);
	}

	if ((p.so != NULL) && (p.so->lock < YAMG_LIGHTLOCK)
			&& (p.so->layer >= layMin) && (p.so->layer < layMax)) {
		p.so->key = abs(it->alt - p.so->alt);
		heap_->add_hex(p.so);
	}

	if ((p.se != NULL) && (p.se->lock < YAMG_LIGHTLOCK)
			&& (p.se->layer >= layMin) && (p.se->layer < layMax)) {
		p.se->key = abs(it->alt - p.se->alt);
		heap_->add_hex(p.se);
	}

	if ((p.sw != NULL) && (p.sw->lock < YAMG_LIGHTLOCK)
			&& (p.sw->layer >= layMin) && (p.sw->layer < layMax)) {
		p.sw->key = abs(it->alt - p.sw->alt);
		heap_->add_hex(p.sw);
	}
}

/**
 *** make_forests:

 First we define the overlays to use, and next choose an hex at random and flood its neighbours till count is over.
 */
void ya_mapgen::make_forests() {
	yamg_hex *it = NULL;
	int num, n, essais, i, j;
	const char **treeTable;
	switch (parms_->type) {
	case 'd':
		treeTable = forest_desert;
		break;
	case 'p':
		treeTable = forest_polar;
		break;
	case 'm':
		treeTable = forest_medit;
		break;
	case 'e':
		treeTable = forest_trop;
		break;
	case 'c':
		treeTable = forest_cust;
		break;
	case 't':
	default:
		treeTable = forest_land;
		break;
	}

	essais = (parms_->height * parms_->width * parms_->forests); // since the random hex selection can result in an endless loop, we set an arbitrary limit
	num = essais / 100;
	//--- forests can be many little woods or wider areas: the rough parameter tunes this, splitting forests in many little parts if high
	n = num / parms_->rough;
	i = 0;
	j = 0;

	while (essais-- > 0) {
		i = (i + m_rand(parms_->height)) % parms_->height;
		j = (j + m_rand(parms_->width)) % parms_->width;

		// select only layer ground & hills hexes, not already covered with forests or something else
		if (!map_[i][j]->done && (map_[i][j]->layer < YAMG_MOUNTAINS)
				&& (map_[i][j]->layer >= YAMG_GROUND)
				&& (map_[i][j]->lock < YAMG_LIGHTLOCK)) {
			it = map_[i][j];
			num = num - n + fill_with(treeTable, it, n);
			if (num <= 0)
				return;
		}
	}
}

/**
 *** fill_with:
 Filling procedure for previous makeForests: this fills an area of layer 3 & 4 terrains with forest overlays.
 It stores adjacent hexes of h in the heap, introducing the constraint to be as leveled as possible.
 */
int ya_mapgen::fill_with(const char *over[], yamg_hex *h, int numb) {
	neighbors v;
	yamg_hex *it;
	int lb, lm1, lm2, ls, k, z;

	heap_->last_ = 0; // this reset the heap
	h->key = 0;
	heap_->add_hex(h);

	// this cut layer 2 & 3 in three, defining 3 levels. This to avoid forest limits being the same as layers limits (not realistic)
	lb = table_[YAMG_SWAMPS];
	ls = table_[YAMG_HILLS];
	lm1 = (ls - lb) / 3;
	lm2 = lb + (2 * lm1);
	lm1 += lb;

	switch (parms_->season) {
	case 's':
	case 'e':
		z = 0;
		break;
	case 'a':
	case 'f':
		z = 1;
		break;
	case 'w':
		z = 2;
		break;
	default:
		z = 0;
		break;
	}

	// using a heap to store neighbors allows to privilegiate expansion on close altitude hexes. This gives a more realistic result than expanding equally in all directions.
	// we stop if no more neighbors are available or we've done enough.
	while ((heap_->last_ > 0) && (--numb > 0)) {
		it = heap_->pick_hex();
		if (it->alt > lm2)
			k = 8;
		else if (it->alt > lm1)
			k = 4;
		else
			k = 0;

		if (it->alt > snow_limit_)
			k += 3;
		else
			k += z;
		it->over = over[k];

		get_neighbors(it, &v);

		if ((v.no != NULL) && (v.no->lock < YAMG_LIGHTLOCK) && (v.no->alt >= lb)
				&& (v.no->alt < ls)) {
			v.no->key = abs(it->alt - v.no->alt);
			heap_->add_hex(v.no);
		}

		if ((v.ne != NULL) && (v.ne->lock < YAMG_LIGHTLOCK) && (v.ne->alt >= lb)
				&& (v.ne->alt < ls)) {
			v.ne->key = abs(it->alt - v.ne->alt);
			heap_->add_hex(v.ne);
		}

		if ((v.nw != NULL) && (v.nw->lock < YAMG_LIGHTLOCK) && (v.nw->alt >= lb)
				&& (v.nw->alt < ls)) {
			v.nw->key = abs(it->alt - v.nw->alt);
			heap_->add_hex(v.nw);
		}

		if ((v.so != NULL) && (v.so->lock < YAMG_LIGHTLOCK) && (v.so->alt >= lb)
				&& (v.so->alt < ls)) {
			v.so->key = abs(it->alt - v.so->alt);
			heap_->add_hex(v.so);
		}

		if ((v.se != NULL) && (v.se->lock < YAMG_LIGHTLOCK) && (v.se->alt >= lb)
				&& (v.se->alt < ls)) {
			v.se->key = abs(it->alt - v.se->alt);
			heap_->add_hex(v.se);
		}

		if ((v.sw != NULL) && (v.sw->lock < YAMG_LIGHTLOCK) && (v.sw->alt >= lb)
				&& (v.sw->alt < ls)) {
			v.sw->key = abs(it->alt - v.sw->alt);
			heap_->add_hex(v.sw);
		}

	}
	heap_->clear_heap();
	return numb;
}

/**
 *** make_houses:

 Creates individual houses (Wesnoth villages)
 */

void ya_mapgen::make_houses() {
	yamg_hex *it = NULL;
	int num, n, essais, i, j, incx, incy;
	const char **treeTable;
	switch (parms_->type) {
	case 'd':
		treeTable = houses_desert;
		break;
	case 'p':
		treeTable = houses_polar;
		break;
	case 'm':
		treeTable = houses_medit;
		break;
	case 'e':
		treeTable = houses_trop;
		break;
	case 'c':
		treeTable = houses_cust;
		break;
	case 't':
	default:
		treeTable = houses_land;
		break;
	}

	essais = (parms_->height * parms_->width * parms_->vill); // since the random hex selection can result in an endless loop, we set an arbitrary limit
	num = parms_->vill;
	incx = (4 * parms_->width) / 5;
	incy = (4 * parms_->height) / 5;
	i = 0;
	j = 0;
	clear_done_flag();

	while ((essais-- > 0) && (num > 0)) {
		i = ((i + (incx) + m_rand(10)) % (parms_->height - 1));
		j = ((j + (incy) + m_rand(10)) % (parms_->width - 1));
		if ((i < 2) || (j < 2))
			continue;
		it = map_[i][j];

		// select only layer 1 - 5 hexes, not already covered with something else
		if ((it->layer < YAMG_MOUNTAINS) && (it->layer >= YAMG_SHALLSEA)
				&& (it->lock < YAMG_LIGHTLOCK) && !it->done) {
			switch (it->layer) {
			case YAMG_SHALLSEA:
			case YAMG_BEACH:
			case YAMG_SWAMPS:
				n = (it->layer * 2) - 2;
				break;
			case YAMG_GROUND:
				n = (it->layer * 2) - 2;
				if ((*it->over == 'F') || (*it->over == 'E'))
					n += 2;
				break;
			case YAMG_HILLS:
				n = 10;
				if ((*it->over == 'F') || (*it->over == 'E'))
					n += 2;
				break;
			default:
				n = 6;
				break;
			}
			n = (it->alt > snow_limit_) ? (n + 1) : n; // this is snow management
			it->over = treeTable[n];
			it->done = true;
			it->lock = YAMG_HARDLOCK;
			num--;
		}
	}
}

/**
 *** make_roads:

 Makes the roads between burgs and castles

 This algorithm find roads building on the map a tree whose root is the first hex in the 'endpoints' list. The tree is built using the 'road' pointer: each hex point to its
 parent, i.e. an adjacent hex closer than it to the root. This tree is built replacing each leave with adjacent hexes not already visited and not locked. The leaves are inserted
 in a heap with a key (distance cost) computed as parent_key + terrain factor + slope factor. This ensure every child has a key
 greater than its parent. The respective weight of terrain factor allow to create more or less bridges, more or less mountains crossing and so on.
 The next leave to extend is pulled out the heap. This ensure the tree grows much faster on low cost terrains.

 When the tree reaches another hex in the 'endpoints' list, we have certainly found the shortest road to the root, since every other leaves have a greater key. Shortest means
 the least cost path according to cost computation, not the shortest geometric path.

 We then mark the stop point as hit, and draw the road using the 'road' pointer back till we reach the tree root.

 */

void ya_mapgen::make_roads() {
	yamg_hex *it, *l, *ro;
	int cost = 0, nStop = 0, costW = parms_->bridges * 3;
	bool bFlag;

	int st_flat = parms_->ro_road;
	int st_hills = parms_->ro_road + 1;
	int st_mounts = parms_->ro_road;
	int st_swamps = parms_->ro_road + 5;

	//1- select the start points and road parts to create the loop. This can be done using a road flag: 0 is nothing, 1 is road, 2 is stop point and more counts the hits.
	it = endpoints_;
	while (it != NULL) {
		it->road = NULL;
		it->key = 0;
		it->road_type = R_STOP;
		it->lock = YAMG_STRONGLOCK;
		it = it->list;
		nStop++;
	}
	if (nStop-- < 2)
		return; // obviously, we can't build roads with less than two points.

	bFlag = (costW > 0);

	if (bFlag) // negative values of bridges disable drawing bridges but not water crossing computation
		costW = parms_->bridges * 3;
	else
		costW = -(parms_->bridges * 3);

	while (nStop > 0) {
		// search a root point in the 'endpoints' list
		it = endpoints_;
		while ((it->road_type != R_STOP) && (it->list != NULL))
			it = it->list;

		if (it->list == NULL)
			return;

		//2- put the root point into the heap with key = 0
		clear_done_flag();
		heap_->clear_heap();
		heap_->add_hex(it);
		it->road_type = R_HIT; // the root point is already hit.

		while ((nStop > 0) && (heap_->last_ > 0)) { // the stop condition is when all start/stop points have been reached, or the heap is empty (some roads are impossible).
			//3- get a leave from the heap
			it = heap_->pick_hex();
			l = sel_neigh(it); // and the next candidate leaves

			while (l != NULL) { // for each one
				//
				switch (l->road_type) {

				case R_NONE:
					if ((*l->base == 'C') || (*l->base == 'K'))
						cost = 0; // if we are in a castle, just cross it
					else {
						switch (l->layer) { // compute the movement cost
						case YAMG_DEEPSEA:
						case YAMG_IMPMOUNTS:
							l = l->next;
							continue; // no roads on deep water and impassable mountains, discard this hex
						case YAMG_SHALLSEA:
							cost = costW;
							break;
						case YAMG_BEACH:
						case YAMG_SWAMPS:
							cost = st_swamps
									+ (abs(l->alt - it->alt)
											/ YAMG_ALTWEIGHT_SWAMP);
							break;
						case YAMG_MOUNTAINS:
							cost = st_mounts
									+ (abs(l->alt - it->alt)
											/ YAMG_ALTWEIGHT_MOUNTAINS);
							break;
						case YAMG_GROUND:
							cost = st_flat
									+ (abs(l->alt - it->alt)
											/ YAMG_ALTWEIGHT_GROUND);
							break;
						case YAMG_HILLS:
							cost = st_hills
									+ (abs(l->alt - it->alt)
											/ YAMG_ALTWEIGHT_HILLS);
							break;
						}
					}
					if (l->water_flag)
						cost = costW;

					// if 'it' is a bridge, we must align it, it->road and l to get a good looking result
					if (bFlag && (it->road != NULL) && (it->water_flag)) {
						if (((l->y - it->road->y) == 0)
								|| (abs(static_cast<int>(l->x) - static_cast<int>(it->road->x)) == 1)) {
							cost = costW * 10000; // discard this point
						}
					}

					l->road = it;   // link to its parent
					l->key = it->key + cost;
					break;

				case R_STOP:   // target point is adjacent to it.
					nStop--;
					// no break on purpose !!!
				case R_ROAD: // we are on a road already opened
					l->road_type = R_HIT;
					ro = it;
					while (ro != NULL) { // creates the road back to root point.

						// Bridges creation.
						// It can't work in all situation because bridges can connect two (ore more) hexes correctly only if they are aligned.

						if (ro->water_flag) { // test if bridges are needed when base terrain is water and bridges are requested
							if (bFlag) {
								if (ro->road == NULL)
									break;
								if (ro->x == ro->road->x) {
									ro->over = bridges[0];
								} else {
									bool tst =
											(ro->y == ro->road->y) ?
													(ro->x < ro->road->x) :
													!(ro->x < ro->road->x);
									tst = ((ro->x % 2) == 0) ? tst : !tst;
									ro->over = tst ? bridges[2] : bridges[1];
								}
							} else
								ro->base = fords; // fords replace bridges if bridges are disabled
						} else {
							if (ro->lock < YAMG_STRONGLOCK) {
								if (*ro->over == 'F')
									ro->base = forest_road;
								else
									ro->base = open_road;
								ro->over = default_over; // clear all overlays (particularly forests).
							}
						}
						ro->road_type = R_ROAD;
						ro->lock = YAMG_HARDLOCK;
						ro = ro->road;
					}
					break;

				case R_HIT: // already hit points don't need to be added

				default:
					l->done = true;
					break;
				}
				heap_->add_hex(l); // add the hex in the heap.
				l = l->next;
			}
		}
		//4- if the result is not satisfactory (nStop > 0), we restart from another root, not already hit.
	}
}

/**
 *** sel_neigh:
 Get the usable neighbours for roads and castles
 -> the source hex
 <- a list built with the 'next' pointer, NULL if no result
 */
yamg_hex *ya_mapgen::sel_neigh(yamg_hex *h) {
	yamg_hex *it, *l, *res = NULL;
	unsigned int x, y, m;

	x = h->x - 1;
	y = h->y - 1;
	m = x % 2;

	if (y > 2) {
		it = map_[y - 1][x];
		if ((it->road == NULL) && (it->lock < YAMG_HARDLOCK)) {
			it->next = res;
			res = it;
		}
		it = map_[y + 1][x];
		if ((it->road == NULL) && (it->lock < YAMG_HARDLOCK)) {
			it->next = res;
			res = it;
		}
	}

	if ((x > 2) && (y > 1)) {
		it = map_[y - m][x - 1];
		if ((it->road == NULL) && (it->lock < YAMG_HARDLOCK)) {
			it->next = res;
			res = it;
		}
		it = map_[y - m][x + 1];
		if ((it->road == NULL) && (it->lock < YAMG_HARDLOCK)) {
			it->next = res;
			res = it;
		}
 		it = map_[y - m + 1][x - 1];
		if ((it->road == NULL) && (it->lock < YAMG_HARDLOCK)) {
			it->next = res;
			res = it;
		}
		it = map_[y - m + 1][x + 1];
		if ((it->road == NULL) && (it->lock < YAMG_HARDLOCK)) {
			it->next = res;
			res = it;
		}
	}
	it = res; // check limits
	l = NULL;
	while (it != NULL) {
		if ((it->x < 1) || (it->x >= parms_->width) || (it->y < 1)
		        || (it->y >= parms_->height)) {
			if (it == res)
				res = it->next;
			else {
				l->next = it->next;
			}
		}
		l = it;
		it = it->next;
	}
	return res;
}

//------------------ Utils -----------------
/**
 This utility clears the 'done' flag on all map hexes
 */
void ya_mapgen::clear_done_flag() {
	for (unsigned int i = 0; i < parms_->height; i++) {
		for (unsigned int j = 0; j < parms_->width; j++) {
			map_[i][j]->done = false;
		}
	}
}

/**
 This utility returns all neighbours of the defined hex in a struct 'voisins' given in parameter.
 -> the pointed hex.
 -> the result struct to fill.
 <- none.
 */
void ya_mapgen::get_neighbors(yamg_hex *h, neighbors *p) {
	unsigned int x, y, m;
	int k, z;

	p->center = h;
	x = h->x - 1;
	y = h->y - 1;
	m = x % 2;
	k = y - m;
	z = static_cast<int>(parms_->height);

	if (y < 1) {
		p->no = NULL;
	} else {
		p->no = map_[y - 1][x];
	}
	if (y >= parms_->height - 2) {
		p->so = NULL;
	} else {
		p->so = map_[y + 1][x];
	}

	if (x < 1) {
		p->nw = p->sw = NULL;
	} else {
		if (k < 0)
			p->nw = NULL;
		else
			p->nw = map_[y - m][x - 1];
		if (k >= z)
			p->sw = NULL;
		else
			p->sw = map_[y - m + 1][x - 1];
	}
	if (x >= parms_->width - 2) {
		p->ne = p->se = NULL;
	} else {
		if (k < 0)
			p->ne = NULL;
		else
			p->ne = map_[y - m][x + 1];
		if (k >= z)
			p->se = NULL;
		else
			p->se = map_[y - m + 1][x + 1];
	}
}

/**
    This utility returns a random number 0 to limit
    The compile flag allows to use either the system RNG or the embedded one
*/
int m_rand(int limit) {
	if(limit == 0)
		return 0;
#ifndef INTERN_RAND
	unsigned int n = rand();
#else
	unsigned int n = genrand();
#endif
   return n % limit;
}

/**
    Initialize the RNG
*/
void init_rand(unsigned int seed) {
#ifndef INTERN_RAND
	srand(seed);
#else
	init_genrand(seed);
#endif
}

/**
    The embedded RNG
*/
#ifdef INTERN_RAND
/*
    This is an adapted Mersenne Twister pseudorandom number generator
    based on code by:
    Makoto Matsumoto and Takuji Nishimura,

    See full version at:
    http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/emt.html
*/

/* Period parameters */
#define N 624
#define M 397
#define MATRIX_A 0x9908b0dfUL   /* constant vector a */
#define UPPER_MASK 0x80000000UL /* most significant w-r bits */
#define LOWER_MASK 0x7fffffffUL /* least significant r bits */

static unsigned long mt[N]; /* the array for the state vector  */
static int mti=N+1; /* mti==N+1 means mt[N] is not initialized */

/* initializes mt[N] with a seed */
void init_genrand(unsigned long s)
{
	mt[0]= s & 0xffffffffUL;
	for (mti=1; mti<N; mti++) {
		mt[mti] =
		(1812433253UL * (mt[mti-1] ^ (mt[mti-1] >> 30)) + mti);
		/* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
		/* In the previous versions, MSBs of the seed affect   */
		/* only MSBs of the array mt[].                        */
		/* 2002/01/09 modified by Makoto Matsumoto             */
		mt[mti] &= 0xffffffffUL;
		/* for >32 bit machines */
	}
}

/* generates a random number on [0,0xffffffff]-interval */
unsigned long genrand(void)
{
	unsigned long y;
	static unsigned long mag01[2]={0x0UL, MATRIX_A};
	/* mag01[x] = x * MATRIX_A  for x=0,1 */

	if (mti >= N) { /* generate N words at one time */
		int kk;

		if (mti == N+1)   /* if init_genrand() has not been called, */
			init_genrand(5489UL); /* a default initial seed is used */

		for (kk=0;kk<N-M;kk++) {
			y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
			mt[kk] = mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1UL];
		}
		for (;kk<N-1;kk++) {
			y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
			mt[kk] = mt[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
		}
		y = (mt[N-1]&UPPER_MASK)|(mt[0]&LOWER_MASK);
		mt[N-1] = mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1UL];

		mti = 0;
	}

	y = mt[mti++];

	/* Tempering */
	y ^= (y >> 11);
	y ^= (y << 7) & 0x9d2c5680UL;
	y ^= (y << 15) & 0xefc60000UL;
	y ^= (y >> 18);

	return y;
}
#endif
