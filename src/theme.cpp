#include "global.hpp"

#include "font.hpp"
#include "language.hpp"
#include "log.hpp"
#include "sdl_utils.hpp"
#include "theme.hpp"
#include "util.hpp"

#include <cstdlib>
#include <sstream>

#define LOG_DP lg::info(lg::display)

namespace {
	const SDL_Rect empty_rect = {0,0,0,0};
	const int XDim = 1024;
	const int YDim = 768;

	const size_t DefaultFontSize = font::SIZE_NORMAL;

	size_t compute(std::string expr, size_t ref1, size_t ref2=0 ) {
		size_t ref = 0;
		if (expr[0] == '=') {
		  ref = ref1;
		  expr = expr.substr(1);
		} else if ((expr[0] == '+') || (expr[0] == '-')) {
		  ref = ref2;
		}

		return ref + atoi(expr.c_str());
	}

	SDL_Rect read_rect(const config& cfg) {
		SDL_Rect rect;
		const std::vector<std::string> items = config::split(cfg["rect"].c_str());
		if(items.size() >= 1)
			rect.x = atoi(items[0].c_str());

		if(items.size() >= 2)
			rect.y = atoi(items[1].c_str());

		if(items.size() >= 3)
			rect.w = atoi(items[2].c_str()) - rect.x;

		if(items.size() >= 4)
			rect.h = atoi(items[3].c_str()) - rect.y;

		return rect;
	}

	std::string resolve_rect(const std::string& rect_str) {
		typedef struct { size_t x1,y1,x2,y2; } _rect;
		static _rect ref_rect = { 0, 0, 0, 0 };
		_rect rect;
		std::stringstream resolved;
		const std::vector<std::string> items = config::split(rect_str.c_str());
		if(items.size() >= 1) {
			rect.x1 = compute(items[0], ref_rect.x1, ref_rect.x2);
			resolved << rect.x1;
		}
		if(items.size() >= 2) {
			rect.y1 = compute(items[1], ref_rect.y1, ref_rect.y2);
			resolved << "," << rect.y1;
		}
		if(items.size() >= 3) {
			rect.x2 = compute(items[2], ref_rect.x2, rect.x1);
			resolved << "," << rect.x2;
		}
		if(items.size() >= 4) {
			rect.y2 = compute(items[3], ref_rect.y2, rect.y1);
			resolved << "," << rect.y2;
		}

		// std::cerr << "Rect " << rect_str << "\t: " << resolved.str() << "\n";

		ref_rect = rect;
		return resolved.str();
	}

	config& resolve_rects(const config& cfg) {
		config* newcfg = new config();
		for(config::all_children_iterator i = cfg.ordered_begin(); i != cfg.ordered_end(); ++i) {
			const std::pair<const std::string*,const config*>& value = *i;
			newcfg->add_child(*value.first,resolve_rects(*value.second));
		}

		for(string_map::const_iterator j = cfg.values.begin(); j != cfg.values.end(); ++j) {
			newcfg->values[j->first] = j->second;
		}

		if (cfg["rect"] != "") {
			newcfg->values["rect"] = resolve_rect(cfg["rect"]);
		}

		return *newcfg;
	}

}

theme::object::object() : loc_(empty_rect), relative_loc_(empty_rect),
                          last_screen_(empty_rect), xanchor_(object::FIXED), yanchor_(object::FIXED)
{}

theme::object::object(const config& cfg)
                   : loc_(read_rect(cfg)), relative_loc_(empty_rect),
				     last_screen_(empty_rect),
                     xanchor_(read_anchor(cfg["xanchor"])),
					 yanchor_(read_anchor(cfg["yanchor"]))
{}

SDL_Rect& theme::object::location(const SDL_Rect& screen) const
{
	if(last_screen_ == screen)
		return relative_loc_;

	last_screen_ = screen;

	switch(xanchor_) {
	case FIXED:
		relative_loc_.x = loc_.x;
		relative_loc_.w = loc_.w;
		break;
	case TOP_ANCHORED:
		relative_loc_.x = loc_.x;
		relative_loc_.w = screen.w - minimum<size_t>(XDim - loc_.w,screen.w);
		break;
	case BOTTOM_ANCHORED:
		relative_loc_.x = screen.w - minimum<size_t>(XDim - loc_.x,screen.w);
		relative_loc_.w = loc_.w;
		break;
	case PROPORTIONAL:
		relative_loc_.x = (loc_.x*screen.w)/XDim;
		relative_loc_.w = (loc_.w*screen.w)/XDim;
		break;
	default:
		assert(false);
	}

	switch(yanchor_) {
	case FIXED:
		relative_loc_.y = loc_.y;
		relative_loc_.h = loc_.h;
		break;
	case TOP_ANCHORED:
		relative_loc_.y = loc_.y;
		relative_loc_.h = screen.h - minimum<size_t>(YDim - loc_.h,screen.h);
		break;
	case BOTTOM_ANCHORED:
		relative_loc_.y = screen.h - minimum<size_t>(YDim - loc_.y,screen.h);
		relative_loc_.h = loc_.h;
		break;
	case PROPORTIONAL:
		relative_loc_.y = (loc_.y*screen.h)/YDim;
		relative_loc_.h = (loc_.h*screen.h)/YDim;
		break;
	default:
		assert(false);
	}

	relative_loc_.x = minimum<int>(relative_loc_.x,screen.w);
	relative_loc_.w = minimum<int>(relative_loc_.w,screen.w - relative_loc_.x);
	relative_loc_.y = minimum<int>(relative_loc_.y,screen.h);
	relative_loc_.h = minimum<int>(relative_loc_.h,screen.h - relative_loc_.y);

	return relative_loc_;
}

theme::object::ANCHORING theme::object::read_anchor(const std::string& str)
{
	static const std::string top_anchor = "top", left_anchor = "left",
	                         bot_anchor = "bottom", right_anchor = "right",
							 fixed_anchor = "fixed", proportional_anchor = "proportional";
	if(str == top_anchor || str == left_anchor)
		return TOP_ANCHORED;
	else if(str == bot_anchor || str == right_anchor)
		return BOTTOM_ANCHORED;
	else if(str == proportional_anchor)
		return PROPORTIONAL;
	else
		return FIXED;
}

theme::label::label()
{}

theme::label::label(const config& cfg)
      : object(cfg), text_(cfg["prefix"] + cfg["text"] + cfg["postfix"]),
	    icon_(cfg["icon"]), font_(atoi(cfg["font_size"].c_str()))
{
	if(font_ == 0)
		font_ = DefaultFontSize;
}

const std::string& theme::label::text() const
{
	return text_;
}

const std::string& theme::label::icon() const
{
	return icon_;
}

bool theme::label::empty() const
{
	return text_.empty() && icon_.empty();
}

size_t theme::label::font_size() const
{
	return font_;
}

theme::status_item::status_item(const config& cfg)
        : object(cfg),
		  prefix_(cfg["prefix"] + cfg["prefix_literal"]),
		  postfix_(cfg["postfix_literal"] + cfg["postfix"]),
          font_(atoi(cfg["font_size"].c_str()))
{
	if(font_ == 0)
		font_ = DefaultFontSize;

	const config* const label_child = cfg.child("label");
	if(label_child != NULL) {
		label_ = label(*label_child);
	}
}

const std::string& theme::status_item::prefix() const
{
	return prefix_;
}

const std::string& theme::status_item::postfix() const
{
	return postfix_;
}

const theme::label* theme::status_item::get_label() const
{
	return label_.empty() ? NULL : &label_;
}

size_t theme::status_item::font_size() const
{
	return font_;
}

theme::panel::panel(const config& cfg) : object(cfg), image_(cfg["image"])
{}

const std::string& theme::panel::image() const
{
	return image_;
}

theme::menu::menu() : context_(false)
{}

theme::menu::menu(const config& cfg) : object(cfg), context_(cfg["is_context_menu"] == "true"),
                                       title_(cfg["title"] + cfg["title_literal"]),
									   image_(cfg["image"]),
									   items_(config::split(cfg["items"]))
{}

bool theme::menu::is_context() const { return context_; }

const std::string& theme::menu::title() const { return title_; }

const std::string& theme::menu::image() const { return image_; }

const std::vector<std::string>& theme::menu::items() const { return items_; }

theme::theme(const config& cfg, const SDL_Rect& screen) : cfg_(resolve_rects(cfg))
{
	set_resolution(screen);
}

bool theme::set_resolution(const SDL_Rect& screen)
{
	bool result = false;

	const config::child_list& resolutions = cfg_.get_children("resolution");
	int current_rating = 1000000;
	config::child_list::const_iterator i, current = resolutions.end();
	for(i = resolutions.begin(); i != resolutions.end(); ++i) {
		const int width = atoi((**i)["width"].c_str());
		const int height = atoi((**i)["height"].c_str());
		LOG_DP << "comparing resolution " << screen.w << "," << screen.h << " to " << width << "," << height << "\n";
		if(screen.w >= width && screen.h >= height) {
			LOG_DP << "loading theme: " << width << "," << height << "\n";
			current = i;
			result = true;
			break;
		}

		const int rating = width*height;
		if(rating < current_rating) {
			current = i;
			current_rating = rating;
		}
	}

	if(current == resolutions.end()) {
		if(!resolutions.empty())
			lg::err(lg::display) << "No valid resolution found\n";
		return false;
	}

	panels_.clear();
	labels_.clear();
	status_.clear();
	menus_.clear();

	const config& cfg = **current;

	const config* const main_map_cfg = cfg.child("main_map");
	if(main_map_cfg != NULL) {
		main_map_ = object(*main_map_cfg);
	} else {
		main_map_ = object();
	}

	const config* const mini_map_cfg = cfg.child("mini_map");
	if(mini_map_cfg != NULL) {
		mini_map_ = object(*mini_map_cfg);
	} else {
		mini_map_ = object();
	}
	
	const config* const status_cfg = cfg.child("status");
	if(status_cfg != NULL) {
		for(config::child_map::const_iterator i = status_cfg->all_children().begin(); i != status_cfg->all_children().end(); ++i) {
			for(config::child_list::const_iterator j = i->second.begin(); j != i->second.end(); ++j) {
				status_.insert(std::pair<std::string,status_item>(i->first,status_item(**j)));
			}
		}
	}

	const config::child_list& panel_list = cfg.get_children("panel");
	for(config::child_list::const_iterator p = panel_list.begin(); p != panel_list.end(); ++p) {
		panels_.push_back(panel(**p));
	}

	const config::child_list& label_list = cfg.get_children("label");
	for(config::child_list::const_iterator lb = label_list.begin(); lb != label_list.end(); ++lb) {
		labels_.push_back(label(**lb));
	}

	const config::child_list& menu_list = cfg.get_children("menu");
	for(config::child_list::const_iterator m = menu_list.begin(); m != menu_list.end(); ++m) {
		const menu new_menu(**m);
		LOG_DP << "adding menu: " << (new_menu.is_context() ? "is context" : "not context") << "\n";
		if(new_menu.is_context())
			context_ = new_menu;
		else
			menus_.push_back(new_menu);

		LOG_DP << "done adding menu...\n";
	}

	return result;
}

const std::vector<theme::panel>& theme::panels() const
{
	return panels_;
}

const std::vector<theme::label>& theme::labels() const
{
	return labels_;
}

const std::vector<theme::menu>& theme::menus() const
{
	return menus_;
}

const theme::menu* theme::context_menu() const
{
	return context_.is_context() ? &context_ : NULL;
}

const theme::status_item* theme::get_status_item(const std::string& key) const
{
	const std::map<std::string,status_item>::const_iterator i = status_.find(key);
	if(i != status_.end())
		return &i->second;
	else
		return NULL;
}

const SDL_Rect& theme::main_map_location(const SDL_Rect& screen) const
{
	return main_map_.location(screen);
}

const SDL_Rect& theme::mini_map_location(const SDL_Rect& screen) const
{
	return mini_map_.location(screen);
}
