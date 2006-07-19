/*
   Copyright (C) 2006 by Patrick Parker <patrick_x99@hotmail.com>
   wesnoth widget Copyright (C) 2003-5 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef CONSTRUCT_DIALOG_H_INCLUDED
#define CONSTRUCT_DIALOG_H_INCLUDED

#include "show_dialog.hpp"

#include "widgets/label.hpp"
#include "widgets/textbox.hpp"
#include "widgets/button.hpp"
#include "widgets/menu.hpp"
#include "key.hpp"
#include "sdl_utils.hpp"

namespace {
static std::vector<std::string> empty_string_vector;
}

namespace gui {

struct dialog_process_info
{
public:
	dialog_process_info() : left_button(true), right_button(true), key_down(true), first_time(true), selection(-1)
	{}
	void clear_buttons() {
		left_button = true;
		right_button = true;
		key_down = true;
	}
	CKey key;
	bool left_button, right_button, key_down, first_time;
	int selection;
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

private:
	handler_vector handler_members() {
		handler_vector h;
		h.push_back(caption_);
		return h;
	}

	surface surf_;
	label *caption_;
};

class dialog_textbox : public textbox {
public:
	dialog_textbox(label *const caption, CVideo &video, int width, const std::string& text="", bool editable=true, size_t max_size = 256, double alpha = 0.4, double alpha_focus = 0.2)
		: textbox(video, width, text, editable, max_size, alpha, alpha_focus, false),
		caption_(caption)
	{}
	~dialog_textbox() { delete caption_; }

	label *caption() const { return caption_; }

private:
	handler_vector handler_members() {
		handler_vector h;
		h.push_back(caption_);
		return h;
	}

	label *caption_;
};

class dialog_button : public button {
public:
	dialog_button(CVideo& video, const std::string& label, TYPE type=TYPE_PRESS, 
		int simple_result=CONTINUE_DIALOG, dialog_button_action *handler=NULL)
		: button(video,label,type,"",DEFAULT_SPACE,false), parent_(NULL),
		  handler_(handler), simple_result_(simple_result)
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
	enum BUTTON_LOCATION { BUTTON_STANDARD, BUTTON_EXTRA, BUTTON_CHECKBOX, BUTTON_CHECKBOX_LEFT };

private:
	typedef std::vector<preview_pane *>::iterator pp_iterator;
	typedef std::vector<dialog_button *>::iterator button_iterator;
	typedef std::vector< std::pair<dialog_button *, BUTTON_LOCATION> >::iterator button_pool_iterator;

public:

	//Static members
	static const std::string default_style;
	static const std::string no_help;
	static const int message_font_size;
	static const int caption_font_size;
	static const int max_menu_width;
	static const size_t left_padding;
	static const size_t right_padding;
	static const size_t image_h_pad;
	static const size_t top_padding;
	static const size_t bottom_padding;

	//Constructor & destructor
	dialog(display &disp, const std::string& title="", const std::string& message="",
				const DIALOG_TYPE type=MESSAGE, const std::string& dialog_style=default_style,
				const std::string& help_topic=no_help);
	virtual ~dialog();

	//Adding components - the dialog will manage the memory of these widgets,
	//therefore do not attempt to reference its widgets after destroying it
	void set_image(dialog_image *const img) { delete image_; image_ = img; }
	void set_menu(menu *const m) { if(menu_ != empty_menu) delete menu_; menu_ = m; }
	void add_pane(preview_pane *const pp) { preview_panes_.push_back(pp); }
	void set_textbox(dialog_textbox *const box) {
		delete text_widget_;
		text_widget_ = box;
	}
	void set_textbox(const std::string& text_widget_label="",
				const std::string &text_widget_text="",
				const int text_widget_max_chars = 256,
				const unsigned int text_box_width = font::relative_size(350));
	void add_button(dialog_button *const btn, BUTTON_LOCATION loc);

	//Launching the dialog
	//show - the return value of this method should be the same as result()
	int show(int &xloc, int &yloc);
	int show();

	//Results
	int result() const { return result_; }
	menu *get_menu();
	bool done() const { return (result_ != CONTINUE_DIALOG); }
	const std::string textbox_text() const { return text_widget_->text();}
	const bool option_checked(unsigned int option_index=0);

	//Backwards compatibility
	//set_action - deprecated, subclass dialog instead and override action()
	void set_action(dialog_action *const da) {action_ = da;}

protected:
	void set_result(const int result) { result_ = result; }

	//action - invoked at the end of the dialog-processing loop
	virtual void action() {
		if(!done() && action_ != NULL) {
			set_result(action_->do_action());
		}
	}
	//refresh - forces the display to refresh
	void refresh();

private:
	//layout - prepare components for display, return the dialog frame
	void layout(int &xloc, int &yloc);
	void draw(surface_restorer &restorer);

	//process - execute a single dialog processing loop and return the result
	int process(dialog_process_info &info);

	class help_button : public dialog_button {
	public:
		help_button(display& disp, const std::string &help_topic);
		int action(dialog_process_info &info);
		const std::string topic() const { return topic_; }
	private:
		display &disp_;
		const std::string topic_;
	};

	//Members
	display &disp_;
	dialog_image *image_;
	const std::string title_, style_;
	std::string message_;
	const DIALOG_TYPE type_;
	gui::menu *menu_;
	std::vector<preview_pane*> preview_panes_;
	std::vector< std::pair<dialog_button*,BUTTON_LOCATION> > button_pool_;
	std::vector<dialog_button*> standard_buttons_;
	std::vector<dialog_button*> extra_buttons_;
	help_button help_button_;
	dialog_textbox *text_widget_;
	dialog_action *action_;
	int result_;
	SDL_Rect message_rect_;
	SDL_Rect frame_rect_;
	int x_, y_;
};

} //end namespace gui
#endif
