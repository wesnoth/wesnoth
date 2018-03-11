# SDL_SavePNG

Minimal libpng interface to save SDL_Surfaces as PNG files.

You might want to take a look in "savepng.h" - it is much shorter and simpler
than this README.

## Install

Add "savepng.c" and "savepng.h" to your project.

Link the libpng library, i.e. add the `-lpng` LDFLAG (even if you already have
`-lSDL_image`).

## Use

```
#include "savepng.h"

SDL_Surface *bmp = ... //your surface
if (SDL_SavePNG(bmp, "image.png")) {	//boring way with error checking
	printf("Unable to save png -- %s\n", SDL_GetError());
}
```

As you can see, `SDL_SavePNG` accepts an SDL_Surface and a filename for it's
input. Similar to SDL_SaveBMP, it is a wrapper around the actual RWops-based
`SDL_SavePNG_RW` function, so you could use that, if needed.

Lastly, there is `SDL_PNGFormatAlpha`, modeled after SDL_DisplayFormatAlpha,
that would convert *any SDL_Surface* to an *SDL_Surface suitable for PNG
output*. Each call to `SDL_PNGFormatAlpha` produces a **new** SDL_Surface that
**must** be freed using `SDL_FreeSurface`.

```
//safest way, useful for 'screen' surface
SDL_Surface *tmp = SDL_PNGFormatAlpha(screen);
SDL_SavePNG(tmp, "screenshot.png");
SDL_FreeSurface(tmp)
```

Such conversion is actually only required for *one* surface format (see below),
and would do **nothing** for all other formats, making it **very fast**. The
format in question is:

### 32-bpp surfaces without alpha

There is a interesting caveat of combining naive libpng and cunning SDL in a
32-bpp video mode.

The *screen* surface (obtained by `SDL_SetVideoMode` or similarly) might (and
will!) ignore it's alpha-component even in the 32bpp mode. Meaning that an
0xAARRGGBB color would be blitted as 0xFFrrggbb irregardless, as if it was a
24bpp color.

Since screen itself is never blitted onto anything else, ignoring the alpha
makes perfect sense. However, unlike 24bpp images, the alpha component *does*
exist. Thus, when such surface is saved, it appears to be completely
transparent, as the alpha values for each pixel are set to 0.

Depending on your video mode, you might or might not need to first convert your
surface using `SDL_PNGFormatAlpha`. If you have absolute control over the video
surface, you can force it to 24bpp (or less) mode, which would avoid the
problem.

If the surface passed to `SDL_PNGFormatAlpha` is already suitable, a no-op is
performed. It is very fast, so you should probably always convert your surfaces
before saving.

### No text chunks

Unfortunately, a simplistic interface such as SDL_SavePNG provides no means to
write PNG meta-data. If you need to add iTXT chunks to your PNGs, you would
have to modify this code or write your own version.

If you have some kind of simple API, that would be thematically consistent with
SDL, in mind -- please share.

## Demo

See `main.c` and `Makefile` for an example program. It too is shorter than this
README.

# About

The problem in question is very simple, and this little piece of functionality
was implemented and re-implemented multiple times by multiple authors (notably,
Angelo "Encelo" Theodorou and Darren Grant, among others). I decided to write
my own version to ensure it's correctness, learn more about libpng, and to
provide a copy-pastable, maintained, libpng15-aware, palette-supporting
variation that I could link to. You can view it as a continuation of their
efforts.

SDL_Image would've been perfect place for this, but that library has different
purposes.

*Next up: code to load SDL_Surfaces as OpenGL 1.1 textures. J/K ;)*

# Copying

SDL_SavePNG is available under the zlib/libpng license.