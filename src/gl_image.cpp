#include "gl_image.hpp"

#include <assert.h>

#include <cstring>
#include "log.hpp"

#define INFO LOG_STREAM(info, display)

namespace gl {

namespace {

bool npot_allowed()
{
	static bool once = false;
	static bool npot = true;
	if (once) return npot;
	once = true;

	const char *supported = reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS));
	const char *version = reinterpret_cast<const char *>(glGetString(GL_VERSION));
	const char *vendor = reinterpret_cast<const char *>(glGetString(GL_VENDOR));

	INFO << "OpenGL version: " << version << "\n";
	INFO << "OpenGL vendor: " << vendor << "\n";
	INFO << "OpenGL extensions: " << supported << "\n";

	// OpenGL >= 2.0 drivers must support NPOT textures
	bool version_2 = (version[0] >= '2');
	npot = version_2;
	// directly test for NPOT extension
	if (std::strstr(supported, "GL_ARB_texture_non_power_of_two")) npot = true;
	if (std::strstr(supported, "GL_EXT_texture_rectangle")) npot = true;

	if (npot) {
		// Use some heuristic to make sure it is HW accelerated. Might need some
		// more work.
		if (std::strstr(vendor, "NVIDIA Corporation")) {
			if (!std::strstr(supported, "NV_fragment_program2") ||
				!std::strstr(supported, "NV_vertex_program3"))
					npot = false;
			else if (std::strstr(vendor, "ATI Technologies"))
				if (!std::strstr(supported, "GL_ARB_texture_non_power_of_two") && !std::strstr(supported, "GL_EXT_texture_rectangle"))
					npot = false;
		}
	}
	INFO << "NPOT textures: " << (npot ? "enabled" : "disabled") << "\n";
	return npot;
}

bool is_pot(unsigned int num)
{
	return (num&(num-1)) == 0;
}

unsigned int next_pot(unsigned int num)
{
	unsigned int res = 1;
	while(res < num) {
		res <<= 1;
	}

	return res;
}
		
}

image::image() : id_(0), width_(0), height_(0),
				 top_(0), bot_(0), left_(0), right_(0)
{}

image::~image()
{
	release();
}

void image::release()
{
	if(id_) {
		glDeleteTextures(1,&id_);
		id_ = 0;
	}
}

image::image(SDL_Surface* surf) : id_(0), width_(0), height_(0),
								  top_(0), bot_(0), left_(0), right_(0)
{
	set(surf);
}

void image::set(SDL_Surface* surf)
{
	width_ = surf->w;
	height_ = surf->h;

	if(!id_) {
		glGenTextures(1,&id_);
	}

	assert(surf->format->BitsPerPixel == 32);

	glBindTexture(GL_TEXTURE_2D,id_);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if(npot_allowed() || surf->w == surf->h && is_pot(surf->w)) {
		top_ = left_ = 0.0;
		bot_ = right_ = 1.0;
		glTexImage2D(GL_TEXTURE_2D,0,4,surf->w,surf->h,0,GL_RGBA,
		             GL_UNSIGNED_BYTE,surf->pixels);
	} else {
		const unsigned int dim = next_pot(std::max(surf->w,surf->h));
		if(dim > 1024) {
			return;
		}
		std::vector<unsigned char> v(dim*dim*4);
		for(unsigned int y = 0; y != surf->h; ++y) {
			const unsigned char* src =
			  reinterpret_cast<unsigned char*>(surf->pixels) +
			  y*surf->w*4;
			unsigned char* dst = &v[y*dim*4];
			memcpy(dst,src,4*surf->w);
		}

		glTexImage2D(GL_TEXTURE_2D,0,4,dim,dim,0,GL_RGBA,
		             GL_UNSIGNED_BYTE,&v[0]);
		top_ = left_ = 0.0;
		right_ = static_cast<GLfloat>(surf->w)/
		         static_cast<GLfloat>(dim);
		bot_ = static_cast<GLfloat>(surf->h)/
		       static_cast<GLfloat>(dim);
	}
}

void image::draw(int x, int y, unsigned int w, unsigned int h,
                 GLfloat red, GLfloat green,
				 GLfloat blue,GLfloat alpha,
				 ORIENTATION orient, VFLIP vflip) const
{
	if(!id_) {
		return;
	}

	GLfloat left = left_;
	GLfloat right = right_;
	GLfloat top = top_;
	GLfloat bot = bot_;

	if(orient == REVERSE_ORIENTATION) {
		std::swap(left,right);
	}

	if(vflip == UPSIDE_DOWN) {
		std::swap(top,bot);
	}

	glBindTexture(GL_TEXTURE_2D,id_);

	glBegin(GL_QUADS);
	glColor4f(red,green,blue,alpha);
	glTexCoord2f(left,top);
	glVertex3i(x,y,0);
	glTexCoord2f(right,top);
	glVertex3i(x+w,y,0);
	glTexCoord2f(right,bot);
	glVertex3i(x+w,y+h,0);
	glTexCoord2f(left,bot);
	glVertex3i(x,y+h,0);
	glEnd();
}

void image::draw(int x, int y, image::ORIENTATION orient,
                 ::image::TYPE type,
                 const time_of_day* tod) const
{
	draw(x,y,width_,height_,orient,type,tod);
}

void image::draw(int x, int y, unsigned int w, unsigned int h,
                 image::ORIENTATION orient, ::image::TYPE type,
                 const time_of_day* tod,
				 image::VFLIP vflip) const
{
	if(!id_) {
		return;
	}

	glBindTexture(GL_TEXTURE_2D,id_);

	GLfloat left = left_;
	GLfloat right = right_;
	GLfloat top = top_;
	GLfloat bot = bot_;
	if(orient == REVERSE_ORIENTATION) {
		std::swap(left,right);
	}

	if(vflip == UPSIDE_DOWN) {
		std::swap(top,bot);
	}

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

	glTexCoord2f(left,top);
	glVertex3i(x,y,0);
	glTexCoord2f(right,top);
	glVertex3i(x+w,y,0);
	glTexCoord2f(right,bot);
	glVertex3i(x+w,y+h,0);
	glTexCoord2f(left,bot);
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
		glTexCoord2f(left,top);
		glVertex3i(x,y,0);
		glTexCoord2f(right,top);
		glVertex3i(x+w,y,0);
		glTexCoord2f(right,bot);
		glVertex3i(x+w,y+h,0);
		glTexCoord2f(left,bot);
		glVertex3i(x,y+h,0);
		glEnd();

		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	}
}

}
