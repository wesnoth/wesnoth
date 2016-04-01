
#include "global.hpp"

#include <fstream>

#include "filesystem.hpp"

#include "config.hpp"
#include "game_config.hpp"
#include "log.hpp"
#include "serialization/string_utils.hpp"
#include "serialization/unicode.hpp"
#include "util.hpp"

#include <SDL_rwops.h>

static lg::log_domain log_filesystem("filesystem");
#define LOG_FS LOG_STREAM(info, log_filesystem)
#define ERR_FS LOG_STREAM(err, log_filesystem)

namespace filesystem {


scoped_istream& scoped_istream::operator=(std::istream *s)
{
	delete stream;
	stream = s;
	return *this;
}

scoped_istream::~scoped_istream()
{
	delete stream;
}

scoped_ostream& scoped_ostream::operator=(std::ostream *s)
{
	delete stream;
	stream = s;
	return *this;
}

scoped_ostream::~scoped_ostream()
{
	delete stream;
}

#ifdef __native_client__
// For performance reasons, on NaCl we only keep preferences and saves in persistent storage.
std::string get_prefs_file()
{
	return "/wesnoth-userdata/preferences";
}

std::string get_save_index_file()
{
	return "/wesnoth-userdata/save_index";
}

std::string get_saves_dir()
{
	const std::string dir_path = "/wesnoth-userdata/saves";
	return get_dir(dir_path);
}

#else

std::string get_prefs_file()
{
	return get_user_config_dir() + "/preferences";
}

std::string get_default_prefs_file()
{
#ifdef HAS_RELATIVE_DEFPREF
	return game_config::path + "/" + game_config::default_preferences_path;
#else
	return game_config::default_preferences_path;
#endif
}

std::string get_save_index_file()
{
	return get_user_data_dir() + "/save_index";
}

std::string get_saves_dir()
{
	const std::string dir_path = get_user_data_dir() + "/saves";
	return get_dir(dir_path);
}
#endif

std::string get_addons_dir()
{
	const std::string dir_path = get_user_data_dir() + "/data/add-ons";
	return get_dir(dir_path);
}

std::string get_intl_dir()
{
#ifdef _WIN32
	return get_cwd() + "/translations";
#else

#ifdef USE_INTERNAL_DATA
	return get_cwd() + "/" LOCALEDIR;
#endif

#if HAS_RELATIVE_LOCALEDIR
	std::string res = game_config::path + "/" LOCALEDIR;
#else
	std::string res = LOCALEDIR;
#endif

	return res;
#endif
}

std::string get_screenshot_dir()
{
	const std::string dir_path = get_user_data_dir() + "/screenshots";
	return get_dir(dir_path);
}

bool looks_like_pbl(const std::string& file)
{
	return utils::wildcard_string_match(utf8::lowercase(file), "*.pbl");
}

file_tree_checksum::file_tree_checksum()
	: nfiles(0), sum_size(0), modified(0)
{}

file_tree_checksum::file_tree_checksum(const config& cfg) :
	nfiles	(cfg["nfiles"].to_size_t()),
	sum_size(cfg["size"].to_size_t()),
	modified(cfg["modified"].to_time_t())
{
}

void file_tree_checksum::write(config& cfg) const
{
	cfg["nfiles"] = nfiles;
	cfg["size"] = sum_size;
	cfg["modified"] = modified;
}

bool file_tree_checksum::operator==(const file_tree_checksum &rhs) const
{
	return nfiles == rhs.nfiles && sum_size == rhs.sum_size &&
		modified == rhs.modified;
}

bool ends_with(const std::string& str, const std::string& suffix)
{
	return str.size() >= suffix.size() && std::equal(suffix.begin(),suffix.end(),str.end()-suffix.size());
}

std::string read_map(const std::string& name)
{
	std::string res;
	std::string map_location = get_wml_location("maps/" + name);
	if(!map_location.empty()) {
		res = read_file(map_location);
	}

	if (res.empty()) {
		res = read_file(get_user_data_dir() + "/editor/maps/" + name);
	}

	return res;
}

static void get_file_tree_checksum_internal(const std::string& path, file_tree_checksum& res)
{

	std::vector<std::string> dirs;
	get_files_in_dir(path,nullptr,&dirs, ENTIRE_FILE_PATH, SKIP_MEDIA_DIR, DONT_REORDER, &res);

	for(std::vector<std::string>::const_iterator j = dirs.begin(); j != dirs.end(); ++j) {
		get_file_tree_checksum_internal(*j,res);
	}
}

const file_tree_checksum& data_tree_checksum(bool reset)
{
	static file_tree_checksum checksum;
	if (reset)
		checksum.reset();
	if(checksum.nfiles == 0) {
		get_file_tree_checksum_internal("data/",checksum);
		get_file_tree_checksum_internal(get_user_data_dir() + "/data/",checksum);
		LOG_FS << "calculated data tree checksum: "
			   << checksum.nfiles << " files; "
			   << checksum.sum_size << " bytes" << std::endl;
	}

	return checksum;
}

static Sint64 ifs_size (struct SDL_RWops * context);
static Sint64 SDLCALL ifs_seek(struct SDL_RWops *context, Sint64 offset, int whence);
static size_t SDLCALL ifs_read(struct SDL_RWops *context, void *ptr, size_t size, size_t maxnum);
static size_t SDLCALL ifs_write(struct SDL_RWops *context, const void *ptr, size_t size, size_t num);
static int SDLCALL ifs_close(struct SDL_RWops *context);

SDL_RWops* load_RWops(const std::string &path) {
	SDL_RWops *rw = SDL_AllocRW();

	rw->size = &ifs_size;
	rw->seek = &ifs_seek;
	rw->read = &ifs_read;
	rw->write = &ifs_write;
	rw->close = &ifs_close;

	rw->type = 7; // Random number that is larger than 5

	std::istream *ifs = istream_file(path);
	if(!ifs) {
		ERR_FS << "load_RWops: istream_file returned nullptr on " << path << '\n';
		return nullptr;
	}

	rw->hidden.unknown.data1 = ifs;

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

static Sint64 SDLCALL ifs_seek(struct SDL_RWops *context, Sint64 offset, int whence) {

	std::ios_base::seekdir seekdir;
	switch(whence){
	case RW_SEEK_SET:
		seekdir = std::ios_base::beg;
		if(offset < 0)
			offset = 0;
		break;
	case RW_SEEK_CUR:
		seekdir = std::ios_base::cur;
		break;
	case RW_SEEK_END:
		seekdir = std::ios_base::end;
		if(offset > 0)
			offset = 0;
		break;
	default:
		assert(false);
		throw "assertion ignored";
	}
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

static size_t SDLCALL ifs_write(struct SDL_RWops * /*context*/, const void * /*ptr*/, size_t /*size*/, size_t /*num*/) {
	SDL_SetError("Writing not implemented");
	return 0;
}
static int SDLCALL ifs_close(struct SDL_RWops *context) {
	if (context) {
		std::istream *ifs = static_cast<std::istream*>(context->hidden.unknown.data1);
		delete ifs;
		SDL_FreeRW(context);
	}
	return 0;
}


}
