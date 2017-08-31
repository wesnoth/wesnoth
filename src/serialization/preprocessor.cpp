/*
   Copyright (C) 2003 by David White <dave@whitevine.net>
   Copyright (C) 2005 - 2017 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
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
 * WML preprocessor.
 */

#include "buffered_istream.hpp"
#include "config.hpp"
#include "filesystem.hpp"
#include "game_config.hpp"
#include "log.hpp"
#include "serialization/binary_or_text.hpp"
#include "serialization/parser.hpp"
#include "serialization/string_utils.hpp"
#include "version.hpp"
#include "wesconfig.h"

#include <stdexcept>
#include <deque>

static lg::log_domain log_preprocessor("preprocessor");
#define ERR_PREPROC LOG_STREAM(err, log_preprocessor)
#define WRN_PREPROC LOG_STREAM(warn, log_preprocessor)
#define LOG_PREPROC LOG_STREAM(info, log_preprocessor)
#define DBG_PREPROC LOG_STREAM(debug, log_preprocessor)

static std::string current_file_str = "CURRENT_FILE";
static std::string current_dir_str = "CURRENT_DIRECTORY";

// map associating each filename encountered to a number
static std::map<std::string, int> file_number_map;

static bool encode_filename = true;

static std::string preprocessor_error_detail_prefix = "\n    ";

static const char OUTPUT_SEPARATOR = '\xFE';

// get filename associated to this code
static std::string get_filename(const std::string& file_code)
{
	if(!encode_filename) {
		return file_code;
	}

	std::stringstream s;
	s << file_code;
	int n = 0;
	s >> std::hex >> n;

	for(const auto& p : file_number_map) {
		if(p.second == n) {
			return p.first;
		}
	}

	return "<unknown>";
}

// Get code associated to this filename
static std::string get_file_code(const std::string& filename)
{
	if(!encode_filename) {
		return filename;
	}

	// Current number of encountered filenames
	static int current_file_number = 0;

	int& fnum = file_number_map[utils::escape(filename, " \\")];
	if(fnum == 0) {
		fnum = ++current_file_number;
	}

	std::ostringstream shex;
	shex << std::hex << fnum;

	return shex.str();
}

// decode the filenames placed in a location
static std::string get_location(const std::string& loc)
{
	std::string res;
	std::vector<std::string> pos = utils::quoted_split(loc, ' ');

	if(pos.empty()) {
		return res;
	}

	std::vector<std::string>::const_iterator i = pos.begin(), end = pos.end();
	while(true) {
		res += get_filename(*(i++));

		if(i == end) {
			break;
		}

		res += ' ';
		res += *(i++);

		if(i == end) {
			break;
		}

		res += ' ';
	}

	return res;
}


// ==================================================================================
// PREPROC_DEFINE IMPLEMENTATION
// ==================================================================================

bool preproc_define::operator==(const preproc_define& v) const
{
	return value == v.value && arguments == v.arguments;
}

bool preproc_define::operator<(const preproc_define& v) const
{
	if(location < v.location) {
		return true;
	}

	if(v.location < location) {
		return false;
	}

	return linenum < v.linenum;
}

void preproc_define::write_argument(config_writer& writer, const std::string& arg) const
{
	const std::string key = "argument";

	writer.open_child(key);

	writer.write_key_val("name", arg);
	writer.close_child(key);
}

void preproc_define::write(config_writer& writer, const std::string& name) const
{
	const std::string key = "preproc_define";
	writer.open_child(key);

	writer.write_key_val("name", name);
	writer.write_key_val("value", value);
	writer.write_key_val("textdomain", textdomain);
	writer.write_key_val("linenum", std::to_string(linenum));
	writer.write_key_val("location", get_location(location));

	for(const std::string& arg : arguments) {
		write_argument(writer, arg);
	}

	writer.close_child(key);
}

void preproc_define::read_argument(const config& cfg)
{
	arguments.push_back(cfg["name"]);
}

void preproc_define::read(const config& cfg)
{
	value = cfg["value"].str();
	textdomain = cfg["textdomain"].str();
	linenum = cfg["linenum"];
	location = cfg["location"].str();

	for(const config& arg : cfg.child_range("argument")) {
		read_argument(arg);
	}
}

preproc_map::value_type preproc_define::read_pair(const config& cfg)
{
	preproc_define second;
	second.read(cfg);

	return preproc_map::value_type(cfg["name"], second);
}

std::ostream& operator<<(std::ostream& stream, const preproc_define& def)
{
	return stream << "value: " << def.value << " arguments: " << def.location;
}

std::ostream& operator<<(std::ostream& stream, const preproc_map::value_type& def)
{
	return stream << def.second;
}

// ==================================================================================
// PREPROCESSOR BASE
// ==================================================================================

class preprocessor_streambuf;

/**
 * Base class for preprocessing an input.
 */
class preprocessor
{
	friend class preprocessor_streambuf;

protected:
	/**
	 * Sets up a new preprocessor for stream buffer \a t.
	 * Saves the current preprocessing context of #parent_. It will be automatically restored on destruction.
	 *
	 * It relies on preprocessor_streambuf so it's implemented after that class is declared.
	 */
	preprocessor(preprocessor_streambuf& t);

	preprocessor_streambuf& parent_;

public:
	virtual ~preprocessor()
	{
	}

	/**
	 * Preprocesses and sends some text to the #parent_ buffer.
	 * @return false when the input has no data left.
	 */
	virtual bool get_chunk() = 0;

	enum MODE { NO_PARSING, PARSES_FILE, PARSES_MACRO };

	/** Returns the appropriate parsing mode for this preprocessor. */
	virtual MODE parse_mode()
	{
		return NO_PARSING;
	}

private:
	std::string old_textdomain_;
	std::string old_location_;

	int old_linenum_;
};


// ==================================================================================
// PREPROCESSOR BUFFER
// ==================================================================================

/**
 * Target for sending preprocessed output.
 * Objects of this class can be plugged into an STL stream.
 */
class preprocessor_streambuf : public std::streambuf
{
public:
	preprocessor_streambuf(preproc_map* def)
		: std::streambuf()
		, out_buffer_("")
		, buffer_()
		, preprocessor_queue_()
		, defines_(def)
		, default_defines_()
		, textdomain_(PACKAGE)
		, location_("")
		, linenum_(0)
		, quoted_(false)
	{
	}

	/** Decodes the filenames placed in a location. */
	std::string get_current_file();

	void error(const std::string&, int);
	void warning(const std::string&, int);

	template<typename T, typename... A>
	void add_preprocessor(A&&... args)
	{
		preprocessor_queue_.emplace_back(new T(*this, std::forward<A>(args)...));
	}

	void drop_preprocessor()
	{
		preprocessor_queue_.pop_back();
	}

	int depth() const
	{
		return preprocessor_queue_.size();
	}

	preprocessor* current() const
	{
		return preprocessor_queue_.empty() ? nullptr : preprocessor_queue_.back().get();
	}

private:
	preprocessor_streambuf(const preprocessor_streambuf& t)
		: std::streambuf()
		, out_buffer_("")
		, buffer_()
		, preprocessor_queue_()
		, defines_(t.defines_)
		, default_defines_()
		, textdomain_(PACKAGE)
		, location_("")
		, linenum_(0)
		, quoted_(t.quoted_)
	{
	}

	virtual int underflow();

	void restore_old_preprocessor();

	/** Buffer read by the STL stream. */
	std::string out_buffer_;

	/** Buffer filled by the #current_ preprocessor. */
	std::stringstream buffer_;

	/** Input preprocessor queue. */
	std::deque<std::unique_ptr<preprocessor>> preprocessor_queue_;

	preproc_map* defines_;
	preproc_map default_defines_;

	std::string textdomain_;
	std::string location_;

	int linenum_;

	/**
	 * Set to true if one preprocessor for this target started to read a string.
	 * Deeper-nested preprocessors are then forbidden to.
	 */
	bool quoted_;

	friend class preprocessor;
	friend class preprocessor_file;
	friend class preprocessor_data;
	friend struct preprocessor_scope_helper;
};

/** Preprocessor constructor. */
preprocessor::preprocessor(preprocessor_streambuf& t)
	: parent_(t)
	, old_textdomain_(t.textdomain_)
	, old_location_(t.location_)
	, old_linenum_(t.linenum_)
{
}

/**
 * Called by an STL stream whenever it has reached the end of #out_buffer_.
 * Fills #buffer_ by calling the #current_ preprocessor, then copies its
 * content into #out_buffer_.
 * @return the first character of #out_buffer_ if any, EOF otherwise.
 */
int preprocessor_streambuf::underflow()
{
	unsigned sz = 0;
	if(char* gp = gptr()) {
		if(gp < egptr()) {
			// Sanity check: the internal buffer has not been totally consumed,
			// should we force the caller to use what remains first?
			return *gp;
		}

		// The buffer has been completely read; fill it again.
		// Keep part of the previous buffer, to ensure putback capabilities.
		sz = out_buffer_.size();
		buffer_.str(std::string());

		if(sz > 3) {
			buffer_ << out_buffer_.substr(sz - 3);
			sz = 3;
		} else {
			buffer_ << out_buffer_;
		}
	} else {
		// The internal get-data pointer is null
	}

	const int desired_fill_amount = 2000;

	while(current() && buffer_.rdbuf()->in_avail() < desired_fill_amount) {
		// Process files and data chunks until the desired buffer size is reached
		if(!current()->get_chunk()) {
			// Drop the current preprocessor item from the queue.
			restore_old_preprocessor();
		}
	}

	// Update the internal state and data pointers
	out_buffer_ = buffer_.str();
	if(out_buffer_.empty()) {
		return EOF;
	}

	char* begin = &*out_buffer_.begin();
	unsigned bs = out_buffer_.size();

	setg(begin, begin + sz, begin + bs);

	if(sz >= bs) {
		return EOF;
	}

	return static_cast<unsigned char>(*(begin + sz));
}

/**
* Restores the old preprocessing context.
* Appends location and domain directives to the buffer, so that the parser
* notices these changes.
*/
void preprocessor_streambuf::restore_old_preprocessor()
{
	preprocessor* current = this->current();

	if(!current->old_location_.empty()) {
		buffer_ << OUTPUT_SEPARATOR << "line " << current->old_linenum_ << ' ' << current->old_location_ << '\n';
	}

	if(!current->old_textdomain_.empty() && textdomain_ != current->old_textdomain_) {
		buffer_ << OUTPUT_SEPARATOR << "textdomain " << current->old_textdomain_ << '\n';
	}

	location_ = current->old_location_;
	linenum_ = current->old_linenum_;
	textdomain_ = current->old_textdomain_;

	// Drop the preprocessor from the queue.
	drop_preprocessor();
}

std::string preprocessor_streambuf::get_current_file()
{
	unsigned nested_level = 0;

	preprocessor* pre = current();

	// Iterate backwards over queue to get the last non-macro preprocessor.
	for(auto p = preprocessor_queue_.rbegin(); p != preprocessor_queue_.rend(); ++p) {
		pre = p->get();

		if(!pre || pre->parse_mode() == preprocessor::PARSES_FILE) {
			break;
		}

		if(pre->parse_mode() == preprocessor::PARSES_MACRO) {
			++nested_level;
		}
	}

	std::string res;
	std::vector<std::string> pos = utils::quoted_split(location_, ' ');

	if(pos.size() <= 2 * nested_level) {
		return res;
	}

	return get_filename(pos[2 * nested_level]);
}

std::string lineno_string(const std::string& lineno)
{
	std::vector<std::string> pos = utils::quoted_split(lineno, ' ');
	std::vector<std::string>::const_iterator i = pos.begin(), end = pos.end();
	std::string included_from = preprocessor_error_detail_prefix + "included from ";
	std::string res;

	while(i != end) {
		const std::string& line = *(i++);
		if(!res.empty())
			res += included_from;
		if(i != end)
			res += get_filename(*(i++));
		else
			res += "<unknown>";
		res += ':' + line;
	}

	if(res.empty()) {
		res = "???";
	}

	return res;
}

void preprocessor_streambuf::error(const std::string& error_type, int l)
{
	std::string position, error;
	std::ostringstream pos;

	pos << l << ' ' << location_;
	position = lineno_string(pos.str());

	error = error_type + '\n';
	error += "at " + position;

	ERR_PREPROC << error << '\n';

	throw preproc_config::error(error);
}

void preprocessor_streambuf::warning(const std::string& warning_type, int l)
{
	std::string position, warning;
	std::ostringstream pos;

	pos << l << ' ' << location_;
	position = lineno_string(pos.str());

	warning = warning_type + '\n';
	warning += "at " + position;

	WRN_PREPROC << warning << '\n';
}


// ==================================================================================
// PREPROCESSOR FILE
// ==================================================================================

/**
 * Specialized preprocessor for handling a file or a set of files.
 * A preprocessor_file object is created when a preprocessor encounters an
 * inclusion directive that resolves to a file or directory, e.g. '{themes/}'.
 */
class preprocessor_file : public preprocessor
{
public:
	/** Constructor. It relies on preprocessor_data so it's implemented after that class is declared. */
	preprocessor_file(preprocessor_streambuf& t, const std::string& name, size_t symbol_index = -1);

	/**
	 * Inserts and processes the next file in the list of included files.
	 * @return	false if there is no next file.
	 */
	virtual bool get_chunk()
	{
		while(pos_ != end_) {
			const std::string& name = *(pos_++);
			unsigned sz = name.size();

			// Use reverse iterator to optimize testing
			if(sz < 5 || !std::equal(name.rbegin(), name.rbegin() + 4, "gfc.")) {
				continue;
			}

			parent_.add_preprocessor<preprocessor_file>(name);
			return true;
		}

		return false;
	}

private:
	std::vector<std::string> files_;
	std::vector<std::string>::const_iterator pos_, end_;
};


// ==================================================================================
// PREPROCESSOR DATA
// ==================================================================================

/**
 * Specialized preprocessor for handling any kind of input stream.
 * This is the core of the preprocessor.
 */
class preprocessor_data : public preprocessor
{
	/** Description of a preprocessing chunk. */
	struct token_desc
	{
		enum TOKEN_TYPE {
			START,        // Toplevel
			PROCESS_IF,   // Processing the "if" branch of a ifdef/ifndef (the "else" branch will be skipped)
			PROCESS_ELSE, // Processing the "else" branch of a ifdef/ifndef
			SKIP_IF,      // Skipping the "if" branch of a ifdef/ifndef (the "else" branch, if any, will be processed)
			SKIP_ELSE,    // Skipping the "else" branch of a ifdef/ifndef
			STRING,       // Processing a string
			VERBATIM,     // Processing a verbatim string
			MACRO_SPACE,  // Processing between chunks of a macro call (skip spaces)
			MACRO_CHUNK,  // Processing inside a chunk of a macro call (stop on space or '(')
			MACRO_PARENS  // Processing a parenthesized macro argument
		};

		token_desc(TOKEN_TYPE type, const int stack_pos, const int linenum)
			: type(type)
			, stack_pos(stack_pos)
			, linenum(linenum)
		{
		}

		TOKEN_TYPE type;

		/** Starting position in #strings_ of the delayed text for this chunk. */
		int stack_pos;
		int linenum;
	};

	/**
	 * Manages the lifetime of the @c std::istream pointer we own.
	 *
	 * Since @ref in_ uses the stream as well this object must be created
	 * before @ref in_ and destroyed after @ref in_ is destroyed.
	 */
	filesystem::scoped_istream in_scope_;

	/** Input stream. */
	buffered_istream in_;

	std::string directory_;

	/** Buffer for delayed input processing. */
	std::vector<std::string> strings_;

	/** Mapping of macro arguments to their content. */
	std::unique_ptr<std::map<std::string, std::string>> local_defines_;

	/** Stack of nested preprocessing chunks. */
	std::vector<token_desc> tokens_;

	/**
	 * Set to true whenever input tokens cannot be directly sent to the target
	 * buffer. For instance, this happens with macro arguments. In that case,
	 * the output is redirected toward #strings_ until it can be processed.
	 */
	int slowpath_;

	/**
	 * Non-zero when the preprocessor has to skip some input text.
	 * Increased whenever entering a conditional branch that is not useful,
	 * e.g. a ifdef that evaluates to false.
	 */
	int skipping_;
	int linenum_;

	/** True iff we are currently parsing a macros content, otherwise false. */
	bool is_define_;

	std::string read_word();
	std::string read_line();
	std::string read_rest_of_line();

	void skip_spaces();
	void skip_eol();
	void push_token(token_desc::TOKEN_TYPE);
	void pop_token();
	void put(char);
	void put(const std::string& /*, int change_line = 0 */);
	void conditional_skip(bool skip);

public:
	preprocessor_data(preprocessor_streambuf&,
			std::istream*,
			const std::string& history,
			const std::string& name,
			int line,
			const std::string& dir,
			const std::string& domain,
			std::map<std::string, std::string>* defines,
			bool is_define = false);

	virtual bool get_chunk();

	virtual preprocessor::MODE parse_mode()
	{
		return is_define_ ? PARSES_MACRO : PARSES_FILE;
	}

	friend bool operator==(preprocessor_data::token_desc::TOKEN_TYPE, char);
	friend bool operator==(char, preprocessor_data::token_desc::TOKEN_TYPE);
	friend bool operator!=(preprocessor_data::token_desc::TOKEN_TYPE, char);
	friend bool operator!=(char, preprocessor_data::token_desc::TOKEN_TYPE);
};

bool operator==(preprocessor_data::token_desc::TOKEN_TYPE, char)
{
	throw std::logic_error("don't compare tokens with characters");
}

bool operator==(char lhs, preprocessor_data::token_desc::TOKEN_TYPE rhs)
{
	return rhs == lhs;
}

bool operator!=(preprocessor_data::token_desc::TOKEN_TYPE rhs, char lhs)
{
	return !(lhs == rhs);
}

bool operator!=(char lhs, preprocessor_data::token_desc::TOKEN_TYPE rhs)
{
	return rhs != lhs;
}

/** preprocessor_file constructor. */
preprocessor_file::preprocessor_file(preprocessor_streambuf& t, const std::string& name, size_t symbol_index)
	: preprocessor(t)
	, files_()
	, pos_()
	, end_()
{
	if(filesystem::is_directory(name)) {
		filesystem::get_files_in_dir(name, &files_, nullptr, filesystem::ENTIRE_FILE_PATH, filesystem::SKIP_MEDIA_DIR,
				filesystem::DO_REORDER);

		for(std::string fname : files_) {
			size_t cpos = fname.rfind(" ");

			if(cpos != std::string::npos && cpos >= symbol_index) {
				std::stringstream ss;
				ss << "Found filename containing whitespace: '" << filesystem::base_name(fname)
				<< "' in included directory '" << name << "'.\nThe included symbol probably looks similar to '"
				<< filesystem::directory_name(fname.substr(symbol_index)) << "'";

				// TODO: find a real linenumber
				parent_.error(ss.str(), -1);
			}
		}
	} else {
		filesystem::scoped_istream file_stream = filesystem::istream_file(name);

		if(!file_stream->good()) {
			ERR_PREPROC << "Could not open file " << name << std::endl;
		} else {
			t.add_preprocessor<preprocessor_data>(file_stream.release(), "",
				filesystem::get_short_wml_path(name), 1,
				filesystem::directory_name(name), t.textdomain_, nullptr
			);
		}
	}

	pos_ = files_.begin();
	end_ = files_.end();
}

preprocessor_data::preprocessor_data(preprocessor_streambuf& t,
		std::istream* i,
		const std::string& history,
		const std::string& name,
		int linenum,
		const std::string& directory,
		const std::string& domain,
		std::map<std::string, std::string>* defines,
		bool is_define)
	: preprocessor(t)
	, in_scope_(i)
	, in_(*i)
	, directory_(directory)
	, strings_()
	, local_defines_(defines)
	, tokens_()
	, slowpath_(0)
	, skipping_(0)
	, linenum_(linenum)
	, is_define_(is_define)
{
	std::ostringstream s;
	s << history;

	if(!name.empty()) {
		if(!history.empty()) {
			s << ' ';
		}

		s << get_file_code(name);
	}

	if(!t.location_.empty()) {
		s << ' ' << t.linenum_ << ' ' << t.location_;
	}

	t.location_ = s.str();
	t.linenum_ = linenum;

	t.buffer_ << OUTPUT_SEPARATOR << "line " << linenum << ' ' << t.location_ << '\n';

	if(t.textdomain_ != domain) {
		t.buffer_ << OUTPUT_SEPARATOR << "textdomain " << domain << '\n';
		t.textdomain_ = domain;
	}

	push_token(token_desc::START);
}

void preprocessor_data::push_token(token_desc::TOKEN_TYPE t)
{
	tokens_.emplace_back(t, strings_.size(), linenum_);

	if(t == token_desc::MACRO_SPACE) {
		// Macro expansions do not have any associated storage at start.
		return;
	} else if(t == token_desc::STRING || t == token_desc::VERBATIM) {
		/* Quoted strings are always inlined in the parent token. So
		 * they need neither storage nor metadata, unless the parent
		 * token is a macro expansion.
		 */
		token_desc::TOKEN_TYPE& outer_type = tokens_[tokens_.size() - 2].type;
		if(outer_type != token_desc::MACRO_SPACE) {
			return;
		}

		outer_type = token_desc::MACRO_CHUNK;
		tokens_.back().stack_pos = strings_.size() + 1;
	}

	std::ostringstream s;
	if(!skipping_ && slowpath_) {
		s << OUTPUT_SEPARATOR << "line " << linenum_ << ' ' << parent_.location_ << "\n"
		  << OUTPUT_SEPARATOR << "textdomain " << parent_.textdomain_ << '\n';
	}

	strings_.push_back(s.str());
}

void preprocessor_data::pop_token()
{
	token_desc::TOKEN_TYPE inner_type = tokens_.back().type;
	unsigned stack_pos = tokens_.back().stack_pos;

	tokens_.pop_back();

	token_desc::TOKEN_TYPE& outer_type = tokens_.back().type;

	if(inner_type == token_desc::MACRO_PARENS) {
		// Parenthesized macro arguments are left on the stack.
		assert(outer_type == token_desc::MACRO_SPACE);
		return;
	}

	if(inner_type == token_desc::STRING || inner_type == token_desc::VERBATIM) {
		// Quoted strings are always inlined.
		assert(stack_pos == strings_.size());
		return;
	}

	if(outer_type == token_desc::MACRO_SPACE) {
		/* A macro expansion does not have any associated storage.
		 * Instead, storage of the inner token is not discarded
		 * but kept as a new macro argument. But if the inner token
		 * was a macro expansion, it is about to be appended, so
		 * prepare for it.
		 */
		if(inner_type == token_desc::MACRO_SPACE || inner_type == token_desc::MACRO_CHUNK) {
			strings_.erase(strings_.begin() + stack_pos, strings_.end());
			strings_.emplace_back();
		}

		assert(stack_pos + 1 == strings_.size());
		outer_type = token_desc::MACRO_CHUNK;

		return;
	}

	strings_.erase(strings_.begin() + stack_pos, strings_.end());
}

void preprocessor_data::skip_spaces()
{
	for(;;) {
		int c = in_.peek();

		if(in_.eof() || (c != ' ' && c != '\t')) {
			return;
		}

		in_.get();
	}
}

void preprocessor_data::skip_eol()
{
	for(;;) {
		int c = in_.get();

		if(c == '\n') {
			++linenum_;
			return;
		}

		if(in_.eof()) {
			return;
		}
	}
}

std::string preprocessor_data::read_word()
{
	std::string res;

	for(;;) {
		int c = in_.peek();

		if(c == preprocessor_streambuf::traits_type::eof() || utils::portable_isspace(c)) {
			// DBG_PREPROC << "(" << res << ")\n";
			return res;
		}

		in_.get();
		res += static_cast<char>(c);
	}
}

std::string preprocessor_data::read_line()
{
	std::string res;

	for(;;) {
		int c = in_.get();

		if(c == '\n') {
			++linenum_;
			return res;
		}

		if(in_.eof()) {
			return res;
		}

		if(c != '\r') {
			res += static_cast<char>(c);
		}
	}
}

std::string preprocessor_data::read_rest_of_line()
{
	std::string res;

	while(in_.peek() != '\n' && !in_.eof()) {
		int c = in_.get();

		if(c != '\r') {
			res += static_cast<char>(c);
		}
	}

	return res;
}

void preprocessor_data::put(char c)
{
	if(skipping_) {
		return;
	}

	if(slowpath_) {
		strings_.back() += c;
		return;
	}

	int cond_linenum = c == '\n' ? linenum_ - 1 : linenum_;

	if(unsigned diff = cond_linenum - parent_.linenum_) {
		parent_.linenum_ = cond_linenum;

		if(diff <= parent_.location_.size() + 11) {
			parent_.buffer_ << std::string(diff, '\n');
		} else {
			parent_.buffer_ << OUTPUT_SEPARATOR << "line " << parent_.linenum_ << ' ' << parent_.location_ << '\n';
		}
	}

	if(c == '\n') {
		++parent_.linenum_;
	}

	parent_.buffer_ << c;
}

void preprocessor_data::put(const std::string& s /*, int line_change*/)
{
	if(skipping_) {
		return;
	}

	if(slowpath_) {
		strings_.back() += s;
		return;
	}

	parent_.buffer_ << s;
	// parent_.linenum_ += line_change;
}

void preprocessor_data::conditional_skip(bool skip)
{
	if(skip) {
		++skipping_;
	}

	push_token(skip ? token_desc::SKIP_ELSE : token_desc::PROCESS_IF);
}

bool preprocessor_data::get_chunk()
{
	char c = in_.get();
	token_desc& token = tokens_.back();

	if(in_.eof()) {
		// The end of file was reached.
		// Make sure we don't have any incomplete tokens.
		char const* s;

		switch(token.type) {
		case token_desc::START:
			return false; // everything is fine
		case token_desc::PROCESS_IF:
		case token_desc::SKIP_IF:
		case token_desc::PROCESS_ELSE:
		case token_desc::SKIP_ELSE:
			s = "#ifdef or #ifndef";
			break;
		case token_desc::STRING:
			s = "Quoted string";
			break;
		case token_desc::VERBATIM:
			s = "Verbatim string";
			break;
		case token_desc::MACRO_CHUNK:
		case token_desc::MACRO_SPACE:
			s = "Macro substitution";
			break;
		case token_desc::MACRO_PARENS:
			s = "Macro argument";
			break;
		default:
			s = "???";
		}

		parent_.error(std::string(s) + " not terminated", token.linenum);
	}

	if(c == '\n') {
		++linenum_;
	}

	if(c == OUTPUT_SEPARATOR) {
		std::string buffer(1, c);

		for(;;) {
			char d = in_.get();

			if(in_.eof() || d == '\n') {
				break;
			}

			buffer += d;
		}

		buffer += '\n';
		// line_change = 1-1 = 0
		put(buffer);
	} else if(token.type == token_desc::VERBATIM) {
		put(c);

		if(c == '>' && in_.peek() == '>') {
			put(in_.get());
			pop_token();
		}
	} else if(c == '<' && in_.peek() == '<') {
		in_.get();
		push_token(token_desc::VERBATIM);
		put('<');
		put('<');
	} else if(c == '"') {
		if(token.type == token_desc::STRING) {
			parent_.quoted_ = false;
			put(c);
			pop_token();
		} else if(!parent_.quoted_) {
			parent_.quoted_ = true;
			push_token(token_desc::STRING);
			put(c);
		} else {
			parent_.error("Nested quoted string", linenum_);
		}
	} else if(c == '{') {
		push_token(token_desc::MACRO_SPACE);
		++slowpath_;
	} else if(c == ')' && token.type == token_desc::MACRO_PARENS) {
		pop_token();
	} else if(c == '#' && !parent_.quoted_) {
		std::string command = read_word();
		bool comment = false;

		if(command == "define") {
			skip_spaces();
			int linenum = linenum_;
			std::vector<std::string> items = utils::split(read_line(), ' ');
			std::map<std::string, std::string> optargs;

			if(items.empty()) {
				parent_.error("No macro name found after #define directive", linenum);
			}

			std::string symbol = items.front();
			items.erase(items.begin());
			int found_arg = 0, found_enddef = 0;
			std::string buffer;
			for(;;) {
				if(in_.eof())
					break;
				char d = in_.get();
				if(d == '\n')
					++linenum_;
				buffer += d;
				if(d == '#') {
					if(in_.peek() == 'a') {
						found_arg = 1;
					} else {
						found_enddef = 1;
					}
				} else {
					if(found_arg > 0 && ++found_arg == 4) {
						if(std::equal(buffer.end() - 3, buffer.end(), "arg")) {
							buffer.erase(buffer.end() - 4, buffer.end());

							skip_spaces();
							std::string argname = read_word();
							skip_eol();

							std::string argbuffer;

							int found_endarg = 0;
							for(;;) {
								if(in_.eof()) {
									break;
								}

								char e = in_.get();
								if(e == '\n') {
									++linenum_;
								}

								argbuffer += e;

								if(e == '#') {
									found_endarg = 1;
								} else if(found_endarg > 0 && ++found_endarg == 7) {
									if(std::equal(argbuffer.end() - 6, argbuffer.end(), "endarg")) {
										argbuffer.erase(argbuffer.end() - 7, argbuffer.end());
										optargs[argname] = argbuffer;
										skip_eol();
										break;
									} else {
										parent_.error("Unterminated #arg definition", linenum_);
									}
								}
							}
						}
					}

					if(found_enddef > 0 && ++found_enddef == 7) {
						if(std::equal(buffer.end() - 6, buffer.end(), "enddef")) {
							break;
						} else {
							found_enddef = 0;
							if(std::equal(buffer.end() - 6, buffer.end(), "define")) { // TODO: Maybe add support for
																					   // this? This would fill feature
																					   // request #21343
								parent_.error(
										"Preprocessor error: #define is not allowed inside a #define/#enddef pair",
										linenum);
							}
						}
					}
				}
			}

			if(found_enddef != 7) {
				parent_.error("Unterminated preprocessor definition", linenum_);
			}

			if(!skipping_) {
				preproc_map::const_iterator old_i = parent_.defines_->find(symbol);
				if(old_i != parent_.defines_->end()) {
					std::ostringstream new_pos, old_pos;
					const preproc_define& old_d = old_i->second;

					new_pos << linenum << ' ' << parent_.location_;
					old_pos << old_d.linenum << ' ' << old_d.location;

					WRN_PREPROC << "Redefining macro " << symbol << " without explicit #undef at "
								<< lineno_string(new_pos.str()) << '\n'
								<< "previously defined at " << lineno_string(old_pos.str()) << '\n';
				}

				buffer.erase(buffer.end() - 7, buffer.end());
				(*parent_.defines_)[symbol]
						= preproc_define(buffer, items, optargs, parent_.textdomain_, linenum, parent_.location_);

				LOG_PREPROC << "defining macro " << symbol << " (location " << get_location(parent_.location_) << ")\n";
			}
		} else if(command == "ifdef") {
			skip_spaces();
			const std::string& symbol = read_word();
			bool found = parent_.defines_->count(symbol) != 0;
			DBG_PREPROC << "testing for macro " << symbol << ": " << (found ? "defined" : "not defined") << '\n';
			conditional_skip(!found);
		} else if(command == "ifndef") {
			skip_spaces();
			const std::string& symbol = read_word();
			bool found = parent_.defines_->count(symbol) != 0;
			DBG_PREPROC << "testing for macro " << symbol << ": " << (found ? "defined" : "not defined") << '\n';
			conditional_skip(found);
		} else if(command == "ifhave") {
			skip_spaces();
			const std::string& symbol = read_word();
			bool found = !filesystem::get_wml_location(symbol, directory_).empty();
			DBG_PREPROC << "testing for file or directory " << symbol << ": " << (found ? "found" : "not found")
						<< '\n';
			conditional_skip(!found);
		} else if(command == "ifnhave") {
			skip_spaces();
			const std::string& symbol = read_word();
			bool found = !filesystem::get_wml_location(symbol, directory_).empty();
			DBG_PREPROC << "testing for file or directory " << symbol << ": " << (found ? "found" : "not found")
						<< '\n';
			conditional_skip(found);
		} else if(command == "ifver" || command == "ifnver") {
			skip_spaces();
			const std::string& vsymstr = read_word();
			skip_spaces();
			const std::string& vopstr = read_word();
			skip_spaces();
			const std::string& vverstr = read_word();

			const VERSION_COMP_OP vop = parse_version_op(vopstr);

			if(vop == OP_INVALID) {
				parent_.error("Invalid #ifver/#ifnver operator", linenum_);
			} else if(parent_.defines_->count(vsymstr) != 0) {
				const preproc_define& sym = (*parent_.defines_)[vsymstr];

				if(!sym.arguments.empty()) {
					parent_.error("First argument macro in #ifver/#ifnver should not require arguments", linenum_);
				}

				version_info const version1(sym.value);
				version_info const version2(vverstr);

				const bool found = do_version_check(version1, vop, version2);
				DBG_PREPROC << "testing version '" << version1.str() << "' against '" << version2.str() << "' ("
							<< vopstr << "): " << (found ? "match" : "no match") << '\n';

				conditional_skip(command == "ifver" ? !found : found);
			} else {
				std::string err = "Undefined macro in #ifver/#ifnver first argument: '";
				err += vsymstr;
				err += "'";
				parent_.error(err, linenum_);
			}
		} else if(command == "else") {
			if(token.type == token_desc::SKIP_ELSE) {
				pop_token();
				--skipping_;
				push_token(token_desc::PROCESS_ELSE);
			} else if(token.type == token_desc::PROCESS_IF) {
				pop_token();
				++skipping_;
				push_token(token_desc::SKIP_IF);
			} else {
				parent_.error("Unexpected #else", linenum_);
			}
		} else if(command == "endif") {
			switch(token.type) {
			case token_desc::SKIP_IF:
			case token_desc::SKIP_ELSE:
				--skipping_;
			case token_desc::PROCESS_IF:
			case token_desc::PROCESS_ELSE:
				break;
			default:
				parent_.error("Unexpected #endif", linenum_);
			}
			pop_token();
		} else if(command == "textdomain") {
			skip_spaces();
			const std::string& s = read_word();
			if(s != parent_.textdomain_) {
				put("#textdomain ");
				put(s);
				parent_.textdomain_ = s;
			}
			comment = true;
		} else if(command == "enddef") {
			parent_.error("Unexpected #enddef", linenum_);
		} else if(command == "undef") {
			skip_spaces();
			const std::string& symbol = read_word();
			if(!skipping_) {
				parent_.defines_->erase(symbol);
				LOG_PREPROC << "undefine macro " << symbol << " (location " << get_location(parent_.location_) << ")\n";
			}
		} else if(command == "error") {
			if(!skipping_) {
				skip_spaces();
				std::ostringstream error;
				error << "#error: \"" << read_rest_of_line() << '"';
				parent_.error(error.str(), linenum_);
			} else
				DBG_PREPROC << "Skipped an error\n";
		} else if(command == "warning") {
			if(!skipping_) {
				skip_spaces();
				std::ostringstream warning;
				warning << "#warning: \"" << read_rest_of_line() << '"';
				parent_.warning(warning.str(), linenum_);
			} else {
				DBG_PREPROC << "Skipped a warning\n";
			}
		} else {
			comment = token.type != token_desc::MACRO_SPACE;
		}

		skip_eol();
		if(comment) {
			put('\n');
		}
	} else if(token.type == token_desc::MACRO_SPACE || token.type == token_desc::MACRO_CHUNK) {
		if(c == '(') {
			// If a macro argument was started, it is implicitly ended.
			token.type = token_desc::MACRO_SPACE;
			push_token(token_desc::MACRO_PARENS);
		} else if(utils::portable_isspace(c)) {
			// If a macro argument was started, it is implicitly ended.
			token.type = token_desc::MACRO_SPACE;
		} else if(c == '}') {
			--slowpath_;
			if(skipping_) {
				pop_token();
				return true;
			}

			// FIXME: is this obsolete?
			// if (token.type == token_desc::MACRO_SPACE) {
			//	if (!strings_.back().empty()) {
			//		std::ostringstream error;
			//		std::ostringstream location;
			//		error << "Can't parse new macro parameter with a macro call scope open";
			//		location<<linenum_<<' '<<parent_.location_;
			//		parent_.error(error.str(), location.str());
			//	}
			//	strings_.pop_back();
			//}

			if(strings_.size() <= static_cast<size_t>(token.stack_pos)) {
				parent_.error("No macro or file substitution target specified", linenum_);
			}

			std::string symbol = strings_[token.stack_pos];
			std::string::size_type pos;
			while((pos = symbol.find(OUTPUT_SEPARATOR)) != std::string::npos) {
				std::string::iterator b = symbol.begin(); // invalidated at each iteration
				symbol.erase(b + pos, b + symbol.find('\n', pos + 1) + 1);
			}

			std::map<std::string, std::string>::const_iterator arg;
			preproc_map::const_iterator macro;

			// If this is a known pre-processing symbol, then we insert it,
			// otherwise we assume it's a file name to load.
			if(symbol == current_file_str && strings_.size() - token.stack_pos == 1) {
				pop_token();
				put(parent_.get_current_file());
			} else if(symbol == current_dir_str && strings_.size() - token.stack_pos == 1) {
				pop_token();
				put(filesystem::directory_name(parent_.get_current_file()));
			} else if(local_defines_ && (arg = local_defines_->find(symbol)) != local_defines_->end()) {
				if(strings_.size() - token.stack_pos != 1) {
					std::ostringstream error;
					error << "Macro argument '" << symbol << "' does not expect any arguments";
					parent_.error(error.str(), linenum_);
				}

				std::ostringstream v;
				v << arg->second << OUTPUT_SEPARATOR << "line " << linenum_ << ' ' << parent_.location_ << "\n"
				  << OUTPUT_SEPARATOR << "textdomain " << parent_.textdomain_ << '\n';

				pop_token();
				put(v.str());
			} else if(parent_.depth() < 100 && (macro = parent_.defines_->find(symbol)) != parent_.defines_->end()) {
				const preproc_define& val = macro->second;
				size_t nb_arg = strings_.size() - token.stack_pos - 1;
				size_t optional_arg_num = 0;

				std::map<std::string, std::string>* defines = new std::map<std::string, std::string>;
				const std::string& dir = filesystem::directory_name(val.location.substr(0, val.location.find(' ')));

				for(size_t i = 0; i < nb_arg; ++i) {
					if(i < val.arguments.size()) {
						// Normal mandatory arguments

						(*defines)[val.arguments[i]] = strings_[token.stack_pos + i + 1];
					} else {
						// These should be optional argument overrides

						std::string str = strings_[token.stack_pos + i + 1];
						size_t equals_pos = str.find_first_of("=");

						if(equals_pos != std::string::npos) {
							size_t argname_pos = str.substr(0, equals_pos).find_last_of(" \n") + 1;

							std::string argname = str.substr(argname_pos, equals_pos - argname_pos);

							if(val.optional_arguments.find(argname) != val.optional_arguments.end()) {
								(*defines)[argname] = str.substr(equals_pos + 1);

								optional_arg_num++;

								DBG_PREPROC << "Found override for " << argname << " in call to macro " << symbol
											<< "\n";
							} else {
								std::ostringstream warning;
								warning << "Unrecognized optional argument passed to macro '" << symbol << "': '"
										<< argname << "'";
								parent_.warning(warning.str(), linenum_);

								optional_arg_num++; // To prevent the argument number check from blowing up
							}
						}
					}
				}

				// If the macro definition has any optional arguments, insert their defaults
				if(val.optional_arguments.size() > 0) {
					for(const auto& argument : val.optional_arguments) {
						if(defines->find(argument.first) == defines->end()) {
							std::unique_ptr<preprocessor_streambuf> buf(new preprocessor_streambuf(parent_));

							buf->textdomain_ = parent_.textdomain_;
							std::istream in(buf.get());

							std::istringstream* buffer = new std::istringstream(argument.second);

							std::map<std::string, std::string>* temp_defines = new std::map<std::string, std::string>;
							temp_defines->insert(defines->begin(), defines->end());

							buf->add_preprocessor<preprocessor_data>(
								buffer, val.location, "", val.linenum, dir, val.textdomain, temp_defines, false);

							std::ostringstream res;
							res << in.rdbuf();

							DBG_PREPROC << "Setting default for optional argument " << argument.first << " in macro "
										<< symbol << "\n";

							(*defines)[argument.first] = res.str();
						}
					}
				}

				if(nb_arg - optional_arg_num != val.arguments.size()) {
					const std::vector<std::string>& locations = utils::quoted_split(val.location, ' ');
					std::ostringstream error;
					error << "Preprocessor symbol '" << symbol << "' defined at " << get_filename(locations[0]) << ":"
						  << val.linenum << " expects " << val.arguments.size() << " arguments, but has "
						  << nb_arg - optional_arg_num << " arguments";
					parent_.error(error.str(), linenum_);
				}

				std::istringstream* buffer = new std::istringstream(val.value);

				pop_token();

				if(!slowpath_) {
					DBG_PREPROC << "substituting macro " << symbol << '\n';

					parent_.add_preprocessor<preprocessor_data>(
						buffer, val.location, "", val.linenum, dir, val.textdomain, defines, true);
				} else {
					DBG_PREPROC << "substituting (slow) macro " << symbol << '\n';

					std::unique_ptr<preprocessor_streambuf> buf(new preprocessor_streambuf(parent_));

					// Make the nested preprocessor_data responsible for
					// restoring our current textdomain if needed.
					buf->textdomain_ = parent_.textdomain_;

					std::ostringstream res;
					{
						std::istream in(buf.get());
						buf->add_preprocessor<preprocessor_data>(
							buffer, val.location, "", val.linenum, dir, val.textdomain, defines, true);

						res << in.rdbuf();
					}

					put(res.str());
				}
			} else if(parent_.depth() < 40) {
				LOG_PREPROC << "Macro definition not found for " << symbol << " , attempting to open as file.\n";
				pop_token();

				std::string nfname = filesystem::get_wml_location(symbol, directory_);
				if(!nfname.empty()) {
					if(!slowpath_)
						// nfname.size() - symbol.size() gives you an index into nfname
						// This does not necessarily match the symbol though, as it can start with ~ or ./
						parent_.add_preprocessor<preprocessor_file>(nfname, nfname.size() - symbol.size());
					else {
						std::unique_ptr<preprocessor_streambuf> buf(new preprocessor_streambuf(parent_));

						std::ostringstream res;
						{
							std::istream in(buf.get());
							buf->add_preprocessor<preprocessor_file>(nfname, nfname.size() - symbol.size());

							res << in.rdbuf();
						}

						put(res.str());
					}
				} else {
					std::ostringstream error;
					error << "Macro/file '" << symbol << "' is missing";
					parent_.error(error.str(), linenum_);
				}
			} else {
				parent_.error("Too many nested preprocessing inclusions", linenum_);
			}
		} else if(!skipping_) {
			if(token.type == token_desc::MACRO_SPACE) {
				std::ostringstream s;
				s << OUTPUT_SEPARATOR << "line " << linenum_ << ' ' << parent_.location_ << "\n"
				  << OUTPUT_SEPARATOR << "textdomain " << parent_.textdomain_ << '\n';

				strings_.push_back(s.str());
				token.type = token_desc::MACRO_CHUNK;
			}
			put(c);
		}
	} else {
		put(c);
	}

	return true;
}


// ==================================================================================
// PREPROCESSOR SCOPE HELPER
// ==================================================================================

struct preprocessor_scope_helper : std::basic_istream<char>
{
	preprocessor_scope_helper(const std::string& fname, preproc_map* defines)
		: std::basic_istream<char>(nullptr)
		, buf_(nullptr)
		, local_defines_(nullptr)
	{
		//
		// If no defines were provided, we create a new local preproc_map and assign
		// it to defines temporarily. In this case, the map will be deleted once this
		// object is destroyed and defines will still be subsequently null.
		//
		if(!defines) {
			local_defines_.reset(new preproc_map);
			defines = local_defines_.get();
		}

		buf_.reset(new preprocessor_streambuf(defines));

		// Begin processing.
		buf_->add_preprocessor<preprocessor_file>(fname);

		//
		// TODO: not sure if this call is needed. Previously, this call was passed a
		// preprocessor_streambuf pointer and the std::basic_istream constructor was
		// called with its contents. However, at that point the preprocessing should
		// already have completed, meaning this call might be redundant. Not sure.
		//
		// - vultraz, 2017-08-31
		//
		init(buf_.get());
	}

	~preprocessor_scope_helper()
	{
		clear(std::ios_base::goodbit);
		exceptions(std::ios_base::goodbit);
		rdbuf(nullptr);
	}

	std::unique_ptr<preprocessor_streambuf> buf_;
	std::unique_ptr<preproc_map> local_defines_;
};


// ==================================================================================
// FREE-STANDING FUNCTIONS
// ==================================================================================

filesystem::scoped_istream preprocess_file(const std::string& fname, preproc_map* defines)
{
	log_scope("preprocessing file " + fname + " ...");

	// NOTE: the preprocessor_scope_helper does *not* take ownership of defines.
	return filesystem::scoped_istream(new preprocessor_scope_helper(fname, defines));
}

void preprocess_resource(const std::string& res_name,
		preproc_map* defines_map,
		bool write_cfg,
		bool write_plain_cfg,
		const std::string& parent_directory)
{
	if(filesystem::is_directory(res_name)) {
		std::vector<std::string> dirs, files;

		filesystem::get_files_in_dir(res_name, &files, &dirs, filesystem::ENTIRE_FILE_PATH, filesystem::SKIP_MEDIA_DIR,
				filesystem::DO_REORDER);

		// Subdirectories
		for(const std::string& dir : dirs) {
			LOG_PREPROC << "processing sub-dir: " << dir << '\n';
			preprocess_resource(dir, defines_map, write_cfg, write_plain_cfg, parent_directory);
		}

		// Files in current directory
		for(const std::string& file : files) {
			preprocess_resource(file, defines_map, write_cfg, write_plain_cfg, parent_directory);
		}

		return;
	}

	// process only config files.
	if(!filesystem::ends_with(res_name, ".cfg")) {
		return;
	}

	LOG_PREPROC << "processing resource: " << res_name << '\n';

	// disable filename encoding to get clear #line in cfg.plain
	encode_filename = false;

	filesystem::scoped_istream stream = preprocess_file(res_name, defines_map);

	std::stringstream ss;

	// Set the failbit so if we get any preprocessor exceptions (e.g.:preproc_config::error)
	// they will be propagated in the main program, instead of just setting the
	// failbit on the stream. This was necessary in order for the MSVC and GCC
	// binaries to behave the same way.
	ss.exceptions(std::ios_base::failbit);

	ss << (*stream).rdbuf();

	LOG_PREPROC << "processing finished\n";

	if(write_cfg || write_plain_cfg) {
		config cfg;
		std::string streamContent = ss.str();

		read(cfg, streamContent);

		const std::string preproc_res_name = parent_directory + "/" + filesystem::base_name(res_name);

		// Write the processed cfg file
		if(write_cfg) {
			LOG_PREPROC << "writing cfg file: " << preproc_res_name << '\n';

			filesystem::create_directory_if_missing_recursive(filesystem::directory_name(preproc_res_name));
			filesystem::scoped_ostream outStream(filesystem::ostream_file(preproc_res_name));

			write(*outStream, cfg);
		}

		// Write the plain cfg file
		if(write_plain_cfg) {
			LOG_PREPROC << "writing plain cfg file: " << (preproc_res_name + ".plain") << '\n';

			filesystem::create_directory_if_missing_recursive(filesystem::directory_name(preproc_res_name));
			filesystem::write_file(preproc_res_name + ".plain", streamContent);
		}
	}
}
