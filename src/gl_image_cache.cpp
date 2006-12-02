#include <map>

#include "SDL_image.h"

#include "filesystem.hpp"
#include "gl_image_cache.hpp"
#include "gl_image.hpp"

namespace gl {

namespace {

typedef std::map<std::string,image*> image_map;
image_map image_cache;

typedef std::map< ::image::locator,image*> loc_image_map;
loc_image_map loc_image_cache;

const std::string images_str = "images";
const std::string units_str = "units/";
		
}

const image& get_image(const std::string& key)
{
	image*& img = image_cache[key];
	if(img != 0) {
		return *img;
	}

	const std::string& loc = get_binary_file_location(images_str,key);

	surface res;

	if(loc.empty() == false) {
		res = IMG_Load(loc.c_str());
	}

	if(res.null()) {
		const std::string& loc = get_binary_file_location(images_str,
		                                          units_str + key);
		if(loc.empty() == false) {
			res = IMG_Load(loc.c_str());
		}
	}

	if(res.null()) {
		img = new image;
	} else {
		if(res->format->BitsPerPixel != 32) {
			res = make_neutral_surface(res);
		}

		img = new image(res);
	}
	
	return *img;
}

const image& get_image(const ::image::locator& loc)
{
	image*& img = loc_image_cache[loc];
	if(img != 0) {
		return *img;
	}

	surface res(loc.load_from_disk());
	if(res.null()) {
		img = new image;
	} else {
		if(res->format->BitsPerPixel != 32) {
			res = make_neutral_surface(res);
		}

		img = new image(res);
	}

	return *img;
}

void flush_image_cache()
{
	for(image_map::iterator i = image_cache.begin();
	    i != image_cache.end(); ++i) {
		delete i->second;
	}

	for(loc_image_map::iterator i = loc_image_cache.begin();
	    i != loc_image_cache.end(); ++i) {
		delete i->second;
	}

	image_cache.clear();
	loc_image_cache.clear();
}
		
}
