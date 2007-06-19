/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "about.hpp"
#include "construct_dialog.hpp"
#include "display.hpp"
#include "font.hpp"
#include "game_config.hpp"
#include "gettext.hpp"
#include "marked-up_text.hpp"
#include "video.hpp"
#include "show_dialog.hpp"

namespace about
{

	static config about_list = config();
	static std::map<std::string , std::string> images;
	static std::string images_default;

std::vector<std::string> get_text(std::string campaign) {
	std::vector< std::string > res;

	const config::child_list& children = about::about_list.get_children("about");

	for(config::child_list::const_iterator cc = children.begin(); cc != children.end(); ++cc) {
	  //just finished a particular campaign
	  if(campaign.size() && campaign == (**cc)["id"]){
	    std::string title=(**cc)["title"];
	    if(title.size()){
	      title = N_("+" + title);
	      res.push_back(title);
	    }
	    std::vector<std::string> lines=utils::split((**cc)["text"],'\n');
	    for(std::vector<std::string>::iterator line=lines.begin();
		line != lines.end(); line++){
	      if((*line)[0] == '+' && (*line).size()>1){
		*line = N_("+  " + (*line).substr(1,(*line).size()-1));
	      }else{
		*line = "-  " + *line;
	      }
	      if(line->size()){
		if ((*line)[0] == '_')
		  *line = gettext(line->substr(1,line->size()-1).c_str());
		res.push_back(*line);
	      }
	    }
	  }
	}

	for(config::child_list::const_iterator acc = children.begin(); acc != children.end(); ++acc) {
	  std::string title=(**acc)["title"];
	  if(title.size()){
	    title = N_("+" + title);
	    res.push_back(title);
	  }
	  std::vector<std::string> lines=utils::split((**acc)["text"],'\n');
	  for(std::vector<std::string>::iterator line=lines.begin();
	      line != lines.end(); line++){
	    if((*line)[0] == '+' && (*line).size()>1){
	      *line = N_("+  " + (*line).substr(1,(*line).size()-1));
	    }else{
	      *line = "-  " + *line;
	    }
	    if(line->size()){
	      if ((*line)[0] == '_')
		*line = gettext(line->substr(1,line->size()-1).c_str());
	      res.push_back(*line);
	    }
	  }
	}

	return res;
}

void set_about(const config& cfg){
	config::child_list about = cfg.get_children("about");
	for(config::child_list::const_iterator A = about.begin(); A != about.end(); A++) {
		config AA = (**A);
		about_list.add_child("about",AA);
		if(!AA["images"].empty()){
			if(images_default.empty()){
				images_default=AA["images"];
			}else{
				images_default+=","+AA["images"];
			}
		}
	}
	config::child_list campaigns = cfg.get_children("campaign");
	for(config::child_list::const_iterator C = campaigns.begin(); C != campaigns.end(); C++) {
		config::child_list about = (**C).get_children("about");
		if(about.size()){
			config temp;
			std::string text;
			std::string title;
			std::string campaign=(**C)["id"];
			title=(**C)["name"];
			temp["title"]=title;
			temp["id"]=(**C)["id"];
			for(config::child_list::const_iterator A = about.begin(); A != about.end(); A++) {
				config AA = (**A);
				std::string subtitle=AA["title"];
				if(subtitle.size()){
				if (subtitle[0] == '_')
						subtitle = gettext(subtitle.substr(1,subtitle.size()-1).c_str());
					text += "+" + subtitle + "\n";
				}
				std::vector<std::string> lines=utils::split(AA["text"],'\n');
				for(std::vector<std::string>::iterator line=lines.begin();
			line != lines.end(); line++){
					text+="    "+(*line)+"\n";
				}

				if(!AA["images"].empty()){
					if(images[campaign].empty()){
						images[campaign]=AA["images"];
					}else{
						images[campaign]+=","+AA["images"];
					}
				}
		}
			temp["text"]=text;
			about_list.add_child("about",temp);
		}
	}
}

void show_about(display &disp, std::string campaign)
{
	CVideo &video = disp.video();
	std::vector<std::string> text = about::get_text(campaign);
	SDL_Rect rect = {0, 0, video.getx(), video.gety()};

	const surface_restorer restorer(&video, rect);

	// Clear the screen
	draw_solid_tinted_rectangle(0,0,video.getx()-1,video.gety()-1,
	                                 0,0,0,1.0,video.getSurface());
	update_whole_screen();

	std::vector<std::string> image_list;
	if(campaign.size() && !images[campaign].empty()){
		image_list=utils::split(images[campaign]);
	}else{
		image_list=utils::split(images_default,',',utils::STRIP_SPACES);
	}
	surface map_image(scale_surface(image::get_image(image_list[0],image::UNSCALED), disp.w(), disp.h()));
	if(! map_image){
		image_list[0]=game_config::game_title;
		map_image=surface(scale_surface(image::get_image(image_list[0],image::UNSCALED), disp.w(), disp.h()));
	}

	SDL_Rect map_rect;
	map_rect.x = video.getx()/2 - map_image->w/2;
	map_rect.y = video.gety()/2 - map_image->h/2;
	map_rect.w = map_image->w;
	map_rect.h = map_image->h;

	gui::button close(video,_("Close"));
	close.set_location((video.getx()/2)-(close.width()/2), video.gety() - 30);
	close.set_volatile(true);

	//substitute in the correct control characters for '+' and '-'
	std::string before_header(2, ' ');
	before_header[0] = font::LARGE_TEXT;
	for(unsigned i = 0; i < text.size(); ++i) {
		std::string &s = text[i];
		if (s.empty()) continue;
		char &first = s[0];
		if (first == '-')
			first = font::SMALL_TEXT;
		else if (first == '+') {
			first = font::LARGE_TEXT;
			text.insert(text.begin() + i, before_header);
			++i;
		}
	}
	text.insert(text.begin(), 10, before_header);

	int startline = 0;

	// the following two lines should be changed if the image of the map is changed
	const int top_margin = 60;		// distance from top of map image to top of scrolling text
	const int bottom_margin = 40;	// distance from bottom of scrolling text to bottom of map image

	int offset = 0;
	bool is_new_line = true;

	int first_line_height = 0;

	// the following rectangles define the top, middle and bottom of the background image
	// the upper and lower part is later used to mask the upper and lower line of scrolling text
	SDL_Rect upper_src = {0, 0, map_rect.w, top_margin};
	SDL_Rect upper_dest = {map_rect.x, map_rect.y, map_rect.w, top_margin};
	SDL_Rect middle_src = {0, top_margin, map_rect.w, map_rect.h - top_margin - bottom_margin};
	SDL_Rect middle_dest = {map_rect.x, map_rect.y + top_margin, map_rect.w, map_rect.h - top_margin - bottom_margin};
	SDL_Rect lower_src = {0, map_rect.h - bottom_margin, map_rect.w, bottom_margin};
	SDL_Rect lower_dest = {map_rect.x, map_rect.y + map_rect.h - bottom_margin, map_rect.w, bottom_margin};

	CKey key;
	bool last_escape;
	int image_count=0;
	do {
		last_escape = key[SDLK_ESCAPE] != 0;

		// check to see if background image has changed
		if(text.size() && (image_count < ((startline*(int)image_list.size())/(int)text.size()))){
			image_count++;
			surface temp=surface(scale_surface(image::get_image(image_list[image_count],image::UNSCALED), disp.w(), disp.h()));
			map_image=temp?temp:map_image;
		}
		// draw map to screen, thus erasing all text

		SDL_BlitSurface(map_image,&middle_src,video.getSurface(),&middle_dest);
		SDL_Rect frame_area = {
			map_rect.x + map_rect.w * 3/32,
			map_rect.y + top_margin,
			map_rect.w * 13 / 16,
			map_rect.h - top_margin - bottom_margin 
		};
		gui::dialog_frame f(disp.video(), "", &gui::dialog::message_style);
		f.layout(frame_area);
        f.draw_background();

		// draw one screen full of text
		const int line_spacing = 5;
		int y = map_rect.y + top_margin - offset;
		int line = startline;
		int cur_line = 0;

		do {
//			SDL_Rect tr2 = font::draw_text(&video,screen_area(),font::SIZE_XLARGE,font::BLACK_COLOUR,
//						text[line], map_rect.x + map_rect.w / 8 + 1,y + 1);
			SDL_Rect tr = font::draw_text(&video,screen_area(),font::SIZE_XLARGE,font::NORMAL_COLOUR,
					              text[line], map_rect.x + map_rect.w / 8,y);

			if(is_new_line) {
				is_new_line = false;
				first_line_height = tr.h + line_spacing;
			}
			line++;
			if(size_t(line) > text.size()-1)
				line = 0;
			y += tr.h + line_spacing;
			cur_line++;
		} while(y<map_rect.y + map_rect.h - bottom_margin);

		// performs the actual scrolling
		const int scroll_speed = 4;		// scroll_speed*50 = speed of scroll in pixel per second

		offset += scroll_speed;
		if(offset>=first_line_height) {
			offset -= first_line_height;
			is_new_line = true;
			startline++;
			if(size_t(startline) == text.size()){
				startline = 0;
				image_count = -1;
			}
		}

		// mask off the upper and lower half of the map,
		// so text will scroll into view rather than
		// suddenly appearing out of nowhere
		SDL_BlitSurface(map_image,&upper_src,video.getSurface(),&upper_dest);
		SDL_BlitSurface(map_image,&lower_src,video.getSurface(),&lower_dest);

		// handle events
		events::pump();
		events::raise_process_event();
		events::raise_draw_event();

		// update screen and wait, so the text does not scroll too fast
		update_rect(map_rect);
		close.set_dirty(true);
		disp.flip();
		disp.delay(20);

	} while(!close.pressed() && (last_escape || !key[SDLK_ESCAPE]));

}

}
