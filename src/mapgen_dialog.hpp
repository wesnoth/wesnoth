#ifndef MAPGEN_DIALOG_HPP_INCLUDED
#define MAPGEN_DIALOG_HPP_INCLUDED

#include "config.hpp"
#include "mapgen.hpp"

class default_map_generator : public map_generator
{
public:
	default_map_generator(const config* game_config);

	bool allow_user_config() const;
	void user_config(display& disp);

	std::string name() const;

	std::string create_map(const std::vector<std::string>& args);
	config create_scenario(const std::vector<std::string>& args);

private:

	std::string generate_map(const std::vector<std::string>& args, std::map<gamemap::location,std::string>* labels=NULL);

	size_t default_width_, default_height_, width_, height_, island_size_, iterations_, hill_size_, max_lakes_, nvillages_, nplayers_;
	bool link_castles_;
	config cfg_;
};

#endif
