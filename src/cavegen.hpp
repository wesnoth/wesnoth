#ifndef CAVEGEN_HPP_INCLUDED
#define CAVEGEN_HPP_INCLUDED

#include "config.hpp"
#include "mapgen.hpp"

#include <set>

class cave_map_generator : public map_generator
{
public:
	cave_map_generator(const config* game_config);

	bool allow_user_config() const;
	void user_config(display& disp);

	std::string name() const;

	std::string create_map(const std::vector<std::string>& args);
	config create_scenario(const std::vector<std::string>& args);

private:

	struct chamber {
		gamemap::location center;
		std::set<gamemap::location> locs;
		config* items;
	};

	struct passage {
		passage(gamemap::location s, gamemap::location d, const config& c)
			: src(s), dst(d), cfg(c)
		{}
		gamemap::location src, dst;
		config cfg;
	};

	void generate_chambers();
	void build_chamber(gamemap::location loc, std::set<gamemap::location>& locs, size_t size, size_t jagged);

	void place_chamber(const chamber& c);
	void place_items(const chamber& c, config::all_children_iterator i1, config::all_children_iterator i2);

	void place_passage(const passage& p);
	
	bool on_board(const gamemap::location& loc) const;
	void set_terrain(gamemap::location loc, gamemap::TERRAIN t);
	void place_castle(const std::string& side, gamemap::location loc);

	gamemap::TERRAIN wall_, clear_, village_, castle_;
	std::vector<std::vector<gamemap::TERRAIN> > map_;

	std::map<std::string,size_t> chamber_ids_;
	std::vector<chamber> chambers_;
	std::vector<passage> passages_;

	config res_;
	const config* cfg_;
	size_t width_, height_, village_density_;

	//the scenario may have a chance to flip all x values or y values
	//to make the scenario appear all random. This is kept track of here.
	bool flipx_, flipy_;

	size_t translate_x(size_t x) const;
	size_t translate_y(size_t y) const;
};

#endif
