/*
 * SDL_SavePNG -- libpng-based SDL_Surface writer.
 *
 * This code is free software, available under zlib/libpng license.
 * http://www.libpng.org/pub/png/src/libpng-LICENSE.txt
 */
#include <SDL.h>

#ifdef __GNUC__
#if __GNUC__ > 4 || __GNUC__== 4 && __GNUC_MINOR__ >= 9
#define HAVE_GCC_4_9
#endif
#endif

#ifdef HAVE_GCC_4_9
#pragma GCC diagnostic push
#ifndef __clang__
#pragma GCC diagnostic ignored "-Wliteral-suffix" //GCC 4.9 with C++11 gives a warning about this in libpng header
#endif
#endif

#include <png.h>

#ifdef HAVE_GCC_4_9
#pragma GCC diagnostic pop
#endif

#include "savepng.h"

#define SAVEPNG_SUCCESS 0
#define SAVEPNG_ERROR -1

#define USE_ROW_POINTERS

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
#define rmask 0xFF000000
#define gmask 0x00FF0000
#define bmask 0x0000FF00
#define amask 0x000000FF
#else
#define rmask 0x000000FF
#define gmask 0x0000FF00
#define bmask 0x00FF0000
#define amask 0xFF000000
#endif

/* libpng callbacks */ 
static void png_error_SDL(png_structp /*ctx*/, png_const_charp str)
{
	SDL_SetError("libpng: %s\n", str);
}
static void png_write_SDL(png_structp png_ptr, png_bytep data, png_size_t length)
{
	SDL_RWops *rw = (SDL_RWops*)png_get_io_ptr(png_ptr);
	SDL_RWwrite(rw, data, sizeof(png_byte), length);
}

SDL_Surface *SDL_PNGFormatAlpha(SDL_Surface *src) 
{
	SDL_Surface *surf;
	SDL_Rect rect = { 0 , 0 , 0 , 0 };

	/* NO-OP for images < 32bpp and 32bpp images that already have Alpha channel */ 
	if (src->format->BitsPerPixel <= 24 || src->format->Amask) {
		src->refcount++;
		return src;
	}

	/* Convert 32bpp alpha-less image to 24bpp alpha-less image */
	rect.w = src->w;
	rect.h = src->h;
	surf = SDL_CreateRGBSurface(src->flags, src->w, src->h, 24,
		src->format->Rmask, src->format->Gmask, src->format->Bmask, 0);
	SDL_LowerBlit(src, &rect, surf, &rect);

	return surf;
}

int SDL_SavePNG_RW(SDL_Surface *surface, SDL_RWops *dst, int freedst) 
{
	png_structp png_ptr;
	png_infop info_ptr;
	png_colorp pal_ptr;
	SDL_Palette *pal;
	int i, colortype;
#ifdef USE_ROW_POINTERS
	png_bytep *row_pointers;
#endif
	/* Initialize and do basic error checking */
	if (!dst)
	{
		SDL_SetError("Argument 2 to SDL_SavePNG_RW can't be NULL, expecting SDL_RWops*\n");
		return (SAVEPNG_ERROR);
	}
	if (!surface)
	{
		SDL_SetError("Argument 1 to SDL_SavePNG_RW can't be NULL, expecting SDL_Surface*\n");
		if (freedst) SDL_RWclose(dst);
		return (SAVEPNG_ERROR);
	}
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, png_error_SDL, NULL); /* err_ptr, err_fn, warn_fn */
	if (!png_ptr) 
	{
		SDL_SetError("Unable to png_create_write_struct on %s\n", PNG_LIBPNG_VER_STRING);
		if (freedst) SDL_RWclose(dst);
		return (SAVEPNG_ERROR);
	}
	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		SDL_SetError("Unable to png_create_info_struct\n");
		png_destroy_write_struct(&png_ptr, NULL);
		if (freedst) SDL_RWclose(dst);
		return (SAVEPNG_ERROR);
	}
	if (setjmp(png_jmpbuf(png_ptr)))	/* All other errors, see also "png_error_SDL" */
	{
		png_destroy_write_struct(&png_ptr, &info_ptr);
		if (freedst) SDL_RWclose(dst);
		return (SAVEPNG_ERROR);
	}

	/* Setup our RWops writer */
	png_set_write_fn(png_ptr, dst, png_write_SDL, NULL); /* w_ptr, write_fn, flush_fn */

	/* Prepare chunks */
	colortype = PNG_COLOR_MASK_COLOR;
	if (surface->format->BytesPerPixel > 0
	&&  surface->format->BytesPerPixel <= 8
	&& (pal = surface->format->palette))
	{
		colortype |= PNG_COLOR_MASK_PALETTE;
		pal_ptr = (png_colorp)malloc(pal->ncolors * sizeof(png_color));
		for (i = 0; i < pal->ncolors; i++) {
			pal_ptr[i].red   = pal->colors[i].r;
			pal_ptr[i].green = pal->colors[i].g;
			pal_ptr[i].blue  = pal->colors[i].b;
		}
		png_set_PLTE(png_ptr, info_ptr, pal_ptr, pal->ncolors);
		free(pal_ptr);
	}
	else if (surface->format->BytesPerPixel > 3 || surface->format->Amask)
		colortype |= PNG_COLOR_MASK_ALPHA;

	png_set_IHDR(png_ptr, info_ptr, surface->w, surface->h, 8, colortype,
		PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

//	png_set_packing(png_ptr);

	/* Allow BGR surfaces */
	if (surface->format->Rmask == bmask
	&& surface->format->Gmask == gmask
	&& surface->format->Bmask == rmask)
		png_set_bgr(png_ptr);

	/* Write everything */
	png_write_info(png_ptr, info_ptr);
#ifdef USE_ROW_POINTERS
	row_pointers = (png_bytep*) malloc(sizeof(png_bytep)*surface->h);
	for (i = 0; i < surface->h; i++)
		row_pointers[i] = (png_bytep)(Uint8*)surface->pixels + i * surface->pitch;
	png_write_image(png_ptr, row_pointers);
	free(row_pointers);
#else
	for (i = 0; i < surface->h; i++)
		png_write_row(png_ptr, (png_bytep)(Uint8*)surface->pixels + i * surface->pitch);
#endif
	png_write_end(png_ptr, info_ptr);

	/* Done */
	png_destroy_write_struct(&png_ptr, &info_ptr);
	if (freedst) SDL_RWclose(dst);
	return (SAVEPNG_SUCCESS);
}
