/* $Id$ */
/*
   copyright (c) 2008 by mark de wever <koraq@xs4all.nl>
   part of the battle for wesnoth project http://www.wesnoth.org/

   this program is free software; you can redistribute it and/or modify
   it under the terms of the gnu general public license version 2
   or at your option any later version.
   this program is distributed in the hope that it will be useful,
   but without any warranty.

   see the copying file for more details.
*/

#include "gui/widgets/helper.hpp"
#include "gui/widgets/settings.hpp"
#include "serialization/string_utils.hpp"

#include "log.hpp"

#include "SDL_ttf.h"

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

namespace gui2 {

namespace {
	static bool initialized_ = false;
}

bool init() {
	if(initialized_) {
		return true;
	}
	
	load_settings();

	initialized_ = true;

	return initialized_;
}

SDL_Rect create_rect(const tpoint& origin, const tpoint& size) 
{ 
	return ::create_rect(origin.x, origin.y, size.x, size.y); 
}

std::ostream &operator<<(std::ostream &stream, const tpoint& point)
{
	stream << point.x << ',' << point.y;
	return stream;
}


int decode_font_style(const std::string& style)
{
	if(style == "bold") {
		return TTF_STYLE_BOLD;
	} else if(style == "italic") {
		return TTF_STYLE_ITALIC;
	} else if(style == "underline") {
		return TTF_STYLE_UNDERLINE;
	} else if(style.empty() || style == "normal") {
		return TTF_STYLE_NORMAL;
	}

	ERR_G << "Unknown style '" << style << "' using 'normal' instead.\n";

	return TTF_STYLE_NORMAL;
}

Uint32 decode_colour(const std::string& colour)
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

namespace {

// Surface locker class based on surface_locker in sdl_utils.hpp.
struct surface_lock
{
	surface_lock(SDL_Surface* surf) : surface_(surf), locked_(false)
	{
		if(SDL_MUSTLOCK(surface_)) {
			const int res = SDL_LockSurface(surface_);
			if(res == 0) {
				locked_ = true;
			}
		}
	}

	~surface_lock()
	{
		if(locked_) {
			SDL_UnlockSurface(surface_);
		}
	}

	Uint32* pixels() { return reinterpret_cast<Uint32*>(surface_->pixels); }
private:
	SDL_Surface* surface_;
	bool locked_;
};

} // namespace

//! Replacement for SDL_BlitSurface.
//!
//! SDL_BlitSurface has problems with blitting partly transparent surfaces so
//! this is a replacement. It ignores the SDL_SRCALPHA and SDL_SRCCOLORKEY 
//! flags. src and dst will have the SDL_RLEACCEL flag removed.
//! The return value of SDL_BlistSurface is normally ignored so no return value.
//! The rectangles are const and will not be modified.
//!
//! @param src          The surface to blit.
//! @param srcrect      The size of the surface to blit, only width and height
//!                     are used.
//! @param dst          The surface to blit on.
//! @param dstrect      The offset to blit the surface on, only x and y are used.
void blit_surface(SDL_Surface* src, 
	const SDL_Rect* srcrect, SDL_Surface* dst, const SDL_Rect* dstrect)
{
	assert(src);
	assert(dst);
	assert((src->flags & SDL_RLEACCEL) == 0);
	assert((dst->flags & SDL_RLEACCEL) == 0);

	// Get the areas to blit
	SDL_Rect dst_rect = { 0, 0, dst->w, dst->h };
	if(dstrect) {
		dst_rect.x = dstrect->x;
		dst_rect.w -= dstrect->x;

		dst_rect.y = dstrect->y;
		dst_rect.h -= dstrect->y;

		assert(dst_rect.x >= 0);
		assert(dst_rect.y >= 0);
	}

	SDL_Rect src_rect = { 0, 0, src->w, src->h };
	if(srcrect && srcrect->w && srcrect->h) { 
		src_rect.w = srcrect->w;
		src_rect.h = srcrect->h;

		assert(src_rect.w <= src->w);
		assert(src_rect.h <= src->h);
	}

	// Get the blit size limits.
	const unsigned width = minimum(src_rect.w, dst_rect.w);
	const unsigned height = minimum(src_rect.h, dst_rect.h);

	{
		// Extra scoping used for the surface_lock.
		surface_lock src_lock(src);
		surface_lock dst_lock(dst);

		Uint32* const src_pixels = reinterpret_cast<Uint32*>(src_lock.pixels());
		Uint32* dst_pixels = reinterpret_cast<Uint32*>(dst_lock.pixels());

		for(unsigned y = 0; y < height; ++y) {
			for(unsigned x = 0; x < width; ++x) {

				// We need to do the blitting using some optimizations
				// if the src is fully transparent we can ignore this pixel 
				// if the src is fully opaque we can overwrite the destination with this pixel
				// if the destination is fully transparent we replace us with the source
				//
				// We do these optimizations between the extraction of the variables
				// to avoid creating variables not used (it might save us some cycles).

				const Uint32 src_pixel = src_pixels[(y + src_rect.y) * src->w + (x + src_rect.x)];
				const Uint8 src_a = (src_pixel & 0xFF000000) >> 24;

				if(!src_a) {
					// Fully transparent source, ignore
					continue;
				}

				const ptrdiff_t dst_offset = (y + dst_rect.y) * dst->w + (x + dst_rect.x);
				if(src_a == 255) {
					// Fully opaque source, copy
					dst_pixels[dst_offset] = src_pixel;
					continue;
				}

				const Uint32 dst_pixel = dst_pixels[dst_offset];
				Uint8 dst_a = (dst_pixel & 0xFF000000) >> 24;

				if(!dst_a) {
					// Fully transparent destination, copy
					dst_pixels[dst_offset] = src_pixel;
					continue;
				}

				const Uint8 src_r = (src_pixel & 0x00FF0000) >> 16;
				const Uint8 src_g = (src_pixel & 0x0000FF00) >> 8;
				const Uint8 src_b = src_pixel & 0x000000FF;

				Uint8 dst_r = (dst_pixel & 0x00FF0000) >> 16;
				Uint8 dst_g = (dst_pixel & 0x0000FF00) >> 8;
				Uint8 dst_b = dst_pixel & 0x000000FF;

				if(dst_a == 255) {
					
					// Destination fully opaque blend the source.
					dst_r = (((src_r - dst_r) * src_a) >> 8 ) + dst_r;
					dst_g = (((src_g - dst_g) * src_a) >> 8 ) + dst_g;
					dst_b = (((src_b - dst_b) * src_a) >> 8 ) + dst_b;

				} else {

					// Destination and source party transparent.

					// aquired the data now do the blitting
					const unsigned tmp_a = 255 - src_a;

					const unsigned tmp_r = 1 + (src_r * src_a) + (dst_r * tmp_a);
					dst_r = (tmp_r + (tmp_r >> 8)) >> 8;

					const unsigned tmp_g = 1 + (src_g * src_a) + (dst_g * tmp_a);
					dst_g = (tmp_g + (tmp_g >> 8)) >> 8;

					const unsigned tmp_b = 1 + (src_b * src_a) + (dst_b * tmp_a);
					dst_b = (tmp_b + (tmp_b >> 8)) >> 8;

					dst_a += (((255 - dst_a) * src_a) >> 8);
				}

				dst_pixels[dst_offset] = (dst_a << 24) | (dst_r << 16) | (dst_g << 8) | (dst_b);

			}
		}
	}
}

} // namespace gui2

