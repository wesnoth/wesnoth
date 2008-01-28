#include "callable_objects.hpp"
#include "pathutils.hpp"

variant location_callable::get_value(const std::string& key) const
{
	if(key == "x") {
		return variant(loc_.x+1);
	} else if(key == "y") {
		return variant(loc_.y+1);
	} else if(key == "adjacent_locs") {
		gamemap::location adj[6];
		get_adjacent_tiles(loc_, adj);

		std::vector<variant> v;
		v.reserve(6);
		for(int n = 0; n != 6; ++n) {
			v.push_back(variant(new location_callable(adj[n])));
		}

		return variant(&v);
	} else {
		return variant();
	}
}

void location_callable::get_inputs(std::vector<game_logic::formula_input>* inputs) const
{
	using game_logic::FORMULA_READ_ONLY;
	inputs->push_back(game_logic::formula_input("x", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("y", FORMULA_READ_ONLY));
}

int location_callable::do_compare(const game_logic::formula_callable* callable) const
{
	const location_callable* loc_callable = dynamic_cast<const location_callable*>(callable);
	if(loc_callable == NULL) {
		return formula_callable::do_compare(callable);
	}

	const gamemap::location& other_loc = loc_callable->loc();
	if(other_loc.x != loc_.x) {
		return loc_.x - other_loc.x;
	}

	return loc_.y - other_loc.y;
}

variant move_map_callable::get_value(const std::string& key) const
{
	using namespace game_logic;
	if(key == "moves") {
		std::vector<variant> vars;
		for(move_map::const_iterator i = srcdst_.begin(); i != srcdst_.end(); ++i) {
			map_formula_callable* item = new map_formula_callable;
			item->add("src", variant(new location_callable(i->first)));
			item->add("dst", variant(new location_callable(i->second)));
			vars.push_back(variant(item));
		}

		return variant(&vars);
	} else {
		return variant();
	}
}

void move_map_callable::get_inputs(std::vector<game_logic::formula_input>* inputs) const
{
	using game_logic::FORMULA_READ_ONLY;
	inputs->push_back(game_logic::formula_input("moves", FORMULA_READ_ONLY));
}

variant unit_callable::get_value(const std::string& key) const
{
	std::cerr << "get_value: '" << key << "': " << (int)&u_ << " -> '" << u_.id() << "'\n";
	if(key == "x") {
		return variant(loc_.x+1);
	} else if(key == "y") {
		return variant(loc_.y+1);
	} else if(key == "loc") {
		return variant(new location_callable(loc_));
	} else if(key == "id") {
		return variant(u_.id());
	} else if(key == "leader") {
		return variant(u_.can_recruit());
	} else if(key == "hitpoints") {
		return variant(u_.hitpoints());
	} else if(key == "max_hitpoints") {
		return variant(u_.max_hitpoints());
	} else if(key == "experience") {
		return variant(u_.experience());
	} else if(key == "max_experience") {
		return variant(u_.max_experience());
	} else if(key == "level") {
		return variant(u_.level());
	} else if(key == "total_movement") {
		return variant(u_.total_movement());
	} else if(key == "movement_left") {
		return variant(u_.movement_left());
	} else if(key == "side") {
		return variant(u_.side());
	} else if(key == "is_enemy") {
		return variant(team_.is_enemy(u_.side()));
	} else if(key == "is_mine") {
		return variant(side_ == u_.side());
	} else {
		return variant();
	}
}

void unit_callable::get_inputs(std::vector<game_logic::formula_input>* inputs) const
{
	using game_logic::FORMULA_READ_ONLY;
	inputs->push_back(game_logic::formula_input("x", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("y", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("loc", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("id", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("leader", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("hitpoints", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("max_hitpoints", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("experience", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("max_experience", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("level", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("total_movement", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("movement_left", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("side", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("is_enemy", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("is_mine", FORMULA_READ_ONLY));
}
