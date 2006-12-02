#ifndef GL_IMAGE_CACHE_HPP_INCLUDED
#define GL_IMAGE_CACHE_HPP_INCLUDED

#include "image.hpp"

#include <string>

namespace gl {

class image;

const image& get_image(const std::string& key);
const image& get_image(const ::image::locator& loc);

void flush_image_cache();
		
}

#endif
