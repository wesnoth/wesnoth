/*
	Copyright (C) 2020 - 2024
	by Iris Morelle <shadowm2006@gmail.com>
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

#include <chrono>
#include <functional>
#include <iosfwd>
#include <utility>

namespace utils {

/**
 * Reports time elapsed at the end of an object scope.
 *
 * The constructor accepts a callable that takes the optimer as its only
 * argument, which can then be used to write the elapsed time to a stream,
 * variable, or anything else that's desirable for a particular use case.
 *
 * @tparam ResolutionType          Clock resolution -- milliseconds, microseconds, etc.
 * @tparam ClockType               Clock type -- a monotonic clock by default.
 */
template<typename ResolutionType, typename ClockType = std::chrono::steady_clock>
struct optimer
{
	using clock					= ClockType;
	using resolution			= ResolutionType;
	using point					= typename clock::time_point;
	using interval				= typename clock::duration;
	using report_callback		= std::function<void(const optimer&)>;

	/**
	 * Constructor, which starts the timer.
	 *
	 * The report callback may be an empty object, in which case the destructor
	 * will not do anything. This may be useful in order to obtain and keep
	 * track of elapsed time manually for other purposes than console printing.
	 */
	explicit optimer(report_callback f = report_callback{})
		: start_()
		, repf_(f)
	{
		reset();
	}

	/**
	 * Destructor, which invokes the report callback.
	 */
	~optimer()
	{
		if(repf_) {
			repf_(*this);
		}
	}

	/** Resets the timer back to zero and returns the previous tick value. */
	point reset()
	{
		return std::exchange(start_, clock::now());
	}

	/** Returns the start time point. */
	point start() const
	{
		return start_;
	}

	/** Returns the elapsed time value. */
	interval elapsed() const
	{
		return clock::now() - start_;
	}

	/**
	 * Resets the starting tick and returns the elapsed time.
	 *
	 * @note This method is preferred to calling elapsed() followed by reset()
	 * since it guarantees resetting to the same tick as is used for duration
	 * (or, in other words, clock::now() is called only once.)
	 */
	interval lap()
	{
		auto prev_start = reset();
		return start_ - prev_start;
	}

private:
	point				start_;
	report_callback		repf_;
};

/**
 * Formats time elapsed for writing to a stream.
 *
 * @note The resulting output does <b>not</b> include a time unit suffix.
 */
template<typename... OpTimerArgs>
inline std::ostream& operator<<(std::ostream& o, const optimer<OpTimerArgs...>& tm)
{
	o << std::chrono::duration_cast<typename optimer<OpTimerArgs...>::resolution>(tm.elapsed()).count();
	return o;
}

/**
 * Time elapsed with millisecond resolution.
 */
using ms_optimer = optimer<std::chrono::milliseconds>;

} // end namespace utils
