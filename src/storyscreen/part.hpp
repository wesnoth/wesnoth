/*
   Copyright (C) 2009 - 2017 by Ignacio R. Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file
 * Storyscreen parts and floating images representation.
 */

#ifndef STORYSCREEN_PART_HPP_INCLUDED
#define STORYSCREEN_PART_HPP_INCLUDED

#include <string>
#include <utility>
#include <vector>

class config;
class vconfig;

namespace storyscreen
{
/**
 * Represents and contains information about image labels used
 * in story screen parts.
 */
class floating_image
{
public:
	/**
	 * WML-based constructor.
	 * @param cfg Object corresponding to a [image] block's contents from
	 *            a [part] node.
	 */
	floating_image(const config& cfg);

	/**
	 * Copy constructor.
	 */
	floating_image(const floating_image& fi);

	floating_image& operator=(const floating_image& fi)
	{
		assign(fi);
		return *this;
	}

	std::string file() const
	{
		return file_;
	}

	/**
	 * Returns the referential X coordinate of the image.
	 * The actual (corrected) value is determined at render time.
	 */
	int ref_x() const
	{
		return x_;
	}

	/**
	 * Returns the referential Y coordinate of the image.
	 * The actual (corrected) value is determined at render time.
	 */
	int ref_y() const
	{
		return y_;
	}

	/**
	 * Whether the image should be automatically scaled as much as
	 * the storyscreen background is.
	 */
	bool autoscale() const
	{
		return autoscaled_;
	}

	/**
	 * Whether the image coordinates specify the location of its
	 * center (true) or top-left corner (false).
	 */
	bool centered() const
	{
		return centered_;
	}

	/** Delay before displaying, in milliseconds. */
	int display_delay() const
	{
		return delay_;
	}

private:
	std::string file_;
	int x_, y_; // referential (non corrected) x,y
	int delay_;
	bool autoscaled_;
	bool centered_;

	/** Copy constructor and operator=() implementation details. */
	void assign(const floating_image& fi);
};

class background_layer
{
public:
	background_layer();

	/**
	 * Constructor. Initializes a background_layer object from a
	 * [background_layer] WML node.
	 */
	background_layer(const config& cfg);

	/** Whether the layer should be scaled horizontally. */
	bool scale_horizontally() const
	{
		return scale_horizontally_;
	}

	/** Sets whether the layer should be scaled horizontally. */
	void set_scale_horizontally(bool b)
	{
		scale_horizontally_ = b;
	}

	/** Whether the layer should be scaled vertically. */
	bool scale_vertically() const
	{
		return scale_vertically_;
	}

	/** Sets whether the layer should be scaled vertically. */
	void set_scale_vertically(bool b)
	{
		scale_vertically_ = b;
	}

	/** Whether the layer should be tiled horizontally. */
	bool tile_horizontally() const
	{
		return tile_horizontally_;
	}

	/** Sets whether the layer should be tiled horizontally. */
	void set_tile_horizontally(bool b)
	{
		tile_horizontally_ = b;
	}

	/** Whether the layer should be tiled vertically. */
	bool tile_vertically() const
	{
		return tile_vertically_;
	}

	/** Sets whether the layer should be tiled vertically. */
	void set_tile_vertically(bool b)
	{
		tile_vertically_ = b;
	}

	/** Whether the aspect ratio should be preserved while scaling. */
	bool keep_aspect_ratio() const
	{
		return keep_aspect_ratio_;
	}

	/** Sets whether the aspect ratio should be preserved. */
	void set_keep_aspect_ratio(bool b)
	{
		keep_aspect_ratio_ = b;
	}

	/** Whether is this layer the base layer. */
	bool is_base_layer() const
	{
		return is_base_layer_;
	}

	/** Sets whether is this layer a base layer. */
	void set_base_layer(bool b)
	{
		is_base_layer_ = b;
	}

	/** The path to the file to load the image from. */
	const std::string& file() const
	{
		return image_file_;
	}

	/** Sets the path to the image file. */
	void set_file(const std::string& str)
	{
		image_file_ = str;
	}

private:
	bool scale_horizontally_;
	bool scale_vertically_;
	bool tile_horizontally_;
	bool tile_vertically_;
	bool keep_aspect_ratio_;
	bool is_base_layer_;
	std::string image_file_;
};

/**
 * Represents and contains information about a single storyscreen part.
 */
class part
{
public:
	/**
	 * Currently used to indicate where the text block should be placed.
	 * Note that it will always take as much space as it is
	 * possible horizontally.
	 */
	enum BLOCK_LOCATION {
		BLOCK_TOP,    /**< Top of the screen. */
		BLOCK_MIDDLE, /**< Center of the screen. */
		BLOCK_BOTTOM  /**< Bottom of the screen. This is the default. */
	};

	/**
	 * Currently used to indicate where the page title should be placed.
	 * It always takes as little space (horizontally) as possible,
	 * and it is always placed at the top of the screen.
	 */
	enum TEXT_ALIGNMENT {
		TEXT_LEFT,     /**< Top-left corner. */
		TEXT_CENTERED, /**< Center on the topmost edge of the screen. */
		TEXT_RIGHT     /**< Top-right corner. */
	};

	/**
	 * Used to signal user actions.
	 */
	enum RESULT {
		NEXT, /**< Jump to next story part. */
		SKIP, /**< Skip all story parts for this set. */
		QUIT  /**< Quit game and go back to main menu. */
	};

	/**
	 * Constructs a storyscreen part from a managed WML node.
	 * @param part_cfg Node object which should correspond to a [part] block's contents.
	 */
	part(const vconfig& part_cfg);

	/** Whether the story screen title should be displayed or not. */
	bool show_title() const
	{
		return show_title_;
	}

	/** Retrieves the story text itself. */
	const std::string& text() const
	{
		return text_;
	}

	/** Changes the story text. */
	void set_text(const std::string& text)
	{
		text_ = text;
	}

	/** Retrieves the story screen title. */
	const std::string& title() const
	{
		return text_title_;
	}

	/** Changes the story screen title. */
	void set_title(const std::string& title)
	{
		text_title_ = title;
	}

	/** Retrieves the background music. */
	const std::string& music() const
	{
		return music_;
	}

	/** Retrieves a one-time-only sound effect. */
	const std::string& sound() const
	{
		return sound_;
	}

	/** Retrieves the area of the screen on which the story text is displayed. */
	BLOCK_LOCATION story_text_location() const
	{
		return text_block_loc_;
	}

	/** Retrieves the alignment of the story text within the text area. */
	TEXT_ALIGNMENT story_text_alignment() const
	{
		return text_alignment_;
	}

	/** Retrieves the alignment of the title text against the screen. */
	TEXT_ALIGNMENT title_text_alignment() const
	{
		return title_alignment_;
	}

	/** Retrieve any associated floating images for this story screen. */
	const std::vector<floating_image>& get_floating_images() const
	{
		return floating_images_;
	}

	/** Retrieve background layers for this story screen. */
	const std::vector<background_layer>& get_background_layers() const
	{
		return background_layers_;
	}

private:
	/** Takes care of initializing and branching properties. */
	void resolve_wml(const vconfig& cfg);

	static BLOCK_LOCATION string_tblock_loc(const std::string& s);
	static TEXT_ALIGNMENT string_title_align(const std::string& s);

	bool show_title_;
	std::string text_;
	std::string text_title_;
	BLOCK_LOCATION text_block_loc_;
	TEXT_ALIGNMENT text_alignment_;
	TEXT_ALIGNMENT title_alignment_;

	std::string music_;
	std::string sound_;

	std::vector<background_layer> background_layers_;
	std::vector<floating_image> floating_images_;
};

} // end namespace storyscreen

#endif /* ! STORYSCREEN_PART_HPP_INCLUDED */
