#ifndef MAP_CREATE_HPP_INCLUDED
#define MAP_CREATE_HPP_INCLUDED

class config;
class map_generator;

#include <string>

std::string random_generate_map(const std::string& parms, const config* cfg);
config random_generate_scenario(const std::string& parms, const config* cfg);

map_generator* create_map_generator(const std::string& name, const config* cfg);

#endif
