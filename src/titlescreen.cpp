#include "cursor.hpp"
#include "display.hpp"
#include "events.hpp"
#include "font.hpp"
#include "game_config.hpp"
#include "hotkeys.hpp"
#include "key.hpp"
#include "language.hpp"
#include "preferences.hpp"
#include "sdl_utils.hpp"
#include "show_dialog.hpp"
#include "titlescreen.hpp"
#include "util.hpp"
#include "video.hpp"

namespace {

void fade_logo(display& screen, int xpos, int ypos)
{
	const scoped_sdl_surface logo(image::get_image(game_config::game_logo,image::UNSCALED));
	if(logo == NULL) {
		std::cerr << "Could not find game logo\n";
		return;
	}

	SDL_Surface* const fb = screen.video().getSurface();

	if(fb == NULL || xpos < 0 || ypos < 0 || xpos + logo->w > fb->w || ypos + logo->h > fb->h) {
		return;
	}

	//only once, when the game is first started, the logo fades in
	static bool faded_in = false;

	CKey key;
	bool last_button = key[SDLK_ESCAPE] || key[SDLK_SPACE];

	std::cerr << "fading logo in....\n";

	std::cerr << "logo size: " << logo->w << "," << logo->h << "\n";

	for(int x = 0; x != logo->w; ++x) {
		SDL_Rect srcrect = {x,0,1,logo->h};
		SDL_Rect dstrect = {xpos+x,ypos,1,logo->h};

		SDL_BlitSurface(logo,&srcrect,fb,&dstrect);

		update_rect(dstrect);

		if(!faded_in && (x%5) == 0) {

			const bool new_button = key[SDLK_ESCAPE] || key[SDLK_SPACE] || key[SDLK_RETURN];
			if(new_button && !last_button) {
				faded_in = true;
			}

			last_button = new_button;

			screen.update_display();
			
			SDL_Delay(10);

			events::pump();
		}
	}

	std::cerr << "logo faded in\n";

	faded_in = true;
}

}

namespace gui {

TITLE_RESULT show_title(display& screen)
{
	cursor::set(cursor::NORMAL);

	const preferences::display_manager disp_manager(&screen);
	const hotkey::basic_handler key_handler(&screen);

	const video_change_detector disp_change_detector(screen.video());
	
	const scoped_sdl_surface title_surface_unscaled(image::get_image(game_config::game_title,image::UNSCALED));
	const scoped_sdl_surface title_surface(scale_surface(title_surface_unscaled,screen.x(),screen.y()));

	if(title_surface == NULL) {
		std::cerr << "Could not find title image\n";
	} else {
		screen.blit_surface(0,0,title_surface);
		update_rect(screen.screen_area());

		std::cerr << "displayed title image\n";
	}

	fade_logo(screen,(game_config::title_logo_x*screen.x())/1024,(game_config::title_logo_y*screen.y())/768);

	std::cerr << "faded logo\n";

	const std::string& version_str = string_table["version"] + " " +
	                                 game_config::version;

	const SDL_Rect version_area = font::draw_text(NULL,screen.screen_area(),10,
	                                    font::NORMAL_COLOUR,version_str,0,0);
	const size_t versiony = screen.y() - version_area.h;

	if(versiony < size_t(screen.y())) {
		font::draw_text(&screen,screen.screen_area(),
		                  10,font::NORMAL_COLOUR,version_str,0,versiony);
	}

	std::cerr << "drew version number\n";

	//members of this array must correspond to the enumeration TITLE_RESULT
	static const std::string button_labels[] = { "tutorial_button", "campaign_button", "multiplayer_button",
		"load_button", "language_button", "preferences", "about_button", "quit_button" };

	static const size_t nbuttons = sizeof(button_labels)/sizeof(*button_labels);

	const int menu_xbase = (game_config::title_buttons_x*screen.x())/1024;
	const int menu_xincr = 0;
	const int menu_ybase = (game_config::title_buttons_y*screen.y())/768;
	const int menu_yincr = 40;
	const int padding = game_config::title_buttons_padding;
	
	std::vector<button> buttons;
	size_t b, max_width = 0;
	for(b = 0; b != nbuttons; ++b) {
		buttons.push_back(button(screen,string_table[button_labels[b]]));
		buttons.back().set_location(menu_xbase + b*menu_xincr, menu_ybase + b*menu_yincr);
		max_width = maximum<size_t>(max_width,buttons.back().width());
	}

	std::string style = "mainmenu";
	draw_dialog_frame(menu_xbase-padding,menu_ybase-padding,max_width+padding*2,menu_yincr*(nbuttons-1)+buttons.back().height()+padding*2,screen,&style);

	events::raise_draw_event();

	std::cerr << "drew buttons dialog\n";

	CKey key;

	bool last_escape = key[SDLK_ESCAPE];

	update_whole_screen();

	std::cerr << "entering interactive loop...\n";

	for(;;) {
		int mousex, mousey;
		const int mouse_flags = SDL_GetMouseState(&mousex,&mousey);
		const bool left_button = mouse_flags&SDL_BUTTON_LMASK;

		for(size_t b = 0; b != buttons.size(); ++b) {
			if(buttons[b].process(mousex,mousey,left_button)) {
				return TITLE_RESULT(b);
			}
		}

		events::raise_process_event();
		events::raise_draw_event();

		screen.video().flip();

		if(!last_escape && key[SDLK_ESCAPE])
			return QUIT_GAME;

		last_escape = key[SDLK_ESCAPE];

		events::pump();

		//if the resolution has changed due to the user resizing the screen,
		//or from changing between windowed and fullscreen
		if(disp_change_detector.changed()) {
			return TITLE_CONTINUE;
		}

		SDL_Delay(20);
	}

	return QUIT_GAME;
}

}
