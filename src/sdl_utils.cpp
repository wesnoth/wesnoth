/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include <iostream>

#include "game.hpp"
#include "sdl_utils.hpp"
#include "util.hpp"
#include "video.hpp"

int sdl_add_ref(SDL_Surface* surface)
{
	if(surface != NULL)
		return surface->refcount++;
	else
		return 0;
}

SDL_Surface* scale_surface(SDL_Surface* surface, int w, int h)
{
	if(surface == NULL)
		return NULL;

	SDL_Surface* const dest = SDL_CreateRGBSurface(SDL_SWSURFACE,w,h,
					                     surface->format->BitsPerPixel,
										 surface->format->Rmask,
										 surface->format->Gmask,
										 surface->format->Bmask,
										 surface->format->Amask);
	if(dest == NULL) {
		std::cerr << "Could not create surface to scale onto\n";
		return NULL;
	}

	const double xratio = static_cast<double>(surface->w)/
			              static_cast<double>(w);
	const double yratio = static_cast<double>(surface->h)/
			              static_cast<double>(h);

	const int srcxpad = is_odd(surface->w) ? 1:0;
	const int dstxpad = is_odd(dest->w) ? 1:0;

	surface_lock dstlock(dest);
	surface_lock srclock(surface);

	double ysrc = 0.0;
	for(int ydst = 0; ydst != h; ++ydst, ysrc += yratio) {
		double xsrc = 0.0;
		for(int xdst = 0; xdst != w; ++xdst, xsrc += xratio) {
			int xsrcint = static_cast<int>(xsrc);
			const int ysrcint = static_cast<int>(ysrc);

			xsrcint += srcxpad*ysrcint;

			const int dstpad = dstxpad*ydst;

			dstlock.pixels()[ydst*w + xdst + dstpad] =
			             srclock.pixels()[ysrcint*surface->w + xsrcint];
		}
	}

	return dest;
}

void adjust_surface_colour(SDL_Surface* surface, int r, int g, int b)
{
	if(r == 0 && g == 0 && b == 0)
		return;

	const int xpad = is_odd(surface->w) ? 1:0;

	short* pixel = reinterpret_cast<short*>(surface->pixels);
	for(int y = 0; y != surface->h; ++y, pixel += xpad) {
		const short* const end = pixel + surface->w;
		while(pixel != end) {
			if(*pixel != 0 && *pixel != short(0xFFFF)) {
				Uint8 red, green, blue;
				SDL_GetRGB(*pixel,surface->format,&red,&green,&blue);

				red = maximum<int>(8,minimum<int>(255,int(red)+r));
				green = maximum<int>(0,minimum<int>(255,int(green)+g));
				blue  = maximum<int>(0,minimum<int>(255,int(blue)+b));

				*pixel = SDL_MapRGB(surface->format,red,green,blue);
			}

			++pixel;
		}
	}
}

SDL_Surface* get_surface_portion(SDL_Surface* src, SDL_Rect& area)
{
	if(area.x + area.w >= src->w || area.y + area.h >= src->h)
		return NULL;

	const SDL_PixelFormat* const fmt = src->format;
	SDL_Surface* const dst = SDL_CreateRGBSurface(0,area.w,area.h,
	                                              fmt->BitsPerPixel,fmt->Rmask,
	                                              fmt->Gmask,fmt->Bmask,
	                                              fmt->Amask);
	SDL_Rect dstarea = {0,0,0,0};

	SDL_BlitSurface(src,&area,dst,&dstarea);

	return dst;
}

/* Following function (C) 2002 Jared Minch, released under GNU GPL version 2 (see LICENSE for a copy), thanks aardvark! */
/* Rotates 8-, 16-, and 32-bit sprites.  24-bit sprites aren't handled; you
 * must convert to one of these color depths before calling this function.
 * */
#ifdef WIN32
#define tanf(value) (float)tan(value)
#define sinf(value) (float)sin(value)
#define cosf(value) (float)cos(value)
#endif

#ifndef M_PI
#define M_PI 3.141592653589793238
#endif

#ifndef M_PI_2
#define M_PI_2 1.570796326794896619
#endif

#define itofix(x) ((x) << 16)
#define ftofix(x) ((long)((x) * 65536.0 + ((x) < 0 ? -0.5 : 0.5)))
#define fixtoi(x) (((x) >> 16) + (((x) & 0x8000) >> 15))
#define fixtof(x) ((float) (x) / 65536.0)
#define fixdiv(x,y) ftofix(fixtof(x) / fixtof(y))

typedef long fixed;

SDL_Surface *rotate( SDL_Surface *s, float ang )
{
	/* Quadrant */
	short q;
	float offset_ang;

	/* Destination surface */
	SDL_Surface *dst;
	/* Destination surface size */
	short dstw, dsth;
	fixed dstwf, dsthf;
	fixed srcwf, srchf;

	/* Precalculated cos and sin */
	float cosang, sinang;

	/* Corner points */
	short corner_top, corner_left, corner_right, corner_bottom;

	/* source step values for each side (How far to move along the source side
	 * for each vertical pixel) */
	fixed src_step_top, src_step_side;

	/* Source step values per horizontal pixel */
	fixed src_step_x, src_step_y;

	/* destination step values (how far to move horizontally for each vertical
	 * pixel) */
	fixed dst_step_0, dst_step_1;

	/* Fixed and integer location source pixel locations */
	fixed src_left;
	fixed xf, yf;
	int xi, yi, xnew, ynew, dx, dy;

	/* Pointer step values */
	int psx, psy;

	/* Destination line and remaining line length */
	fixed dstl_left, dstl_right;
	int dsty, dstx;

	/* Maximum line length (Calculated to prevent accumulated error to cause
	 * an overrun) */
	int maxx;

	/* Source and destination pointers */
	char *sp;
	char *dp, *dlp, *dfp;

	/* Current side segments: = 0 for first segment, = 1 for second */
	short seg_left, seg_right;

	char bpp;

	bpp = s->format->BytesPerPixel;

	ang = ((ang * M_PI) / 180.0); /* convert to radians, line added by Chris Thielen under author's permission */

	cosang = cosf(ang);
	sinang = sinf(ang);

	/* Determine the quadrant (0 - 3) */
	if(sinang >= 0) {
		q = (cosang >= 0) ? 0 : 1;
	} else {
		q = (cosang >= 0) ? 3 : 2;
	}

	offset_ang = ang - (M_PI_2 * (float)q);

	/* Determine the size of the rotated surface, and the corner points */
	if(q == 0 || q == 2) {
		corner_top = (s->w - 1) * cosf(offset_ang);
		corner_left = (s->w - 1) * sinf(offset_ang);
		corner_right = (s->h - 1) * cosf(offset_ang);
		corner_bottom = (s->h - 1) * sinf(offset_ang);
	} else {
		corner_top = (s->h - 1) * cosf(offset_ang);
		corner_left = (s->h - 1) * sinf(offset_ang);
		corner_right = (s->w - 1) * cosf(offset_ang);
		corner_bottom = (s->w - 1) * sinf(offset_ang);
	}

	/* Width = w cos * h sin; Height = h cos * w sin */
	dstw = corner_top + corner_bottom + 1;
	dsth = corner_left + corner_right + 1;
	dstwf = itofix(dstw - 1);
	dsthf = itofix(dsth - 1);

	srcwf = itofix(s->w - 1);
	srchf = itofix(s->h - 1);

	/* Create the destination surface */
	dst = SDL_CreateRGBSurface(SDL_SWSURFACE, dstw, dsth, 
								s->format->BitsPerPixel,
								s->format->Rmask, s->format->Gmask,
								s->format->Bmask, s->format->Amask);
	SDL_SetColorKey(dst, SDL_SRCCOLORKEY | SDL_RLEACCEL, (Uint32)SDL_MapRGB(dst->format, 0, 0, 0));

//      aotf_assert(ERR_FATAL, dst,
//                                      "Allocation of new zoomed surface failed.");

	/* Calculate step values */
	src_step_x = ftofix(cosang);
	src_step_y = ftofix(sinang);
	src_step_top = -ftofix(1.0 / sinang);
	src_step_side = ftofix(1.0 / cosang);
	dst_step_0 = ftofix(1.0 / tanf(offset_ang));
	dst_step_1 = ftofix(tanf(offset_ang));

	psx = bpp;
	psy = s->pitch;

	/* All right; do the actual rotation copy */
	SDL_LockSurface(dst);
	SDL_LockSurface(s);
	seg_left = seg_right = 0;
	dlp = (char*)dst->pixels;
	dstl_left = dstl_right = itofix(corner_top);

	switch(q) {
		case 0:
			src_left = srcwf;
			break;
		case 1:
			src_left = srchf;
		break;
		case 2:
			src_left = 0;
		break;
		case 3:
			src_left = 0;
		break;
	}

	for(dsty = 0; dsty < dsth; dsty++, dlp += dst->pitch) {

		/* If at a corner point, reset pointers and update seg_foo */
		if(dsty == corner_left) {
			dstl_left = 0;
			seg_left = 1;
			switch(q) {
				case 0:
					src_left = 0;
					break;
				case 1:
					src_left = srcwf;
					break;
				case 2:
					src_left = srchf;
					break;
				case 3:
					src_left = 0;
					break;
			}
		}

		if(dsty == corner_right) {
			dstl_right = dstwf;
			seg_right = 1;
		}

		/* Set the destination pointer */
		dp = dlp + fixtoi(dstl_left) * bpp;

		/* Set up source pointer and values */
		if(!seg_left) {
			switch(q) {
				case 0:
					xf = src_left;
					yf = 0;
					break;
				case 1:
					xf = srcwf;
					yf = src_left;
					break;
				case 2:
					xf = src_left;
					yf = srchf;
					break;
				case 3:
					xf = 0;
					yf = src_left;
					break;
			}
		} else {
			switch(q) {
				case 0:
					xf = 0;
					yf = src_left;
					break;
				case 1:
					xf = src_left;
					yf = 0;
					break;
				case 2:
					xf = srcwf;
					yf = src_left;
					break;
				case 3:
					xf = src_left;
					yf = srchf;
					break;
			}
		}

		/* Determine the line length */
		dstx = fixtoi(dstl_right - dstl_left) + 1;

		if(src_step_x) {
			if(src_step_x > 0)
				maxx = fixdiv((srcwf - xf), src_step_x) >> 16;
			else
				maxx = fixdiv(xf, -src_step_x) >> 16;

			if(maxx < dstx) dstx = maxx;
		}

		if(src_step_y) {
			if(src_step_y > 0)
				maxx = fixdiv((srchf - yf), src_step_y) >> 16;
			else
				maxx = fixdiv(yf, -src_step_y) >> 16;

			if(maxx < dstx) dstx = maxx;
		}

		xi = fixtoi(xf);  yi = fixtoi(yf);

		sp = (char *)s->pixels + xi * psx + yi * psy;

		/* Find the boundary bytes for the destination */
		dfp = dp + dstx * bpp;

		/* Copy from the source to the destination */
		while(dp < dfp) {
			switch(bpp) {
				case 1:
					*(Uint8 *)dp = *(Uint8 *)sp;
					break;
				case 2:
					*(Uint16 *)dp = *(Uint16 *)sp;
					break;
				case 4:
					*(Uint32 *)dp = *(Uint32 *)sp;
					break;
			}

			xf += src_step_x;  yf += src_step_y;
			xnew = fixtoi(xf);  ynew = fixtoi(yf);
			dx = xnew - xi;  dy = ynew - yi;
			sp += (dx * psx) + (dy * psy);
			xi = xnew;  yi = ynew;
			dp += bpp;
		}

		/* The following if block is identical to the following commented out
		 * block.  This is wordier but more efficient. */
		if(q == 0 || q == 2) {
			if(seg_left) {
				dstl_left += dst_step_1;
				src_left += src_step_side;
			} else {
				dstl_left -= dst_step_0;
				src_left += src_step_top;
			}
		} else {
			if(seg_left) {
				dstl_left += dst_step_1;
				src_left += src_step_top;
			} else {
				dstl_left -= dst_step_0;
				src_left += src_step_side;
			}
		}

#if 0
		dstl_left += seg_left ? (dst_step_1) : (-dst_step_0);
		if(q == 0 || q == 2)
			src_left += seg_left ? src_step_side : src_step_top;
		else 
			src_left += seg_left ? src_step_top : src_step_side;
#endif
		dstl_right += seg_right ? (-dst_step_0) : (dst_step_1);

		/* Make sure we're staying on the surface */
		/* OPTIMIZEME */
#if 0
		if(dstl_left < 0) dstl_left = 0;
		if(dstl_right < 0) dstl_right = 0;
		if(dstl_left > dstwf) dstl_left = dstwf;
		if(dstl_right > dstwf) dstl_right = dstwf;
		if(src_left < 0) src_left = 0;
		if(src_left > srcwf) src_left = srcwf;
#endif

	}

	/* Finished; unlock surfaces */
	SDL_UnlockSurface(dst);
	SDL_UnlockSurface(s);

	return dst;
}

surface_restorer::surface_restorer(SDL_Surface* surface, SDL_Rect& rect)
: target_(surface), rect_(rect), surface_(NULL)
{
	update();
}

surface_restorer::~surface_restorer()
{
	restore();
}

void surface_restorer::restore()
{
	::SDL_BlitSurface(surface_,NULL,target_,&rect_);
	update_rect(rect_);
}

void surface_restorer::update()
{
	surface_.assign(::get_surface_portion(target_,rect_));
}
