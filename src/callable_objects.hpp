#ifndef CALLABLE_OBJECTS_HPP_INCLUDED
#define CALLABLE_OBJECTS_HPP_INCLUDED

#include <map>

#include "formula_callable.hpp"
#include "map.hpp"
#include "unit.hpp"

#define CALLABLE_WRAPPER_START(klass) \
class klass##_callable : public game_logic::formula_callable { \
	const klass& object_; \
public: \
	explicit klass##_callable(const klass& object) : object_(object) \
	{} \
	\
	const klass& get_##klass() const { return object_; } \
	void get_inputs(std::vector<game_logic::formula_input>* inputs) const \
	{ \
	} \
	\
	variant get_value(const std::string& key) const {

#define CALLABLE_WRAPPER_VAR(VAR) \
	if(key == #VAR) { \
		return variant(object_.VAR); \
	} else

#define CALLABLE_WRAPPER_FN(VAR) \
	if(key == #VAR) { \
		return variant(object_.VAR()); \
	} else

#define CALLABLE_WRAPPER_END \
		{ return variant(); } \
	} \
};

class terrain_callable : public game_logic::formula_callable {
public:
	typedef gamemap::location location;
	terrain_callable(const terrain_type& t, const location loc)
	  : loc_(loc), t_(t)
	{}

	variant get_value(const std::string& key) const;
	void get_inputs(std::vector<game_logic::formula_input>* inputs) const;
private:
	const location loc_;
	const terrain_type &t_;
};

CALLABLE_WRAPPER_START(gamemap)
	if(key == "terrain") {
		int w = object_.w();
		int h = object_.h();
		std::vector<variant> vars;
		for(int i = 0;i < w; i++) {
			for(int j = 0;j < h; j++) {
				const gamemap::location loc(i,j);
				vars.push_back(variant(new terrain_callable(object_.get_terrain_info(loc), loc)));
			}
		}
		return variant(&vars);
	} else
	CALLABLE_WRAPPER_FN(w)
	CALLABLE_WRAPPER_FN(h)
CALLABLE_WRAPPER_END

class location_callable : public game_logic::formula_callable {
	gamemap::location loc_;

	variant get_value(const std::string& key) const;

	void get_inputs(std::vector<game_logic::formula_input>* inputs) const;
	int do_compare(const game_logic::formula_callable* callable) const;

public:
	explicit location_callable(const gamemap::location& loc) : loc_(loc)
	{}

	const gamemap::location& loc() const { return loc_; }

	void serialize_to_string(std::string& str) const;
};

class move_callable : public game_logic::formula_callable {
	gamemap::location src_, dst_;
	variant get_value(const std::string& key) const {
		if(key == "src") {
			return variant(new location_callable(src_));
		} else if(key == "dst") {
			return variant(new location_callable(dst_));
		} else {
			return variant();
		}
	}
	void get_inputs(std::vector<game_logic::formula_input>* inputs) const {
		inputs->push_back(game_logic::formula_input("src", game_logic::FORMULA_READ_ONLY));
		inputs->push_back(game_logic::formula_input("dst", game_logic::FORMULA_READ_ONLY));
	}
public:
	move_callable(const gamemap::location& src, const gamemap::location& dst) :
	  src_(src), dst_(dst)
	{}

	const gamemap::location& src() const { return src_; }
	const gamemap::location& dst() const { return dst_; }
};

class move_map_callable : public game_logic::formula_callable {
	typedef std::multimap<gamemap::location, gamemap::location> move_map;
	const move_map& srcdst_;
	const move_map& dstsrc_;

	variant get_value(const std::string& key) const;
	void get_inputs(std::vector<game_logic::formula_input>* inputs) const;
public:
	move_map_callable(const move_map& srcdst, const move_map& dstsrc)
	  : srcdst_(srcdst), dstsrc_(dstsrc)
	{}

	const move_map& srcdst() const { return srcdst_; }
	const move_map& dstsrc() const { return dstsrc_; }
};

class unit_callable : public game_logic::formula_callable {
public:
	typedef gamemap::location location;
	unit_callable(const std::pair<location, unit>& pair, const team& current_team, unsigned int side)
	  : loc_(pair.first), u_(pair.second), team_(current_team), side_(side)
	{}

	const unit& get_unit() const { return u_; }
	variant get_value(const std::string& key) const;
	void get_inputs(std::vector<game_logic::formula_input>* inputs) const;
private:
	const location& loc_;
	const unit& u_;
	const team& team_;
	unsigned int side_;
};

#endif
