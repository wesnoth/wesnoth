#ifndef PLAYER_HPP_INCLUDED
#define PLAYER_HPP_INCLUDED

#include "../config.hpp"

#include <string>

class player
{
public:
	player(const std::string& n, config& cfg);

	const std::string& name() const;

	config* config_address();

private:
	std::string name_;
	config& cfg_;
};

#endif
