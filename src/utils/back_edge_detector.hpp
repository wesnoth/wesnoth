/*
	Copyright (C) 2021 - 2024
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

#include <boost/graph/depth_first_search.hpp>
#include <type_traits>

namespace utils
{
/**
 * A helper for boost::depth_first_search (DFS) usage with the purpose of detecting cycles.
 *
 * The callback_ is called whenever a back edge is found.
 */
template<typename Callback>
class back_edge_detector : public boost::dfs_visitor<>
{
public:
	explicit back_edge_detector(Callback callback)
		: callback_(std::move(callback))
	{
	}

	template<typename Graph>
	void back_edge(typename Graph::edge_descriptor edge, Graph&)
	{
		static_assert(std::is_invocable_v<Callback, typename Graph::edge_descriptor>);
		callback_(edge);
	}

private:
	Callback callback_;
};

} // namespace utils
