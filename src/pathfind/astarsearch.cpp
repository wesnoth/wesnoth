/*
   Copyright (C) 2003 by David White <dave@whitevine.net>
                 2005 - 2015 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "log.hpp"
#include "map/map.hpp"
#include "pathfind/pathfind.hpp"
#include "pathfind/teleport.hpp"

#include <queue>
#include <map>

static lg::log_domain log_engine("engine");
#define LOG_PF LOG_STREAM(info, log_engine)
#define DBG_PF LOG_STREAM(debug, log_engine)
#define ERR_PF LOG_STREAM(err, log_engine)

namespace pathfind {


namespace {
double heuristic(const map_location& src, const map_location& dst)
{
	// We will mainly use the distances in hexes
	// but we subtract a tiny bonus for shorter Euclidean distance
	// based on how the path looks on the screen.

	// 0.75 comes from the horizontal hex imbrication
	double xdiff = (src.x - dst.x) * 0.75;
	// we must add 0.5 to the y coordinate when x is odd
	double ydiff = (src.y - dst.y) + ((src.x & 1) - (dst.x & 1)) * 0.5;

	// we assume a map with a maximum diagonal of 300 (bigger than a 200x200)
	// and we divide by 90000 * 10000 to avoid interfering with the defense subcost
	// (see shortest_path_calculator::cost)
	// NOTE: In theory, such heuristic is barely 'admissible' for A*,
	// But not a problem for our current A* (we use heuristic only for speed)
	// Plus, the euclidean fraction stay below the 1MP minimum and is also
	// a good heuristic, so we still find the shortest path efficiently.
	return distance_between(src, dst) + (xdiff*xdiff + ydiff*ydiff) / 900000000.0;

	// TODO: move the heuristic function into the cost_calculator
	// so we can use case-specific heuristic
	// and clean the definition of these numbers
}

// values 0 and 1 mean uninitialized
const unsigned bad_search_counter = 0;
// The number of nodes already processed.
static unsigned search_counter = bad_search_counter;

struct node {
	double g, h, t;
	map_location curr, prev;
	/**
	 * If equal to search_counter, the node is off the list.
	 * If equal to search_counter + 1, the node is on the list.
	 * Otherwise it is outdated.
	 */
	unsigned in;

	node()
		: g(1e25)
		, h(1e25)
		, t(1e25)
		, curr()
		, prev()
		, in(bad_search_counter)
	{
	}
	node(double s, const map_location &c, const map_location &p, const map_location &dst, bool i, const teleport_map* teleports):
		g(s), h(heuristic(c, dst)), t(g + h), curr(c), prev(p), in(search_counter + i)
	{
		if (teleports && !teleports->empty()) {

			double new_srch = 1.0;
			std::set<map_location> sources;
			teleports->get_sources(sources);

			std::set<map_location>::const_iterator it = sources.begin();
			for(; it != sources.end(); ++it) {
				const double tmp_srch = heuristic(c, *it);
				if (tmp_srch < new_srch) { new_srch = tmp_srch; }
			}


			double new_dsth = 1.0;
			std::set<map_location> targets;
			teleports->get_targets(targets);

			for(it = targets.begin(); it != targets.end(); ++it) {
				const double tmp_dsth = heuristic(*it, dst);
				if (tmp_dsth < new_dsth) { new_dsth = tmp_dsth; }
			}

			double new_h = new_srch + new_dsth + 1.0;
			if (new_h < h) {
				h = new_h;
				t = g + h;
			}
		}
	}

	bool operator<(const node& o) const {
		return t < o.t;
	}
};

class comp {
	const std::vector<node>& nodes_;

public:
	comp(const std::vector<node>& n) : nodes_(n) { }
	bool operator()(int a, int b) const {
		return nodes_[b] < nodes_[a];
	}
};

class indexer {
	size_t w_;

public:
	indexer(size_t w) : w_(w) { }
	size_t operator()(const map_location& loc) const {
		return loc.y * w_ + loc.x;
	}
};
}//anonymous namespace


plain_route a_star_search(const map_location& src, const map_location& dst,
                          double stop_at, const cost_calculator& calc,
                          const size_t width, const size_t height,
                          const teleport_map *teleports, bool border) {
	//----------------- PRE_CONDITIONS ------------------
	assert(src.valid(width, height, border));
	assert(dst.valid(width, height, border));
	assert(stop_at <= calc.getNoPathValue());
	//---------------------------------------------------

	DBG_PF << "A* search: " << src << " -> " << dst << '\n';

	if (calc.cost(dst, 0) >= stop_at) {
		LOG_PF << "aborted A* search because Start or Dest is invalid\n";
		plain_route locRoute;
		locRoute.move_cost = int(calc.getNoPathValue());
		return locRoute;
	}

	// increment search_counter but skip the range equivalent to uninitialized
	search_counter += 2;
	if (search_counter - bad_search_counter <= 1u)
		search_counter += 2;

	static std::vector<node> nodes;
	nodes.resize(width * height);  // this create uninitialized nodes

	indexer index(width);
	comp node_comp(nodes);

	nodes[index(dst)].g = stop_at + 1;
	nodes[index(src)] = node(0, src, map_location::null_location(), dst, true, teleports);

	std::vector<int> pq;
	pq.push_back(index(src));

	while (!pq.empty()) {
		node& n = nodes[pq.front()];

		n.in = search_counter;

		std::pop_heap(pq.begin(), pq.end(), node_comp);
		pq.pop_back();

		if (n.t >= nodes[index(dst)].g) break;

		std::vector<map_location> locs(6);

		if (teleports && !teleports->empty()) {

			std::set<map_location> allowed_teleports;
			teleports->get_adjacents(allowed_teleports, n.curr);
			locs.insert(locs.end(), allowed_teleports.begin(), allowed_teleports.end());
		}

		int i = locs.size();

		get_adjacent_tiles(n.curr, locs.data());

		for (; i-- > 0;) {
			if (!locs[i].valid(width, height, border)) continue;
			if (locs[i] == n.curr) continue;
			node& next = nodes[index(locs[i])];

			double thresh = (next.in - search_counter <= 1u) ? next.g : stop_at + 1;
			// cost() is always >= 1  (assumed and needed by the heuristic)
			if (n.g + 1 >= thresh) continue;
			double cost = n.g + calc.cost(locs[i], n.g);
			if (cost >= thresh) continue;

			bool in_list = next.in == search_counter + 1;

			next = node(cost, locs[i], n.curr, dst, true, teleports);

			if (in_list) {
				std::push_heap(pq.begin(), std::find(pq.begin(), pq.end(), static_cast<int>(index(locs[i]))) + 1, node_comp);
			} else {
				pq.push_back(index(locs[i]));
				std::push_heap(pq.begin(), pq.end(), node_comp);
			}
		}
	}

	plain_route route;
	if (nodes[index(dst)].g <= stop_at) {
		DBG_PF << "found solution; calculating it...\n";
		route.move_cost = static_cast<int>(nodes[index(dst)].g);
		for (node curr = nodes[index(dst)]; curr.prev != map_location::null_location(); curr = nodes[index(curr.prev)]) {
			route.steps.push_back(curr.curr);
		}
		route.steps.push_back(src);
		std::reverse(route.steps.begin(), route.steps.end());
	} else {
		LOG_PF << "aborted a* search  " << "\n";
		route.move_cost = static_cast<int>(calc.getNoPathValue());
	}

	return route;
}


}//namespace pathfind
