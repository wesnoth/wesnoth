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

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <string_view>

#if __cpp_lib_chrono >= 201907L
#define CPP20_CHRONO_SUPPORT
#endif

namespace chrono
{
#ifdef CPP20_CHRONO_SUPPORT

using std::chrono::days;
using std::chrono::weeks;
using std::chrono::months;
using std::chrono::years;

#else

using days   = std::chrono::duration<int, std::ratio<86400>>;
using weeks  = std::chrono::duration<int, std::ratio<604800>>;
using months = std::chrono::duration<int, std::ratio<2629746>>;
using years  = std::chrono::duration<int, std::ratio<31556952>>;

#endif

inline auto parse_timestamp(long long val)
{
	return std::chrono::system_clock::from_time_t(val);
}

inline auto parse_timestamp(const config_attribute_value& val)
{
	return std::chrono::system_clock::from_time_t(val.to_long_long());
}

inline auto serialize_timestamp(const std::chrono::system_clock::time_point& time)
{
	return std::chrono::system_clock::to_time_t(time);
}

inline auto format_local_timestamp(const std::chrono::system_clock::time_point& time, std::string_view format = "%F %T")
{
	std::ostringstream ss;
	auto as_time_t = std::chrono::system_clock::to_time_t(time);
	ss << std::put_time(std::localtime(&as_time_t), format.data());
	return ss.str();
}

template<typename Duration>
inline auto parse_duration(const config_attribute_value& val, const Duration& def = Duration{0})
{
	return Duration{val.to_long_long(def.count())};
}

template<typename RepE, typename PeriodE, typename RepD, typename PeriodD>
constexpr double normalize_progress(
	const std::chrono::duration<RepE, PeriodE>& elapsed,
	const std::chrono::duration<RepD, PeriodD>& duration)
{
	return std::clamp(std::chrono::duration<double, PeriodE>{elapsed} / duration, 0.0, 1.0);
}

template<typename... Ts, typename Rep, typename Period>
constexpr auto deconstruct_duration(const std::tuple<Ts...>&, const std::chrono::duration<Rep, Period>& span)
{
	auto time_remaining = std::chrono::duration_cast<std::common_type_t<Ts...>>(span);
	return std::tuple{[&time_remaining]() {
		auto duration = std::chrono::duration_cast<Ts>(time_remaining);
		time_remaining -= duration;
		return duration;
	}()...};
}

/** Helper types to be used with @ref deconstruct_duration */
namespace format
{
constexpr auto days_hours_mins_secs = std::tuple<
	chrono::days,
	std::chrono::hours,
	std::chrono::minutes,
	std::chrono::seconds
>{};

} // namespace format

} // namespace chrono
