#ifndef SPRITESHEET_HPP_INCLUDED
#define SPRITESHEET_HPP_INCLUDED

class config;

#include <string>

/**
 * @todo this is a proof-of-concept version and undocumented. It should be
 * documented later and also allow WML to use spritesheets.
 */

/** Contains the definition of a spritesheet's individual sprites for a unit(type). */
struct sprite_data {

	sprite_data(const config& cfg);

	//enum location { X_COOR, Y_COOR, WIDTH, HEIGHT };  replace with array?

	std::string image;
	
	// if moved to array place these 5 values into it?
	int id;
	int x_coordinate;
	int y_coordinate;
	int height;
	int width;
};

#endif
