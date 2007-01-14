#ifndef GL_IMAGE_HPP_INCLUDED
#define GL_IMAGE_HPP_INCLUDED

#include <GL/gl.h>

#include "gamestatus.hpp"
#include "image.hpp"
#include "surface.hpp"

namespace gl {

class image
{
public:
	image();
	~image();
	explicit image(SDL_Surface* surf);

	void set(SDL_Surface* surf);

	unsigned int width() const { return width_; }
	unsigned int height() const { return height_; }

	enum ORIENTATION { NORMAL_ORIENTATION, REVERSE_ORIENTATION };
	enum VFLIP { RIGHT_SIDE_UP, UPSIDE_DOWN };

	void draw(int x, int y, unsigned int w, unsigned int h,
	          GLfloat red, GLfloat green,
			  GLfloat blue, GLfloat alpha,
			  ORIENTATION o=NORMAL_ORIENTATION,
			  VFLIP vflip=RIGHT_SIDE_UP) const;
	void draw(int x, int y, ORIENTATION o=NORMAL_ORIENTATION,
	          ::image::TYPE type=::image::SCALED,
			  const time_of_day* tod=0) const;
	void draw(int x, int y, unsigned int w, unsigned int h,
	          ORIENTATION o=NORMAL_ORIENTATION,
			  ::image::TYPE type=::image::SCALED,
			  const time_of_day* tod=0,
			  VFLIP vflip=RIGHT_SIDE_UP) const;
private:
	void release();

	image(const image&);
	void operator=(const image&);

	GLuint id_;
	unsigned int width_, height_;
	GLfloat top_, bot_, left_, right_;
};
		
}

#endif
