#ifndef MAP_LABEL_HPP_INCLUDED
#define MAP_LABEL_HPP_INCLUDED

class config;
#include "map.hpp"

#include <map>
#include <string>

class display;
class team;

class map_labels
{
public:
	map_labels(const display& disp, const gamemap& map);
	map_labels(const display& disp, const config& cfg, const gamemap& map);
	~map_labels();

	void write(config& res) const;
	void read(const config& cfg);

	static int get_max_chars();

	const std::string& get_label(const gamemap::location& loc) const;
	void set_label(const gamemap::location& loc, const std::string& text);
	void clear();

	void scroll(double xmove, double ymove);

	void recalculate_labels();

	void recalculate_shroud();

	typedef std::map<gamemap::location,int> label_map;
	const label_map& labels() { return labels_; }
	const std::string& get_label(int index) const;

private:
	map_labels(const map_labels&);
	void operator=(const map_labels&);

	const display& disp_;
	const team* team_;
	const gamemap& map_;

	label_map labels_;
};

#endif
