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

#include "formula/string_utils.hpp"
#include "serialization/chrono.hpp"

#include <array>
#include <chrono>
#include <string>
#include <vector>

namespace utils
{
namespace implementation
{
static constexpr std::array descriptors {
	// TRANSLATORS: The "timespan^$num xxxxx" strings originating from the same file
	// as the string with this comment MUST be translated following the usual rules
	// for WML variable interpolation -- that is, without including or translating
	// the caret^ prefix, and leaving the $num variable specification intact, since
	// it is technically code. The only translatable natural word to be found here
	// is the time unit (year, month, etc.) For example, for French you would
	// translate "timespan^$num years" as "$num ans", thus allowing the game UI to
	// generate output such as "39 ans" after variable interpolation.
	std::tuple{ N_n("timespan^$num year",   "timespan^$num years")   },
	std::tuple{ N_n("timespan^$num month",  "timespan^$num months")  },
	std::tuple{ N_n("timespan^$num week",   "timespan^$num weeks")   },
	std::tuple{ N_n("timespan^$num day",    "timespan^$num days")    },
	std::tuple{ N_n("timespan^$num hour",   "timespan^$num hours")   },
	std::tuple{ N_n("timespan^$num minute", "timespan^$num minutes") },
	std::tuple{ N_n("timespan^$num second", "timespan^$num seconds") },
};

// Each duration type should have its description at its matching descriptor index
static constexpr auto deconstruct_format = std::tuple<
	chrono::years,
	chrono::months,
	chrono::weeks,
	chrono::days,
	std::chrono::hours,
	std::chrono::minutes,
	std::chrono::seconds
>{};

} // namespace implementation

/**
 * Formats a timespan into human-readable text for player authentication functions.
 *
 * This is generally meant for player-facing text rather than lightweight tasks like
 * debug logging. The resulting output may differ based on current language settings.
 *
 * This is intentionally not a very thorough representation of time intervals.
 * See <https://github.com/wesnoth/wesnoth/issues/6036> for more information.
 *
 * @param span     The timespan to format
 * @param detailed Whether to display more specific values such as "3 months, 2 days,
 *                 30 minutes, and 1 second". If not specified or set to @a false, the
 *                 return value will ONLY include most significant time unit (e.g. "3
 *                 months").
 * @return         A human-readable timespan description.
 *
 * @note The implementation formats the given timespan according to periods defined by
 *       the C++ chrono standard. As such, a year is defined as its average Gregorian
 *       length of 365.2425 days, while months are exactly 1/12 of a year. Furthermore,
 *       it doesn't take into account leap years or leap seconds. If you need to
 *       account for those, you are better off importing a new library and providing it
 *       with more specific information about the start and end times of the interval;
 *       otherwise your next best option is to hire a fortune teller to manually service
 *       your requests every time instead of this function.
 */
template<typename Rep, typename Period>
static std::string format_timespan(const std::chrono::duration<Rep, Period>& span, bool detailed = false)
{
	if(span.count() <= 0) {
		return _("timespan^expired");
	}

	std::vector<t_string> display_text;
	const auto push_description = [&](const auto& time_component, const auto& description)
	{
		auto amount = time_component.count();
		if(amount <= 0) {
			return true; // Continue to next element
		}

		const auto& [fmt_singular, fmt_plural] = description;
		display_text.emplace_back(VNGETTEXT(fmt_singular, fmt_plural, amount, {{"num", std::to_string(amount)}}));
		return detailed;
	};

	std::apply(
		[&push_description](auto&&... args) {
			std::size_t i{0};
			(... && push_description(args, implementation::descriptors[i++]));
		},
		chrono::deconstruct_duration(implementation::deconstruct_format, span));

	return format_conjunct_list(_("timespan^expired"), display_text);
}

} // namespace utils
