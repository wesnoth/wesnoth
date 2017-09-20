/*
   Copyright (C) 2017 by the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include <SDL.h>
#include <SDL_rwops.h>

#include "filesystem.hpp"
#include "log.hpp"

#include <algorithm>

static lg::log_domain log_filesystem("filesystem");
#define ERR_FS LOG_STREAM(err, log_filesystem)

namespace filesystem {

// Arbitrary numbers larger than 5
static const Uint32 read_type = 7;
static const Uint32 write_type = 8;

static Sint64 ifs_size (struct SDL_RWops * context);
static Sint64 ofs_size (struct SDL_RWops * context);
static Sint64 SDLCALL ifs_seek(struct SDL_RWops *context, Sint64 offset, int whence);
static Sint64 SDLCALL ofs_seek(struct SDL_RWops *context, Sint64 offset, int whence);
static size_t SDLCALL ifs_read(struct SDL_RWops *context, void *ptr, size_t size, size_t maxnum);
static size_t SDLCALL ofs_read(struct SDL_RWops *context, void *ptr, size_t size, size_t maxnum);
static size_t SDLCALL ifs_write(struct SDL_RWops *context, const void *ptr, size_t size, size_t num);
static size_t SDLCALL ofs_write(struct SDL_RWops *context, const void *ptr, size_t size, size_t num);
static int SDLCALL ifs_close(struct SDL_RWops *context);
static int SDLCALL ofs_close(struct SDL_RWops *context);

rwops_ptr read_RWops(const std::string &path) {
	rwops_ptr rw(SDL_AllocRW(), &SDL_FreeRW);

	rw->size = &ifs_size;
	rw->seek = &ifs_seek;
	rw->read = &ifs_read;
	rw->write = &ifs_write;
	rw->close = &ifs_close;

	rw->type = read_type;

	scoped_istream ifs = istream_file(path);
	if(!ifs) {
		ERR_FS << "read_RWops: istream_file returned NULL on " << path << '\n';
		rw.reset();
		return rw;
	}

	rw->hidden.unknown.data1 = ifs.release();

	return rw;
}
rwops_ptr write_RWops(const std::string &path) {
	rwops_ptr rw(SDL_AllocRW(), &SDL_FreeRW);

	rw->size = &ofs_size;
	rw->seek = &ofs_seek;
	rw->read = &ofs_read;
	rw->write = &ofs_write;
	rw->close = &ofs_close;

	rw->type = write_type;

	scoped_ostream ofs = ostream_file(path);
	if(!ofs) {
		ERR_FS << "write_RWops: ostream_file returned NULL on " << path << '\n';
		rw.reset();
		return rw;
	}

	rw->hidden.unknown.data1 = ofs.release();

	return rw;
}

static Sint64 ifs_size (struct SDL_RWops * context) {
	std::istream *ifs = static_cast<std::istream*>(context->hidden.unknown.data1);
	std::streampos orig = ifs->tellg();

	ifs->seekg(0, std::ios::end);

	std::streampos len = ifs->tellg();

	ifs->seekg(orig);

	return len;
}
static Sint64 ofs_size (struct SDL_RWops * context) {
	std::ostream *ofs = static_cast<std::ostream*>(context->hidden.unknown.data1);
	std::streampos orig = ofs->tellp();

	ofs->seekp(0, std::ios::end);

	std::streampos len = ofs->tellp();

	ofs->seekp(orig);

	return len;
}

typedef std::pair<Sint64, std::ios_base::seekdir> offset_dir;
static offset_dir translate_seekdir(Sint64 offset, int whence){
	switch(whence){
	case RW_SEEK_SET:
		return std::make_pair(std::max(0l, offset), std::ios_base::beg);
	case RW_SEEK_CUR:
		return std::make_pair(offset, std::ios_base::cur);
	case RW_SEEK_END:
		return std::make_pair(std::min(0l, offset), std::ios_base::end);
	default:
		assert(false);
		throw "assertion ignored";
	}
}
static Sint64 SDLCALL ifs_seek(struct SDL_RWops *context, Sint64 offset, int whence) {
	std::ios_base::seekdir seekdir;
	std::tie(offset, seekdir) = translate_seekdir(offset, whence);

	std::istream *ifs = static_cast<std::istream*>(context->hidden.unknown.data1);
	const std::ios_base::iostate saved_state = ifs->rdstate();

	ifs->seekg(offset, seekdir);

	if(saved_state != ifs->rdstate() && offset < 0) {
		ifs->clear(saved_state);
		ifs->seekg(0, std::ios_base::beg);
	}

	std::streamsize pos = ifs->tellg();
	return static_cast<int>(pos);
}
static Sint64 SDLCALL ofs_seek(struct SDL_RWops *context, Sint64 offset, int whence) {
	std::ios_base::seekdir seekdir;
	std::tie(offset, seekdir) = translate_seekdir(offset, whence);

	std::ostream *ofs = static_cast<std::ostream*>(context->hidden.unknown.data1);
	const std::ios_base::iostate saved_state = ofs->rdstate();

	ofs->seekp(offset, seekdir);

	if(saved_state != ofs->rdstate() && offset < 0) {
		ofs->clear(saved_state);
		ofs->seekp(0, std::ios_base::beg);
	}

	std::streamsize pos = ofs->tellp();
	return static_cast<int>(pos);
}

static size_t SDLCALL ifs_read(struct SDL_RWops *context, void *ptr, size_t size, size_t maxnum) {
	std::istream *ifs = static_cast<std::istream*>(context->hidden.unknown.data1);

	// This seems overly simplistic, but it's the same as mem_read's implementation
	ifs->read(static_cast<char*>(ptr), maxnum * size);
	std::streamsize num = ifs->good() ? maxnum : ifs->gcount() / size;

	// EOF sticks unless we clear it. Bad is an actual I/O error
	if(!ifs->bad())
		ifs->clear();

	return static_cast<int>(num);
}
static size_t SDLCALL ofs_read(struct SDL_RWops * /*context*/, void * /*ptr*/, size_t /*size*/, size_t /*maxnum*/) {
	SDL_SetError("Reading not implemented");
	return 0;
}

static size_t SDLCALL ifs_write(struct SDL_RWops * /*context*/, const void * /*ptr*/, size_t /*size*/, size_t /*num*/) {
	SDL_SetError("Writing not implemented");
	return 0;
}
static size_t SDLCALL ofs_write(struct SDL_RWops *context, const void *ptr, size_t size, size_t num) {
	std::ostream *ofs = static_cast<std::ostream*>(context->hidden.unknown.data1);

	const std::streampos before = ofs->tellp();
	ofs->write(static_cast<const char*>(ptr), num * size);
	const std::streampos after = ofs->tellp();

	const int written = (after - before) / size;

	// Fail sticks unless we clear it. Bad is an actual I/O error
	if(!ofs->bad())
		ofs->clear();

	return static_cast<int>(written);
}

static int SDLCALL ifs_close(struct SDL_RWops *context) {
	if (context) {
		std::istream *ifs = static_cast<std::istream*>(context->hidden.unknown.data1);
		delete ifs;
		SDL_FreeRW(context);
	}
	return 0;
}
static int SDLCALL ofs_close(struct SDL_RWops *context) {
	if (context) {
		std::ostream *ofs = static_cast<std::ostream*>(context->hidden.unknown.data1);
		delete ofs;
		SDL_FreeRW(context);
	}
	return 0;
}

}
