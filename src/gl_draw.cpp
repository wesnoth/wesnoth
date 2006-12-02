#include "gl_draw.hpp"
#include <GL/gl.h>
#include <SDL.h>
#include <algorithm>
#include <iostream>

namespace {

const size_t SurfaceCacheSize = 503;

SDL_Surface* surfaceCache[SurfaceCacheSize];
GLuint textureCache[SurfaceCacheSize];
bool surfaceCacheInit = false;
int cacheHits = 0, cacheMisses = 0;

void load_surface(SDL_Surface* surf)
{
	if(surfaceCacheInit == false) {
		surfaceCacheInit = true;
		glGenTextures(SurfaceCacheSize,textureCache);
		SDL_Surface* nullSurf= NULL;
		std::fill(surfaceCache,surfaceCache+SurfaceCacheSize,nullSurf);
	}

	const size_t index =
	         reinterpret_cast<size_t>(surf)%SurfaceCacheSize;
	if(surfaceCache[index] == surf) {
		++cacheHits;
		glBindTexture(GL_TEXTURE_2D,textureCache[index]);
		return;
	}

	++cacheMisses;
	if((cacheMisses%100) == 0) {
		std::cerr << cacheHits << "/" << cacheMisses << "\n";
	}

	if(surfaceCache[index] != NULL) {
		SDL_FreeSurface(surfaceCache[index]);
	}

	surfaceCache[index] = surf;
	++surf->refcount;

	glBindTexture(GL_TEXTURE_2D,textureCache[index]);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	assert(surf->format->BitsPerPixel == 32);
	glTexImage2D(GL_TEXTURE_2D,0,4,surf->w,surf->h,0,GL_RGBA,
	             GL_UNSIGNED_BYTE,surf->pixels);
	glBindTexture(GL_TEXTURE_2D,textureCache[index]);
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
	load_surface(surf);

	glColor4f(red,green,blue,alpha);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0,0.0);
	glVertex3i(x,y,0);
	glTexCoord2f(1.0,0.0);
	glVertex3i(x+surf->w,y,0);
	glTexCoord2f(1.0,1.0);
	glVertex3i(x+surf->w,y+surf->h,0);
	glTexCoord2f(0.0,1.0);
	glVertex3i(x,y+surf->h,0);
	glEnd();
}

void draw_surface(SDL_Surface* surf, int x, int y, int w, int h)
{
	load_surface(surf);

	glColor4f(1.0,1.0,1.0,1.0);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0,0.0);
	glVertex3i(x,y,0);
	glTexCoord2f(1.0,0.0);
	glVertex3i(x+w,y,0);
	glTexCoord2f(1.0,1.0);
	glVertex3i(x+w,y+h,0);
	glTexCoord2f(0.0,1.0);
	glVertex3i(x,y+h,0);
	glEnd();
}
		
}
