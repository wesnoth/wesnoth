#ifndef MAP_GEN_HPP_INCLUDED
#define MAP_GEN_HPP_INCLUDED

#include "config.hpp"
#include "display.hpp"

#include <string>

std::string random_generate_map(const std::string& parms);

class map_generator
{
public:

	//returns true iff the map generator has an interactive screen
	//which allows the user to modify how the generator behaves
	virtual bool allow_user_config() const = 0;

	//display the interactive screen which allows the user to
	//modify how the generator behaves. (This function will not
	//be called if allow_user_config() returns false)
	virtual void user_config(display& disp) = 0;

	//returns a string identifying the generator by name. The name should
	//not contain spaces.
	virtual std::string name() const = 0;

	//creates a new map and returns it. args may contain arguments to
	//the map generator
	virtual std::string create_map(const std::vector<std::string>& args) const = 0;

	struct manager
	{
		manager(const config& game_config);
		~manager();
	};
};

map_generator* get_map_generator(const std::string& name);

class default_map_generator : public map_generator
{
public:
	default_map_generator(const config& game_config);

	bool allow_user_config() const;
	void user_config(display& disp);

	std::string name() const;

	std::string create_map(const std::vector<std::string>& args) const;

private:
	size_t width_, height_, iterations_, hill_size_, max_lakes_, nvillages_, nplayers_;
	const config* cfg_;
};

#endif