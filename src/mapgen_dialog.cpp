#include "global.hpp"

#include "mapgen_dialog.hpp"

#include "display.hpp"
#include "events.hpp"
#include "font.hpp"
#include "gettext.hpp"
#include "mapgen.hpp"
#include "show_dialog.hpp"
#include "util.hpp"
#include "video.hpp"

#include "widgets/button.hpp"
#include "widgets/slider.hpp"

namespace {
	const size_t max_island = 10;
	const size_t max_coastal = 5;
}

default_map_generator::default_map_generator(const config* cfg)
: width_(40), height_(40), island_size_(0), iterations_(1000), hill_size_(10), max_lakes_(20),
  nvillages_(25), nplayers_(2), link_castles_(true)
{
	if(cfg != NULL) {
		cfg_ = *cfg;

		const int width = ::atoi((*cfg)["map_width"].c_str());
		if(width > 0)
			width_ = width;

		const int height = ::atoi((*cfg)["map_height"].c_str());
		if(height > 0)
			height_ = height;

		default_width_ = width_;
		default_height_ = height_;

		const int iterations = ::atoi((*cfg)["iterations"].c_str());
		if(iterations > 0)
			iterations_ = iterations;

		const int hill_size = ::atoi((*cfg)["hill_size"].c_str());
		if(hill_size > 0)
			hill_size_ = hill_size;

		const int max_lakes = ::atoi((*cfg)["max_lakes"].c_str());
		if(max_lakes > 0)
			max_lakes_ = max_lakes;

		const int nvillages = ::atoi((*cfg)["villages"].c_str());
		if(nvillages > 0)
			nvillages_ = nvillages;

		const int nplayers = ::atoi((*cfg)["players"].c_str());
		if(nplayers > 0)
			nplayers_ = nplayers;
	}
}

bool default_map_generator::allow_user_config() const { return true; }

void default_map_generator::user_config(display& disp)
{
	const events::resize_lock prevent_resizing;
	const events::event_context dialog_events_context;

	const int width = 600;
	const int height = 400;
	const int xpos = disp.x()/2 - width/2;
	int ypos = disp.y()/2 - height/2;

	surface_restorer restorer;

	gui::button close_button(disp,_("Close Window"));
	std::vector<gui::button*> buttons(1,&close_button);

	gui::draw_dialog(xpos,ypos,width,height,disp,_("Map Generator"),NULL,&buttons,&restorer);

	SDL_Rect dialog_rect = {xpos,ypos,width,height};
	surface_restorer dialog_restorer(&disp.video(),dialog_rect);

	const std::string& players_label = _("Players:");
	const std::string& width_label = _("Width:");
	const std::string& height_label = _("Height:");
	const std::string& iterations_label = _("Iterations:");
	const std::string& hillsize_label = _("Bump Size:");
	const std::string& villages_label = _("Villages:");
	const std::string& landform_label = _("Landform:");

	SDL_Rect players_rect = font::draw_text(NULL,screen_area(),font::SIZE_NORMAL,font::NORMAL_COLOUR,players_label,0,0);
	SDL_Rect width_rect = font::draw_text(NULL,screen_area(),font::SIZE_NORMAL,font::NORMAL_COLOUR,width_label,0,0);
	SDL_Rect height_rect = font::draw_text(NULL,screen_area(),font::SIZE_NORMAL,font::NORMAL_COLOUR,height_label,0,0);
	SDL_Rect iterations_rect = font::draw_text(NULL,screen_area(),font::SIZE_NORMAL,font::NORMAL_COLOUR,iterations_label,0,0);
	SDL_Rect hillsize_rect = font::draw_text(NULL,screen_area(),font::SIZE_NORMAL,font::NORMAL_COLOUR,hillsize_label,0,0);
	SDL_Rect villages_rect = font::draw_text(NULL,screen_area(),font::SIZE_NORMAL,font::NORMAL_COLOUR,villages_label,0,0);
	SDL_Rect landform_rect = font::draw_text(NULL,screen_area(),font::SIZE_NORMAL,font::NORMAL_COLOUR,landform_label,0,0);

	const int horz_margin = 5;
	const int text_right = xpos + horz_margin +
	        maximum<int>(maximum<int>(maximum<int>(maximum<int>(maximum<int>(
		         players_rect.w,width_rect.w),height_rect.w),iterations_rect.w),hillsize_rect.w),villages_rect.w);

	players_rect.x = text_right - players_rect.w;
	width_rect.x = text_right - width_rect.w;
	height_rect.x = text_right - height_rect.w;
	iterations_rect.x = text_right - iterations_rect.w;
	hillsize_rect.x = text_right - hillsize_rect.w;
	villages_rect.x = text_right - villages_rect.w;
	landform_rect.x = text_right - landform_rect.w;
	
	const int vertical_margin = 20;
	players_rect.y = ypos + vertical_margin*2;
	width_rect.y = players_rect.y + players_rect.h + vertical_margin;
	height_rect.y = width_rect.y + width_rect.h + vertical_margin;
	iterations_rect.y = height_rect.y + height_rect.h + vertical_margin;
	hillsize_rect.y = iterations_rect.y + iterations_rect.h + vertical_margin;
	villages_rect.y = hillsize_rect.y + hillsize_rect.h + vertical_margin;
	landform_rect.y = villages_rect.y + villages_rect.h + vertical_margin;

	const int max_players = 9;

	const int right_space = 100;

	const int slider_left = text_right + 10;
	const int slider_right = xpos + width - horz_margin - right_space;
	SDL_Rect slider_rect = { slider_left,players_rect.y,slider_right-slider_left,players_rect.h};
	gui::slider players_slider(disp);
	players_slider.set_location(slider_rect);
	players_slider.set_min(2);
	players_slider.set_max(max_players);
	players_slider.set_value(nplayers_);

	const int min_width = 20;
	const int max_width = 100;
	const int max_height = 100;
	const int extra_size_per_player = 2;
	
	slider_rect.y = width_rect.y;
	gui::slider width_slider(disp);
	width_slider.set_location(slider_rect);
	width_slider.set_min(min_width+(players_slider.value()-2)*extra_size_per_player);
	width_slider.set_max(max_width);
	width_slider.set_value(width_);

	slider_rect.y = height_rect.y;
	gui::slider height_slider(disp);
	height_slider.set_location(slider_rect);
	height_slider.set_min(min_width+(players_slider.value()-2)*extra_size_per_player);
	height_slider.set_max(max_height);
	height_slider.set_value(height_);

	const int min_iterations = 10;
	const int max_iterations = 3000;

	slider_rect.y = iterations_rect.y;
	gui::slider iterations_slider(disp);
	iterations_slider.set_location(slider_rect);
	iterations_slider.set_min(min_iterations);
	iterations_slider.set_max(max_iterations);
	iterations_slider.set_value(iterations_);

	const int min_hillsize = 1;
	const int max_hillsize = 50;

	slider_rect.y = hillsize_rect.y;
	gui::slider hillsize_slider(disp);
	hillsize_slider.set_location(slider_rect);
	hillsize_slider.set_min(min_hillsize);
	hillsize_slider.set_max(max_hillsize);
	hillsize_slider.set_value(hill_size_);

	const int min_villages = 0;
	const int max_villages = 50;

	slider_rect.y = villages_rect.y;
	gui::slider villages_slider(disp);
	villages_slider.set_location(slider_rect);
	villages_slider.set_min(min_villages);
	villages_slider.set_max(max_villages);
	villages_slider.set_value(nvillages_);

	const int min_landform = 0;
	const int max_landform = int(max_island);
	slider_rect.y = landform_rect.y;
	gui::slider landform_slider(disp);
	landform_slider.set_location(slider_rect);
	landform_slider.set_min(min_landform);
	landform_slider.set_max(max_landform);
	landform_slider.set_value(island_size_);

	SDL_Rect link_rect = slider_rect;
	link_rect.y = link_rect.y + link_rect.h + vertical_margin;

	gui::button link_castles(disp,_("Roads Between Castles"),gui::button::TYPE_CHECK);
	link_castles.set_check(link_castles_);
	link_castles.set_location(link_rect);

	for(bool draw = true;; draw = false) {
		nplayers_ = players_slider.value();
		width_ = width_slider.value();
		height_ = height_slider.value();
		iterations_ = iterations_slider.value();
		hill_size_ = hillsize_slider.value();
		nvillages_ = villages_slider.value();
		island_size_ = landform_slider.value();

		dialog_restorer.restore();
		close_button.set_dirty(true);
		if (close_button.pressed())
			break;

		players_slider.set_dirty();
		width_slider.set_dirty();
		height_slider.set_dirty();
		iterations_slider.set_dirty();
		hillsize_slider.set_dirty();
		villages_slider.set_dirty();
		landform_slider.set_dirty();
		link_castles.set_dirty();

		width_slider.set_min(min_width+(players_slider.value()-2)*extra_size_per_player);
		height_slider.set_min(min_width+(players_slider.value()-2)*extra_size_per_player);

		events::raise_process_event();
		events::raise_draw_event();

		font::draw_text(&disp.video(),screen_area(),font::SIZE_NORMAL,font::NORMAL_COLOUR,players_label,players_rect.x,players_rect.y);
		font::draw_text(&disp.video(),screen_area(),font::SIZE_NORMAL,font::NORMAL_COLOUR,width_label,width_rect.x,width_rect.y);
		font::draw_text(&disp.video(),screen_area(),font::SIZE_NORMAL,font::NORMAL_COLOUR,height_label,height_rect.x,height_rect.y);
		font::draw_text(&disp.video(),screen_area(),font::SIZE_NORMAL,font::NORMAL_COLOUR,iterations_label,iterations_rect.x,iterations_rect.y);
		font::draw_text(&disp.video(),screen_area(),font::SIZE_NORMAL,font::NORMAL_COLOUR,hillsize_label,hillsize_rect.x,hillsize_rect.y);
		font::draw_text(&disp.video(),screen_area(),font::SIZE_NORMAL,font::NORMAL_COLOUR,villages_label,villages_rect.x,villages_rect.y);
		font::draw_text(&disp.video(),screen_area(),font::SIZE_NORMAL,font::NORMAL_COLOUR,landform_label,landform_rect.x,landform_rect.y);

		std::stringstream players_str;
		players_str << nplayers_;
		font::draw_text(&disp.video(),screen_area(),font::SIZE_NORMAL,font::NORMAL_COLOUR,players_str.str(),
		                slider_right+horz_margin,players_rect.y);

		std::stringstream width_str;
		width_str << width_;
		font::draw_text(&disp.video(),screen_area(),font::SIZE_NORMAL,font::NORMAL_COLOUR,width_str.str(),
		                slider_right+horz_margin,width_rect.y);

		std::stringstream height_str;
		height_str << height_;
		font::draw_text(&disp.video(),screen_area(),font::SIZE_NORMAL,font::NORMAL_COLOUR,height_str.str(),
		                slider_right+horz_margin,height_rect.y);
		
		std::stringstream villages_str;
		villages_str << nvillages_ << _("/1000 tiles");
		font::draw_text(&disp.video(),screen_area(),font::SIZE_NORMAL,font::NORMAL_COLOUR,villages_str.str(),
		                slider_right+horz_margin,villages_rect.y);

		std::stringstream landform_str;
		landform_str << gettext(island_size_ == 0 ? N_("Inland") : (island_size_ < max_coastal ? N_("Coastal") : N_("Island")));
		font::draw_text(&disp.video(),screen_area(),font::SIZE_NORMAL,font::NORMAL_COLOUR,landform_str.str(),
			            slider_right+horz_margin,landform_rect.y);

		update_rect(xpos,ypos,width,height);

		disp.update_display();
		SDL_Delay(10);
		events::pump();
	}

	link_castles_ = link_castles.checked();
}

std::string default_map_generator::name() const { return "default"; }

std::string default_map_generator::create_map(const std::vector<std::string>& args)
{
	return generate_map(args);
}

std::string default_map_generator::generate_map(const std::vector<std::string>& args, std::map<gamemap::location,std::string>* labels)
{
	size_t iterations = (iterations_*width_*height_)/(default_width_*default_height_);
	size_t island_size = 0;
	size_t island_off_center = 0;
	size_t max_lakes = max_lakes_;

	if(island_size_ >= max_coastal) {

		//islands look good with much fewer iterations than normal, and fewer lake
		iterations /= 10;
		max_lakes /= 9;

		//the radius of the island should be up to half the width of the map
		const size_t island_radius = 50 + ((max_island - island_size_)*50)/(max_island - max_coastal);
		island_size = (island_radius*(width_/2))/100;
	} else if(island_size_ > 0) {
		std::cerr << "coastal...\n";
		//the radius of the island should be up to twice the width of the map
		const size_t island_radius = 40 + ((max_coastal - island_size_)*40)/max_coastal;
		island_size = (island_radius*width_*2)/100;
		island_off_center = minimum<size_t>(width_,height_);
		std::cerr << "calculated coastal params...\n";
	}

	return default_generate_map(width_,height_,island_size,island_off_center,iterations,hill_size_,max_lakes,(nvillages_*width_*height_)/1000,nplayers_,link_castles_,labels,cfg_);
}

config default_map_generator::create_scenario(const std::vector<std::string>& args)
{
	std::cerr << "creating scenario...\n";
	config res;

	const config* const scenario = cfg_.child("scenario");
	if(scenario != NULL) {
		res = *scenario;
	}

	std::cerr << "got scenario data...\n";

	std::map<gamemap::location,std::string> labels;
	std::cerr << "generating map...\n";
	res["map_data"] = generate_map(args,&labels);
	std::cerr << "done generating map..\n";

	for(std::map<gamemap::location,std::string>::const_iterator i = labels.begin(); i != labels.end(); ++i) {
		if(i->first.x >= 0 && i->first.y >= 0 && i->first.x < width_ && i->first.y < height_) {
			config& label = res.add_child("label");
			label["text"] = i->second;
			i->first.write(label);
		}
	}

	return res;
}
