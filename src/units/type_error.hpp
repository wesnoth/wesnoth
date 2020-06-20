
#pragma once

#include "game_errors.hpp"
#include <string>

class unit_type_error : public game::game_error
{
public:
	unit_type_error(const std::string& msg)
		: game::game_error(msg)
	{
	}
};