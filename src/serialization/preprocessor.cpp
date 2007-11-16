/* $Id$ */
/*
   Copyright (C) 2003 by David White <dave@whitevine.net>
   Copyright (C) 2005 - 2007 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file serialization/preprocessor.cpp
//! WML preprocessor.

#include "../global.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <vector>

#include "../filesystem.hpp"
#include "../log.hpp"
#include "../wassert.hpp"
#include "../wesconfig.h"
#include "preprocessor.hpp"
#include "string_utils.hpp"

#define ERR_CF LOG_STREAM(err, config)
#define LOG_CF LOG_STREAM(info, config)

using std::streambuf;

bool preproc_define::operator==(preproc_define const &v) const {
	return value == v.value && arguments == v.arguments;
}

struct preproc_config {
	struct error {
		error(const std::string& msg) : message(msg) {}
		std::string message;
	};
};

class preprocessor;
class preprocessor_file;
class preprocessor_data;
class preprocessor_streambuf;
struct preprocessor_deleter;

class preprocessor
{
	preprocessor *const old_preprocessor_;
	std::string old_textdomain_;
	std::string old_location_;
	int old_linenum_;
protected:
	preprocessor_streambuf &target_;
	preprocessor(preprocessor_streambuf &);
public:
	virtual bool get_chunk() = 0;
	virtual ~preprocessor();
};

class preprocessor_streambuf: public streambuf
{
	std::string out_buffer_;
	virtual int underflow();
	std::ostringstream buffer_;
	preprocessor *current_;
	preproc_map *defines_;
	preproc_map default_defines_;
	std::string textdomain_;
	std::string location_;
	int linenum_;
	int depth_;
	int buffer_size_;
	bool quoted_;
	friend class preprocessor;
	friend class preprocessor_file;
	friend class preprocessor_data;
	friend struct preprocessor_deleter;
	preprocessor_streambuf(preprocessor_streambuf const &);
public:
	preprocessor_streambuf(preproc_map *);
};

preprocessor_streambuf::preprocessor_streambuf(preproc_map *def) :
	streambuf(), 
	out_buffer_(""), 
	buffer_(), 
	current_(NULL), 
	defines_(def),
	default_defines_(),
	textdomain_(PACKAGE), 
	location_(""), 
	linenum_(0), 
	depth_(0),
	buffer_size_(0), 
	quoted_(false)
{
}

preprocessor_streambuf::preprocessor_streambuf(preprocessor_streambuf const &t) :
	streambuf(),
	out_buffer_(""),
	buffer_(),
	current_(NULL),
	defines_(t.defines_),
	default_defines_(),
	textdomain_(PACKAGE),
	location_(""),
	linenum_(0),
	depth_(t.depth_),
	buffer_size_(0), 
	quoted_(t.quoted_)
{
}

// underflow is called when the internal buffer has been consumed 
// so that more can be prepared
int preprocessor_streambuf::underflow()
{
	unsigned sz = 0;
	if (char *gp = gptr()) {
		if (gp < egptr()) {
			// Sanity check: the internal buffer has not been totally consumed,
			// should we force the caller to use what remains first?
			return *gp;
		}
		// The buffer has been completely read; fill it again.
		// Keep part of the previous buffer, to ensure putback capabilities.
		sz = out_buffer_.size();
		if (sz > 3) {
			out_buffer_ = out_buffer_.substr(sz - 3);
			sz = 3;
		}
		buffer_.str(std::string());
		buffer_ << out_buffer_;
        buffer_size_ = out_buffer_.size();
	} else {
		// The internal get-data pointer is null
	}
	const int desired_fill_amount = 2000;
	while (current_ && buffer_size_ < desired_fill_amount) {
		// Process files and data chunks until the desired buffer size is reached
		if (!current_->get_chunk()) {
			 // Delete the current preprocessor item to restore its predecessor
			delete current_;
		}
	}
	// Update the internal state and data pointers
	out_buffer_ = buffer_.str();
	char *begin = &*out_buffer_.begin();
	unsigned bs = out_buffer_.size();
	setg(begin, begin + sz, begin + bs);
	if (sz >= bs)
		return EOF;
	return static_cast<unsigned char>(*(begin + sz));
}

/**
 * preprocessor
 * 
 * This is the base class for all input to be parsed by the preprocessor.
 * When initialized, it will inform the stream (target_) that it is the current scope.
 * When it reaches its end of its scope, the stream will delete it, 
 * and when it is deleted, it will manage the stream 
 * to cause the previous scope to resume.
 */
preprocessor::preprocessor(preprocessor_streambuf &t) :
	old_preprocessor_(t.current_),
	old_textdomain_(t.textdomain_),
	old_location_(t.location_),
	old_linenum_(t.linenum_),
	target_(t)
{
	++target_.depth_;
	target_.current_ = this;
}

namespace {
void count_extra_digits(int const& line_number, int& buffer_size) {
	for(int digit_mark = 10; line_number >= digit_mark; digit_mark *= 10) {
		// The number is larger than one digit, calculate additional string size
		++buffer_size;
	}
}
} // end anonymous namespace

preprocessor::~preprocessor()
{
	wassert(target_.current_ == this);
	target_.current_  = old_preprocessor_;
	target_.location_ = old_location_;
	target_.linenum_  = old_linenum_;
	target_.textdomain_ = old_textdomain_;
	if (!old_location_.empty()) {
		target_.buffer_ << "\376line " << old_linenum_ << ' ' << old_location_ << '\n';
		const int fixed_char_count = 9;
		count_extra_digits(old_linenum_, target_.buffer_size_);
		target_.buffer_size_ += old_location_.size() + fixed_char_count;
    }
	if (!old_textdomain_.empty()) {
		target_.buffer_ << "\376textdomain " << old_textdomain_ << '\n';
		const int fixed_char_count = 13;
		target_.buffer_size_ += old_textdomain_.size() + fixed_char_count;
    }
	--target_.depth_;
}

/**
 *  preprocessor_file
 *
 *  This represents a WML element that resolves to a directory or file inclusion, 
 *  such as '{themes/}'
 */
class preprocessor_file: preprocessor
{
	std::vector< std::string > files_;
	std::vector< std::string >::const_iterator pos_, end_;
public:
	preprocessor_file(preprocessor_streambuf &, std::string const &);
	virtual bool get_chunk();
};

class preprocessor_data: preprocessor
{
	struct token_desc
	{
		//! @todo FIXME: add enum for token type, with explanation of the different types
		char type;
		int stack_pos;
		int linenum;
	};
	scoped_istream in_;
	std::string directory_;
	std::vector< std::string > strings_;
	std::vector< token_desc > tokens_;
	int slowpath_, //! @todo FIXME: add explanation of this variable
		skipping_, //FIXME: add explanation of this variable
		linenum_;

	std::string read_word();
	std::string read_line();
	void skip_spaces();
	void skip_eol();
	void push_token(char);
	void pop_token();
	void put(char);
	void put(std::string const &);
public:
	preprocessor_data(preprocessor_streambuf &, std::istream *,
	                  std::string const &history,
	                  std::string const &name, int line,
	                  std::string const &dir, std::string const &domain);
	virtual bool get_chunk();
};

preprocessor_file::preprocessor_file(preprocessor_streambuf &t,
		std::string const &name) :
	preprocessor(t),
	files_(),
	pos_(),
	end_()
{
	if (is_directory(name))
		get_files_in_dir(name, &files_, NULL, ENTIRE_FILE_PATH, DO_REORDER);
	else
		new preprocessor_data(t, istream_file(name), "", name, 1, directory_name(name), t.textdomain_);
	pos_ = files_.begin();
	end_ = files_.end();
}

/**
 * preprocessor_file::get_chunk()
 *
 * Inserts and processes the next file in the list of included files.
 * @return	false if there is no next file.
 */
bool preprocessor_file::get_chunk()
{
	while (pos_ != end_) {
		std::string const &name = *(pos_++);
		unsigned sz = name.size();
		if (sz < 5 || !std::equal(name.begin() + sz - 4, name.end(), ".cfg"))
			continue;
		new preprocessor_file(target_, name);
		return true;
	}
	return false;
}

preprocessor_data::preprocessor_data(preprocessor_streambuf &t, std::istream *i,
		std::string const &history, std::string const &name, int linenum, 
		std::string const &directory, std::string const &domain) :
	preprocessor(t), 
	in_(i), 
	directory_(directory),
	strings_(),
	tokens_(),
	slowpath_(0), 
	skipping_(0), 
	linenum_(linenum)
{
	std::ostringstream s;

	s << history;
	if (!name.empty()) {
		std::string ename(name);
		if (!history.empty())
			s << ' ';
		s << utils::escape(ename, " \\");
	}
	if (!t.location_.empty())
		s << ' ' << t.linenum_ << ' ' << t.location_;
	t.location_ = s.str();
	t.linenum_ = linenum;
	t.textdomain_ = domain;

	t.buffer_ << "\376line " << linenum
		<< ' ' << t.location_ << "\n\376textdomain " << domain << '\n';
	const int fixed_char_count = 22;
	count_extra_digits(linenum, t.buffer_size_);
	t.buffer_size_ += t.location_.size() + domain.size() + fixed_char_count;

	push_token('*');
}

void preprocessor_data::push_token(char t)
{
	token_desc token = { t, strings_.size(), linenum_ };
	tokens_.push_back(token);
	std::ostringstream s;
	if (!skipping_ && slowpath_) {
		s << "\376line " << linenum_ << ' ' << target_.location_
		  << "\n\376textdomain " << target_.textdomain_ << '\n';
	}
	strings_.push_back(s.str());
}

void preprocessor_data::pop_token()
{
	strings_.erase(strings_.begin() + tokens_.back().stack_pos, strings_.end());
	tokens_.pop_back();
}

void preprocessor_data::skip_spaces()
{
	for(;;) {
		int c = in_->peek();
		if (!in_->good() || (c != ' ' && c != '\t'))
			return;
		in_->get();
	}
}

void preprocessor_data::skip_eol()
{
	for(;;) {
		int c = in_->get();
		if (c == '\n') {
			++linenum_;
			return;
		}
		if (!in_->good())
			return;
	}
}

std::string preprocessor_data::read_word()
{
	std::string res;
	for(;;) {
		int c = in_->peek();
		if (c == preprocessor_streambuf::traits_type::eof() || utils::portable_isspace(c)) {
			// LOG_CF << "(" << res << ")\n";
			return res;
		}
		in_->get();
		res += static_cast<char>(c);
	}
}

std::string preprocessor_data::read_line()
{
	std::string res;
	for(;;) {
		int c = in_->get();
		if (c == '\n') {
			++linenum_;
			return res;
		}
		if (!in_->good())
			return res;
		if (c != '\r')
			res += static_cast<char>(c);
	}
}

void preprocessor_data::put(char c)
{
	if (skipping_)
		return;
	if (slowpath_) {
		strings_.back() += c;
		return;
	}
	if (linenum_ != target_.linenum_ && c != '\n' ||
	    linenum_ != target_.linenum_ + 1 && c == '\n') {
		target_.linenum_ = linenum_;
		if (c == '\n')
			--target_.linenum_;

		target_.buffer_ << "\376line " << target_.linenum_
			<< ' ' << target_.location_ << '\n';
		const int fixed_char_count = 9;
		count_extra_digits(target_.linenum_, target_.buffer_size_);
		target_.buffer_size_ += target_.location_.size() + fixed_char_count;
	}
	if (c == '\n')
		++target_.linenum_;
	target_.buffer_ << c;
    target_.buffer_size_ += 1;
}

void preprocessor_data::put(std::string const &s)
{
	if (skipping_)
		return;
	if (slowpath_) {
		strings_.back() += s;
		return;
	}
	target_.buffer_ << s;
    target_.buffer_size_ += s.size();
	target_.linenum_ += std::count(s.begin(), s.end(), '\n');
	target_.linenum_ -= std::count(s.begin(), s.end(), '\376');
}

bool preprocessor_data::get_chunk()
{
	char c = in_->get();
	token_desc &token = tokens_.back();
	if (!in_->good()) {
		// The end of file was reached. 
		// Make sure we don't have any incomplete tokens.
		char const *s;
		switch (token.type) {
		case '*': return false; // everything is fine
		case 'i':
		case 'I':
		case 'j':
		case 'J': s = "#ifdef or #ifndef"; break;
		case '"': s = "quoted string"; break;
		case '[':
		case '{': s = "macro substitution"; break;
		case '(': s = "macro argument"; break;
		default: s = "???";
		}
		std::ostringstream error;
		error << s << " not terminated, started at "
		      << token.linenum << ' ' << target_.location_;
		ERR_CF << error.str() << '\n';
		pop_token();
		throw preproc_config::error(error.str());
	}
	if (c == '\n')
		++linenum_;
	if (c == '\376') {
		std::string buffer(1, c);
		for(;;) {
			char d = in_->get();
			if (!in_->good() || d == '\n')
				break;
			buffer += d;
		}
		buffer += '\n';
		put(buffer);
	} else if (c == '"') {
		if (token.type == '"') {
			target_.quoted_ = false;
			put(c);
			if (!skipping_ && slowpath_) {
				std::string tmp = strings_.back();
				pop_token();
				strings_.back() += tmp;
			} else
				pop_token();
		} else if (!target_.quoted_) {
			target_.quoted_ = true;
			if (token.type == '{')
				token.type = '[';
			push_token('"');
			put(c);
		} else {
			std::ostringstream error;
			error << "nested quoted string started at "
			      << linenum_ << ' ' << target_.location_;
			ERR_CF << error.str() << '\n';
			throw preproc_config::error(error.str());
		}
	} else if (c == '{') {
		if (token.type == '{')
			token.type = '[';
		push_token('{');
		++slowpath_;
	} else if (c == ')' && token.type == '(') {
		tokens_.pop_back();
		strings_.push_back(std::string());
	} else if (c == '#' && !target_.quoted_) {
		std::string command = read_word();
		bool comment = false;
		if (command == "define") {
			skip_spaces();
			int linenum = linenum_;
			std::vector< std::string > items = utils::split(read_line(), ' ');
			if (items.empty()) {
				std::ostringstream error;
				error << "no macro name found after #define directive at "
				      << linenum_ << ' ' << target_.location_;
				ERR_CF << error.str() << '\n';
				throw preproc_config::error(error.str());
			}
			std::string symbol = items.front();
			items.erase(items.begin());
			int found_enddef = 0;
			std::string buffer;
			for(;;) {
				if (!in_->good())
					break;
				char d = in_->get();
				if (d == '\n')
					++linenum_;
				buffer += d;
				if (d == '#')
					found_enddef = 1;
				else if (found_enddef > 0)
					if (++found_enddef == 7)
						if (std::equal(buffer.end() - 6, buffer.end(), "enddef"))
							break;
						else
							found_enddef = 0;
			}
			if (found_enddef != 7) {
				std::ostringstream error;
				error << "unterminated preprocessor definition at "
				      << linenum << ' ' << target_.location_;
				ERR_CF << error.str() << '\n';
				throw preproc_config::error(error.str());
			}
			if (!skipping_) {
				buffer.erase(buffer.end() - 7, buffer.end());
				target_.defines_->insert(std::make_pair(
					symbol, preproc_define(buffer, items, target_.textdomain_,
					                       linenum + 1, target_.location_)));
				LOG_CF << "defining macro " << symbol << " (location " << target_.location_ << ")\n";
			}
		} else if (command == "ifdef") {
			skip_spaces();
			std::string const &symbol = read_word();
			bool skip = target_.defines_->count(symbol) == 0;
			LOG_CF << "testing for macro " << symbol << ": " << (skip ? "not defined" : "defined") << '\n';
			if (skip)
				++skipping_;
			push_token(skip ? 'J' : 'i');
		} else if (command == "ifndef") {
			skip_spaces();
			std::string const &symbol = read_word();
			bool skip = target_.defines_->count(symbol) != 0;
			LOG_CF << "testing for macro " << symbol << ": " << (skip ? "not defined" : "defined") << '\n';
			if (skip)
				++skipping_;
			push_token(skip ? 'J' : 'i');
		} else if (command == "else") {
			if (token.type == 'J') {
				pop_token();
				--skipping_;
				push_token('j');
			} else if (token.type == 'i') {
				pop_token();
				++skipping_;
				push_token('I');
			} else {
				std::ostringstream error;
				error << "unexpected #else at "
				      << linenum_ << ' ' << target_.location_;
				ERR_CF << error.str() << '\n';
				throw preproc_config::error(error.str());
			}
		} else if (command == "endif") {
			switch (token.type) {
			case 'I':
			case 'J': --skipping_;
			case 'i':
			case 'j': break;
			default:
				std::ostringstream error;
				error << "unexpected #endif at "
				      << linenum_ << ' ' << target_.location_;
				ERR_CF << error.str() << '\n';
				throw preproc_config::error(error.str());
			}
			pop_token();
		} else if (command == "textdomain") {
			skip_spaces();
			std::string const &s = read_word();
			put("#textdomain ");
			put(s);
			target_.textdomain_ = s;
			comment = true;
		} else if (command == "enddef") {
			std::ostringstream error;
			error << "unexpected #enddef at "
			      << linenum_ << ' ' << target_.location_;
			ERR_CF << error.str() << '\n';
			throw preproc_config::error(error.str());
		} else if (command == "undef") {
			skip_spaces();
			std::string const &symbol = read_word();
			target_.defines_->erase(symbol);
			LOG_CF << "undefine macro " << symbol << " (location " << target_.location_ << ")\n";
		} else
			comment = true;
		skip_eol();
		if (comment)
			put('\n');
	} else if (token.type == '{' || token.type == '[') {
		if (c == '(') {
			if (token.type == '[')
				token.type = '{';
			else
				strings_.pop_back();
			push_token('(');
		} else if (utils::portable_isspace(c)) {
			if (token.type == '[') {
				strings_.push_back(std::string());
				token.type = '{';
			}
		} else if (c == '}') {
			--slowpath_;
			if (skipping_) {
				pop_token();
				return true;
			}
			if (token.type == '{') {
				wassert(strings_.back().empty());
				strings_.pop_back();
			}

			std::string symbol = strings_[token.stack_pos];
			std::string::size_type pos;
			while ((pos = symbol.find('\376')) != std::string::npos) {
				std::string::iterator b = symbol.begin(); // invalidated at each iteration
				symbol.erase(b + pos, b + symbol.find('\n', pos + 1) + 1);
			}
			// If this is a known pre-processing symbol, then we insert it, 
			// otherwise we assume it's a file name to load.
			preproc_map::const_iterator macro = target_.defines_->find(symbol),
			                            unknown_macro = target_.defines_->end();
			if (macro != unknown_macro) {
				preproc_define const &val = macro->second;
				size_t nb_arg = strings_.size() - token.stack_pos - 1;
				if (nb_arg != val.arguments.size()) {
					std::ostringstream error;
					error << "preprocessor symbol '" << symbol << "' expects "
					      << val.arguments.size() << " arguments, but has "
					      << nb_arg << " arguments at " << linenum_
					      << ' ' << target_.location_;
					ERR_CF << error.str() << '\n';
					throw preproc_config::error(error.str());
				}

				std::stringstream *buffer = new std::stringstream;
				std::string::const_iterator i_bra = val.value.end();
				int macro_num = val.linenum;
				std::string macro_textdomain = val.textdomain;
				bool quoting = false;
				for(std::string::const_iterator i = val.value.begin(),
				    i_end = val.value.end(); i != i_end; ++i) {
					char c = *i;
					if (c == '\n') {
						++macro_num;
					} else if (c == '"') {
						quoting = !quoting;
					}

					if (c == '{') {
						if (i_bra != i_end)
							buffer->write(&*i_bra - 1, i - i_bra + 1);
						i_bra = i + 1;
					} else if (i_bra == i_end) {
						if (c == '#' && !quoting) {
							// Keep track of textdomain changes in the body of the
							// macro, so they can be restored after each substitution
							// of a macro argument.
							std::string::const_iterator i_beg = i + 1;
							if (i_end - i_beg >= 13 &&
							    std::equal(i_beg, i_beg + 10, "textdomain")) {
								i_beg += 10;
								i = std::find(i_beg, i_end, '\n');
								if (i_beg != i)
									++i_beg;
								macro_textdomain = std::string(i_beg, i);
								*buffer << "#textdomain " << macro_textdomain;
								++macro_num;
								c = '\n';
							} else if((i_end - i_beg < 6 || (!std::equal(i_beg, i_beg + 6, "define")
							&& !std::equal(i_beg, i_beg + 6, "ifndef")))
							&& (i_end - i_beg < 5 || (!std::equal(i_beg, i_beg + 5, "ifdef") 
							&& !std::equal(i_beg, i_beg + 5, "endif") && !std::equal(i_beg, i_beg + 5, "undef")))
							&& (i_end - i_beg < 4 || !std::equal(i_beg, i_beg + 4, "else"))) {
								// Check for define, ifdef, ifndef, endif, undef, else.
								// Otherwise, this is a comment and should be skipped.
								i = std::find(i_beg, i_end, '\n');
								++macro_num;
								c = '\n';
							}
						}
						buffer->put(c);
					} else if (c == '}') {
						size_t sz = i - i_bra;
						for(size_t n = 0; n < nb_arg; ++n) {
							std::string const &arg = val.arguments[n];
							if (arg.size() != sz ||
							    !std::equal(i_bra, i, arg.begin()))
								continue;
							*buffer << strings_[token.stack_pos + n + 1]
							        << "\376line " << macro_num
							        << ' ' << val.location << "\n\376textdomain "
							        << macro_textdomain << '\n';
							i_bra = i_end;
							break;
						}
						if (i_bra != i_end) {
							// The bracketed text was no macro argument
							buffer->write(&*i_bra - 1, sz + 2);
							i_bra = i_end;
						}
					}
				}

				pop_token();
				std::string const &dir = directory_name(val.location.substr(0, val.location.find(' ')));
				if (!slowpath_) {
					LOG_CF << "substituting macro " << symbol << '\n';
					new preprocessor_data(target_, buffer, val.location, "",
					                      val.linenum, dir, val.textdomain);
				} else {
					LOG_CF << "substituting (slow) macro " << symbol << '\n';
					std::ostringstream res;
					preprocessor_streambuf *buf =
						new preprocessor_streambuf(target_);
					{	std::istream in(buf);
						new preprocessor_data(*buf, buffer, val.location, "",
						                      val.linenum, dir, val.textdomain);
						res << in.rdbuf(); }
					delete buf;
					strings_.back() += res.str();
				}
			} else if (target_.depth_ < 40) {
				pop_token();
				std::string prefix;
				std::string nfname;
				std::string const &newfilename = symbol;
				{
					// If the filename begins with a '~', 
					//  then look in the user's data directory. 
					// If the filename begins with a '@', 
					//  then we look in the user's data directory,
					// but default to the standard data directory if it's not found there.
					if(newfilename != "" && (newfilename[0] == '~' || newfilename[0] == '@')) {
						nfname = newfilename;
						nfname.erase(nfname.begin(),nfname.begin()+1);
						nfname = get_user_data_dir() + "/data/" + nfname;

						LOG_CF << "got relative name '" << newfilename << "' -> '" << nfname << "'\n";

						if(newfilename[0] == '@' && file_exists(nfname) == false && is_directory(nfname) == false) {
							nfname = "data/" + newfilename.substr(1);
						}
					} else
					if(newfilename.size() >= 2 && newfilename[0] == '.' &&
						newfilename[1] == '/' ) {
						// If the filename begins with a "./", 
						// then look in the same directory as the file 
						// currrently being preprocessed.
						nfname = newfilename;
						nfname.erase(nfname.begin(),nfname.begin()+2);
						nfname = directory_ + nfname;

					} else {

							nfname = "data/" + newfilename;
					}

					if (!slowpath_)
						new preprocessor_file(target_, nfname);
					else {
						std::ostringstream res;
						preprocessor_streambuf *buf =
							new preprocessor_streambuf(target_);
						{	std::istream in(buf);
							new preprocessor_file(*buf, nfname);
							res << in.rdbuf(); }
						delete buf;
						strings_.back() += res.str();
					}
				}
			} else {
				ERR_CF << "too much nested preprocessing inclusions at "
				       << linenum_ << ' ' << target_.location_
				       << ". Aborting.\n";
				pop_token();
			}
		} else {
			strings_.back() += c;
			token.type = '[';
		}
	} else
		put(c);
	return true;
}

struct preprocessor_deleter: std::basic_istream<char>
{
	preprocessor_streambuf *buf_;
	preproc_map *defines_;
	preprocessor_deleter(preprocessor_streambuf *buf, preproc_map *defines);
	~preprocessor_deleter();
};

preprocessor_deleter::preprocessor_deleter(preprocessor_streambuf *buf, preproc_map *defines)
	: std::basic_istream<char>(buf), buf_(buf), defines_(defines)
{
}

preprocessor_deleter::~preprocessor_deleter()
{
	rdbuf(NULL);
	delete buf_;
	delete defines_;
}


std::istream *preprocess_file(std::string const &fname,
                              preproc_map *defines)
{
	log_scope("preprocessing file...");
	preproc_map *owned_defines = NULL;
	if (!defines) {
		// If no preproc_map has been given, create a new one, 
		// and ensure it is destroyed when the stream is 
// ??
		// by giving it to the deleter.
		owned_defines = new preproc_map;
		defines = owned_defines;
	}
	preprocessor_streambuf *buf = new preprocessor_streambuf(defines);
	new preprocessor_file(*buf, fname);
	return new preprocessor_deleter(buf, owned_defines);
}

