#ifndef MAP_LABEL_HPP_INCLUDED
#define MAP_LABEL_HPP_INCLUDED

#include "config.hpp"
#include "map.hpp"

#include <map>
#include <string>

class display;

class map_labels
{
public:
	map_labels(const display& disp);
	map_labels(const display& disp, const config& cfg);
	~map_labels();

	config write() const;
	void read(const config& cfg);

	const std::string& get_label(const gamemap::location& loc) const;
	void set_label(const gamemap::location& loc, const std::string& text);
	void clear();

	void scroll(double xmove, double ymove);

	void recalculate_labels();

private:
	map_labels(const map_labels&);
	void operator=(const map_labels&);

	const display& disp_;

	typedef std::map<gamemap::location,int> label_map;
	label_map labels_;
};

#endif