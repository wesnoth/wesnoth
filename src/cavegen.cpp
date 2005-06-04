#include "global.hpp"

#include "cavegen.hpp"
#include "log.hpp"
#include "pathfind.hpp"
#include "util.hpp"
#include "wassert.hpp"
#include "serialization/string_utils.hpp"

#define LOG_NG LOG_STREAM(info, engine)

cave_map_generator::cave_map_generator(const config* cfg) : wall_('W'), clear_('u'), village_('D'), castle_('o'),
                                                            cfg_(cfg), width_(50), height_(50), village_density_(0),
							    flipx_(false), flipy_(false)
{
	if(cfg_ == NULL) {
		static const config default_cfg;
		cfg_ = &default_cfg;
	}

	width_ = atoi((*cfg_)["map_width"].c_str());
	height_ = atoi((*cfg_)["map_height"].c_str());
	village_density_ = atoi((*cfg_)["village_density"].c_str());

	const int r = rand()%100;
	const int chance = atoi((*cfg_)["flipx_chance"].c_str());

	flipx_ = r < chance;

	LOG_NG << "flipx: " << r << " < " << chance << " = " << (flipx_ ? "true" : "false") << "\n";
	flipy_ = (rand()%100) < atoi((*cfg_)["flipy_chance"].c_str());


}

size_t cave_map_generator::translate_x(size_t x) const
{
	if(flipx_) {
		x = width_ - x - 1;
	}

	return x;
}

size_t cave_map_generator::translate_y(size_t y) const
{
	if(flipy_) {
		y = height_ - y - 1;
	}

	return y;
}

bool cave_map_generator::allow_user_config() const { return true; }

void cave_map_generator::user_config(display& disp) { return; }

std::string cave_map_generator::name() const { return "cave"; }

std::string cave_map_generator::create_map(const std::vector<std::string>& args)
{
	const config res = create_scenario(args);
	return res["map_data"];
}

config cave_map_generator::create_scenario(const std::vector<std::string>& args)
{
	map_ = std::vector<std::vector<gamemap::TERRAIN> >(width_,std::vector<gamemap::TERRAIN>(height_,wall_));
	chambers_.clear();
	passages_.clear();

	res_.clear();
	const config* const settings = cfg_->child("settings");
	if(settings != NULL) {
		res_ = *settings;
	}

	LOG_NG << "creating scenario....\n";
	generate_chambers();

	LOG_NG << "placing chambers...\n";
	for(std::vector<chamber>::const_iterator c = chambers_.begin(); c != chambers_.end(); ++c) {
		place_chamber(*c);
	}

	LOG_NG << "placing passages...\n";

	for(std::vector<passage>::const_iterator p = passages_.begin(); p != passages_.end(); ++p) {
		place_passage(*p);
	}

	LOG_NG << "outputting map....\n";
	std::stringstream out;
	for(size_t y = 0; y != height_; ++y) {
		for(size_t x = 0; x != width_; ++x) {
			out << map_[x][y];
		}

		out << "\n";
	}

	res_["map_data"] = out.str();

	LOG_NG << "returning result...\n";

	return res_;
}

void cave_map_generator::build_chamber(gamemap::location loc, std::set<gamemap::location>& locs, size_t size, size_t jagged)
{
	if(size == 0 || locs.count(loc) != 0 || !on_board(loc))
		return;

	locs.insert(loc);

	gamemap::location adj[6];
	get_adjacent_tiles(loc,adj);
	for(size_t n = 0; n != 6; ++n) {
		if((rand()%100) < (100-jagged)) {
			build_chamber(adj[n],locs,size-1,jagged);
		}
	}
}

void cave_map_generator::generate_chambers()
{
	const config::child_list& chambers = cfg_->get_children("chamber");
	for(config::child_list::const_iterator i = chambers.begin(); i != chambers.end(); ++i) {
		//if there is only a chance of the chamber appearing, deal with that here.
		const std::string& chance = (**i)["chance"];
		if(chance != "" && (rand()%100) < atoi(chance.c_str())) {
			continue;
		}

		const std::string& xpos = (**i)["x"];
		const std::string& ypos = (**i)["y"];

		size_t min_xpos = 0, min_ypos = 0, max_xpos = width_, max_ypos = height_;

		if(xpos != "") {
			const std::vector<std::string>& items = utils::split(xpos, '-');
			if(items.empty() == false) {
				min_xpos = atoi(items.front().c_str()) - 1;
				max_xpos = atoi(items.back().c_str());
			}
		}

		if(ypos != "") {
			const std::vector<std::string>& items = utils::split(ypos, '-');
			if(items.empty() == false) {
				min_ypos = atoi(items.front().c_str()) - 1;
				max_ypos = atoi(items.back().c_str());
			}
		}

		const size_t x = translate_x(min_xpos + (rand()%(max_xpos-min_xpos)));
		const size_t y = translate_y(min_ypos + (rand()%(max_ypos-min_ypos)));

		const std::string& size = (**i)["size"];
		size_t chamber_size = 3;
		if(size != "") {
			chamber_size = atoi(size.c_str());
		}

		const std::string& jagged = (**i)["jagged"];
		size_t jagged_edges = 0;
		if(jagged != "") {
			jagged_edges = atoi(jagged.c_str());
		}

		chamber new_chamber;
		new_chamber.center = gamemap::location(x,y);
		build_chamber(new_chamber.center,new_chamber.locs,chamber_size,jagged_edges);

		new_chamber.items = (**i).child("items");

		const std::string& id = (**i)["id"];
		if(id != "") {
			chamber_ids_[id] = chambers_.size();
		}

		chambers_.push_back(new_chamber);

		const config::child_list& passages = (**i).get_children("passage");
		for(config::child_list::const_iterator p = passages.begin(); p != passages.end(); ++p) {
			const std::string& dst = (**p)["destination"];

			//find the destination of this passage
			const std::map<std::string,size_t>::const_iterator itor = chamber_ids_.find(dst);
			if(itor == chamber_ids_.end())
				continue;

			wassert(itor->second < chambers_.size());

			passages_.push_back(passage(new_chamber.center,chambers_[itor->second].center,**p));
		}
	}
}

void cave_map_generator::place_chamber(const chamber& c)
{
	for(std::set<gamemap::location>::const_iterator i = c.locs.begin(); i != c.locs.end(); ++i) {
		set_terrain(*i,clear_);
	}

	if(c.items != NULL) {
		place_items(c,c.items->ordered_begin(),c.items->ordered_end());
	}
}

void cave_map_generator::place_items(const chamber& c, config::all_children_iterator i1, config::all_children_iterator i2)
{
	if(c.locs.empty()) {
		return;
	}

	size_t index = 0;
	while(i1 != i2) {
		const std::string& key = *(*i1).first;
		config cfg = *(*i1).second;
		config* const filter = cfg.child("filter");
		config* const object = cfg.child("object");
		config* object_filter = NULL;
		if(object != NULL) {
			object_filter = object->child("filter");
		}

		if(cfg["same_location_as_previous"] != "yes") {
			index = rand()%c.locs.size();
		}

		std::set<gamemap::location>::const_iterator loc = c.locs.begin();
		std::advance(loc,index);

		char buf[50];
		sprintf(buf,"%d",loc->x+1);
		cfg.values["x"] = buf;
		if(filter != NULL) {
			(*filter)["x"] = buf;
		}

		if(object_filter != NULL) {
			(*object_filter)["x"] = buf;
		}

		sprintf(buf,"%d",loc->y+1);
		cfg.values["y"] = buf;
		if(filter != NULL) {
			(*filter)["y"] = buf;
		}

		if(object_filter != NULL) {
			(*object_filter)["y"] = buf;
		}

		//if this is a side, place a castle for the side
		if(key == "side" && cfg["no_castle"] != "yes") {
			place_castle(cfg["side"],*loc);
		}

		res_.add_child(key,cfg);

		++i1;
	}
}

struct passage_path_calculator : cost_calculator
{
	passage_path_calculator(const std::vector<std::vector<gamemap::TERRAIN> >& mapdata, gamemap::TERRAIN wall, double laziness, size_t windiness)
		        : map_(mapdata), wall_(wall), laziness_(laziness), windiness_(windiness)
	{}

	virtual double cost(const gamemap::location& loc, const double so_far, const bool is_dest) const;
private:
	const std::vector<std::vector<gamemap::TERRAIN> >& map_;
	gamemap::TERRAIN wall_;
	double laziness_;
	size_t windiness_;
};

double passage_path_calculator::cost(const gamemap::location& loc, const double, const bool) const
{
	wassert(loc.x >= 0 && loc.y >= 0 && size_t(loc.x) < map_.size() &&
	        !map_.empty() && size_t(loc.y) < map_.front().size());

	double res = 1.0;
	if(map_[loc.x][loc.y] == wall_) {
		res = laziness_;
	}

	if(windiness_ > 1) {
		res *= double(rand()%windiness_);
	}

	return res;
}

void cave_map_generator::place_passage(const passage& p)
{
	const std::string& chance = p.cfg["chance"];
	if(chance != "" && (rand()%100) < atoi(chance.c_str())) {
		return;
	}


	const size_t windiness = atoi(p.cfg["windiness"].c_str());
	const double laziness = maximum<double>(1.0,atof(p.cfg["laziness"].c_str()));

	passage_path_calculator calc(map_,wall_,laziness,windiness);

	const paths::route rt = a_star_search(p.src, p.dst, 10000.0, &calc, width_, height_);

	const size_t width = maximum<size_t>(1,atoi(p.cfg["width"].c_str()));

	const size_t jagged = atoi(p.cfg["jagged"].c_str());

	for(std::vector<gamemap::location>::const_iterator i = rt.steps.begin(); i != rt.steps.end(); ++i) {
		std::set<gamemap::location> locs;
		build_chamber(*i,locs,width,jagged);
		for(std::set<gamemap::location>::const_iterator j = locs.begin(); j != locs.end(); ++j) {
			set_terrain(*j,clear_);
		}
	}
}

bool cave_map_generator::on_board(const gamemap::location& loc) const
{
	return loc.x >= 0 && loc.y >= 0 && loc.x < width_ && loc.y < height_;
}

void cave_map_generator::set_terrain(gamemap::location loc, gamemap::TERRAIN t)
{
	if(on_board(loc)) {
		if(t == clear_ && (rand()%1000) < village_density_) {
			t = village_;
		}

		gamemap::TERRAIN& c = map_[loc.x][loc.y];
		if(c == clear_ || c == wall_ || c == village_) {
			c = t;
		}
	}
}

void cave_map_generator::place_castle(const std::string& side, gamemap::location loc)
{
	if(side != "") {
		set_terrain(loc,side[0]);
	}

	gamemap::location adj[6];
	get_adjacent_tiles(loc,adj);
	for(size_t n = 0; n != 6; ++n) {
		set_terrain(adj[n],castle_);
	}
}
