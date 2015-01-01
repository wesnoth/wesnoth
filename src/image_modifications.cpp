/*
   Copyright (C) 2009 - 2015 by Ignacio R. Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "color_range.hpp"
#include "config.hpp"
#include "display.hpp"
#include "game_config.hpp"
#include "image.hpp"
#include "image_modifications.hpp"
#include "log.hpp"
#include "sdl/alpha.hpp"
#include "serialization/string_utils.hpp"

#include <boost/foreach.hpp>

#include <map>

#define GETTEXT_DOMAIN "wesnoth-lib"

static lg::log_domain log_display("display");
#define ERR_DP LOG_STREAM(err, log_display)
#define LOG_DP LOG_STREAM(info, log_display)

namespace image {


/** Adds @a mod to the queue (unless mod is NULL). */
void modification_queue::push(modification * mod)
{
	// Null pointers do not get stored. (Shouldn't happen, but just in case.)
	if ( mod != NULL )
		priorities_[mod->priority()].push_back(mod);
}

/** Removes the top element from the queue */
void modification_queue::pop()
{
	map_type::iterator top_pair = priorities_.begin();
	std::vector<modification *> & top_vector = top_pair->second;

	// Erase the top element.
	top_vector.erase(top_vector.begin());
	if ( top_vector.empty() )
		// We need to keep the map clean.
		priorities_.erase(top_pair);
}

/** Returns the number of elements in the queue. */
size_t modification_queue::size() const
{
	size_t count = 0;
	BOOST_FOREACH ( const map_type::value_type & pair, priorities_ )
		count += pair.second.size();
	return count;
}

/** Returns the top element in the queue . */
modification * modification_queue::top() const
{
	return priorities_.begin()->second.front();
}


namespace {

/// A function used to parse modification arguments
typedef modification* (*mod_parser)(const std::string&);

/** A map of all registered mod parsers
 *
 * The mapping is between the modification name and the parser function pointer
 * An example of an entry would be "TC" -> &parse_TC_mod
 */
std::map<std::string, mod_parser> mod_parsers;

/** Decodes a single modification using an appropriate mod_parser
 *
 * @param encoded_mod A string representing a single modification
 *
 * @return A pointer to the decoded modification object
 * @retval NULL if the string is invalid or a parser isn't found
 */
modification* decode_modification(const std::string& encoded_mod)
{
	std::vector<std::string> split = utils::parenthetical_split(encoded_mod);

	if(split.size() != 2) {
		ERR_DP << "error parsing image modifications: "
		       << encoded_mod << "\n";
		return NULL;
	}

	std::string mod_type = split[0];
	std::string args = split[1];

	if(mod_parsers.find(mod_type) == mod_parsers.end()) {
		ERR_DP << "unknown image function in path: "
		       << mod_type << '\n';
		return NULL;
	}

	return (*mod_parsers[mod_type])(args);
}
} // end anon namespace


modification::texception::texception(const std::stringstream& message_stream)
	: message(message_stream.str())
{
}

modification::texception::texception(const std::string& message)
		: message(message)
{
}

/** Decodes the modification string
 *
 * Important:
 * It creates new objects which need to be deleted after use
 *
 * @param encoded_mods A string representing any number of modifications
 *
 * @return A modification_queue filled with decoded modification pointers
 */
modification_queue modification::decode(const std::string& encoded_mods)
{
	modification_queue mods;

	BOOST_FOREACH(const std::string& encoded_mod,
		utils::parenthetical_split(encoded_mods, '~')) {
		modification* mod = decode_modification(encoded_mod);

		if(mod) {
			mods.push(mod);
		}
	}

	return mods;
}

surface rc_modification::operator()(const surface& src) const
{
	// unchecked
	return recolor_image(src, rc_map_);
}

surface fl_modification::operator()(const surface& src) const
{
	surface ret = src;

	if ( horiz_  && vert_ ) {
		// Slightly faster than doing both a flip and a flop.
		ret = rotate_180_surface(ret);
	}

	else if(horiz_) {
		ret = flip_surface(ret);
	}

	else if(vert_) {
		ret = flop_surface(ret);
	}

	return ret;
}

surface rotate_modification::operator()(const surface& src) const
{
	// Convert the number of degrees to the interval [0,360].
	const int normalized = degrees_ >= 0 ?
		degrees_ - 360*(degrees_/360) :
		degrees_ + 360*(1 + (-degrees_)/360); // In case compilers disagree as to what -90/360 is.

	switch ( normalized )
	{
		case 0:   return src;
		case 90:  return rotate_90_surface(src, true);
		case 180: return rotate_180_surface(src);
		case 270: return rotate_90_surface(src, false);
		case 360: return src;
	}

	return rotate_any_surface(src, normalized, zoom_, offset_);
}

surface gs_modification::operator()(const surface& src) const
{
	return greyscale_image(src);
}

surface plot_alpha_modification::operator()(const surface& src) const
{
	return alpha_to_greyscale(src);
}

surface wipe_alpha_modification::operator()(const surface& src) const
{
	return wipe_alpha(src);
}

surface adjust_alpha_modification::operator()(const surface & src) const
{
	return adjust_surface_alpha(src, amount_);
}

surface crop_modification::operator()(const surface& src) const
{
	SDL_Rect area = slice_;
	if(area.w == 0) {
		area.w = src->w;
	}
	if(area.h == 0) {
		area.h = src->h;
	}
	if(area.x < 0) {
		ERR_DP << "start X coordinate of CROP modification is negative - truncating to zero" << std::endl;
		area.x = 0;
	}
	if(area.y < 0) {
		ERR_DP << "start Y coordinate of CROP modification is negative - truncating to zero" << std::endl;
		area.y = 0;
	}

	/*
	 * Unlike other image functions cut_surface does not convert the input
	 * surface to a neutral surface, nor does it convert its return surface
	 * to an optimised surface.
	 *
	 * Since it seems to work for most cases, rather change this caller instead
	 * of the function signature. (The issue was discovered in bug #20876).
	 */
	return create_optimized_surface(
			cut_surface(make_neutral_surface(src), area));
}

const SDL_Rect& crop_modification::get_slice() const
{
	return slice_;
}

surface blit_modification::operator()(const surface& src) const
{
	if(x_ >= src->w) {
		std::stringstream sstr;
		sstr << "~BLIT(): x-coordinate '"
			<< x_ << "' larger than destination image's width '"
			<< src->w << "' no blitting performed.\n";

		throw texception(sstr);
	}

	if(y_ >= src->h) {
		std::stringstream sstr;
		sstr << "~BLIT(): y-coordinate '"
			<< y_ << "' larger than destination image's height '"
			<< src->h << "' no blitting performed.\n";

		throw texception(sstr);
	}

	if(surf_->w + x_ > src->w) {
		std::stringstream sstr;
		sstr << "~BLIT(): offset and width '"
			<< x_ + surf_->w << "' larger than destination image's width '"
			<< src->w << "' no blitting performed.\n";

		throw texception(sstr);
	}

	if(surf_->h + y_ > src->h) {
		std::stringstream sstr;
		sstr << "~BLIT(): offset and height '"
			<< y_ + surf_->h << "' larger than destination image's height '"
			<< src->h << "' no blitting performed.\n";

		throw texception(sstr);
	}

	//blit_surface want neutral surfaces
	surface nsrc = make_neutral_surface(src);
	surface nsurf = make_neutral_surface(surf_);
	SDL_Rect r = sdl::create_rect(x_, y_, 0, 0);
	blit_surface(nsurf, NULL, nsrc, &r);
	return nsrc;
}

const surface& blit_modification::get_surface() const
{
	return surf_;
}

int blit_modification::get_x() const
{
	return x_;
}

int blit_modification::get_y() const
{
	return y_;
}

surface mask_modification::operator()(const surface& src) const
{
	if(src->w == mask_->w &&  src->h == mask_->h && x_ == 0 && y_ == 0)
		return mask_surface(src, mask_);
	SDL_Rect r = sdl::create_rect(x_, y_, 0, 0);
	surface new_mask = create_neutral_surface(src->w, src->h);
	blit_surface(mask_, NULL, new_mask, &r);
	return mask_surface(src, new_mask);
}

const surface& mask_modification::get_mask() const
{
	return mask_;
}

int mask_modification::get_x() const
{
	return x_;
}

int mask_modification::get_y() const
{
	return y_;
}

surface light_modification::operator()(const surface& src) const {
	if(src == NULL) { return NULL; }

	//light_surface wants a neutral surface having same dimensions
	surface nsurf;
	if(surf_->w != src->w || surf_->h != src->h)
		nsurf = scale_surface(surf_, src->w, src->h, false);
	else
		nsurf = make_neutral_surface(surf_);
	return light_surface(src, nsurf);;
}

const surface& light_modification::get_surface() const
{
	return surf_;
}

surface scale_modification::operator()(const surface& src) const
{
	const int old_w = src->w;
	const int old_h = src->h;
	int w = w_;
	int h = h_;

	if(w <= 0) {
		if(w < 0) {
			ERR_DP << "width of SCALE is negative - resetting to original width" << std::endl;
		}
		w = old_w;
	}
	if(h <= 0) {
		if(h < 0) {
			ERR_DP << "height of SCALE is negative - resetting to original height" << std::endl;
		}
		h = old_h;
	}

	return scale_surface(src, w, h);
}

int scale_modification::get_w() const
{
	return w_;
}

int scale_modification::get_h() const
{
	return h_;
}

surface scale_sharp_modification::operator()(const surface& src) const
{
	const int old_w = src->w;
	const int old_h = src->h;
	int w = w_;
	int h = h_;

	if(w <= 0) {
		if(w < 0) {
			ERR_DP << "width of SCALE_SHARP is negative - resetting to original width" << std::endl;
		}
		w = old_w;
	}
	if(h <= 0) {
		if(h < 0) {
			ERR_DP << "height of SCALE_SHARP is negative - resetting to original height" << std::endl;
		}
		h = old_h;
	}

	return scale_surface_sharp(src, w, h);
}

int scale_sharp_modification::get_w() const
{
	return w_;
}

int scale_sharp_modification::get_h() const
{
	return h_;
}

surface xbrz_modification::operator()(const surface& src) const
{
	if (z_ == 1) {
		return src;
	}

	return scale_surface_xbrz(src, z_);
}

surface o_modification::operator()(const surface& src) const
{
	return adjust_surface_alpha(src, ftofxp(opacity_));
}

float o_modification::get_opacity() const
{
	return opacity_;
}

surface cs_modification::operator()(const surface& src) const
{
	return(
		(r_ != 0 || g_ != 0 || b_ != 0) ?
		adjust_surface_color(src, r_, g_, b_) :
		src
	);
}

int cs_modification::get_r() const
{
	return r_;
}

int cs_modification::get_g() const
{
	return g_;
}

int cs_modification::get_b() const
{
	return b_;
}

surface blend_modification::operator()(const surface& src) const
{
	return blend_surface(src, a_, display::rgb(r_, g_, b_));

}

int blend_modification::get_r() const
{
	return r_;
}

int blend_modification::get_g() const
{
	return g_;
}

int blend_modification::get_b() const
{
	return b_;
}

float blend_modification::get_a() const
{
	return a_;
}

surface bl_modification::operator()(const surface& src) const
{
	return blur_alpha_surface(src, depth_);
}

int bl_modification::get_depth() const
{
	return depth_;
}

surface brighten_modification::operator()(const surface &src) const
{
	surface ret = make_neutral_surface(src);
	surface tod_bright(image::get_image(game_config::images::tod_bright));
	if (tod_bright)
		blit_surface(tod_bright, NULL, ret, NULL);
	return ret;
}

surface darken_modification::operator()(const surface &src) const
{
	surface ret = make_neutral_surface(src);
	surface tod_dark(image::get_image(game_config::images::tod_dark));
	if (tod_dark)
		blit_surface(tod_dark, NULL, ret, NULL);
	return ret;
}

surface background_modification::operator()(const surface &src) const
{
	surface ret = make_neutral_surface(src);
#if SDL_VERSION_ATLEAST(2,0,0)
	SDL_FillRect(ret, NULL, SDL_MapRGBA(ret->format, color_.r, color_.g,
					    color_.b, color_.a));
#else
	SDL_FillRect(ret, NULL, SDL_MapRGBA(ret->format, color_.r, color_.g,
					    color_.b, color_.unused));
#endif
	SDL_SetAlpha(src, SDL_SRCALPHA, SDL_ALPHA_OPAQUE);
	blit_surface(src, NULL, ret, NULL);
	return ret;
}

const SDL_Color& background_modification::get_color() const
{
	return color_;
}

namespace {

/** A macro for automatic modification parser registration
 *
 * It automatically registers the created parser in the mod_parsers map
 * It should be used just like a function header (look at the uses below)
 * It should only be used within an anonymous namespace
 *
 * @param type The modification type to be registered (unquoted)
 * @param args_var The name for the string argument provided
 */
#define REGISTER_MOD_PARSER(type, args_var)     	               \
	modification* parse_##type##_mod(const std::string&);          \
	struct parse_##type##_mod_registration                         \
	{                                                              \
	        parse_##type##_mod_registration()                      \
	        {                                                      \
			mod_parsers[#type] = &parse_##type##_mod;      \
                }                                                      \
        } parse_##type##_mod_registration_aux;                         \
        modification* parse_##type##_mod(const std::string& args_var)

// Color-range-based recoloring
REGISTER_MOD_PARSER(TC, args)
{
	std::vector<std::string> params = utils::split(args,',');

	if(params.size() < 2) {
		ERR_DP << "too few arguments passed to the ~TC() function" << std::endl;

		return NULL;
	}

	int side_n = lexical_cast_default<int>(params[0], -1);
	std::string team_color;
	if (side_n < 1) {
		ERR_DP << "invalid team (" << side_n
		       << ") passed to the ~TC() function\n";
		return NULL;
	}
	else if (side_n < static_cast<int>(image::get_team_colors().size())) {
		team_color = image::get_team_colors()[side_n - 1];
	}
	else {
		// This side is not initialized; use default "n"
		try {
			team_color = lexical_cast<std::string>(side_n);
		} catch(bad_lexical_cast const&) {
			ERR_DP << "bad things happen" << std::endl;

			return NULL;
		}
	}

	//
	// Pass argseters for RC functor
	//
	if(!game_config::tc_info(params[1]).size()){
		ERR_DP << "could not load TC info for '" << params[1]
		       << "' palette\n"
		       << "bailing out from TC\n";

		return NULL;
	}

	std::map<Uint32, Uint32> rc_map;
	try {
		color_range const& new_color =
			game_config::color_info(team_color);
		std::vector<Uint32> const& old_color =
			game_config::tc_info(params[1]);

		rc_map = recolor_range(new_color,old_color);
	}
	catch(config::error const& e) {
		ERR_DP << "caught config::error while processing TC: "
		       << e.message
		       << '\n'
		       << "bailing out from TC\n";

		return NULL;
	}

	return new rc_modification(rc_map);
}

// Team-color-based color range selection and recoloring
REGISTER_MOD_PARSER(RC, args)
{
	const std::vector<std::string> recolor_params = utils::split(args,'>');

	if(recolor_params.size()>1){
		//
		// recolor source palette to color range
		//
		std::map<Uint32, Uint32> rc_map;
		try {
			color_range const& new_color =
				game_config::color_info(recolor_params[1]);
			std::vector<Uint32> const& old_color =
				game_config::tc_info(recolor_params[0]);

			rc_map = recolor_range(new_color,old_color);
		}
		catch (config::error& e) {
			ERR_DP
				<< "caught config::error while processing color-range RC: "
				<< e.message
				<< '\n';
			ERR_DP
				<< "bailing out from RC\n";
			rc_map.clear();
		}

		return new rc_modification(rc_map);
	}
	else {
		///@Deprecated 1.6 palette switch syntax
		if(args.find('=') != std::string::npos) {
			lg::wml_error << "the ~RC() image function cannot be used for palette switch (A=B) in 1.7.x; use ~PAL(A>B) instead\n";
		}
	}

	return NULL;
}

// Palette switch
REGISTER_MOD_PARSER(PAL, args)
{
	const std::vector<std::string> remap_params = utils::split(args,'>');

	if(remap_params.size() < 2) {
		ERR_DP << "not enough arguments passed to the ~PAL() function: "
		       << args << "\n";

		return NULL;
	}


	try {
		std::map<Uint32, Uint32> rc_map;
		std::vector<Uint32> const& old_palette =
			game_config::tc_info(remap_params[0]);
		std::vector<Uint32> const& new_palette =
			game_config::tc_info(remap_params[1]);

		for(size_t i = 0; i < old_palette.size() && i < new_palette.size(); ++i) {
			rc_map[old_palette[i]] = new_palette[i];
		}

		return new rc_modification(rc_map);
	}
	catch(config::error& e) {
		ERR_DP
			<< "caught config::error while processing PAL function: "
			<< e.message
			<< '\n';
		ERR_DP
			<< "bailing out from PAL\n";

		return NULL;
	}
}

// Flip/flop
REGISTER_MOD_PARSER(FL, args)
{
	bool horiz = (args.empty() || args.find("horiz") != std::string::npos);
	bool vert = (args.find("vert") != std::string::npos);

	return new fl_modification(horiz, vert);
}

// Rotations
REGISTER_MOD_PARSER(ROTATE, args)
{
	std::vector<std::string> const& slice_params = utils::split(args, ',', utils::STRIP_SPACES);
	const size_t s = slice_params.size();

	switch (s) {
		case 0:
			return new rotate_modification();
			break;
		case 1:
			return new rotate_modification(
					lexical_cast_default<int>(slice_params[0]));
			break;
		case 2:
			return new rotate_modification(
					lexical_cast_default<int>(slice_params[0]),
					lexical_cast_default<int>(slice_params[1]));
			break;
		case 3:
			return new rotate_modification(
					lexical_cast_default<int>(slice_params[0]),
					lexical_cast_default<int>(slice_params[1]),
					lexical_cast_default<int>(slice_params[2]));
			break;
	}
	return NULL;
}

// Grayscale
REGISTER_MOD_PARSER(GS, )
{
	return new gs_modification;
}

// Plot Alpha
REGISTER_MOD_PARSER(PLOT_ALPHA, )
{
	return new plot_alpha_modification;
}

// Wipe Alpha
REGISTER_MOD_PARSER(WIPE_ALPHA, )
{
	return new wipe_alpha_modification;
}

// Adjust Alpha
REGISTER_MOD_PARSER(ADJUST_ALPHA, args)
{
	const std::vector<std::string>& params = utils::split(args, ',');

	if(params.size() != 1) {
		ERR_DP << "~ADJUST_ALPHA() requires exactly 1 arguments" << std::endl;
		return NULL;
	}

	std::string opacity_str = params.at(0);

	const size_t p100_pos = opacity_str.find('%');

	if (p100_pos == std::string::npos) {
		return new adjust_alpha_modification(lexical_cast_default<fixed_t> (opacity_str));
	} else {
		float opacity = lexical_cast_default<float>(opacity_str.substr(0,p100_pos)) / 100.0f;
		return new adjust_alpha_modification(static_cast<fixed_t> (255 * opacity));
	}
}

// Color-shift
REGISTER_MOD_PARSER(CS, args)
{
	std::vector<std::string> const factors = utils::split(args, ',');
	const size_t s = factors.size();

	if(s == 0) {
		ERR_DP << "no arguments passed to the ~CS() function" << std::endl;
		return NULL;
	}

	int r = 0, g = 0, b = 0;

	r = lexical_cast_default<int>(factors[0]);

	if( s > 1 ) {
		g = lexical_cast_default<int>(factors[1]);
	}
	if( s > 2 ) {
		b = lexical_cast_default<int>(factors[2]);
	}

	return new cs_modification(r, g, b);
}

// Color blending
REGISTER_MOD_PARSER(BLEND, args)
{
	const std::vector<std::string>& params = utils::split(args, ',');

	if(params.size() != 4) {
		ERR_DP << "~BLEND() requires exactly 4 arguments" << std::endl;
		return NULL;
	}

	float opacity = 0.0f;
	const std::string& opacity_str = params[3];
	const std::string::size_type p100_pos = opacity_str.find('%');

	if(p100_pos == std::string::npos)
		opacity = lexical_cast_default<float>(opacity_str);
	else {
		// make multiplier
		const std::string& parsed_field = opacity_str.substr(0, p100_pos);
		opacity = lexical_cast_default<float>(parsed_field);
		opacity /= 100.0f;
	}

	return new blend_modification(
		lexical_cast_default<int>(params[0]),
		lexical_cast_default<int>(params[1]),
		lexical_cast_default<int>(params[2]),
		opacity);
}

// Crop/slice
REGISTER_MOD_PARSER(CROP, args)
{
	std::vector<std::string> const& slice_params = utils::split(args, ',', utils::STRIP_SPACES);
	const size_t s = slice_params.size();

	if(s == 0 || (s == 1 && slice_params[0].empty())) {
		ERR_DP << "no arguments passed to the ~CROP() function" << std::endl;
		return NULL;
	}

	SDL_Rect slice_rect = { 0, 0, 0, 0 };

	slice_rect.x = lexical_cast_default<Sint16, const std::string&>(slice_params[0]);

	if(s > 1) {
		slice_rect.y = lexical_cast_default<Sint16, const std::string&>(slice_params[1]);
	}
	if(s > 2) {
		slice_rect.w = lexical_cast_default<Uint16, const std::string&>(slice_params[2]);
	}
	if(s > 3) {
		slice_rect.h = lexical_cast_default<Uint16, const std::string&>(slice_params[3]);
	}

	return new crop_modification(slice_rect);
}

static bool check_image(const image::locator& img, std::stringstream & message)
{
	if(img.file_exists()) return true;
	message << " image not found: '" << img.get_filename() << "'\n";
	ERR_DP << message.str();
	return false;
}

// Blit
REGISTER_MOD_PARSER(BLIT, args)
{
	std::vector<std::string> param = utils::parenthetical_split(args, ',');
	const size_t s = param.size();

	if(s == 0 || (s == 1 && param[0].empty())){
		ERR_DP << "no arguments passed to the ~BLIT() function" << std::endl;
		return NULL;
	}

	int x = 0, y = 0;

	if(s == 3) {
		x = lexical_cast_default<int>(param[1]);
		y = lexical_cast_default<int>(param[2]);
	}

	if(x < 0 || y < 0) { //required by blit_surface
		ERR_DP << "negative position arguments in ~BLIT() function" << std::endl;
		return NULL;
	}

	const image::locator img(param[0]);
	std::stringstream message;
	message << "~BLIT():";
	if(!check_image(img, message))
		return NULL;
	surface surf = get_image(img);

	return new blit_modification(surf, x, y);
}

// Mask
REGISTER_MOD_PARSER(MASK, args)
{
	std::vector<std::string> param = utils::parenthetical_split(args, ',');
	const size_t s = param.size();

	if(s == 0 || (s == 1 && param[0].empty())){
		ERR_DP << "no arguments passed to the ~MASK() function" << std::endl;
		return NULL;
	}

	int x = 0, y = 0;

	if(s == 3) {
		x = lexical_cast_default<int>(param[1]);
		y = lexical_cast_default<int>(param[2]);
	}

	if(x < 0 || y < 0) { //required by blit_surface
		ERR_DP << "negative position arguments in ~MASK() function" << std::endl;
		return NULL;
	}

	const image::locator img(param[0]);
	std::stringstream message;
	message << "~MASK():";
	if(!check_image(img, message))
		return NULL;
	surface surf = get_image(img);

	return new mask_modification(surf, x, y);
}

// Light
REGISTER_MOD_PARSER(L, args)
{
	if(args.empty()){
		ERR_DP << "no arguments passed to the ~L() function" << std::endl;
		return NULL;
	}

	surface surf = get_image(args);

	return new light_modification(surf);
}

// Scale
REGISTER_MOD_PARSER(SCALE, args)
{
	std::vector<std::string> const& scale_params = utils::split(args, ',', utils::STRIP_SPACES);
	const size_t s = scale_params.size();

	if(s == 0 || (s == 1 && scale_params[0].empty())) {
		ERR_DP << "no arguments passed to the ~SCALE() function" << std::endl;
		return NULL;
	}

	int w = 0, h = 0;

	w = lexical_cast_default<int, const std::string&>(scale_params[0]);

	if(s > 1) {
		h = lexical_cast_default<int, const std::string&>(scale_params[1]);
	}

	return new scale_modification(w, h);
}

REGISTER_MOD_PARSER(SCALE_SHARP, args)
{
	std::vector<std::string> const& scale_params = utils::split(args, ',', utils::STRIP_SPACES);
	const size_t s = scale_params.size();

	if(s == 0 || (s == 1 && scale_params[0].empty())) {
		ERR_DP << "no arguments passed to the ~SCALE_SHARP() function" << std::endl;
		return NULL;
	}

	int w = 0, h = 0;

	w = lexical_cast_default<int, const std::string&>(scale_params[0]);

	if(s > 1) {
		h = lexical_cast_default<int, const std::string&>(scale_params[1]);
	}

	return new scale_sharp_modification(w, h);
}


// xBRZ
REGISTER_MOD_PARSER(XBRZ, args)
{
	int z = lexical_cast_default<int, const std::string &>(args);
	if (z < 1 || z > 5) {
		z = 5; //only values 2 - 5 are permitted for xbrz scaling factors.
	}

	return new xbrz_modification(z);
}

// scale

// Gaussian-like blur
REGISTER_MOD_PARSER(BL, args)
{
	const int depth = std::max<int>(0, lexical_cast_default<int>(args));

	return new bl_modification(depth);
}

// Opacity-shift
REGISTER_MOD_PARSER(O, args)
{
	const std::string::size_type p100_pos = args.find('%');
	float num = 0.0f;
	if(p100_pos == std::string::npos)
		num = lexical_cast_default<float,const std::string&>(args);
	else {
		// make multiplier
		const std::string parsed_field = args.substr(0, p100_pos);
		num = lexical_cast_default<float,const std::string&>(parsed_field);
		num /= 100.0f;
	}

	return new o_modification(num);
}

//
// ~R(), ~G() and ~B() are the children of ~CS(). Merely syntactic sugar.
// Hence they are at the end of the evaluation.
//
// Red component color-shift
REGISTER_MOD_PARSER(R, args)
{
	const int r = lexical_cast_default<int>(args);

	return new cs_modification(r,0,0);
}

// Green component color-shift
REGISTER_MOD_PARSER(G, args)
{
	const int g = lexical_cast_default<int>(args);

	return new cs_modification(0,g,0);
}

// Blue component color-shift
REGISTER_MOD_PARSER(B, args)
{
	const int b = lexical_cast_default<int>(args);

	return new cs_modification(0,0,b);
}

REGISTER_MOD_PARSER(NOP, )
{
	return NULL;
}

// Fake image function used by GUI2 portraits until
// Mordante gets rid of it. *tsk* *tsk*
REGISTER_MOD_PARSER(RIGHT, )
{
	return NULL;
}

// Add a bright overlay.
REGISTER_MOD_PARSER(BRIGHTEN, )
{
	return new brighten_modification;
}

// Add a dark overlay.
REGISTER_MOD_PARSER(DARKEN, )
{
	return new darken_modification;
}

// Add a background color.
REGISTER_MOD_PARSER(BG, args)
{
	int c[4] = { 0, 0, 0, 255 };
	std::vector<std::string> factors = utils::split(args, ',');

	for (int i = 0; i < std::min<int>(factors.size(), 4); ++i) {
		c[i] = lexical_cast_default<int>(factors[i]);
	}

	return new background_modification(create_color(c[0], c[1], c[2], c[3]));
}

} // end anon namespace

} /* end namespace image */
