#include "mapgen_dialog.hpp"

#include "events.hpp"
#include "font.hpp"
#include "language.hpp"
#include "mapgen.hpp"
#include "show_dialog.hpp"
#include "util.hpp"

#include "widgets/button.hpp"
#include "widgets/slider.hpp"

default_map_generator::default_map_generator(const config& game_config)
: width_(40), height_(40), iterations_(1000), hill_size_(10), max_lakes_(20),
  nvillages_(300), nplayers_(2), cfg_(NULL)
{
	const config* const cfg = game_config.find_child("map_generator","name",name());
	if(cfg != NULL) {
		cfg_ = cfg;

		const int width = ::atoi((*cfg)["map_width"].c_str());
		if(width > 0)
			width_ = width;

		const int height = ::atoi((*cfg)["map_height"].c_str());
		if(height > 0)
			height_ = height;

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
	const int ypos = disp.y()/2 - height/2;

	SDL_Rect dialog_rect = {xpos-10,ypos-10,width+20,height+20};
	surface_restorer restorer(&disp.video(),dialog_rect);

	gui::draw_dialog_frame(xpos,ypos,width,height,disp);

	SDL_Rect title_rect = font::draw_text(NULL,disp.screen_area(),24,font::NORMAL_COLOUR,
	                                            string_table["map_generator"],0,0);

	gui::button close_button(disp,string_table["close_window"]);

	close_button.set_x(xpos + width/2 - close_button.width()/2);
	close_button.set_y(ypos + height - close_button.height()-14);

	const std::string& players_label = string_table["num_players"] + ":";
	const std::string& width_label = string_table["map_width"] + ":";
	const std::string& height_label = string_table["map_height"] + ":";
	const std::string& iterations_label = string_table["mapgen_iterations"] + ":";
	const std::string& hillsize_label = string_table["mapgen_hillsize"] + ":";
	const std::string& villages_label = string_table["mapgen_villages"] + ":";

	SDL_Rect players_rect = font::draw_text(NULL,disp.screen_area(),14,font::NORMAL_COLOUR,players_label,0,0);
	SDL_Rect width_rect = font::draw_text(NULL,disp.screen_area(),14,font::NORMAL_COLOUR,width_label,0,0);
	SDL_Rect height_rect = font::draw_text(NULL,disp.screen_area(),14,font::NORMAL_COLOUR,height_label,0,0);
	SDL_Rect iterations_rect = font::draw_text(NULL,disp.screen_area(),14,font::NORMAL_COLOUR,iterations_label,0,0);
	SDL_Rect hillsize_rect = font::draw_text(NULL,disp.screen_area(),14,font::NORMAL_COLOUR,hillsize_label,0,0);
	SDL_Rect villages_rect = font::draw_text(NULL,disp.screen_area(),14,font::NORMAL_COLOUR,villages_label,0,0);

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
	
	const int vertical_margin = 20;
	players_rect.y = ypos + title_rect.h + vertical_margin*2;
	width_rect.y = players_rect.y + players_rect.h + vertical_margin;
	height_rect.y = width_rect.y + width_rect.h + vertical_margin;
	iterations_rect.y = height_rect.y + height_rect.h + vertical_margin;
	hillsize_rect.y = iterations_rect.y + iterations_rect.h + vertical_margin;
	villages_rect.y = hillsize_rect.y + hillsize_rect.h + vertical_margin;

	const int max_players = 8;

	const int right_space = 100;

	const int slider_left = text_right + 10;
	const int slider_right = xpos + width - horz_margin - right_space;
	SDL_Rect slider_rect = { slider_left,players_rect.y,slider_right-slider_left,players_rect.h};
	gui::slider players_slider(disp,slider_rect,gui::slider::normalize(nplayers_,2,max_players));

	const int min_width = 20;
	const int max_width = 200;
	const int min_height = 20;
	const int max_height = 200;

	slider_rect.y = width_rect.y;
	gui::slider width_slider(disp,slider_rect,gui::slider::normalize(width_,min_height,max_height));

	slider_rect.y = height_rect.y;
	gui::slider height_slider(disp,slider_rect,gui::slider::normalize(height_,min_width,max_width));

	const int min_iterations = 10;
	const int max_iterations = 3000;

	slider_rect.y = iterations_rect.y;
	gui::slider iterations_slider(disp,slider_rect,gui::slider::normalize(iterations_,min_iterations,max_iterations));

	const int min_hillsize = 1;
	const int max_hillsize = 50;

	slider_rect.y = hillsize_rect.y;
	gui::slider hillsize_slider(disp,slider_rect,gui::slider::normalize(hill_size_,min_hillsize,max_hillsize));

	const int min_villages = 10;
	const int max_villages = 10000;

	slider_rect.y = villages_rect.y;
	gui::slider villages_slider(disp,slider_rect,gui::slider::normalize(nvillages_,min_villages,max_villages));

	for(bool draw = true;; draw = false) {
		int mousex, mousey;
		const int mouse_flags = SDL_GetMouseState(&mousex,&mousey);

		const bool left_button = mouse_flags&SDL_BUTTON_LMASK;

		if(close_button.process(mousex,mousey,left_button)) {
			break;
		}

		const double new_players = players_slider.process(mousex,mousey,left_button);
		if(new_players >= 0.0) {
			nplayers_ = gui::slider::denormalize(new_players,2,max_players);
			std::cerr << "set players to " << nplayers_ << "," << new_players << "\n";
			draw = true;
		}

		const double new_width = width_slider.process(mousex,mousey,left_button);
		if(new_width >= 0.0) {
			width_ = gui::slider::denormalize(new_width,min_width,max_width);
			draw = true;
		}

		const double new_height = height_slider.process(mousex,mousey,left_button);
		if(new_height >= 0.0) {
			height_ = gui::slider::denormalize(new_height,min_height,max_height);
			draw = true;
		}

		const double new_iterations = iterations_slider.process(mousex,mousey,left_button);
		if(new_iterations >= 0.0) {
			iterations_ = gui::slider::denormalize(new_iterations,min_iterations,max_iterations);
			draw = true;
		}

		const double new_hillsize = hillsize_slider.process(mousex,mousey,left_button);
		if(new_hillsize >= 0.0) {
			hill_size_ = gui::slider::denormalize(new_hillsize,min_hillsize,max_hillsize);
			draw = true;
		}

		const double new_villages = villages_slider.process(mousex,mousey,left_button);
		if(new_villages >= 0.0) {
			nvillages_ = gui::slider::denormalize(new_villages,min_villages,max_villages);
			draw = true;
		}

		if(draw) {

			gui::draw_dialog_frame(xpos,ypos,width,height,disp);

			title_rect = font::draw_text(&disp,disp.screen_area(),24,font::NORMAL_COLOUR,
	                       string_table["map_generator"],xpos+(width-title_rect.w)/2,ypos+10);

			font::draw_text(&disp,disp.screen_area(),14,font::NORMAL_COLOUR,players_label,players_rect.x,players_rect.y);
			font::draw_text(&disp,disp.screen_area(),14,font::NORMAL_COLOUR,width_label,width_rect.x,width_rect.y);
			font::draw_text(&disp,disp.screen_area(),14,font::NORMAL_COLOUR,height_label,height_rect.x,height_rect.y);
			font::draw_text(&disp,disp.screen_area(),14,font::NORMAL_COLOUR,iterations_label,iterations_rect.x,iterations_rect.y);
			font::draw_text(&disp,disp.screen_area(),14,font::NORMAL_COLOUR,hillsize_label,hillsize_rect.x,hillsize_rect.y);
			font::draw_text(&disp,disp.screen_area(),14,font::NORMAL_COLOUR,villages_label,villages_rect.x,villages_rect.y);

			std::stringstream players_str;
			players_str << nplayers_;
			font::draw_text(&disp,disp.screen_area(),14,font::NORMAL_COLOUR,players_str.str(),
			                slider_right+horz_margin,players_rect.y);

			std::stringstream width_str;
			width_str << width_;
			font::draw_text(&disp,disp.screen_area(),14,font::NORMAL_COLOUR,width_str.str(),
			                slider_right+horz_margin,width_rect.y);

			std::stringstream height_str;
			height_str << height_;
			font::draw_text(&disp,disp.screen_area(),14,font::NORMAL_COLOUR,height_str.str(),
			                slider_right+horz_margin,height_rect.y);

			players_slider.draw();
			width_slider.draw();
			height_slider.draw();
			iterations_slider.draw();
			hillsize_slider.draw();
			villages_slider.draw();
			close_button.draw();

			update_rect(xpos,ypos,width,height);
		}

		disp.update_display();
		SDL_Delay(10);
		events::pump();
	}
}

std::string default_map_generator::name() const { return "default"; }

std::string default_map_generator::create_map(const std::vector<std::string>& args) const
{
	if(cfg_ != NULL)
		return default_generate_map(width_,height_,iterations_,hill_size_,max_lakes_,nvillages_,nplayers_,*cfg_);
	else
		return "";
}