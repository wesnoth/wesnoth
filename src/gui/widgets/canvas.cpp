/* $Id$ */
/*
   Copyright (C) 2007 - 2008 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file canvas.cpp
//! Implementation of canvas.hpp.

#include "gui/widgets/canvas.hpp"

#include "config.hpp"
#include "font.hpp"
#include "formula.hpp"
#include "image.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "serialization/parser.hpp"
#include "variable.hpp"
#include "wml_exception.hpp"

#include <algorithm>
#include <cassert>

#define DBG_G LOG_STREAM(debug, gui)
#define LOG_G LOG_STREAM(info, gui)
#define WRN_G LOG_STREAM(warn, gui)
#define ERR_G LOG_STREAM(err, gui)

#define DBG_G_D LOG_STREAM(debug, gui_draw)
#define LOG_G_D LOG_STREAM(info, gui_draw)
#define WRN_G_D LOG_STREAM(warn, gui_draw)
#define ERR_G_D LOG_STREAM(err, gui_draw)

#define DBG_G_E LOG_STREAM(debug, gui_event)
#define LOG_G_E LOG_STREAM(info, gui_event)
#define WRN_G_E LOG_STREAM(warn, gui_event)
#define ERR_G_E LOG_STREAM(err, gui_event)

#define DBG_G_P LOG_STREAM(debug, gui_parse)
#define LOG_G_P LOG_STREAM(info, gui_parse)
#define WRN_G_P LOG_STREAM(warn, gui_parse)
#define ERR_G_P LOG_STREAM(err, gui_parse)

static Uint32 decode_colour(const std::string& colour);

static Uint32 decode_colour(const std::string& colour)
{
	std::vector<std::string> fields = utils::split(colour);

	// make sure we have four fields
	while(fields.size() < 4) fields.push_back("0");

	Uint32 result = 0;
	for(int i = 0; i < 4; ++i) {
		// shift the previous value before adding, since it's a nop on the
		// first run there's no need for an if.
		result = result << 8;
		result |= lexical_cast_default<int>(fields[i]);
	}

	return result;
}

//! Reads a value as formula or value depending on the contents.
//!
//! A value can either be formula or a text containing a value, this function
//! determines the type and sets either the formula or the value part.
//! Formulas always start with a opening bracket '('. 
//!
//! @param str               The text with the value or formula. (Since there
//!                          are some problems with tempories when a value is
//!                          taken from a (v)config it uses a copy instead of
//!                          a reference.)
//! @param value             The value part, if the text is a value this will
//!                          be modified otherwise left untouched.
//! @param formula           The value part, if the text is a formula this will
//!                          be modified otherwise left untouched.
template <class T>
static void read_possible_formula(const std::string str, T& value, std::string& formula)
{
	if(str.empty()) {
		return;
	}

	if(str[0] == '(') {
		formula = str;
	} else {
		value = lexical_cast_default<T>(str);
	}
}

static void read_possible_formula(const std::string str, t_string& value, std::string& formula)
{
	if(str.empty()) {
		return;
	}

	if(str[0] == '(') {
		formula = str;
	} else {
		value = t_string(str);
	}
}

static void read_possible_formula(const std::string str, std::string& value, std::string& formula)
{
	if(str.empty()) {
		return;
	}

	if(str[0] == '(') {
		formula = str;
	} else {
		value = str;
	}
}

namespace gui2{

tcanvas::tcanvas() :
	shapes_(),
	dirty_(true),
	w_(0),
	h_(0),
	canvas_(),
	variables_()
{
}

tcanvas::tcanvas(const config& cfg) :
	shapes_(),
	dirty_(true),
	w_(0),
	h_(0),
	canvas_(),
	variables_()
{
	parse_cfg(cfg);
}

void tcanvas::draw(const config& cfg)
{
	parse_cfg(cfg);
	draw(true);
}

void tcanvas::draw(const bool force)
{
	log_scope2(gui_draw, "Canvas: drawing.");
	if(!dirty_ && !force) {
		DBG_G_D << "Canvas: nothing to draw.\n";
		return;
	}

	if(dirty_) {
		variables_.add("width",variant(w_));
		variables_.add("height",variant(h_));
	}

	// set surface sizes (do nothing now)
#if 0	
	if(fixme test whether -> can be used otherwise we crash w_ != canvas_->w || h_ != canvas_->h) {
		// create new

	} else {
		// fill current
	}
#endif
	// instead we overwrite the entire thing for now
	DBG_G_D << "Canvas: create new empty canvas.\n";
	canvas_.assign(SDL_CreateRGBSurface(SDL_SWSURFACE, w_, h_, 32, 0xFF0000, 0xFF00, 0xFF, 0xFF000000));

	// draw items 
	for(std::vector<tshape_ptr>::iterator itor = 
			shapes_.begin(); itor != shapes_.end(); ++itor) {
		log_scope2(gui_draw, "Canvas: draw shape.");
		
		(*itor)->draw(canvas_, variables_);
	}

	dirty_ = false;
}

void tcanvas::parse_cfg(const config& cfg)
{
	log_scope2(gui_parse, "Canvas: parsing config.");
	shapes_.clear();

	for(config::all_children_iterator itor = 
			cfg.ordered_begin(); itor != cfg.ordered_end(); ++itor) {

		const std::string& type = *((*itor).first);;
		const vconfig data(&(*((*itor).second)));

		DBG_G_P << "Canvas: found shape of the type " << type << ".\n";

		if(type == "line") {
			shapes_.push_back(new tline(data));
		} else if(type == "rectangle") {
			shapes_.push_back(new trectangle(data));
		} else if(type == "image") {
			shapes_.push_back(new timage(data));
		} else if(type == "text") {
			shapes_.push_back(new ttext(data));
		} else {
			ERR_G_P << "Canvas: found a shape of an invalid type " << type << ".\n";
			assert(false); // FIXME remove in production code.
		}
	}
}

void tcanvas::tshape::put_pixel(unsigned start, Uint32 colour, unsigned w, unsigned x, unsigned y)
{
	// fixme the 4 is true due to Uint32..
	*reinterpret_cast<Uint32*>(start + (y * w * 4) + x * 4) = colour;
}

// the surface should be locked
// the colour should be a const and the value send should already
// be good for the wanted surface
void tcanvas::tshape::draw_line(surface& canvas, Uint32 colour, 
		const unsigned x1, unsigned y1, const unsigned x2, unsigned y2)
{
	colour = SDL_MapRGBA(canvas->format, 
		((colour & 0xFF000000) >> 24),
		((colour & 0x00FF0000) >> 16),
		((colour & 0x0000FF00) >> 8),
		((colour & 0x000000FF)));

	unsigned start = reinterpret_cast<unsigned>(canvas->pixels);
	unsigned w = canvas->w;

	DBG_G_D << "Shape: draw line from " 
		<< x1 << ',' << y1 << " to " << x2 << ',' << y2
		<< " canvas width " << w << " canvas height "
		<< canvas->h << ".\n";

	assert(x1 < canvas->w);
	assert(x2 < canvas->w);
	assert(y1 < canvas->h);
	assert(y2 < canvas->h);

	// use a special case for vertical lines
	if(x1 == x2) {
		if(y2 < y1) {
			std::swap(y1, y2);	
		}

		for(unsigned y = y1; y <= y2; ++y) {
			put_pixel(start, colour, w, x1, y);
		}
		return;
	} 

	// use a special case for horizontal lines
	if(y1 == y2) {
		for(unsigned x  = x1; x <= x2; ++x) {
			put_pixel(start, colour, w, x, y1);
		}
		return;
	} 

	// draw based on Bresenham on wikipedia
	int dx = x2 - x1;
	int dy = y2 - y1;
	int slope = 1;
	if (dy < 0) {
		slope = -1;
		dy = -dy;
	}

	// Bresenham constants
	int incE = 2 * dy;
	int incNE = 2 * dy - 2 * dx;
	int d = 2 * dy - dx;
	int y = y1;

	// Blit
	for (unsigned x = x1; x <= x2; ++x) {
		put_pixel(start, colour, w, x, y);
		if (d <= 0) {
			d += incE;
		} else {
			d += incNE;
			y += slope;
		}
	}
}

tcanvas::tline::tline(const vconfig& cfg) :
	x1_(0),
	y1_(0),
	x2_(0),
	y2_(0),
	x1_formula_(),
	y1_formula_(),
	x2_formula_(),
	y2_formula_(),
	colour_(decode_colour(cfg["colour"])),
	thickness_(lexical_cast_default<unsigned>(cfg["thickness"]))
{
/*WIKI
 * [line]
 * Definition of a line.
 * Keys: 
 *     x1 (f_unsigned = 0)             The x coordinate of the startpoint.
 *     y1 (f_unsigned = 0)             The y coordinate of the startpoint.
 *     x2 (f_unsigned = 0)             The x coordinate of the endpoint.
 *     y2 (f_unsigned = 0)             The y coordinate of the endpoint.
 *     colour (widget = "")            The colour of the line.
 *     thickness = (unsigned = 0)      The thickness of the line if 0 nothing
 *                                     is drawn.
 *     debug = (string = "")           Debug message to show upon creation
 *                                     this message is not stored.
 *
 * Variables:
 *     width unsigned                  The width of the canvas.
 *     height unsigned                 The height of the canvas.
 *     text tstring                    The text to render on the widget.
 *
 * Note when drawing the valid coordinates are:
 * 0 -> width - 1
 * 0 -> height -1
 *
 * Drawing outside this area will result in unpredicatable results including
 * crashing. (That should be fixed, when encountered.)
 *
 * [/line]
 */

/*WIKI
 * This code can be used by a parser to generate the wiki page
 * structure
 * [tag name]
 * param type_info description
 *
 * param                               Name of the parameter.
 * 
 * type_info = ( type = default_value) The info about a optional parameter.
 * type_info = ( type )                The info about a mandatory parameter
 * type_info = [ type_info ]           The info about a conditional parameter 
 *                                     description should explain the reason.
 *
 * description                         Description of the parameter.
 *
 *
 *
 * types
 * unsigned                            Unsigned number (positive whole numbers
 *                                     and zero).
 * f_unsigned                          Unsinged number or formula returning an
 *                                     unsigned number.
 * int                                 Signed number (whole numbers).
 * f_int                               Signed number or formula returning an
 *                                     signed number.
 * string                              A text.
 * tstring                             A translatable string.
 * f_tstring                           Formula returning a translatable string.
 *
 * colour                              A string which constains the colour, this
 *                                     a group of 4 numbers between 0 and 255
 *                                     separated by a comma. The numbers are red
 *                                     component, green component, blue 
 *                                     component and alpha. A colour of 0 is not
 *                                     available. An alpha of 0 is fully 
 *                                     transparent. Ommitted values are set to 0.
 *
 *
 * Formulas are a funtion between brackets, that way the engine can see whether
 * there is standing a plain number or a formula eg:
 * 0     A value of zero
 * (0)   A formula returning zero
 *
 * When formulas are available the text should state the available variables
 * which are available in that function.
 */

	read_possible_formula(cfg["x1"], x1_, x1_formula_);
	read_possible_formula(cfg["y1"], y1_, y1_formula_);
	read_possible_formula(cfg["x2"], x2_, x2_formula_);
	read_possible_formula(cfg["y2"], y2_, y2_formula_);

	const std::string& debug = (cfg["debug"]);
	if(!debug.empty()) {
		DBG_G_P << "Line: found debug message '" << debug << "'.\n";
	}

}

void tcanvas::tline::draw(surface& canvas,
	const game_logic::map_formula_callable& variables)
{
	//@todo formulas are now recalculated every draw cycle which is a 
	// bit silly unless there has been a resize. So to optimize we should
	// use an extra flag or do the calculation in a separate routine.
	if(!x1_formula_.empty()) {
		DBG_G_D << "Line: execute x1 formula '" << x1_formula_ << "'.\n";
		x1_ = game_logic::formula(x1_formula_).execute(variables).as_int();
	}

	if(!y1_formula_.empty()) {
		DBG_G_D << "Line: execute y1 formula '" << y1_formula_ << "'.\n";
		y1_ = game_logic::formula(y1_formula_).execute(variables).as_int();
	}

	if(!x2_formula_.empty()) {
		DBG_G_D << "Line: execute x2 formula '" << x2_formula_ << "'.\n";
		x2_ = game_logic::formula(x2_formula_).execute(variables).as_int();
	}

	if(!y2_formula_.empty()) {
		DBG_G_D << "Line: execute y2 formula '" << y2_formula_ << "'.\n";
		y2_ = game_logic::formula(y2_formula_).execute(variables).as_int();
	}

	DBG_G_D << "Line: draw from " 
		<< x1_ << ',' << y1_ << " to " << x2_ << ',' << y2_ 
		<< " canvas size " << canvas->w << ',' << canvas->h << ".\n";

	VALIDATE(x1_ < canvas->w && x2_ < canvas->w && y1_ < canvas->h 
		&& y2_ < canvas->h, _("Line doesn't fit on canvas."));

	// FIXME respect the thickness.

	// now draw the line we use Bresenham's algorithm, which doesn't
	// support antialiasing. The advantage is that it's easy for testing.
	
	// lock the surface
	surface_lock locker(canvas);
	if(x1_ > x2_) {
		// invert points
		draw_line(canvas, colour_, x2_, y2_, x1_, y1_);
	} else {
		draw_line(canvas, colour_, x1_, y1_, x2_, y2_);
	}
	
}

tcanvas::trectangle::trectangle(const vconfig& cfg) :
	x_(0),
	y_(0),
	w_(0),
	h_(0),
	x_formula_(),
	y_formula_(),
	w_formula_(),
	h_formula_(),
	border_thickness_(lexical_cast_default<unsigned>(cfg["border_thickness"])),
	border_colour_(decode_colour(cfg["border_colour"])),
	fill_colour_(decode_colour(cfg["fill_colour"]))
{
/*WIKI
 * [rectangle]
 * Definition of a rectangle.
 * Keys: 
 *     x (f_unsigned = 0)              The x coordinate of the top left corner.
 *     y (f_unsigned = 0)              The y coordinate of the top left corner.
 *     w (f_unsigned = 0)              The width of the rectangle.
 *     h (f_unsigned = 0)              The height of the rectangle.
 *     border_thickness (unsigned = 0) The thickness of the border if the 
 *                                     thickness is zero it's not drawn.
 *     border_colour (colour = "")     The colour of the border if empty it's
 *                                     not drawn.
 *     fill_colour (colour = "")       The colour of the interior if ommitted
 *                                     it's not drawn.
 *     debug = (string = "")           Debug message to show upon creation
 *                                     this message is not stored.
 *
 * Variables:
 * See [line].
 *
 * [/rectangle]
 */
	read_possible_formula(cfg["x"], x_, x_formula_);
	read_possible_formula(cfg["y"], y_, y_formula_);
	read_possible_formula(cfg["w"], w_, w_formula_);
	read_possible_formula(cfg["h"], h_, h_formula_);

	if(border_colour_ == 0) {
		border_thickness_ = 0;
	}

	const std::string& debug = (cfg["debug"]);
	if(!debug.empty()) {
		DBG_G_P << "Rectangle: found debug message '" << debug << "'.\n";
	}
}

void tcanvas::trectangle::draw(surface& canvas,
	const game_logic::map_formula_callable& variables)
{

	//@todo formulas are now recalculated every draw cycle which is a 
	// bit silly unless there has been a resize. So to optimize we should
	// use an extra flag or do the calculation in a separate routine.
	if(!x_formula_.empty()) {
		DBG_G_D << "Rectangle: execute x formula '" << x_formula_ << "'.\n";
		x_ = game_logic::formula(x_formula_).execute(variables).as_int();
	}

	if(!y_formula_.empty()) {
		DBG_G_D << "Rectangle: execute y formula '" << y_formula_ << "'.\n";
		y_ = game_logic::formula(y_formula_).execute(variables).as_int();
	}

	if(!w_formula_.empty()) {
		DBG_G_D << "Rectangle: execute width formula '" << w_formula_ << "'.\n";
		w_ = game_logic::formula(w_formula_).execute(variables).as_int();
	}

	if(!h_formula_.empty()) {
		DBG_G_D << "Rectangle: execute height formula '" << h_formula_ << "'.\n";
		h_ = game_logic::formula(h_formula_).execute(variables).as_int();
	}

	DBG_G_D << "Rectangle: draw from " << x_ << ',' << y_
		<< " width " << w_ << " height " << h_ 
		<< " canvas size " << canvas->w << ',' << canvas->h << ".\n";

	VALIDATE(x_ < canvas->w && x_ + w_ <= canvas->w && y_ < canvas->h 
		&& y_ + h_ <= canvas->h, _("Rectangle doesn't fit on canvas."));


	surface_lock locker(canvas);

	// draw the border
	for(unsigned i = 0; i < border_thickness_; ++i) {

		const unsigned left = x_ + i;
		const unsigned right = left + w_ - (i * 2) - 1;
		const unsigned top = y_ + i;
		const unsigned bottom = top + h_ - (i * 2) - 1;

		// top horizontal (left -> right)
		draw_line(canvas, border_colour_, left, top, right, top);

		// right vertical (top -> bottom)
		draw_line(canvas, border_colour_, right, top, right, bottom);

		// bottom horizontal (left -> right)
		draw_line(canvas, border_colour_, left, bottom, right, bottom);

		// left vertical (top -> bottom)
		draw_line(canvas, border_colour_, left, top, left, bottom);

	}

	// The fill_rect_alpha code below fails, can't remember the exact cause
	// so use the slow line drawing method to fill the rect.
	if(fill_colour_) {
		
		const unsigned left = x_ + border_thickness_;
		const unsigned right = left + w_ - (2 * border_thickness_) - 1;
		const unsigned top = y_ + border_thickness_;
		const unsigned bottom = top + h_ - (2 * border_thickness_);

		for(unsigned i = top; i < bottom; ++i) {
			
			draw_line(canvas, fill_colour_, left, i, right, i);
		}
	}
/*
	const unsigned left = x_ + border_thickness_ + 1;
	const unsigned top = y_ + border_thickness_ + 1;
	const unsigned width = w_ - (2 * border_thickness_) - 2;
	const unsigned height = h_ - (2 * border_thickness_) - 2;
	SDL_Rect rect = create_rect(left, top, width, height);

	const Uint32 colour = fill_colour_ & 0xFFFFFF00;
	const Uint8 alpha = fill_colour_ & 0xFF;

	// fill
	fill_rect_alpha(rect, colour, alpha, canvas);
	canvas = blend_surface(canvas, 255, 0xAAAA00);	
*/	
}

tcanvas::timage::timage(const vconfig& cfg) :
	src_clip_(),
	dst_clip_(),
	image_()
{

//FIXME enhance the options and write the wiki block in the new style.

/*WIKI
 * [image]
 *     name = (string)                The name of the image.
 *     debug = (string = "")          Debug message to show upon creation
 * [/image]
 */

	image_.assign(image::get_image(image::locator(cfg["name"])));
	src_clip_ = create_rect(0, 0, image_->w, image_->h);

	const std::string& debug = (cfg["debug"]);
	if(!debug.empty()) {
		DBG_G_P << "Image: found debug message '" << debug << "'.\n";
	}
}

void tcanvas::timage::draw(surface& canvas,
	const game_logic::map_formula_callable& variables)
{
	DBG_G_D << "Image: draw.\n";

	SDL_Rect src_clip = src_clip_;
	SDL_Rect dst_clip = dst_clip_;
	SDL_BlitSurface(image_, &src_clip, canvas, &dst_clip);
}

tcanvas::ttext::ttext(const vconfig& cfg) :
	x_(0),
	y_(0),
	w_(0),
	h_(0),
	x_formula_(""),
	y_formula_(""),
	w_formula_(""),
	h_formula_(""),
	font_size_(lexical_cast_default<unsigned>(cfg["font_size"])),
	colour_(decode_colour(cfg["colour"])),
	text_(""),
	text_formula_("")
{

/*WIKI
 * [text]
 * Definition of text.
 * Keys: 
 *     x (f_unsigned = 0)              The x coordinate of the top left corner.
 *     y (f_unsigned = 0)              The y coordinate of the top left corner.
 *     w (f_unsigned = 0)              The width of the rectangle.
 *     h (f_unsigned = 0)              The height of the rectangle.
 *     font_size (unsigned = 0)        The size of the font to draw in.
 *     colour (colour = "")            The colour of the text.
 *     text (tstring = "")             The text to draw (translatable).
 *     debug = (string = "")           Debug message to show upon creation
 *                                     this message is not stored.
 *
 * NOTE there's no option of font style yet, alignment can be done with the
 * forumulas.
 *
 * Variables:
 *     text_width unsigned             The width of the rendered text.
 *     text_height unsigned            The height of the renedered text.
 * And also the ones defined in [line].
 *
 * [/text]
 */

	read_possible_formula(cfg["x"], x_, x_formula_);
	read_possible_formula(cfg["y"], y_, y_formula_);
	read_possible_formula(cfg["w"], w_, w_formula_);
	read_possible_formula(cfg["h"], h_, h_formula_);
	read_possible_formula(cfg["text"], text_, text_formula_);

	const std::string& debug = (cfg["debug"]);
	if(!debug.empty()) {
		DBG_G_P << "Text: found debug message '" << debug << "'.\n";
	}
}

void tcanvas::ttext::draw(surface& canvas,
	const game_logic::map_formula_callable& variables)
{

	assert(variables.has_key("text"));

	// We first need to determine the size of the text which need the rendered 
	// text. So resolve and render the text first and then start to resolve
	// the other formulas.
	if(!text_formula_.empty()) {
		DBG_G_D << "Text: execute text formula '" << text_formula_ << "'.\n";
		text_ = t_string(game_logic::formula(text_formula_).execute(variables).as_string());
	}

	SDL_Color col = { (colour_ >> 24), (colour_ >> 16), (colour_ >> 8), colour_ };
	surface surf(font::get_rendered_text(text_, font_size_, col, TTF_STYLE_NORMAL));

	game_logic::map_formula_callable local_variables(variables);
	local_variables.add("text_width", variant(surf->w));
	local_variables.add("text_height", variant(surf->h));


	//@todo formulas are now recalculated every draw cycle which is a 
	// bit silly unless there has been a resize. So to optimize we should
	// use an extra flag or do the calculation in a separate routine.
	if(!x_formula_.empty()) {
		DBG_G_D << "Text: execute x formula '" << x_formula_ << "'.\n";
		x_ = game_logic::formula(x_formula_).execute(local_variables).as_int();
	}

	if(!y_formula_.empty()) {
		DBG_G_D << "Text: execute y formula '" << y_formula_ << "'.\n";
		y_ = game_logic::formula(y_formula_).execute(local_variables).as_int();
	}

	if(!w_formula_.empty()) {
		DBG_G_D << "Text: execute width formula '" << w_formula_ << "'.\n";
		w_ = game_logic::formula(w_formula_).execute(local_variables).as_int();
	}

	if(!h_formula_.empty()) {
		DBG_G_D << "Text: execute height formula '" << h_formula_ << "'.\n";
		h_ = game_logic::formula(h_formula_).execute(local_variables).as_int();
	}

	DBG_G_D << "Text: drawint text '" << text_
		<< "' drawn from " << x_ << ',' << y_
		<< " width " << w_ << " height " << h_ 
		<< " canvas size " << canvas->w << ',' << canvas->h << ".\n";

	VALIDATE(x_ < canvas->w && y_ < canvas->h, _("Text doesn't start on canvas."));

	// A text might be to long and will be clipped.
	if(surf->w > w_) {
		WRN_G_D << "Text: text is too wide for the canvas and will be clipped.\n";
	}
	
	if(surf->h > h_) {
		WRN_G_D << "Text: text is too high for the canvas and will be clipped.\n";
	}

	//FIXME make sure text is rendered properly.
	//
	// A hack to make the letters show up a bit readable it does however
	// clear the back ground. This needs to be fixed but don't want to stall
	// development too long on it.
	SDL_SetAlpha(surf, 0, 0);

	SDL_Rect dst = { x_, y_, canvas->w, canvas->h };
	SDL_BlitSurface(surf, 0, canvas, &dst);
}

} // namespace gui2
