#include "player.hpp"

player::player(const std::string& n, config& cfg) : name_(n), cfg_(cfg)
{
	cfg_["name"] = n;
}

const std::string& player::name() const
{
	return name_;
}

config* player::config_address()
{
	return &cfg_;
}
