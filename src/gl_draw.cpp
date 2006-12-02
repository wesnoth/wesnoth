#include "gl_draw.hpp"
#include "gl_image.hpp"
#include <GL/gl.h>
#include <SDL.h>
#include <algorithm>
#include <iostream>
#include "wassert.hpp"

namespace {

const size_t SurfaceCacheSize = 97;

SDL_Surface* surfaceCache[SurfaceCacheSize];
gl::image textureCache[SurfaceCacheSize];
bool surfaceCacheInit = false;
int cacheHits = 0, cacheMisses = 0;

const gl::image& load_surface(SDL_Surface* surf)
{
	if(surfaceCacheInit == false) {
		surfaceCacheInit = true;
		SDL_Surface* nullSurf= NULL;
		std::fill(surfaceCache,surfaceCache+SurfaceCacheSize,nullSurf);
	}

	const size_t index =
	         reinterpret_cast<size_t>(surf)%SurfaceCacheSize;
	if(surfaceCache[index] == surf) {
		++cacheHits;
		return textureCache[index];
	}

	++cacheMisses;
	//if((cacheMisses%100) == 0) {
	//	std::cerr << cacheHits << "/" << cacheMisses << "\n";
	//}

	if(surfaceCache[index] != NULL) {
		SDL_FreeSurface(surfaceCache[index]);
	}

	surfaceCache[index] = surf;
	++surf->refcount;
	textureCache[index].set(surf);
	return textureCache[index];
}
		
}

namespace gl {

void prepare_frame()
{
	const SDL_Surface* fb = SDL_GetVideoSurface();
	if(fb == NULL) {
		return;
	}
	
	glViewport(0,0,fb->w,fb->h);
	glShadeModel(GL_FLAT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0,fb->w,fb->h,0,-1.0,1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//glClearColor(0.0,0.0,0.0,0.0);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void flip()
{
	SDL_GL_SwapBuffers();
}

void rect(const SDL_Rect& r, GLubyte red, GLubyte green, GLubyte blue)
{
	glDisable(GL_TEXTURE_2D);
	glColor3ub(red,green,blue);
	glRecti(r.x,r.y,r.x+r.w,r.y+r.h);
	glEnable(GL_TEXTURE_2D);
}

void rect(const SDL_Rect& r, GLubyte red, GLubyte green, GLubyte blue,
                             GLubyte alpha)
{
	glDisable(GL_TEXTURE_2D);
	glColor4ub(red,green,blue,alpha);
	glRecti(r.x,r.y,r.x+r.w,r.y+r.h);
	glEnable(GL_TEXTURE_2D);
}

void draw_surface(SDL_Surface* surf, int x, int y,
              GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
	const gl::image& img = load_surface(surf);
	img.draw(x,y,surf->w,surf->h,red,green,blue,alpha);
}

void draw_surface(SDL_Surface* surf, int x, int y, int w, int h)
{
	const gl::image& img = load_surface(surf);
	img.draw(x,y,w,h,1.0,1.0,1.0,1.0);
}
		
}
