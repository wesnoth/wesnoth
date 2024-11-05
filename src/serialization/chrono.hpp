/*
	Copyright (C) 2024
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

#include "config_attribute_value.hpp"

#include <chrono>

namespace chrono
{
inline auto parse_timestamp(const config_attribute_value& val)
{
	return std::chrono::system_clock::from_time_t(val.to_long_long());
}

inline auto serialize_timestamp(const std::chrono::system_clock::time_point& time)
{
	return std::chrono::system_clock::to_time_t(time);
}

template<typename Duration>
inline auto parse_duration(const config_attribute_value& val, const Duration& def = Duration{0})
{
	return Duration{val.to_long_long(def.count())};
}

} // namespace chrono
