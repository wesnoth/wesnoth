/*
	Copyright (C) 2022 - 2024
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "draw.hpp"

#include "color.hpp"
#include "log.hpp"
#include "sdl/rect.hpp"
#include "sdl/texture.hpp"
#include "sdl/utils.hpp" // sdl::runtime_at_least
#include "video.hpp"

#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>

static lg::log_domain log_draw("draw");
#define DBG_D LOG_STREAM(debug, log_draw)
#define WRN_D LOG_STREAM(warn, log_draw)

static SDL_Renderer* renderer()
{
	return video::get_renderer();
}

/**************************************/
/* basic drawing and pixel primatives */
/**************************************/

void draw::clear()
{
	DBG_D << "clear";
	SDL_BlendMode b;
	SDL_GetRenderDrawBlendMode(renderer(), &b);
	SDL_SetRenderDrawBlendMode(renderer(), SDL_BLENDMODE_NONE);
	fill(0, 0, 0, 0);
	SDL_SetRenderDrawBlendMode(renderer(), b);
}

void draw::fill(
	const SDL_Rect& area,
	uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	DBG_D << "fill " << area << ' ' << color_t{r,g,b,a};
	SDL_SetRenderDrawColor(renderer(), r, g, b, a);
	SDL_RenderFillRect(renderer(), &area);
}

void draw::fill(
	const SDL_Rect& area,
	uint8_t r, uint8_t g, uint8_t b)
{
	draw::fill(area, r, g, b, SDL_ALPHA_OPAQUE);
}

void draw::fill(const SDL_Rect& area, const color_t& c)
{
	draw::fill(area, c.r, c.g, c.b, c.a);
}

void draw::fill(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	DBG_D << "fill " << color_t{r,g,b,a};
	SDL_SetRenderDrawColor(renderer(), r, g, b, a);
	SDL_RenderFillRect(renderer(), nullptr);
}

void draw::fill(uint8_t r, uint8_t g, uint8_t b)
{
	draw::fill(r, g, b, SDL_ALPHA_OPAQUE);
}

void draw::fill(const color_t& c)
{
	draw::fill(c.r, c.g, c.b, c.a);
}

void draw::fill(const SDL_FRect& rect, const color_t& c)
{
	DBG_D << "sub-pixel fill";
	SDL_SetRenderDrawColor(renderer(), c.r, c.g, c.b, c.a);
	SDL_RenderFillRectF(renderer(), &rect);
}

void draw::fill(const SDL_Rect& area)
{
	DBG_D << "fill " << area;
	SDL_RenderFillRect(renderer(), &area);
}

void draw::fill()
{
	DBG_D << "fill";
	SDL_RenderFillRect(renderer(), nullptr);
}

void draw::set_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	DBG_D << "set color " << color_t{r,g,b,a};
	SDL_SetRenderDrawColor(renderer(), r, g, b, a);
}

void draw::set_color(uint8_t r, uint8_t g, uint8_t b)
{
	DBG_D << "set color " << color_t{r,g,b};
	SDL_SetRenderDrawColor(renderer(), r, g, b, SDL_ALPHA_OPAQUE);
}

void draw::set_color(const color_t& c)
{
	DBG_D << "set color " << c;
	SDL_SetRenderDrawColor(renderer(), c.r, c.g, c.b, c.a);
}

void draw::set_blend_mode(SDL_BlendMode b)
{
	SDL_SetRenderDrawBlendMode(renderer(), b);
}

/** Some versions of SDL have a bad rectangle drawing implementation. */
static bool sdl_bad_at_rects()
{
	// This could be done once at program start and cached,
	// but it isn't all that heavy.
	if (sdl::runtime_at_least(2,0,15) && !sdl::runtime_at_least(2,0,18)) {
		return true;
	}
	return false;
}

/** For some SDL versions, draw rectangles as lines. */
static void draw_rect_as_lines(const SDL_Rect& rect)
{
	// w and h indicate the final pixel width/height of the box.
	// This is 1 greater than the difference in corner coordinates.
	if (rect.w <= 0 || rect.h <= 0) {
		return;
	}
	int x2 = rect.x + rect.w - 1;
	int y2 = rect.y + rect.h - 1;
	draw::line(rect.x, rect.y, x2, rect.y);
	draw::line(rect.x, rect.y, rect.x, y2);
	draw::line(x2, rect.y, x2, y2);
	draw::line(rect.x, y2, x2, y2);
}

void draw::rect(const SDL_Rect& rect)
{
	DBG_D << "rect " << rect;
	if (sdl_bad_at_rects()) {
		return draw_rect_as_lines(rect);
	}
	SDL_RenderDrawRect(renderer(), &rect);
}

void draw::rect(const SDL_Rect& rect,
	uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	DBG_D << "rect " << rect << ' ' << color_t{r,g,b,a};
	SDL_SetRenderDrawColor(renderer(), r, g, b, a);
	if (sdl_bad_at_rects()) {
		return draw_rect_as_lines(rect);
	}
	SDL_RenderDrawRect(renderer(), &rect);
}

void draw::rect(const SDL_Rect& rect, uint8_t r, uint8_t g, uint8_t b)
{
	draw::rect(rect, r, g, b, SDL_ALPHA_OPAQUE);
}

void draw::rect(const SDL_Rect& rect, const color_t& c)
{
	draw::rect(rect, c.r, c.g, c.b, c.a);
}

void draw::line(int from_x, int from_y, int to_x, int to_y)
{
	DBG_D << "line from (" << from_x << ',' << from_y
	      << ") to (" << to_x << ',' << to_y << ')';
	SDL_RenderDrawLine(renderer(), from_x, from_y, to_x, to_y);
}

void draw::line(int from_x, int from_y, int to_x, int to_y, const color_t& c)
{
	DBG_D << "line from (" << from_x << ',' << from_y
	      << ") to (" << to_x << ',' << to_y
	      << ") with colour " << c;
	SDL_SetRenderDrawColor(renderer(), c.r, c.g, c.b, c.a);
	SDL_RenderDrawLine(renderer(), from_x, from_y, to_x, to_y);
}

void draw::points(const std::vector<SDL_Point>& points)
{
	DBG_D << points.size() << " points";
	SDL_RenderDrawPoints(renderer(), points.data(), points.size());
}

void draw::point(int x, int y)
{
	DBG_D << "point (" << x << ',' << y << ')';
	SDL_RenderDrawPoint(renderer(), x, y);
}

void draw::circle(int cx, int cy, int r, const color_t& c, uint8_t octants)
{
	draw::set_color(c);
	draw::circle(cx, cy, r, octants);
}

void draw::circle(int cx, int cy, int r, uint8_t octants)
{
	DBG_D << "circle (" << cx << ',' << cy
	      << ") -> " << r << ", oct " << int(octants);

	// Algorithm based on
	// http://de.wikipedia.org/wiki/Rasterung_von_Kreisen#Methode_von_Horn
	// version of 2011.02.07.
	int d = -r;
	int x = r;
	int y = 0;

	std::vector<SDL_Point> points;

	while(!(y > x)) {
		if(octants & 0x04) points.push_back({cx + x, cy + y});
		if(octants & 0x02) points.push_back({cx + x, cy - y});
		if(octants & 0x20) points.push_back({cx - x, cy + y});
		if(octants & 0x40) points.push_back({cx - x, cy - y});

		if(octants & 0x08) points.push_back({cx + y, cy + x});
		if(octants & 0x01) points.push_back({cx + y, cy - x});
		if(octants & 0x10) points.push_back({cx - y, cy + x});
		if(octants & 0x80) points.push_back({cx - y, cy - x});

		d += 2 * y + 1;
		++y;
		if(d > 0) {
			d += -2 * x + 2;
			--x;
		}
	}

	draw::points(points);
}

void draw::disc(int cx, int cy, int r, const color_t& c, uint8_t octants)
{
	draw::set_color(c);
	draw::disc(cx, cy, r, octants);
}

void draw::disc(int cx, int cy, int r, uint8_t octants)
{
	DBG_D << "disc (" << cx << ',' << cy
	      << ") -> " << r << ", oct " << int(octants);

	int d = -r;
	int x = r;
	int y = 0;

	while(!(y > x)) {
		// I use the formula of Bresenham's line algorithm
		// to determine the boundaries of a segment.
		// The slope of the line is always 1 or -1 in this case.
		if(octants & 0x04)
			// x2 - 1 = y2 - (cy + 1) + cx
			draw::line(cx + x, cy + y + 1, cx + y + 1, cy + y + 1);
		if(octants & 0x02)
			// x2 - 1 = cy - y2 + cx
			draw::line(cx + x, cy - y, cx + y + 1, cy - y);
		if(octants & 0x20)
			// x2 + 1 = (cy + 1) - y2 + (cx - 1)
			draw::line(cx - x - 1, cy + y + 1, cx - y - 2, cy + y + 1);
		if(octants & 0x40)
			// x2 + 1 = y2 - cy + (cx - 1)
			draw::line(cx - x - 1, cy - y, cx - y - 2, cy - y);

		if(octants & 0x08)
			// y2 = x2 - cx + (cy + 1)
			draw::line(cx + y, cy + x + 1, cx + y, cy + y + 1);
		if(octants & 0x01)
			// y2 = cx - x2 + cy
			draw::line(cx + y, cy - x, cx + y, cy - y);
		if(octants & 0x10)
			// y2 = (cx - 1) - x2 + (cy + 1)
			draw::line(cx - y - 1, cy + x + 1, cx - y - 1, cy + y + 1);
		if(octants & 0x80)
			// y2 = x2 - (cx - 1) + cy
			draw::line(cx - y - 1, cy - x, cx - y - 1, cy - y);

		d += 2 * y + 1;
		++y;
		if(d > 0) {
			d += -2 * x + 2;
			--x;
		}
	}
}


/*******************/
/* texture drawing */
/*******************/


void draw::blit(const texture& tex, const SDL_Rect& dst)
{
	if (dst == sdl::empty_rect) {
		return draw::blit(tex);
	}

	if (!tex) { DBG_D << "null blit"; return; }
	DBG_D << "blit " << dst;

	SDL_RenderCopy(renderer(), tex, tex.src(), &dst);
}

void draw::blit(const texture& tex)
{
	if (!tex) { DBG_D << "null blit"; return; }
	DBG_D << "blit";

	SDL_RenderCopy(renderer(), tex, tex.src(), nullptr);
}


static SDL_RendererFlip get_flip(bool flip_h, bool flip_v)
{
	// This should be easier than it is.
	return static_cast<SDL_RendererFlip>(
		static_cast<int>(flip_h ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE)
		| static_cast<int>(flip_v ? SDL_FLIP_VERTICAL : SDL_FLIP_NONE)
	);
}

void draw::flipped(
	const texture& tex,
	const SDL_Rect& dst,
	bool flip_h,
	bool flip_v)
{
	if (dst == sdl::empty_rect) {
		return draw::flipped(tex, flip_h, flip_v);
	}

	if (!tex) { DBG_D << "null flipped"; return; }
	DBG_D << "flipped (" << flip_h << '|' << flip_v
	      << ") to " << dst;

	SDL_RendererFlip flip = get_flip(flip_h, flip_v);
	SDL_RenderCopyEx(renderer(), tex, tex.src(), &dst, 0.0, nullptr, flip);
}

void draw::flipped(const texture& tex, bool flip_h, bool flip_v)
{
	if (!tex) { DBG_D << "null flipped"; return; }
	DBG_D << "flipped (" << flip_h << '|' << flip_v << ')';

	SDL_RendererFlip flip = get_flip(flip_h, flip_v);
	SDL_RenderCopyEx(renderer(), tex, tex.src(), nullptr, 0.0, nullptr, flip);
}


// TODO: highdpi - maybe expose this mirrored mode to WML somehow
void draw::tiled(const texture& tex, const SDL_Rect& dst, bool centered,
	bool mirrored)
{
	if (!tex) { DBG_D << "null tiled"; return; }
	DBG_D << "tiled (" << centered << '|' << mirrored
	      << ") " << dst;

	// Reduce clip to dst.
	auto clipper = draw::reduce_clip(dst);

	const int xoff = centered ? (dst.w - tex.w()) / 2 : 0;
	const int yoff = centered ? (dst.h - tex.h()) / 2 : 0;

	// Just blit the image however many times is necessary.
	bool vf = false;
	SDL_Rect t{dst.x - xoff, dst.y - yoff, tex.w(), tex.h()};
	for (; t.y < dst.y + dst.h; t.y += t.h, vf = !vf) {
		bool hf = false;
		for (t.x = dst.x - xoff; t.x < dst.x + dst.w; t.x += t.w, hf = !hf) {
			if (mirrored) {
				draw::flipped(tex, t, hf, vf);
			} else {
				draw::blit(tex, t);
			}
		}
	}
}

void draw::tiled_highres(const texture& tex, const SDL_Rect& dst,
	bool centered, bool mirrored)
{
	if (!tex) { DBG_D << "null tiled_highres"; return; }
	DBG_D << "tiled_highres (" << centered << '|' << mirrored
	      << ") " << dst;

	const int pixel_scale = video::get_pixel_scale();

	// Reduce clip to dst.
	auto clipper = draw::reduce_clip(dst);

	const ::point size = tex.get_raw_size();
	const float w = float(size.x) / float(pixel_scale);
	const float h = float(size.y) / float(pixel_scale);
	const float xoff = centered ? (dst.w - w) / 2 : 0.0f;
	const float yoff = centered ? (dst.h - h) / 2 : 0.0f;

	// Just blit the image however many times is necessary.
	bool vf = false;
	SDL_FRect t{dst.x - xoff, dst.y - yoff, w, h};
	for (; t.y < dst.y + dst.h; t.y += t.h, vf = !vf) {
		bool hf = false;
		for (t.x = dst.x - xoff; t.x < dst.x + dst.w; t.x += t.w, hf = !hf) {
			if (mirrored) {
				SDL_RendererFlip flip = get_flip(hf, vf);
				SDL_RenderCopyExF(renderer(), tex, nullptr, &t, 0.0, nullptr, flip);
			} else {
				SDL_RenderCopyF(renderer(), tex, nullptr, &t);
			}
		}
	}
}

void draw::smooth_shaded(const texture& tex, const SDL_Rect& dst,
	const SDL_Color& cTL, const SDL_Color& cTR,
	const SDL_Color& cBL, const SDL_Color& cBR,
	const SDL_FPoint& uvTL, const SDL_FPoint& uvTR,
	const SDL_FPoint& uvBL, const SDL_FPoint& uvBR)
{
	const SDL_FPoint pTL{float(dst.x), float(dst.y)};
	const SDL_FPoint pTR{float(dst.x + dst.w), float(dst.y)};
	const SDL_FPoint pBL{float(dst.x), float(dst.y + dst.h)};
	const SDL_FPoint pBR{float(dst.x + dst.w), float(dst.y + dst.h)};
	std::array<SDL_Vertex,4> verts {
		SDL_Vertex{pTL, cTL, uvTL},
		SDL_Vertex{pTR, cTR, uvTR},
		SDL_Vertex{pBL, cBL, uvBL},
		SDL_Vertex{pBR, cBR, uvBR},
	};
	draw::smooth_shaded(tex, verts);
}

void draw::smooth_shaded(const texture& tex, const SDL_Rect& dst,
	const SDL_Color& cTL, const SDL_Color& cTR,
	const SDL_Color& cBL, const SDL_Color& cBR)
{
	SDL_FPoint uv[4] = {
		{0.f, 0.f}, // top left
		{1.f, 0.f}, // top right
		{0.f, 1.f}, // bottom left
		{1.f, 1.f}, // bottom right
	};
	draw::smooth_shaded(tex, dst, cTL, cTR, cBL, cBR,
		uv[0], uv[1], uv[2], uv[3]);
}

void draw::smooth_shaded(const texture& tex,
	const std::array<SDL_Vertex, 4>& verts)
{
	DBG_D << "smooth shade, verts:";
	for (const SDL_Vertex& v : verts) {
		DBG_D << "  {(" << v.position.x << ',' << v.position.y << ") "
			<< v.color << " (" << v.tex_coord.x << ',' << v.tex_coord.y
			<< ")}";
	}
	int indices[6] = {0, 1, 2, 2, 1, 3};
	SDL_RenderGeometry(renderer(), tex, &verts[0], 4, indices, 6);
}

/***************************/
/* RAII state manipulation */
/***************************/


draw::clip_setter::clip_setter(const SDL_Rect& clip)
	: c_(draw::get_clip()), clip_enabled_(draw::clip_enabled())
{
	draw::force_clip(clip);
}

draw::clip_setter::~clip_setter()
{
	if (clip_enabled_) {
		draw::force_clip(c_);
	} else {
		draw::disable_clip();
	}
}

draw::clip_setter draw::override_clip(const SDL_Rect& clip)
{
	return draw::clip_setter(clip);
}

draw::clip_setter draw::reduce_clip(const SDL_Rect& clip)
{
	if (!draw::clip_enabled()) {
		return draw::clip_setter(clip);
	}
	return draw::clip_setter(draw::get_clip().intersect(clip));
}

void draw::force_clip(const SDL_Rect& clip)
{
	// TODO: highdpi - fix whatever reason there is for this guard (CI fail)
	if (!renderer()) {
		WRN_D << "trying to force clip will null renderer";
		return;
	}
	DBG_D << "forcing clip to " << clip;

	SDL_RenderSetClipRect(renderer(), &clip);
}

rect draw::get_clip()
{
	// TODO: highdpi - fix whatever reason there is for this guard (CI fail)
	if (!renderer()) {
		return sdl::empty_rect;
	}

	if (!SDL_RenderIsClipEnabled(renderer())) {
		return draw::get_viewport();
	}

	::rect clip;
	SDL_RenderGetClipRect(renderer(), &clip);
	return clip;
}

bool draw::clip_enabled()
{
	if (!renderer()) {
		return false;
	}
	return SDL_RenderIsClipEnabled(renderer());
}

void draw::disable_clip()
{
	if (!renderer()) {
		return;
	}
	SDL_RenderSetClipRect(renderer(), nullptr);
	DBG_D << "clip disabled";
}

bool draw::null_clip()
{
	if (!renderer()) {
		return true;
	}
	if (!SDL_RenderIsClipEnabled(renderer())) {
		return false;
	}
	SDL_Rect clip;
	SDL_RenderGetClipRect(renderer(), &clip);
	return clip.w <= 0 || clip.h <= 0;
}


draw::viewport_setter::viewport_setter(const SDL_Rect& view)
	: v_(), c_(), clip_enabled_(draw::clip_enabled())
{
	v_ = draw::get_viewport();
	draw::force_viewport(view);
	if (clip_enabled_) {
		c_ = draw::get_clip();
		// adjust clip for difference in viewport position
		SDL_Rect c_view = {
			c_.x + v_.x - view.x,
			c_.y + v_.y - view.y,
			c_.w, c_.h
		};
		draw::force_clip(c_view);
	}
}

draw::viewport_setter::~viewport_setter()
{
	draw::force_viewport(v_);
	if (clip_enabled_) {
		draw::force_clip(c_);
	} else {
		draw::disable_clip();
	}
}

draw::viewport_setter draw::set_viewport(const SDL_Rect& viewport)
{
	return draw::viewport_setter(viewport);
}

void draw::force_viewport(const SDL_Rect& viewport)
{
	if (!renderer()) {
		WRN_D << "trying to force viewport will null renderer";
		return;
	}
	DBG_D << "forcing viewport to " << viewport;

	SDL_RenderSetViewport(renderer(), &viewport);
}

SDL_Rect draw::get_viewport()
{
	if (!renderer()) {
		WRN_D << "no renderer available to get viewport";
		return sdl::empty_rect;
	}

	SDL_Rect viewport;
	SDL_RenderGetViewport(renderer(), &viewport);

	if (viewport == sdl::empty_rect) {
		return video::draw_area();
	}
	return viewport;
}

draw::render_target_setter::render_target_setter(const texture& t)
	: target_()
	, viewport_()
	, clip_()
{
	// Validate we can render to this texture.
	assert(!t || t.get_access() == SDL_TEXTUREACCESS_TARGET);

	if (!renderer()) {
		WRN_D << "can't set render target with null renderer";
		return;
	}

	target_ = video::get_render_target();
	SDL_RenderGetViewport(renderer(), &viewport_);
	SDL_RenderGetClipRect(renderer(), &clip_);

	if (t) {
		video::force_render_target(t);
	} else {
		video::reset_render_target();
	}
}

draw::render_target_setter::~render_target_setter()
{
	if (!renderer()) {
		WRN_D << "can't reset render target with null renderer";
		return;
	}
	video::force_render_target(target_);
	SDL_RenderSetViewport(renderer(), &viewport_);
	if(clip_ == sdl::empty_rect) return;
	SDL_RenderSetClipRect(renderer(), &clip_);
}

draw::render_target_setter draw::set_render_target(const texture& t)
{
	if (t) {
		DBG_D << "setting render target to "
		      << t.w() << 'x' << t.h() << " texture";
	} else {
		DBG_D << "setting render target to main render buffer";
	}
	return draw::render_target_setter(t);
}
