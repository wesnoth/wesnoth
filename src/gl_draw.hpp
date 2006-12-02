#ifndef GL_DRAW_HPP_INCLUDED
#define GL_DRAW_HPP_INCLUDED

#include <GL/gl.h>
#include <SDL.h>

namespace gl {

void prepare_frame();
void flip();

void rect(const SDL_Rect& r, GLubyte red, GLubyte green, GLubyte blue);
void rect(const SDL_Rect& r, GLubyte red, GLubyte green, GLubyte blue,
                             GLubyte alpha);

void draw_surface(SDL_Surface* surf, int x, int y,
              GLfloat red=1.0, GLfloat green=1.0,
              GLfloat blue=1.0, GLfloat alpha=1.0);

void draw_surface(SDL_Surface* surf, int x, int y, int w, int h);

}

#endif
