#include "player.hpp"

player::player(const std::string& n, config& cfg) : name_(n), cfg_(cfg)
{
	cfg_["name"] = n;
	mark_available(true);
}

void player::mark_available(bool val)
{
	cfg_.values["available"] = (val ? "yes" : "no");
}

const std::string& player::name() const
{
	return name_;
}

config* player::config_address()
{
	return &cfg_;
}
