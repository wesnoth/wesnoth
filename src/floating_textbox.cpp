#include "floating_textbox.hpp"

#include "font.hpp"
#include "preferences.hpp"

namespace gui{
	floating_textbox::floating_textbox() : box(NULL), check(NULL), mode(TEXTBOX_NONE), label(0)
	{}

	void floating_textbox::close(display& gui)
	{
		if(!active()) {
			return;
		}
		if(check != NULL) {
			if(mode == TEXTBOX_MESSAGE) {
				preferences::set_message_private(check->checked());
			}
		}
		box.assign(NULL);
		check.assign(NULL);
		font::remove_floating_label(label);
		mode = TEXTBOX_NONE;
		gui.invalidate_all();
	}

	void floating_textbox::update_location(display& gui)
	{
		if (box == NULL)
			return;

		const SDL_Rect& area = gui.map_area();

		const int border_size = 10;

		const int ypos = area.y+area.h-30 - (check != NULL ? check->height() + border_size : 0);

		if (label != 0)
			font::remove_floating_label(label);

		label = font::add_floating_label(label_string,font::SIZE_NORMAL,
				font::YELLOW_COLOUR,area.x+border_size,ypos,0,0,-1, area,font::LEFT_ALIGN);

		if (label == 0)
			return;

		const SDL_Rect& label_area = font::get_floating_label_rect(label);
		const int textbox_width = area.w - label_area.w - border_size*3;

		if(textbox_width <= 0) {
			font::remove_floating_label(label);
			return;
		}

		if(box != NULL) {
			box->set_volatile(true);
			const SDL_Rect rect = {
				area.x + label_area.w + border_size*2, ypos,
				textbox_width, box->height()
			};
			box->set_location(rect);
		}

		if(check != NULL) {
			check->set_volatile(true);
			check->set_location(box->location().x,box->location().y + box->location().h + border_size);
		}
	}

	void floating_textbox::show(gui::TEXTBOX_MODE mode, const std::string& label, 
		const std::string& check_label, bool checked, display& gui)
	{
		close(gui);

		label_string = label;
		mode = mode;

		if(check_label != "") {
			check.assign(new gui::button(gui.video(),check_label,gui::button::TYPE_CHECK));
			check->set_check(checked);
		}


		box.assign(new gui::textbox(gui.video(),100,"",true,256,0.8,0.6));

		update_location(gui);
	}

	void floating_textbox::tab(std::vector<team>& teams, const unit_map& units, display& gui)
	{
		if(active() == false) {
			return;
		}

		switch(mode) {
		case gui::TEXTBOX_MESSAGE:
		{
			std::string text = box->text();
			std::string semiword;
			bool beginning;

			const size_t last_space = text.rfind(" ");

			//if last character is a space return
			if(last_space == text.size() -1) {
				return;
			}

			if(last_space == std::string::npos) {
				beginning = true;
				semiword = text;
			}else{
				beginning = false;
				semiword.assign(text,last_space+1,text.size());
			}

			std::vector<std::string> matches;
			std::string best_match = semiword;

			for(size_t n = 0; n != teams.size(); ++n) {
				if(teams[n].is_empty()) {
					continue;
				}
				const unit_map::const_iterator leader = team_leader(n+1,units);
				if(leader != units.end()) {
					const std::string& name = leader->second.description();
					if( name.size() >= semiword.size() && 
							std::equal(semiword.begin(),semiword.end(),name.begin(),chars_equal_insensitive)) {
						if(matches.empty()) {
							best_match = name;
						} else {
							int j= 0;;
							while(best_match[j] == name[j]) j++;
							best_match.erase(best_match.begin()+j,best_match.end());
						}
						matches.push_back(name);
					}
				}
			}

			if(matches.empty()) {
				const std::set<std::string>& observers = gui.observers();
				for(std::set<std::string>::const_iterator i = observers.begin(); i != observers.end(); ++i) {
					if( i->size() >= semiword.size() && 
							std::equal(semiword.begin(),semiword.end(),i->begin(),chars_equal_insensitive)) {
						if(matches.empty()) {
							best_match = *i;
						} else {
							int j;
							while(best_match[j] == (*i)[j]) j++;
							best_match.erase(best_match.begin()+j,best_match.end());
						}
						matches.push_back(*i);
					}
				}
			}

			if(!matches.empty()) {
				std::string add = beginning ? ": " : " ";
				text.replace(last_space+1, semiword.size(), best_match);
				if(matches.size() == 1) {
					text.append(add);
				} else {
					std::string completion_list;
					std::vector<std::string>::iterator it;
					for(it =matches.begin();it!=matches.end();it++) {
						completion_list += " ";
						completion_list += *it;
					}
					gui.add_chat_message("",0,completion_list,display::MESSAGE_PRIVATE);
				}
				box->set_text(text);
			}
			break;
		}
		default:
			LOG_STREAM(err, display) << "unknown textbox mode\n";
		}
	}
}
