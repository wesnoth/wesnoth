/*
   Copyright (C) 2012 - 2018 by Mark de Wever <koraq@xs4all.nl>
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
 * Helper class for buffering a @c std::istream.
 */

#pragma once

#include <cstdio>
#include <sstream>

/**
 * Helper class for buffering a @c std::istream.
 *
 * This class is used to buffer a @c std::istream which is used for small
 * reads; a character at a time. The @c std::istream needs to create a
 * sentinel object for every read and profiling showed the @c std::istream
 * class was causing a lot of overhead when parsing WML. This class helps by
 * reading chunks from the @c std::stream and store them in an internal
 * buffer. Then the next request can deliver data from this buffer.
 *
 * Since the class is only designed for small reads it only offers the @ref
 * get() and the @ref peek() to get data and @ref eof() to signal the end of
 * data. The original stream should not be used from, while being owned by this
 * class.
 */
class buffered_istream
{
public:

	explicit buffered_istream(std::istream& in)
		: stream_(in)
		, buffer_()
		, buffer_size_(0)
		, buffer_offset_(0)
		, eof_(false)
	{
	}

	/**
	 * Gets and consumes a character from the buffer.
	 *
	 * @returns                   The character read.
	 * @retval EOF                The end of input has been read.
	 */
	int get()
	{
		fill_buffer();

		if(eof_) {
			return EOF;
		} else {
			/*
			 * The data needs to be casted to an unsigned value before it
			 * is promoted to an int. The char might be signed and contain
			 * a negative value, resulting in a negative result, and cause
			 * problems. (Using gcc on x86 has this issue.)
			 */
			unsigned char c = buffer_[buffer_offset_];
			++buffer_offset_;
			return c;
		}
	}

	/**
	 * Gets a character from the buffer.
	 *
	 * This version only gets a character, but doesn't consume it.
	 *
	 * @returns                   The character read.
	 * @retval EOF                The end of input has been read.
	 */
	int peek()
	{
		fill_buffer();

		if(eof_) {
			return EOF;
		} else {
			/* See get() */
			return static_cast<unsigned char>(buffer_[buffer_offset_]);
		}
	}

	/** Is the end of input reached? */
	bool eof() const
	{
		return eof_;
	}

	/** Returns the owned stream. */
	std::istream& stream()
	{
		return stream_;
	}

private:

	/** The input to read from. */
	std::istream& stream_;

	/**
	 * Buffer to store the data read from @c std::istream.
	 *
	 * Reading from @c std::istream isn't to fast, especially not a byte at a
	 * time. This buffer is used to buffer x bytes at a time. The size of the
	 * buffer is determined experimentally.
	 */
	char buffer_[1024];

	/**
	 * The size of @ref buffer_.
	 *
	 * When buffering the data there might be less data in the stream as in the
	 * buffer. This variable contains the exact size of the buffer. For example
	 * the last chunk read from the stream is unlikely to have the same size a
	 * @ref buffer_.
	 */
	unsigned buffer_size_;

	/**
	 * The offset of the current character in the buffer.
	 *
	 * @ref buffer_[buffer_offset_] is the current character, and can be peaked
	 * or consumed.
	 *
	 * @note the @ref buffer_offset_ may be beyond the @ref buffer_ so
	 * functions should test before directly using this variable.
	 */
	unsigned buffer_offset_;

	/** Is the end of input reached? */
	bool eof_;

	/**
	 * Fills the buffer.
	 *
	 * @warning This function must be called before @ref peek() and @ref get()
	 * to make sure the buffer state is valid before accessing it.
	 */
	void fill_buffer()
	{
		if(buffer_offset_ >= buffer_size_) {
			/*
			 * This does not only test for the EOF, but also makes sure the
			 * data is available in the buffer. Without it readsome will read
			 * nothing, after its first call, even if the EOF has not been
			 * reached.
			 */
			if(stream_.rdbuf()->sgetc() == EOF) {
				eof_ = true;
			} else {
				buffer_offset_ = 0;
				buffer_size_ = stream_.readsome(buffer_, sizeof(buffer_));
			}
		}
	}
};
