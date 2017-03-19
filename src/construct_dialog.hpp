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

#ifndef CONSTRUCT_DIALOG_H_INCLUDED
#define CONSTRUCT_DIALOG_H_INCLUDED

#include "show_dialog.hpp"

#include "widgets/label.hpp"
#include "widgets/textbox.hpp"
#include "key.hpp"

namespace gui {

struct dialog_process_info
{
public:
	dialog_process_info() :
		key(),
		left_button(true),
		right_button(true),
		key_down(true),
		first_time(true),
		double_clicked(false),
		new_left_button(false),
		new_right_button(false),
		new_key_down(false),
		selection(-1),
		clear_buttons_(false)
	{}

	void clear_buttons() {
		clear_buttons_ = true;
	}

	void cycle() {
		if(clear_buttons_) {
			left_button = true;
			right_button = true;
			key_down = true;
			clear_buttons_ = false;
		} else {
			left_button = new_left_button;
			right_button = new_right_button;
			key_down = new_key_down;
		}
	}
	CKey key;
	bool left_button, right_button, key_down, first_time, double_clicked;
	bool new_left_button, new_right_button, new_key_down;
	int selection;
private:
	bool clear_buttons_;
};

class dialog_image : public widget {
public:
	dialog_image(label *const caption, CVideo &video, surface img) : widget(video, false),
	  surf_(img), caption_(caption)
	{
		if(!img.null()) {
			set_measurements(img->w, img->h);
		}
	}
	~dialog_image() { delete caption_; }

	//surface surface() const { return surf_; }
	label *caption() const { return caption_; }
	void draw_contents();

	sdl_handler_vector handler_members() {
		sdl_handler_vector h;
		if(caption_) h.push_back(caption_);
		return h;
	}
private:

	surface surf_;
	label *caption_;
};

class dialog_textbox : public textbox {
public:
	dialog_textbox(label *const label_widget, CVideo &video, int width, const std::string& text="", bool editable=true, size_t max_size = 256, int font_size = font::SIZE_PLUS, double alpha = 0.4, double alpha_focus = 0.2)
		: textbox(video, width, text, editable, max_size, font_size, alpha, alpha_focus, false),
		label_(label_widget)
	{}
	virtual ~dialog_textbox();

	label *get_label() const { return label_; }

	sdl_handler_vector handler_members() {
		sdl_handler_vector h = textbox::handler_members();
		if(label_) h.push_back(label_);
		return h;
	}
private:
	//forbidden operations
	dialog_textbox(const dialog_textbox&);
	void operator=(const dialog_textbox&);

	label *label_;
};

class dialog;

class filter_textbox : public gui::dialog_textbox {
public:
	filter_textbox(CVideo& video, const std::string& header,
			const std::vector<std::string>& items,
			const std::vector<std::string>& items_to_filter, size_t header_row,
			dialog& dialog, int width = 250) :
		dialog_textbox(new label(video, header), video, width),
		items_(items),
		items_to_filter_(items_to_filter),
		filtered_items_(),
		index_map_(),
		last_words(1, ""), // dummy word to trigger an update
		header_row_(header_row),
		dialog_(dialog)
	{
		set_text("");
	}

	// current menu selection is based on a possibly filtered view,
	// and thus may differ from the original, unfiltered index
	int get_index(int selection) const;
	void delete_item(int selection);

private:
	std::vector<std::string> items_, items_to_filter_, filtered_items_;
	std::vector<int> index_map_;
	std::vector<std::string> last_words;
	size_t header_row_;
	gui::dialog& dialog_;
	virtual void handle_text_changed(const ucs4::string& text);
};

class dialog_button : public button {
public:
	dialog_button(CVideo& video, const std::string& label, TYPE type=TYPE_PRESS,
		int simple_result=CONTINUE_DIALOG, dialog_button_action *handler=nullptr)
		: button(video,label,type,"",DEFAULT_SPACE,false), simple_result_(simple_result),
		parent_(nullptr), handler_(handler)
	{}
	void set_parent(class dialog *parent) {
		parent_ = parent;
	}
	bool is_option() const {
		return (type_ == TYPE_CHECK);
	}
	virtual int action(dialog_process_info &info);
protected:
	class dialog *dialog() const { return parent_; }
	const int simple_result_;
private:
	class dialog *parent_;
	dialog_button_action *handler_;
};

class standard_dialog_button : public dialog_button {
public:
	standard_dialog_button(CVideo& video, const std::string& label, const int index, const bool is_last)
		: dialog_button(video,label,TYPE_PRESS,index), is_last_(is_last)
	{}
	int action(dialog_process_info &info);
private:
	const bool is_last_;
};


class dialog {
public:
	enum BUTTON_LOCATION { BUTTON_STANDARD, BUTTON_EXTRA, BUTTON_EXTRA_LEFT, BUTTON_CHECKBOX, BUTTON_CHECKBOX_LEFT, BUTTON_HELP, BUTTON_TOP };
	struct dimension_measurements {
		dimension_measurements();
		int x, y;
		SDL_Rect interior, message, textbox;
		unsigned int menu_width;
		std::map<preview_pane *, SDL_Rect > panes;
		int label_x, label_y;
		int menu_x, menu_y, menu_height;
		int image_x, image_y, caption_x, caption_y;
		std::map<dialog_button *, std::pair<int,int> > buttons;
		//use get_frame().get_layout() to check frame dimensions
	};
	typedef dialog_frame::style style;

private:
	typedef std::vector<preview_pane *>::iterator pp_iterator;
	typedef std::vector<preview_pane *>::const_iterator pp_const_iterator;
	typedef std::vector<dialog_button *>::iterator button_iterator;
	typedef std::vector<dialog_button *>::const_iterator button_const_iterator;
	typedef std::vector< std::pair<dialog_button *, BUTTON_LOCATION> >::iterator button_pool_iterator;
	typedef std::vector< std::pair<dialog_button *, BUTTON_LOCATION> >::const_iterator button_pool_const_iterator;

public:

	//Static members
	static const style& default_style;
	static const style& message_style;
	static const style hotkeys_style;
	static const int message_font_size;
	static const int caption_font_size;
	static const int max_menu_width;
	static const size_t left_padding;
	static const size_t right_padding;
	static const size_t image_h_pad;
	static const size_t top_padding;
	static const size_t bottom_padding;

	//Constructor & destructor
	//dialog - throws button::error() if standard buttons fail to initialize
	//         throws utf8::invalid_utf8_exception() if message is invalid
	dialog(CVideo& video,
	       const std::string& title="",
	       const std::string& message="",
	       const DIALOG_TYPE type=MESSAGE,
	       const style& dialog_style=default_style);
	virtual ~dialog();

	//Adding components - the dialog will manage the memory of
	//these widgets, therefore do not attempt to reference its
	//widgets after destroying it
	void set_image(dialog_image *const img) { delete image_; image_ = img; }
	void set_image(surface surf, const std::string &caption="");
	void set_menu(menu *const m) { if ( menu_ != empty_menu ) delete menu_;
	                               menu_ =  m == nullptr ? empty_menu : m; }
	void set_menu(const std::vector<std::string> & menu_items, menu::sorter* sorter=nullptr);
	void set_menu_items(const std::vector<std::string> &menu_items, bool keep_selection=false);

	//add_pane - preview panes are not currently memory managed
	//(for backwards compatibility)
	void add_pane(preview_pane *const pp) { preview_panes_.push_back(pp); }
	void set_panes(std::vector<preview_pane*> panes) { preview_panes_ = panes; }
	void set_textbox(dialog_textbox *const box) {
		delete text_widget_;
		text_widget_ = box;
	}
	void set_textbox(const std::string& text_widget_label="",
				const std::string &text_widget_text="",
				const int text_widget_max_chars = 256,
				const unsigned int text_box_width = font::relative_size(350));
	void add_button(dialog_button *const btn, BUTTON_LOCATION loc);
	void add_button(dialog_button_info btn_info, BUTTON_LOCATION loc=BUTTON_EXTRA);
	void add_option(const std::string& label, bool checked=false, BUTTON_LOCATION loc=BUTTON_CHECKBOX, const std::string& help_string = "");

	//Specific preparations
	//layout - determines dialog measurements based on all components
	virtual dimension_measurements layout(int xloc=-1, int yloc=-1);
	void set_layout(dimension_measurements &new_dim);
	dimension_measurements get_layout() const { return dim_; }
	dialog_frame& get_frame();
	void set_basic_behavior(DIALOG_TYPE type) { type_ = type; }

	//Launching the dialog
	//show - the return value of this method should be the same as result()
	int show(int xloc, int yloc);
	int show();

	//Results
	int result() const { return result_; }
	menu &get_menu() { return *menu_; }
	bool done() const { return (result_ != CONTINUE_DIALOG); }
	std::string textbox_text() const { return text_widget_->text();}
	dialog_textbox& get_textbox() const { return *text_widget_; }
	bool option_checked(unsigned int option_index=0);
	CVideo& get_video() { return video_; }

	/// Explicit freeing of class static resources.
	/// Must not be called if any instances of this class exist.
	/// Should be called if the display goes out of scope.
	/// (Currently called by ~game_launcher.)
	static void delete_empty_menu()  { delete empty_menu; empty_menu = nullptr; }

protected:
	void set_result(const int result) { result_ = result; }

	//action - invoked at the end of the dialog-processing loop
	virtual void action(dialog_process_info &dp_info);
	//refresh - forces the display to refresh
	void refresh();

	label& get_message() const { return *message_; }

private:
	void clear_background();
	void draw_frame();
	void update_widget_positions();
	void draw_contents();

	//process - execute a single dialog processing loop and return the result
	int process(dialog_process_info &info);

	/// A pointer to this empty menu is used instead of nullptr (for menu_).
	static menu * empty_menu;
	/// Provides create-on-use semantics for empty_menu.
	static menu * get_empty_menu(CVideo& video);

	//Members
	CVideo& video_;
	dialog_image *image_;
	std::string title_;
	const style& style_;
	label *title_widget_, *message_;
	DIALOG_TYPE type_;
	gui::menu *menu_; // Never nullptr; it equals empty_menu if there is currently no menu.
	std::vector<preview_pane*> preview_panes_;
	std::vector< std::pair<dialog_button*,BUTTON_LOCATION> > button_pool_;
	std::vector<dialog_button*> standard_buttons_;
	std::vector<dialog_button*> extra_buttons_;
	std::vector<dialog_button*> top_buttons_;
	std::vector<button*> frame_buttons_;
	std::string topic_;
	dialog_button *help_button_;
	dialog_textbox *text_widget_;
	dialog_frame *frame_;
	dimension_measurements dim_;
	int result_;
};

typedef Uint32 msecs;

} //end namespace gui
#endif
