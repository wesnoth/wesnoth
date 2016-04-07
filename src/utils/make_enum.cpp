#include "utils/make_enum.hpp"
#include "wml_exception.hpp"
#include "game_config.hpp"

namespace make_enum_detail
{
	void debug_conversion_error(const std::string& temp, const bad_enum_cast & e)
	{
		if (!temp.empty() && game_config::debug) {
			FAIL( e.what() );
		}
	}
}

