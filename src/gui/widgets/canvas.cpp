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
#include "gui/widgets/helper.hpp"
#include "log.hpp"
#include "serialization/parser.hpp"
#include "wml_exception.hpp"

#include <boost/static_assert.hpp>

#include <algorithm>
#include <cassert>

#define DBG_G LOG_STREAM_INDENT(debug, gui)
#define LOG_G LOG_STREAM_INDENT(info, gui)
#define WRN_G LOG_STREAM_INDENT(warn, gui)
#define ERR_G LOG_STREAM_INDENT(err, gui)

#define DBG_G_D LOG_STREAM_INDENT(debug, gui_draw)
#define LOG_G_D LOG_STREAM_INDENT(info, gui_draw)
#define WRN_G_D LOG_STREAM_INDENT(warn, gui_draw)
#define ERR_G_D LOG_STREAM_INDENT(err, gui_draw)

#define DBG_G_E LOG_STREAM_INDENT(debug, gui_event)
#define LOG_G_E LOG_STREAM_INDENT(info, gui_event)
#define WRN_G_E LOG_STREAM_INDENT(warn, gui_event)
#define ERR_G_E LOG_STREAM_INDENT(err, gui_event)

#define DBG_G_P LOG_STREAM_INDENT(debug, gui_parse)
#define LOG_G_P LOG_STREAM_INDENT(info, gui_parse)
#define WRN_G_P LOG_STREAM_INDENT(warn, gui_parse)
#define ERR_G_P LOG_STREAM_INDENT(err, gui_parse)

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

namespace gui2{

namespace {

//! Template class can hold a value or a formula calculating the value.
template <class T>
class tformula
{
public:	
	tformula<T>(const std::string& str) : 
		formula_(),
		value_()
	{
		if(str.empty()) {
			return;
		}

		if(str[0] == '(') {
			formula_ = str;
		} else {
			convert(str);
		}

	}

	//! Returns the value, can only be used it the data is no formula.
	//! 
	//! Another option would be to cache the output of the formula in value_
	//! and always allow this function. But for now decided that the caller
	//! needs to do the caching. It might be changed later.
	T operator()() const
	{
		assert(!has_formula());
		return value_;
	}
	
	//! Returns the value, can always be used.
	T operator() (const game_logic::map_formula_callable& variables) const
	{
		if(has_formula()) {
			DBG_G_D << "Formula: execute '" << formula_ << "'.\n";
			return execute(variables);
		} else {
			return value_;
		}
	}

	//! Determine whether the class contains a formula.
	bool has_formula() const { return !formula_.empty(); }

private:
	
	//! Converts the string ot the template value.
	void convert(const std::string& str);

	T execute(const game_logic::map_formula_callable& variables) const;

	//! If there is a formula it's stored in this string, empty if no formula.
	std::string formula_;

	//! If no formula it contains the value.
	T value_;

};

template<>
bool tformula<bool>::execute(const game_logic::map_formula_callable& variables) const
{
	return game_logic::formula(formula_).execute(variables).as_bool();
}

template<>
int tformula<int>::execute(const game_logic::map_formula_callable& variables) const
{
	return game_logic::formula(formula_).execute(variables).as_int();
}

template<>
unsigned tformula<unsigned>::execute(const game_logic::map_formula_callable& variables) const
{
	return game_logic::formula(formula_).execute(variables).as_int();
}

template<>
std::string tformula<std::string>::execute(const game_logic::map_formula_callable& variables) const
{
	return game_logic::formula(formula_).execute(variables).as_string();
}

template<>
t_string tformula<t_string>::execute(const game_logic::map_formula_callable& variables) const
{
	return game_logic::formula(formula_).execute(variables).as_string();
}

template<class T>
T tformula<T>::execute(const game_logic::map_formula_callable& variables) const
{
	// Every type needs it's own execute function avoid instantiation of the
	// default execute.
	BOOST_STATIC_ASSERT(sizeof(T) == 0);
	return T();
}

template<>
void tformula<bool>::convert(const std::string& str)
{
	value_ = utils::string_bool(str);
}

template<>
void tformula<std::string>::convert(const std::string& str)
{
	value_ = str;
}

template<>
void tformula<t_string>::convert(const std::string& str)
{
	value_ = str;
}

template<class T>
void tformula<T>::convert(const std::string& str)
{ 
	value_ = lexical_cast_default<T>(str); 
}

//! Definition of a line shape.
class tline : public tcanvas::tshape
{
public:
	tline(const config& cfg);

	//! Implement shape::draw().
	void draw(surface& canvas,
		const game_logic::map_formula_callable& variables);

private:
	tformula<unsigned> 
		x1_, 
		y1_,
		x2_,
		y2_;

	Uint32 colour_;
	//! The thickness of the line:
	//! if the value is odd the x and y are the middle of the line.
	//! if the value is even the x and y are the middle of a line
	//! with width - 1. (0 is special case, does nothing.)
	unsigned thickness_;
};

tline::tline(const config& cfg) :
	x1_(cfg["x1"]),
	y1_(cfg["y1"]),
	x2_(cfg["x2"]),
	y2_(cfg["y2"]),
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
 * bool                                A boolean value accepts the normal 
 *                                     values as the rest of the game.
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
 * font_style                          A string which contains the style of the
 *                                     font:
 *                                     * normal    normal font
 *                                     * bold      bold font
 *                                     * italic    italic font
 *                                     * underline underlined font
 *                                     Since SDL has problems combining these
 *                                     styles only one can be picked. Once SDL
 *                                     will allow multiple options, this type
 *                                     will be transformed to a comma separated
 *                                     list. If empty we default to the normal
 *                                     style.
 *
 * Formulas are a funtion between brackets, that way the engine can see whether
 * there is standing a plain number or a formula eg:
 * 0     A value of zero
 * (0)   A formula returning zero
 *
 * When formulas are available the text should state the available variables
 * which are available in that function.
 */

	const std::string& debug = (cfg["debug"]);
	if(!debug.empty()) {
		DBG_G_P << "Line: found debug message '" << debug << "'.\n";
	}

}

void tline::draw(surface& canvas,
	const game_logic::map_formula_callable& variables)
{
	//@todo formulas are now recalculated every draw cycle which is a 
	// bit silly unless there has been a resize. So to optimize we should
	// use an extra flag or do the calculation in a separate routine.
	
	const unsigned x1 = x1_(variables);
	const unsigned y1 = y1_(variables);
	const unsigned x2 = x2_(variables);
	const unsigned y2 = y2_(variables);

	DBG_G_D << "Line: draw from " 
		<< x1 << ',' << y1 << " to " << x2 << ',' << y2 
		<< " canvas size " << canvas->w << ',' << canvas->h << ".\n";

	VALIDATE(x1 < canvas->w && x2 < canvas->w && y1 < canvas->h 
		&& y2 < canvas->h, _("Line doesn't fit on canvas."));

	// FIXME respect the thickness.

	// now draw the line we use Bresenham's algorithm, which doesn't
	// support antialiasing. The advantage is that it's easy for testing.
	
	// lock the surface
	surface_lock locker(canvas);
	if(x1 > x2) {
		// invert points
		draw_line(canvas, colour_, x2, y2, x1, y1);
	} else {
		draw_line(canvas, colour_, x1, y1, x2, y2);
	}
}



//! Definition of a rectangle shape.
class trectangle : public tcanvas::tshape
{
public:
	trectangle(const config& cfg);

	//! Implement shape::draw().
	void draw(surface& canvas,
		const game_logic::map_formula_callable& variables);

private:
	tformula<unsigned> 
		x_, 
		y_,
		w_,
		h_;

	//! Border thickness if 0 the fill colour is used for the entire 
	//! widget.
	unsigned border_thickness_;
	Uint32 border_colour_;

	Uint32 fill_colour_;
};

trectangle::trectangle(const config& cfg) :
	x_(cfg["x"]),
	y_(cfg["y"]),
	w_(cfg["w"]),
	h_(cfg["h"]),
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
	if(border_colour_ == 0) {
		border_thickness_ = 0;
	}

	const std::string& debug = (cfg["debug"]);
	if(!debug.empty()) {
		DBG_G_P << "Rectangle: found debug message '" << debug << "'.\n";
	}
}

void trectangle::draw(surface& canvas,
	const game_logic::map_formula_callable& variables)
{

	//@todo formulas are now recalculated every draw cycle which is a 
	// bit silly unless there has been a resize. So to optimize we should
	// use an extra flag or do the calculation in a separate routine.
	const unsigned x = x_(variables);
	const unsigned y = y_(variables);
	const unsigned w = w_(variables);
	const unsigned h = h_(variables);

	DBG_G_D << "Rectangle: draw from " << x << ',' << y
		<< " width " << w << " height " << h 
		<< " canvas size " << canvas->w << ',' << canvas->h << ".\n";

	VALIDATE(x < canvas->w && x + w <= canvas->w && y < canvas->h 
		&& y + h <= canvas->h, _("Rectangle doesn't fit on canvas."));


	surface_lock locker(canvas);

	// draw the border
	for(unsigned i = 0; i < border_thickness_; ++i) {

		const unsigned left = x + i;
		const unsigned right = left + w - (i * 2) - 1;
		const unsigned top = y + i;
		const unsigned bottom = top + h - (i * 2) - 1;

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
		
		const unsigned left = x + border_thickness_;
		const unsigned right = left + w - (2 * border_thickness_) - 1;
		const unsigned top = y + border_thickness_;
		const unsigned bottom = top + h - (2 * border_thickness_);

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



//! Definition of an image shape.
class timage : public tcanvas::tshape
{
public:
	timage(const config& cfg);
	
	//! Implement shape::draw().
	void draw(surface& canvas,
		const game_logic::map_formula_callable& variables);

private:
	tformula<unsigned>
		x_, 
		y_,
		w_,
		h_;

	SDL_Rect src_clip_;
	SDL_Rect dst_clip_;
	surface image_;

	bool stretch_;
};

timage::timage(const config& cfg) :
	x_(cfg["x"]),
	y_(cfg["y"]),
	w_(cfg["w"]),
	h_(cfg["h"]),
	src_clip_(),
	dst_clip_(),
	image_(),
	stretch_(utils::string_bool(cfg["stretch"]))
{
/*WIKI
 * [image]
 * Definition of an image.
 * Keys: 
 *     x (f_unsigned = 0)              The x coordinate of the top left corner.
 *     y (f_unsigned = 0)              The y coordinate of the top left corner.
 *     w (f_unsigned = 0)              The width of the image, if not zero the
 *                                     image will be scaled to the desired width.
 *     h (f_unsigned = 0)              The height of the image, if not zero the
 *                                     image will be scaled to the desired height.
 *     stretch (bool = false)          Border images often need to be either 
 *                                     stretched in the width or the height. If
 *                                     that's the case use stretch. It only works
 *                                     if only the heigth or the width is not zero.
 *                                     It will copy the first pixel the the others.
 *     name (string = "")              The name of the image.
 *     debug = (string = "")           Debug message to show upon creation
 *                                     this message is not stored.
 *
 * Variables:
 * See [line].
 *
 * [/image]
 */

	image_.assign(image::get_image(image::locator(cfg["name"])));
	src_clip_ = create_rect(0, 0, image_->w, image_->h);

	const std::string& debug = (cfg["debug"]);
	if(!debug.empty()) {
		DBG_G_P << "Image: found debug message '" << debug << "'.\n";
	}
}

void timage::draw(surface& canvas,
	const game_logic::map_formula_callable& variables)
{
	DBG_G_D << "Image: draw.\n";

	//@todo formulas are now recalculated every draw cycle which is a 
	// bit silly unless there has been a resize. So to optimize we should
	// use an extra flag or do the calculation in a separate routine.
	const unsigned x = x_(variables);
	const unsigned y = y_(variables);
	unsigned w = w_(variables);
	unsigned h = h_(variables);

	// Copy the data to local variables to avoid overwriting the originals.
	SDL_Rect src_clip = src_clip_;
	SDL_Rect dst_clip = dst_clip_;
	dst_clip.x = x;
	dst_clip.y = y;
	surface surf;

	// Test whether we need to scale and do the scaling if needed.
	if(w || h) {
		bool done = false;
		bool stretch = stretch_ && (!!w ^ !!h);
		if(!w) {
			if(stretch) { 
				DBG_G_D << "Image: vertical stretch from " << image_->w 
					<< ',' << image_->h << " to a height of " << h << ".\n";

				surf = stretch_surface_vertical(image_, h);	
				done = true;
			}
			w = image_->w;
		}

		if(!h) {
			if(stretch) { 
				DBG_G_D << "Image: horizontal stretch from " << image_->w 
					<< ',' << image_->h << " to a width of " << w << ".\n";

				surf = stretch_surface_horizontal(image_, w);	
				done = true;
			}
			h = image_->h;
		}

		if(!done) {

			DBG_G_D << "Image: scaling from " << image_->w 
				<< ',' << image_->h << " to " << w << ',' << h << ".\n";

			surf = scale_surface(image_, w, h);
		}
		src_clip.w = w;
		src_clip.h = h;
	} else {
		surf = image_;
	}

	SDL_BlitSurface(surf, &src_clip, canvas, &dst_clip);
}



//! Definition of a text shape.
class ttext : public tcanvas::tshape
{
public:
	ttext(const config& cfg);
	
	//! Implement shape::draw().
	void draw(surface& canvas,
		const game_logic::map_formula_callable& variables);

private:
	tformula<unsigned>
		x_, 
		y_,
		w_,
		h_;

	unsigned font_size_;
	int font_style_;
	Uint32 colour_;

	tformula<t_string> text_;
};

ttext::ttext(const config& cfg) :
	x_(cfg["x"]),
	y_(cfg["y"]),
	w_(cfg["w"]),
	h_(cfg["h"]),
	font_size_(lexical_cast_default<unsigned>(cfg["font_size"])),
	font_style_(decode_font_style(cfg["font_style"])),
	colour_(decode_colour(cfg["colour"])),
	text_(cfg["text"])
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
 *     font_style (font_style = "")    The style of the text.
 *     colour (colour = "")            The colour of the text.
 *     text (tstring = "")             The text to draw (translatable).
 *     debug = (string = "")           Debug message to show upon creation
 *                                     this message is not stored.
 *
 * NOTE alignment can be done with the forumulas.
 *
 * Variables:
 *     text_width unsigned             The width of the rendered text.
 *     text_height unsigned            The height of the renedered text.
 * And also the ones defined in [line].
 *
 * [/text]
 */

	const std::string& debug = (cfg["debug"]);
	if(!debug.empty()) {
		DBG_G_P << "Text: found debug message '" << debug << "'.\n";
	}
}

void ttext::draw(surface& canvas,
	const game_logic::map_formula_callable& variables)
{

	assert(variables.has_key("text"));

	// We first need to determine the size of the text which need the rendered 
	// text. So resolve and render the text first and then start to resolve
	// the other formulas.
	const t_string text = text_(variables);

	if(text.empty()) {
		DBG_G_D << "Text: no text to render, leave.\n";
		return;
	}

	SDL_Color col = { (colour_ >> 24), (colour_ >> 16), (colour_ >> 8), colour_ };
	surface surf(font::get_rendered_text(text, font_size_, col, font_style_));
	assert(surf);

	game_logic::map_formula_callable local_variables(variables);
	local_variables.add("text_width", variant(surf->w));
	local_variables.add("text_height", variant(surf->h));


	//@todo formulas are now recalculated every draw cycle which is a 
	// bit silly unless there has been a resize. So to optimize we should
	// use an extra flag or do the calculation in a separate routine.
	const unsigned x = x_(local_variables);
	const unsigned y = y_(local_variables);
	const unsigned w = w_(local_variables);
	const unsigned h = h_(local_variables);

	DBG_G_D << "Text: drawing text '" << text
		<< "' drawn from " << x << ',' << y
		<< " width " << w << " height " << h 
		<< " canvas size " << canvas->w << ',' << canvas->h << ".\n";

	VALIDATE(x < canvas->w && y < canvas->h, _("Text doesn't start on canvas."));

	// A text might be to long and will be clipped.
	if(surf->w > w) {
		WRN_G_D << "Text: text is too wide for the canvas and will be clipped.\n";
	}
	
	if(surf->h > h) {
		WRN_G_D << "Text: text is too high for the canvas and will be clipped.\n";
	}

	//FIXME make sure text is rendered properly.
	//
	// A hack to make the letters show up a bit readable it does however
	// clear the back ground. This needs to be fixed but don't want to stall
	// development too long on it.
	SDL_SetAlpha(surf, 0, 0);

	SDL_Rect dst = { x, y, canvas->w, canvas->h };
	SDL_BlitSurface(surf, 0, canvas, &dst);
}

} // namespace

tcanvas::tcanvas() :
	shapes_(),
	w_(0),
	h_(0),
	canvas_(),
	variables_(),
	dirty_(true)
{
}

tcanvas::tcanvas(const config& cfg) :
	shapes_(),
	w_(0),
	h_(0),
	canvas_(),
	variables_(),
	dirty_(true)
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
		const config& data = *((*itor).second);

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

void tcanvas::tshape::put_pixel(ptrdiff_t start, Uint32 colour, unsigned w, unsigned x, unsigned y)
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

	ptrdiff_t start = reinterpret_cast<ptrdiff_t>(canvas->pixels);
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


} // namespace gui2
