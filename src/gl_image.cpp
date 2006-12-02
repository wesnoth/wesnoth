#include "gl_image.hpp"

#include <assert.h>

namespace gl {

image::image() : id_(0), width_(0), height_(0)
{}

image::~image()
{
	release();
}

void image::release()
{
	if(id_) {
		glDeleteTextures(1,&id_);
	}
}

image::image(surface surf) : id_(0), width_(0), height_(0)
{
	set(surf);
}

void image::set(surface surf)
{
	release();

	width_ = surf->w;
	height_ = surf->h;

	glGenTextures(1,&id_);
	assert(surf->format->BitsPerPixel == 32);

	glBindTexture(GL_TEXTURE_2D,id_);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D,0,4,surf->w,surf->h,0,GL_RGBA,
	             GL_UNSIGNED_BYTE,surf->pixels);
}

void image::draw(int x, int y, image::ORIENTATION orient,
                 ::image::TYPE type,
                 const time_of_day* tod) const
{
	draw(x,y,width_,height_,orient,type,tod);
}

void image::draw(int x, int y, unsigned int w, unsigned int h,
                 image::ORIENTATION orient, ::image::TYPE type,
                 const time_of_day* tod) const
{
	if(!id_) {
		return;
	}

	glBindTexture(GL_TEXTURE_2D,id_);

	const GLfloat left = orient == NORMAL_ORIENTATION ? 0.0 : 1.0;
	const GLfloat right = 1.0 - left;

	glBegin(GL_QUADS);

	switch(type) {
	case ::image::GREYED:
		glColor4f(0.2,0.2,0.2,1.0);
		break;
	case ::image::DARKENED:
		glColor4f(0.4,0.4,0.6,1.0);
		break;
	default:
		if(tod) {
			glColor4f((100.0+tod->red)/100.0,(100.0+tod->green)/100.0,
			          (100.0+tod->blue)/100.0,1.0);
		} else {
			glColor4f(1.0,1.0,1.0,1.0);
		}
		break;
	}

	glTexCoord2f(left,0.0);
	glVertex3i(x,y,0);
	glTexCoord2f(right,0.0);
	glVertex3i(x+w,y,0);
	glTexCoord2f(right,1.0);
	glVertex3i(x+w,y+h,0);
	glTexCoord2f(left,1.0);
	glVertex3i(x,y+h,0);
	glEnd();

	//if the image is brightened, then we draw it a second time,
	//with additive blending, to make it brighter
	if(type == ::image::BRIGHTENED ||
	   type == ::image::SEMI_BRIGHTENED) {
		glBlendFunc(GL_ONE,GL_ONE);

		if(type == ::image::BRIGHTENED) {
			glColor4f(0.5,0.5,0.5,1.0);
		} else {
			glColor4f(0.2,0.2,0.2,1.0);
		}

		glBegin(GL_QUADS);
		glTexCoord2f(left,0.0);
		glVertex3i(x,y,0);
		glTexCoord2f(right,0.0);
		glVertex3i(x+w,y,0);
		glTexCoord2f(right,1.0);
		glVertex3i(x+w,y+h,0);
		glTexCoord2f(left,1.0);
		glVertex3i(x,y+h,0);
		glEnd();

		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	}
}

}
