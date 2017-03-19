/*
   Copyright (C) 2006 - 2017 by Patrick Parker <patrick_x99@hotmail.com>
   wesnoth widget Copyright (C) 2003-5 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "construct_dialog.hpp"

#include "config_assign.hpp"
#include "formula/string_utils.hpp"
#include "game_config.hpp"
#include "gettext.hpp"
#include "sound.hpp"
#include "log.hpp"
#include "font/marked-up_text.hpp"
#include "font/standard_colors.hpp"
#include "scripting/plugins/context.hpp"
#include "scripting/plugins/manager.hpp"
#include "sdl/surface.hpp"
#include "sdl/rect.hpp"
#include "utils/general.hpp"

#include "utils/functional.hpp"

static lg::log_domain log_display("display");
#define ERR_DP LOG_STREAM(err, log_display)
#define LOG_DP LOG_STREAM(info, log_display)
#define DBG_DP LOG_STREAM(debug, log_display)
#define ERR_G  LOG_STREAM(err, lg::general)

namespace gui {

//static initialization
//note: style names are directly related to the panel image file names
const dialog::style& dialog::default_style = dialog_frame::default_style;
const dialog::style& dialog::message_style = dialog_frame::message_style;
const dialog::style dialog::hotkeys_style("menu", 0);
const int dialog::message_font_size = font::SIZE_PLUS;
const int dialog::caption_font_size = font::SIZE_LARGE;
const size_t dialog::left_padding = font::relative_size(10);
const size_t dialog::right_padding = font::relative_size(10);
const size_t dialog::image_h_pad = font::relative_size(/*image_ == nullptr ? 0 :*/ 10);
const size_t dialog::top_padding = font::relative_size(10);
const size_t dialog::bottom_padding = font::relative_size(10);

const int dialog::max_menu_width = -1;

menu * dialog::empty_menu = nullptr;
}

namespace {

std::vector<std::string> empty_string_vector;

} //end anonymous namespace

namespace gui {

dialog_textbox::~dialog_textbox()
{
	delete label_;
}

dialog::dimension_measurements::dimension_measurements() :
	x(-1),
	y(-1),
	interior(sdl::empty_rect),
	message(sdl::empty_rect),
	textbox(sdl::empty_rect),
	menu_width(0),
	panes(),
	label_x(-1),
	label_y(-1),
	menu_x(-1),
	menu_y(-1),
	menu_height(-1),
	image_x(-1),
	image_y(-1),
	caption_x(-1),
	caption_y(-1),
	buttons()
{
	//note: this is not defined in the header file to C++ ODR (one-definition rule)
	//since each inclusion of the header file uses a different version of empty_rect
	//(unnamed namespace and/or const object defined at declaration time).
}

dialog::dialog(CVideo& video, const std::string& title, const std::string& message,
		const DIALOG_TYPE type, const style& dialog_style) :
	video_(video),
	image_(nullptr),
	title_(title),
	style_(dialog_style),
	title_widget_(nullptr),
	message_(nullptr),
	type_(type),
	menu_(get_empty_menu(video)),
	preview_panes_(),
	button_pool_(),
	standard_buttons_(),
	extra_buttons_(),
	top_buttons_(),
	frame_buttons_(),
	topic_(),
	help_button_(nullptr),
	text_widget_(nullptr),
	frame_(nullptr),
	dim_(),
	result_(CONTINUE_DIALOG)
{
	switch(type)
	{
	case MESSAGE:
	default:
		break;
	case OK_ONLY:
		add_button(new standard_dialog_button(video_,_("OK"),0,true), BUTTON_STANDARD);
		break;
	case YES_NO:
		add_button(new standard_dialog_button(video_,_("Yes"),0,false), BUTTON_STANDARD);
		add_button(new standard_dialog_button(video_,_("No"),1,true), BUTTON_STANDARD);
		break;
	case OK_CANCEL:
		add_button(new standard_dialog_button(video_,_("OK"),0,false), BUTTON_STANDARD);
		add_button(new standard_dialog_button(video_,_("Cancel"),1,true), BUTTON_STANDARD);
		break;
	case CANCEL_ONLY:
		add_button(new standard_dialog_button(video_,_("Cancel"),0,true), BUTTON_STANDARD);
		break;
	case CLOSE_ONLY:
		add_button(new standard_dialog_button(video_,_("Close"),0,true), BUTTON_STANDARD);
		break;
	}
	//dialog creator should catch(button::error&) ?

	try {
		std::string msg = font::word_wrap_text(message, message_font_size, video_.getx() / 2, video_.gety() / 2);
		message_ = new label(video_, msg, message_font_size, font::NORMAL_COLOR, false);
	} catch(utf8::invalid_utf8_exception&) {
		ERR_DP << "Problem handling utf8 in message '" << message << "'" << std::endl;
		throw;
	}

}

dialog::~dialog()
{
	if(menu_ != empty_menu)
	{
		delete menu_;
	}
	delete title_widget_;
	delete message_;
	delete text_widget_;
	delete image_;
	delete frame_;

	button_pool_iterator b;
	for (b = button_pool_.begin(); b != button_pool_.end(); ++b) {
		delete b->first;
	}
//	pp_iterator p;
//	for (p = preview_panes_.begin(); p != preview_panes_.end(); ++p) {
//		delete (*p);
//	}
}

bool dialog::option_checked(unsigned int option_index)
{
	unsigned int i = 0;
	button_pool_iterator b;
	for (b = button_pool_.begin(); b != button_pool_.end(); ++b) {
		if(b->first->is_option()) {
			if(option_index == i++) {
				return b->first->checked();
			}
		}
	}
	return false;
}

void dialog::add_button(dialog_button *const btn, BUTTON_LOCATION loc)
{
	std::pair<dialog_button *, BUTTON_LOCATION> new_pair(btn,loc);
	button_pool_.push_back(new_pair);
	switch(loc)
	{
	case BUTTON_HELP:
		delete help_button_;
		help_button_ = btn;
		break;
	case BUTTON_EXTRA:
	case BUTTON_EXTRA_LEFT:
	case BUTTON_CHECKBOX:
	case BUTTON_CHECKBOX_LEFT:
		extra_buttons_.push_back(btn);
		break;
	case BUTTON_STANDARD:
		standard_buttons_.push_back(btn);
		break;
	case BUTTON_TOP:
		top_buttons_.push_back(btn);
		break;
	default:
		break;
	}
	btn->set_parent(this);
}

void dialog::add_button(dialog_button_info btn_info, BUTTON_LOCATION loc)
{
	dialog_button *btn = new dialog_button(video_, btn_info.label, button::TYPE_PRESS, CONTINUE_DIALOG, btn_info.handler);
	add_button(btn, loc);
}

void dialog::add_option(const std::string& label, bool checked, BUTTON_LOCATION loc, const std::string& help_string)
{
	gui::dialog_button *btn = new dialog_button(video_, label, button::TYPE_CHECK);
	btn->set_check(checked);
	btn->set_help_string(help_string);
	add_button(btn, loc);
}

void dialog::set_textbox(const std::string& text_widget_label,
				const std::string& text_widget_text,
				const int text_widget_max_chars, const unsigned int text_box_width)
{
	label *label_ptr = new label(video_, text_widget_label, message_font_size, font::NORMAL_COLOR, false);
	const bool editable_textbox = std::find(text_widget_text.begin(),text_widget_text.end(),'\n') == text_widget_text.end();
	text_widget_ = new dialog_textbox(label_ptr, video_, text_box_width, text_widget_text, editable_textbox, text_widget_max_chars);
	text_widget_->set_wrap(!editable_textbox);
}

void dialog::set_menu(const std::vector<std::string> &menu_items, menu::sorter* sorter)
{
	set_menu(new gui::menu(video_, menu_items, (type_==MESSAGE),
		-1, dialog::max_menu_width, sorter, &menu::default_style, false));
}

/**
 * Changes (or sets, if not previously set) the dialog's menu items to the
 * provided list. If @a keep_selected is true, the selection will remain at
 * the same index if possible (otherwise it gets reset to zero).
 */
void dialog::set_menu_items(const std::vector<std::string> &menu_items, bool keep_selection)
{
	if(menu_ == empty_menu) {
		set_menu(menu_items);
	} else {
		menu_->set_items(menu_items, true, keep_selection);

		for(pp_iterator i = preview_panes_.begin(); i != preview_panes_.end(); ++i) {
			(**i).set_selection(menu_->selection());
		}
	}
}

/**
 * Provides create-on-use semantics for empty_menu.
 * This is called by dialog's constructor, so other code can use empty_menu directly.
 */
menu * dialog::get_empty_menu(CVideo& video)
{
	if ( empty_menu == nullptr ) {
		empty_menu = new gui::menu(video, empty_string_vector, false, -1, -1, nullptr, &menu::simple_style);
		empty_menu->leave();
	}
	return empty_menu;
}

int dialog::show(int xloc, int yloc)
{
	layout(xloc, yloc);
	return show();
}

int dialog::show()
{
	if (video_.faked()) {
		plugins_manager * pm = plugins_manager::get();
		if (pm && pm->any_running()) {
			pm->notify_event("show_dialog", config(config_of
					("title",	title_)
					("message",	message_->get_text())
			));

			plugins_context pc("Dialog");
			pc.set_callback("set_result", [this](const config& cfg) { set_result(cfg["result"].to_int(CLOSE_DIALOG)); }, false);

			while (pm->any_running() && result() == CONTINUE_DIALOG) {
				pc.play_slice();
			}
			return result();
		}
		return CLOSE_DIALOG;
	}

	if(video_.update_locked()) {
		ERR_DP << "display locked ignoring dialog '" << title_ << "' '" << message_->get_text() << "'" << std::endl;
		return CLOSE_DIALOG;
	}

	LOG_DP << "showing dialog '" << title_ << "' '" << message_->get_text() << "'\n";
	if(dim_.interior == sdl::empty_rect) { layout(); }

	//create the event context, remember to instruct any passed-in widgets to join it
	const events::event_context dialog_events_context;
	const dialog_manager manager;

	get_frame().join();

	//draw
	draw_frame();
	update_widget_positions();
	draw_contents();

	//process
	plugins_manager::get()->notify_event("show_dialog", config(config_of
			("title",	title_)
			("message",	message_->get_text())
		));

	plugins_context pc("Dialog");
	pc.set_callback("set_result", [this](const config& cfg) { set_result(cfg["result"].to_int(CLOSE_DIALOG)); }, false);

	dialog_process_info dp_info;
	do
	{
		events::pump();
		set_result(process(dp_info));
		if(!done()) {
			refresh();
		}
		action(dp_info);
		dp_info.cycle();

		pc.play_slice();
	} while(!done());

	clear_background();
	return result();
}

void dialog::draw_contents()
{
	if(!preview_panes_.empty()) {
		for(pp_iterator i = preview_panes_.begin(); i != preview_panes_.end(); ++i) {
			preview_pane *pane = *i;
			if(!pane->handler_members().empty())
			{
				pane->draw();
				pane->needs_restore_ = false; //prevent panes from drawing over members
			}
		}
	}
	events::raise_draw_event(); //draw widgets

	video_.flip();
}

dialog_frame& dialog::get_frame()
{
	if(frame_ == nullptr) {
		frame_buttons_.clear();
		for(button_iterator b = standard_buttons_.begin(); b != standard_buttons_.end(); ++b)
		{
			frame_buttons_.push_back(*b);
		}
		frame_ = new dialog_frame(video_, title_, style_,  true, &frame_buttons_, help_button_);
	}
	return *frame_;
}

void dialog::clear_background() {
	delete frame_;
	frame_ = nullptr;
}

void dialog::draw_frame()
{
	get_frame().draw();
}

void dialog::update_widget_positions()
{
	if(!preview_panes_.empty()) {
		for(pp_iterator i = preview_panes_.begin(); i != preview_panes_.end(); ++i) {
			preview_pane *pane = *i;
			pane->leave();
			pane->join();
			pane->set_location(dim_.panes.find(pane)->second);
		}
	}
	if(text_widget_) {
		text_widget_->leave();
		text_widget_->join();
		text_widget_->set_location(dim_.textbox);
		if(text_widget_->get_label()) {
			text_widget_->get_label()->set_location(dim_.label_x, dim_.label_y);
		}
	}
	if(get_menu().height() > 0) {
		menu_->leave();
		menu_->join();
		menu_->set_numeric_keypress_selection(text_widget_ == nullptr);
		menu_->set_width( dim_.menu_width );
		menu_->set_max_width( dim_.menu_width ); //lock the menu width
		if(dim_.menu_height >= 0) {
			menu_->set_max_height( dim_.menu_height );
		}
		menu_->set_location( dim_.menu_x, dim_.menu_y );
	}
	if(image_) {
		image_->leave();
		image_->join();
		image_->set_location(dim_.image_x, dim_.image_y);
		if(image_->caption()) {
			image_->caption()->set_location(dim_.caption_x, dim_.caption_y);
		}
	}
	button_iterator b;
	for(b = top_buttons_.begin(); b != top_buttons_.end(); ++b) {
		dialog_button *btn = *b;
		btn->leave();
		btn->join();
		std::pair<int,int> coords = dim_.buttons.find(btn)->second;
		btn->set_location(coords.first, coords.second);
	}
	for(b = extra_buttons_.begin(); b != extra_buttons_.end(); ++b) {
		dialog_button *btn = *b;
		btn->leave();
		btn->join();
		std::pair<int,int> coords = dim_.buttons.find(btn)->second;
		btn->set_location(coords.first, coords.second);
	}
	for(b = standard_buttons_.begin(); b != standard_buttons_.end(); ++b) {
		dialog_button *btn = *b;
		btn->leave();
		btn->join();
	}
	if(help_button_) {
		help_button_->leave();
		help_button_->join();
	}
	message_->set_location(dim_.message);
	message_->leave();
	message_->join();
}

void dialog::refresh()
{
	video_.flip();
	CVideo::delay(10);
}

dialog::dimension_measurements dialog::layout(int xloc, int yloc)
{
	CVideo& screen = video_;
	const surface& scr = screen.getSurface();

	dimension_measurements dim;
	dim.x = xloc;
	dim.y = yloc;

	const bool use_textbox = (text_widget_ != nullptr);
	int text_widget_width = 0;
	int text_widget_height = 0;
	if(use_textbox) {
		const SDL_Rect& area = font::text_area(text_widget_->text(),message_font_size);
		dim.textbox.w = std::min<size_t>(screen.getx()/2,std::max<size_t>(area.w,text_widget_->width()));
		dim.textbox.h = std::min<size_t>(screen.gety()/2,std::max<size_t>(area.h,text_widget_->height()));
		text_widget_width = dim.textbox.w;
		text_widget_width += (text_widget_->get_label() == nullptr) ? 0 : text_widget_->get_label()->width();
		text_widget_height = dim.textbox.h + message_font_size;
	}

	const bool use_menu = (get_menu().height() > 0);
	if(!message_->get_text().empty()) {
		dim.message.w = message_->width();
		dim.message.h = message_->height();
	}
	unsigned int caption_width = 0;
	unsigned int caption_height = 0;
	if (image_ != nullptr && image_->caption() != nullptr) {
		caption_width = image_->caption()->width();
		caption_height = image_->caption()->height();
	}

	int check_button_height = 0;
	int left_check_button_height = 0;
	int top_button_height = 0;
	const int button_height_padding = 5;

	for(button_pool_const_iterator b = button_pool_.begin(); b != button_pool_.end(); ++b) {
		dialog_button const *const btn = b->first;
		switch(b->second)
		{
		case BUTTON_EXTRA:
		case BUTTON_CHECKBOX:
			check_button_height += btn->height() + button_height_padding;
			break;
		case BUTTON_EXTRA_LEFT:
		case BUTTON_CHECKBOX_LEFT:
			left_check_button_height += btn->height() + button_height_padding;
			break;
		case BUTTON_TOP:
			top_button_height += btn->height() + button_height_padding;
			break;
		case BUTTON_STANDARD:
		default:
			break;
		}
	}
	check_button_height = std::max<int>(check_button_height, left_check_button_height);

	size_t above_preview_pane_height = 0, above_left_preview_pane_width = 0, above_right_preview_pane_width = 0;
	size_t preview_pane_height = 0, left_preview_pane_width = 0, right_preview_pane_width = 0;
	if(!preview_panes_.empty()) {
		for(pp_const_iterator i = preview_panes_.begin(); i != preview_panes_.end(); ++i) {
			preview_pane const *const pane = *i;
			const SDL_Rect& rect = pane->location();
			if(pane->show_above() == false) {
				preview_pane_height = std::max<size_t>(rect.h,preview_pane_height);
				if(pane->left_side()) {
					left_preview_pane_width += rect.w;
				} else {
					right_preview_pane_width += rect.w;
				}
			} else {
				above_preview_pane_height = std::max<size_t>(rect.h,above_preview_pane_height);
				if(pane->left_side()) {
					above_left_preview_pane_width += rect.w;
				} else {
					above_right_preview_pane_width += rect.w;
				}
			}
		}
	}

	const int menu_hpadding = font::relative_size((dim.message.h > 0 && use_menu) ? 10 : 0);
	const size_t image_h_padding = (image_ == nullptr)? 0 : image_h_pad;
	const size_t padding_width = left_padding + right_padding + image_h_padding;
	const size_t padding_height = top_padding + bottom_padding + menu_hpadding;
	const size_t image_width = (image_ == nullptr) ? 0 : image_->width();
	const size_t image_height = (image_ == nullptr) ? 0 : image_->height();
	const size_t total_text_height = dim.message.h + caption_height;

	size_t text_width = dim.message.w;
	if(caption_width > text_width)
		text_width = caption_width;

	// Prevent the menu to be larger than the screen
	dim.menu_width = menu_->width();
	if(dim.menu_width + image_width + padding_width + left_preview_pane_width + right_preview_pane_width > static_cast<size_t>(scr->w))
		dim.menu_width = scr->w - image_width - padding_width - left_preview_pane_width - right_preview_pane_width;
	if(dim.menu_width > text_width)
		text_width = dim.menu_width;


	size_t total_width = image_width + text_width + padding_width;

	if(text_widget_width+left_padding+right_padding > total_width)
		total_width = text_widget_width+left_padding+right_padding;

	//Prevent the menu from being too skinny
	if(use_menu && preview_panes_.empty() &&
		total_width > dim.menu_width + image_width + padding_width) {
		dim.menu_width = total_width - image_width - padding_width;
	}

	const size_t text_and_image_height = image_height > total_text_height ? image_height : total_text_height;

	const int top_widgets_height = std::max<int>(text_widget_height, top_button_height);

	const int total_height = text_and_image_height + padding_height + menu_->height() +
		top_widgets_height + check_button_height;

	dim.interior.w = std::max<int>(total_width,above_left_preview_pane_width + above_right_preview_pane_width);
	dim.interior.h = std::max<int>(total_height,static_cast<int>(preview_pane_height));
	dim.interior.x = std::max<int>(0,dim.x >= 0 ? dim.x : scr->w/2 - (dim.interior.w + left_preview_pane_width + right_preview_pane_width)/2);
	dim.interior.y = std::max<int>(0,dim.y >= 0 ? dim.y : scr->h/2 - (dim.interior.h + above_preview_pane_height)/2);

	DBG_DP << "above_preview_pane_height: " << above_preview_pane_height << "; "
		<< "dim.interior.y: " << scr->h/2 << " - " << (dim.interior.h + above_preview_pane_height)/2 << " = "
		<< dim.interior.y << "; " << "dim.interior.h: " << dim.interior.h << "\n";

	if(dim.x <= -1 || dim.y <= -1) {
		dim.x = dim.interior.x + left_preview_pane_width;
		dim.y = dim.interior.y + above_preview_pane_height;
	}

	if(dim.x + dim.interior.w > scr->w) {
		dim.x = scr->w - dim.interior.w;
		if(dim.x < dim.interior.x) {
			dim.interior.x = dim.x;
		}
	}

	const int frame_top_pad = get_frame().top_padding();
	const int frame_bottom_pad = get_frame().bottom_padding();
	if(dim.y + dim.interior.h + frame_bottom_pad > scr->h) {
		dim.y = std::max<int>(frame_top_pad, scr->h - dim.interior.h - frame_bottom_pad);
		if(dim.y < dim.interior.y) {
			dim.interior.y = dim.y;
		}
	}

	dim.interior.w += left_preview_pane_width + right_preview_pane_width;
	dim.interior.h += above_preview_pane_height;

	const int max_height = scr->h - dim.interior.y - frame_bottom_pad;
	if(dim.interior.h > max_height) {
		//try to rein in the menu height a little bit
		const int menu_height = menu_->height();
		if(menu_height > 0) {
			dim.menu_height = std::max<int>(1, max_height - dim.interior.h + menu_height);
			dim.interior.h -= menu_height - dim.menu_height;
		}
	}

	//calculate the positions of the preview panes to the sides of the dialog
	if(!preview_panes_.empty()) {

		int left_preview_pane = dim.interior.x;
		int right_preview_pane = dim.interior.x + total_width + left_preview_pane_width;
		int above_left_preview_pane = dim.interior.x + dim.interior.w/2;
		int above_right_preview_pane = above_left_preview_pane;

		for(pp_const_iterator i = preview_panes_.begin(); i != preview_panes_.end(); ++i) {
		preview_pane const *const pane = *i;
			SDL_Rect area = pane->location();

			if(pane->show_above() == false) {
				area.y = dim.y;
				area.h = dim.interior.h;
				if(pane->left_side()) {
					area.x = left_preview_pane;
					left_preview_pane += area.w;
				} else {
					area.x = right_preview_pane;
					right_preview_pane += area.w;
				}
			} else {
				area.y = dim.interior.y;
				area.h = above_preview_pane_height;
				if(pane->left_side()) {
					area.x = above_left_preview_pane - area.w;
					above_left_preview_pane -= area.w;
				} else {
					area.x = above_right_preview_pane;
					above_right_preview_pane += area.w;
				}
			}
			dim.panes[*i] = area;
		}
	}

	const int text_widget_y = dim.y + top_padding + text_and_image_height - 6 + menu_hpadding;

	if(use_textbox) {
		dim.textbox.x = dim.x + left_padding + text_widget_width - dim.textbox.w;
		dim.textbox.y = text_widget_y + (text_widget_height - dim.textbox.h)/2;
		dim.label_x = dim.x+left_padding;
		dim.label_y = dim.textbox.y;
	}

	if(top_buttons_.empty() == false) {
		int top_buttons_y = text_widget_y;

		for(button_const_iterator b = top_buttons_.begin(); b != top_buttons_.end(); ++b) {
			const dialog_button& btn = **b;

			std::pair<int, int> coords(0, 0);

			coords.first = dim.x + total_width - btn.width() - ButtonHPadding;
			// As a special case, if there's only one button
			// and a text box, try to align the button to it
			if(top_buttons_.size() == 1 && use_textbox) {
				coords.second = text_widget_y + (text_widget_height - btn.height())/2;
			} else {
				coords.second = top_buttons_y;
			}

			top_buttons_y += btn.height() + button_height_padding;

			dim.buttons[*b] = coords;
		}
	}

	dim.menu_x = dim.x+image_width+left_padding+image_h_padding;
	dim.menu_y = dim.y+top_padding+text_and_image_height+menu_hpadding+ std::max<int>(top_button_height - button_height_padding, (use_textbox ? text_widget_->location().h + top_padding : 0));

	dim.message.x = dim.x + left_padding;
	dim.message.y = dim.y + top_padding + caption_height;

	if(image_ != nullptr) {
		const int x = dim.x + left_padding;
		const int y = dim.y + top_padding;
		dim.message.x += image_width + image_h_padding;
		dim.image_x = x;
		dim.image_y = y;
		dim.caption_x = dim.x + image_width + left_padding + image_h_padding;
		dim.caption_y = dim.y + top_padding;
	}

	//set the position of any tick boxes. by default, they go right below the menu,
	//slammed against the right side of the dialog
	if(extra_buttons_.empty() == false) {
		int options_y = dim.menu_y + menu_->height() + menu_hpadding + button_height_padding;
		int options_left_y = options_y;
		for(button_pool_const_iterator b = button_pool_.begin(); b != button_pool_.end(); ++b) {
		dialog_button const *const btn = b->first;
		std::pair<int,int> coords;
			switch(b->second)
			{
			case BUTTON_EXTRA:
			case BUTTON_CHECKBOX:
				coords.first = dim.x + total_width - btn->width() - ButtonHPadding;
				coords.second = options_y;
				dim.buttons[b->first] = coords;
				options_y += btn->height() + button_height_padding;
				break;
			case BUTTON_EXTRA_LEFT:
			case BUTTON_CHECKBOX_LEFT:
				coords.first = dim.x + ButtonHPadding;
				coords.second = options_left_y;
				dim.buttons[b->first] = coords;
				options_left_y += btn->height() + button_height_padding;
				break;
			case BUTTON_STANDARD:
			default:
				break;
			}
		}
	}
	set_layout(dim);
	return dim;
}

void dialog::set_layout(dimension_measurements &new_dim) {
	get_frame().layout(new_dim.interior);
	dim_ = new_dim;
}


int dialog::process(dialog_process_info &info)
{

	int mousex, mousey;
	int mouse_flags = SDL_GetMouseState(&mousex,&mousey);

	info.new_right_button = (mouse_flags&SDL_BUTTON_RMASK) != 0;
	info.new_left_button = (mouse_flags&SDL_BUTTON_LMASK) != 0;
	info.new_key_down = info.key[SDLK_SPACE] || info.key[SDLK_RETURN] ||
					info.key[SDLK_ESCAPE] || info.key[SDLK_KP_ENTER];
	info.double_clicked = menu_->double_clicked();
	const bool use_menu = (menu_ != empty_menu);
	const bool use_text_input = (text_widget_!=nullptr);
	const bool has_input = (use_menu||use_text_input);//input of any sort has to be made

	if((((!info.key_down && (info.key[SDLK_RETURN] || info.key[SDLK_KP_ENTER])) || info.double_clicked) &&
	   (type_ == YES_NO || type_ == OK_CANCEL || type_ == OK_ONLY || type_ == CLOSE_ONLY))) {

		return (use_menu ? menu_->selection() : 0);
	}

	//escape quits from the dialog -- unless it's an "ok" dialog with input
	if(!info.key_down && info.key[SDLK_ESCAPE] && !(type_ == OK_ONLY && has_input)) {
		return (CLOSE_DIALOG);
	}

	//inform preview panes when there is a new menu selection
	if((menu_->selection() != info.selection) || info.first_time) {
		info.selection = menu_->selection();
		int selection = info.selection;
		if(selection < 0) {
			selection = 0;
		}
		if(!preview_panes_.empty()) {
			for(pp_iterator i = preview_panes_.begin(); i != preview_panes_.end(); ++i) {
				(**i).set_selection(selection);
				if(info.first_time) {
					(**i).set_dirty();
				}
			}
		}
	}

	info.first_time = false;

	if(use_menu) {
		//get any drop-down choice or context-menu click
		const int selection = menu_->process();
		if(selection != -1)
		{
			return (selection);
		}
	}

	draw_frame();
	//draw_contents();

	events::raise_process_event();
	events::raise_draw_event();

	//left-clicking outside of a drop-down or context-menu should close it
	if (info.new_left_button && !info.left_button) {
		if (standard_buttons_.empty() && !sdl::point_in_rect(mousex,mousey, menu_->location())) {
			if (use_menu)
				sound::play_UI_sound(game_config::sounds::button_press);
			return CLOSE_DIALOG;
			}
	}

	//right-clicking outside of a dialog should close it unless a choice is required
	//note: this will also close any context-menu or drop-down when it is right-clicked
	//      but that may be changed to allow right-click selection instead.
	if (info.new_right_button && !info.right_button) {
		if( standard_buttons_.empty()
		|| (!sdl::point_in_rect(mousex,mousey,get_frame().get_layout().exterior)
		&& type_ != YES_NO && !(type_ == OK_ONLY && has_input))) {
			sound::play_UI_sound(game_config::sounds::button_press);
			return CLOSE_DIALOG;
		}
	}

	//any keypress should close a dialog if it has one standard button (or less)
	//and no menu options.
	if (info.new_key_down && !info.key_down) {
		if (standard_buttons_.size() < 2 && !has_input)
			return CLOSE_DIALOG;
	}

	//now handle any button presses
	for(button_pool_iterator b = button_pool_.begin(); b != button_pool_.end(); ++b) {
		if(b->first->pressed()) {
			return b->first->action(info);
		}
	}

	return CONTINUE_DIALOG;
}

int dialog_button::action(dialog_process_info &info) {
	if(handler_ != nullptr) {
		menu &menu_ref = parent_->get_menu();
		dialog_button_action::RESULT res = handler_->button_pressed(menu_ref.selection());

		if(res == DELETE_ITEM || res == CLOSE_DIALOG) {
			return res;
		}

		//reset button-tracking flags so that if the action displays a dialog, a button-press
		//at the end of the dialog won't be mistaken for a button-press in this dialog.
		//(We should eventually use a proper event-handling system instead of tracking
		//flags to avoid problems like this altogether).
		info.clear_buttons();
		return CONTINUE_DIALOG;
	}
	return simple_result_;
}

void dialog::action(dialog_process_info& info)
{
	//default way of handling a "delete item" request
	if(result() == DELETE_ITEM) {
		const int selection = menu_->selection();
		if(selection >= 0) {
			menu_->erase_item(selection);
		}
		// was used before to auto close empty menu
		//if(menu_->number_of_items() == 0) {
		//	set_result(CLOSE_DIALOG);
		//} else {

		set_result(CONTINUE_DIALOG);
				info.first_time = true;

	}
}

int standard_dialog_button::action(dialog_process_info &/*info*/) {
	//if the menu is not used, then return the index of the
	//button pressed, otherwise return the index of the menu
	//item selected if the last button is not pressed, and
	//cancel (-1) otherwise
	if(dialog()->get_menu().height() <= 0) {
		return simple_result_;
	} else if((simple_result_ == 0 && is_last_) || !is_last_) {
		return (dialog()->get_menu().selection());
	}
	return CLOSE_DIALOG;
}

void dialog::set_image(surface surf, const std::string &caption)
{
	label *label_ptr = nullptr;
	if(!caption.empty()) {
		label_ptr = new label(video_, caption, caption_font_size, font::NORMAL_COLOR, false);
	}
	set_image( new dialog_image(label_ptr, video_, surf ));
}

void dialog_image::draw_contents()
{
	video().blit_surface(location().x, location().y, surf_);
}

int filter_textbox::get_index(int selection) const {
	// don't translate special values
	if(selection < 0) {
		return selection;
	}
	//we must the header row value to the index to ignore this row and
	//then subtract it from the result to return the index not including
	//the possible header row.

	if (selection + header_row_ >= index_map_.size()) {
		return -1; // bad index, cancel
	}

	return index_map_[selection+header_row_]-header_row_;
}

void filter_textbox::delete_item(int selection) {
	// use the real selection
	size_t adjusted_selection = selection + header_row_;

	if (adjusted_selection >= index_map_.size())
		return;

	filtered_items_.erase(filtered_items_.begin() + adjusted_selection);
	items_to_filter_.erase(items_to_filter_.begin() + index_map_[adjusted_selection]);
	items_.erase(items_.begin() + index_map_[adjusted_selection]);
	index_map_.erase(index_map_.begin() + adjusted_selection);

	// don't forget to also shift the next index values
	// this assume that index_map_and items_ have the same order
	for(size_t i = adjusted_selection; i < index_map_.size(); ++i) {
		index_map_[i] = index_map_[i]-1;
	}

	//for now, assume the dialog menu item is deleted using DELETE_ITEM
	/* dialog_.set_menu_items(filtered_items_); */
}

void filter_textbox::handle_text_changed(const ucs4::string& text) {
	const std::vector<std::string> words = utils::split(unicode_cast<utf8::string>(text),' ');
	if (words == last_words)
		return;
	last_words = words;

	filtered_items_.clear();
	index_map_.clear();

	if(header_row_ == 1) {
			filtered_items_.push_back(items_[0]);
			index_map_.push_back(0);
	}

	// we keep all items containing each word
	for(size_t n = header_row_; n < items_to_filter_.size(); ++n) {
		std::vector<std::string>::const_iterator w = words.begin();
		for(; w != words.end(); ++w)
		{
			if (std::search(items_to_filter_[n].begin(), items_to_filter_[n].end(),
						w->begin(), w->end(),
						chars_equal_insensitive) == items_to_filter_[n].end())
				break; // one word doesn't match, we don't reach words.end()
		}
		if (w == words.end()) {
			// all words have matched, keep the item
			filtered_items_.push_back(items_[n]);
			index_map_.push_back(n);
		}
	}

	dialog_.set_menu_items(filtered_items_);
}

}//end namespace gui
