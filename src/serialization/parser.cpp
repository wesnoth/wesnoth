/* $Id$ */
/*
   Copyright (C) 2003 by David White <dave@whitevine.net>
   Copyright (C) 2005 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Copyright (C) 2005 - 2011 by Philippe Plantier <ayin@anathas.org>
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
 * Read/Write & analyze WML- and config-files.
 */


#include "serialization/parser.hpp"

#include "config.hpp"
#include "log.hpp"
#include "gettext.hpp"
#include "loadscreen.hpp"
#include "wesconfig.h"
#include "serialization/preprocessor.hpp"
#include "serialization/tokenizer.hpp"
#include "serialization/string_utils.hpp"
#include "serialization/validator.hpp"
#include "foreach.hpp"

#include <stack>
#include <algorithm>

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/variant.hpp>

static lg::log_domain log_config("config");
#define ERR_CF LOG_STREAM(err, log_config)
#define WRN_CF LOG_STREAM(warn, log_config)
#define LOG_CF LOG_STREAM(info, log_config)

static const size_t max_recursion_levels = 1000;

namespace {

class parser
{
	parser();
	parser(const parser&);
	parser& operator=(const parser&);
public:
	parser(config& cfg, std::istream& in,
		   abstract_validator * validator = NULL);
	~parser();
	void operator()();

private:
	void parse_element();
	void parse_variable();
	std::string lineno_string(utils::token_map &map, std::string const &lineno, const char *error_string);
	void error(const std::string& message);

	config& cfg_;
	tokenizer *tok_;
	abstract_validator *validator_;

	struct element {
		element(config *cfg, config::t_token const &name,
			int start_line = 0, const std::string &file = "") :
			cfg(cfg), name(name), start_line(start_line), file(file) {}

		config* cfg;
		config::t_token name;
		int start_line;
		std::string file;
	};

	std::stack<element> elements_;
};

parser::parser(config &cfg, std::istream &in, abstract_validator * validator)
			   :cfg_(cfg),
			   tok_(new tokenizer(in)),
			   validator_(validator),
			   elements_()
{
}


parser::~parser() {
	delete tok_;
}

void parser::operator()() {
	static const config::t_token & z_empty( generate_safe_static_const_t_interned(n_token::t_token("")) );
	static const config::t_token & z_tag( generate_safe_static_const_t_interned(n_token::t_token("tag")) );

	cfg_.clear();
	elements_.push(element(&cfg_, z_empty));

	do {
		tok_->next_token();

		switch(tok_->current_token().type) {
		case token::LF:
			continue;
		case '[':
			parse_element();
			break;
		case token::STRING:
			parse_variable();
			break;
		default:
			if (static_cast<unsigned char>((*tok_->current_token().value())[0]) == 0xEF &&
			    static_cast<unsigned char>((*tok_->next_token().value())[0])    == 0xBB &&
			    static_cast<unsigned char>((*tok_->next_token().value())[0])    == 0xBF)
			{
				ERR_CF << "Skipping over a utf8 BOM\n";
			} else {
				error(_("Unexpected characters at line start"));
			}
			break;
		case token::END:
			break;
		}
		loadscreen::increment_progress();
	} while (tok_->current_token().type != token::END);

	// The main element should be there. If it is not, this is a parser error.
	assert(!elements_.empty());

	if(elements_.size() != 1) {
		utils::token_map i18n_symbols;
		i18n_symbols[z_tag] = elements_.top().name;
		std::stringstream ss;
		ss << elements_.top().start_line << " " << elements_.top().file;
		error(lineno_string(i18n_symbols, ss.str(),
				N_("Missing closing tag for tag $tag at $pos")));
	}
}

void parser::parse_element() {
	tok_->next_token();
	config::t_token elname;
	config* current_element = NULL;
	switch(tok_->current_token().type) {
	case token::STRING: // [element]
		elname = tok_->current_token().value();
		if (tok_->next_token().type != ']') {
			error(_("Unterminated [element] tag")); }
		// Add the element
		current_element = &(elements_.top().cfg->add_child(elname));
		elements_.push(element(current_element, elname, tok_->get_start_line(), tok_->get_file()));
		if (validator_){
			validator_->open_tag(elname, tok_->get_start_line(), tok_->get_file()); }
		break;

	case '+': // [+element]
		if (tok_->next_token().type != token::STRING){
			error(_("Invalid tag name")); }
		elname = tok_->current_token().value();
		if (tok_->next_token().type != ']') {
			error(_("Unterminated [+element] tag")); }

		// Find the last child of the current element whose name is
		// element
		if (config &c = elements_.top().cfg->child(elname, -1)) {
			current_element = &c;
			if (validator_) {
				validator_->open_tag(elname,tok_->get_start_line(), tok_->get_file(),true); }
		} else {
			current_element = &elements_.top().cfg->add_child(elname);
			if (validator_) {
				validator_->open_tag(elname,tok_->get_start_line(), tok_->get_file()); }
		}
		elements_.push(element(current_element, elname, tok_->get_start_line(), tok_->get_file()));
		break;

	case '/': // [/element]
		if(tok_->next_token().type != token::STRING)
			error(_("Invalid closing tag name"));
		elname = tok_->current_token().value();
		if(tok_->next_token().type != ']')
			error(_("Unterminated closing tag"));
		if(elements_.size() <= 1)
			error(_("Unexpected closing tag"));
		if(elname != elements_.top().name) {
			static const config::t_token & z_tag1( generate_safe_static_const_t_interned(n_token::t_token("tag1")) );
			static const config::t_token & z_tag2( generate_safe_static_const_t_interned(n_token::t_token("tag2")) );
			utils::token_map i18n_symbols;
			i18n_symbols[z_tag1] = elements_.top().name;
			i18n_symbols[z_tag2] = elname;
			std::stringstream ss;
			ss << elements_.top().start_line << " " << elements_.top().file;
			error(lineno_string(i18n_symbols, ss.str(),
					N_("Found invalid closing tag $tag2 for tag $tag1 (opened at $pos)")));
		}
		if(validator_){
			element & el= elements_.top();
			validator_->validate(*el.cfg,el.name,el.start_line,el.file);
			validator_->close_tag();
		}
		elements_.pop();
		break;
	default:
		error(_("Invalid tag name"));
	}
}

void parser::parse_variable()
{
	static const config::t_token & z_empty( generate_safe_static_const_t_interned(n_token::t_token("")) );

	assert(!elements_.empty());
	assert(elements_.top().cfg);

	config& cfg = *elements_.top().cfg;
	std::vector<config::t_token> variables;
	variables.push_back(z_empty);

	while (tok_->current_token().type != '=') {
		switch(tok_->current_token().type) {
		case token::STRING:
			variables.back() = config::t_token( (*variables.back())
												+ ((!variables.back().empty()) ? " " :"")
												+ (*tok_->current_token().value()));
			break;
		case ',':
			if(variables.back().empty()) {
				error(_("Empty variable name"));
			} else {
				variables.push_back(z_empty);
			}
			break;
		default:
			error(_("Unexpected characters after variable name (expected , or =)"));
			break;
		}
		tok_->next_token();
	}
	if(variables.back().empty())
		error(_("Empty variable name"));

	t_string_base buffer;

	std::vector<config::t_token>::const_iterator curvar = variables.begin();

	bool ignore_next_newlines = false, previous_string = false;
	while(1) {
		tok_->next_token();
		assert(curvar != variables.end());

		switch (tok_->current_token().type) {
		case ',':
			if ((curvar+1) != variables.end()) {
				if (buffer.translatable())
					cfg[*curvar] = t_string(buffer);
				else
					cfg[*curvar] = buffer.token();
				if(validator_) {
					validator_->validate_key (cfg,*curvar,buffer.value(), tok_->get_start_line (), tok_->get_file ()); }
				buffer = t_string_base();
				++curvar;
			} else {
				buffer += ",";
			}
			break;
		case '_':
			tok_->next_token();
			switch (tok_->current_token().type) {
			case token::UNTERMINATED_QSTRING:
				error(_("Unterminated quoted string"));
				break;
			case token::QSTRING:
				buffer += t_string_base(tok_->current_token().value(), tok_->textdomain());
				break;
			default:
				buffer += "_";
				buffer += tok_->current_token().value();
				break;
			case token::END:
			case token::LF:
				buffer += "_";
				goto finish;
			}
			break;
		case '+':
			ignore_next_newlines = true;
			continue;
		case token::STRING:
			if (previous_string) buffer += " ";
			//nobreak
		default:
			buffer += tok_->current_token().value();
			break;
		case token::QSTRING:
			buffer += tok_->current_token().value();
			break;
		case token::UNTERMINATED_QSTRING:
			error(_("Unterminated quoted string"));
			break;
		case token::LF:
			if (ignore_next_newlines) continue;
			//nobreak
		case token::END:
			goto finish;
		}

		previous_string = tok_->current_token().type == token::STRING;
		ignore_next_newlines = false;
	}

	finish:
	if (buffer.translatable())
		cfg[*curvar] = t_string(buffer);
	else
		cfg[*curvar] = buffer.token();
	if(validator_){
		validator_->validate_key (cfg, *curvar,buffer.value(), tok_->get_start_line (), tok_->get_file ()); }
	while (++curvar != variables.end()) {
		cfg[*curvar] = z_empty; }
}

/**
 * This function is crap. Don't use it on a string_map with prefixes.
 */
std::string parser::lineno_string(utils::token_map &i18n_symbols,
	std::string const &lineno, const char *error_string) {
	static const config::t_token & z_pos( generate_safe_static_const_t_interned(n_token::t_token("pos")) );

	i18n_symbols[z_pos] = ::lineno_string(lineno);
	std::string result = _(error_string);
	foreach(utils::token_map::value_type& var, i18n_symbols)
		boost::algorithm::replace_all(result, std::string("$") + (*var.first), std::string(var.second));
	return result;
}

void parser::error(const std::string& error_type)
{
	static const config::t_token & z_error( generate_safe_static_const_t_interned(n_token::t_token("error")) );
	static const config::t_token & z_value( generate_safe_static_const_t_interned(n_token::t_token("value")) );
#ifdef DEBUG
	static const config::t_token & z_previous_value( generate_safe_static_const_t_interned(n_token::t_token("previous_value")) );
#endif
	utils::token_map i18n_symbols;
	i18n_symbols[z_error] = error_type;
	i18n_symbols[z_value] = tok_->current_token().value();
	std::stringstream ss;
	ss << tok_->get_start_line() << " " << tok_->get_file();
#ifdef DEBUG
	i18n_symbols[z_previous_value] = tok_->previous_token().value();
	throw config::error(
		lineno_string(i18n_symbols, ss.str(),
		              N_("$error, value '$value', previous '$previous_value' at $pos")));
#else
	throw config::error(
		lineno_string(i18n_symbols, ss.str(),
		              N_("$error, value '$value' at $pos")));
#endif
}

} // end anon namespace

void read(config &cfg, std::istream &in, abstract_validator * validator)
{
	parser(cfg, in, validator)();
}

void read(config &cfg, std::string &in, abstract_validator * validator)
{
	std::istringstream ss(in);
	parser(cfg, ss, validator)();
}

void read_gz(config &cfg, std::istream &file, abstract_validator * validator)
{
	//an empty gzip file seems to confuse boost on msvc
	//so return early if this is the case
	if (file.peek() == EOF) {
		return;
	}
	boost::iostreams::filtering_stream<boost::iostreams::input> filter;
	filter.push(boost::iostreams::gzip_decompressor());
	filter.push(file);

	parser(cfg, filter,validator)();
}

static std::string escaped_string(const std::string &value)
{
	std::string res;
	std::string::const_iterator iter = value.begin();
	while (iter != value.end()) {
		const char c = *iter;
		res.append(c == '"' ? 2 : 1, c);
		++iter;
	}
	return res;
}

struct write_key_val_visitor : public config::attribute_value::default_visitor {
	//using default_visitor::operator();
	std::ostream &out_;
	unsigned level_;
	std::string &textdomain_;
	const std::string &key_;

	write_key_val_visitor(std::ostream &out, unsigned level,
		std::string &textdomain, const std::string &key)
		: out_(out), level_(level), textdomain_(textdomain), key_(key) {
}

	inline void write_start_not_tstring();

	void operator()()
	{ write_start_not_tstring(); out_ << "\"\""; }
	void operator()(bool b)
	{ write_start_not_tstring();  out_ << (b ? "yes" : "no"); }
	void operator()(int i)
	{ write_start_not_tstring();  out_ << i; }
	void operator()(double d)
	{ write_start_not_tstring(); int i = d; if (d == i) out_ << i; else out_ << d; }
	void operator()(config::t_token const &s)
	{ write_start_not_tstring();  out_ << '"' << escaped_string(*s) << '"'; }
	inline void operator()(t_string const &s) ;
};

/**
 * Writes all the parts of a translatable string.
 * @note If the first part is translatable and in the wrong textdomain,
 *       the textdomain change has to happen before the attribute name.
 *       That is the reason for not outputting the key beforehand and
 *       letting this function do it.
 */
static void write_key_val_tstring(std::ostream &out_, unsigned level_, std::string &textdomain_,
						   const std::string &key_, t_string const &value) {
	bool first = true;

	for (t_string::walker w(value); !w.eos(); w.next())
	{
		std::string part(w.begin(), w.end());

		if (!first)
			out_ << " +\n";

		if (w.translatable() && w.textdomain() != textdomain_) {
			textdomain_ = w.textdomain();
			out_ << "#textdomain " << textdomain_ << '\n';
		}

		for (unsigned i = 0; i < level_; ++i) out_ << '\t';

		if (first)
			out_ << key_ << '=';
		else
			out_ << '\t';

		if (w.translatable())
			out_ << '_';

		out_ << '"' << escaped_string(part) << '"';
		first = false;
	}
}
void write_key_val_visitor::operator()(t_string const &value) {
	write_key_val_tstring(out_,  level_, textdomain_, key_, value); }

void write_key_val_visitor::write_start_not_tstring(){
	for (unsigned i = 0; i < level_; ++i) out_ << '\t';
	out_ << key_ << '=';}

void write_key_val(std::ostream &out, const std::string &key,
	const config::attribute_value &value, unsigned level,
	std::string& textdomain)
{
	write_key_val_visitor visitor(out, level, textdomain, key);
	value.apply_visitor(visitor);
	out << '\n';
}

void write_open_child(std::ostream &out, const std::string &child, unsigned int level)
{
	out << std::string(level, '\t') << '[' << child << "]\n";
}

void write_close_child(std::ostream &out, const std::string &child, unsigned int level)
{
	out << std::string(level, '\t') << "[/" << child << "]\n";
}

static void write_internal(config const &cfg, std::ostream &out, std::string& textdomain, size_t tab = 0)
{
	if (tab > max_recursion_levels)
		throw config::error("Too many recursion levels in config write");

	foreach (const config::attribute &i, cfg.attribute_range()) {
		write_key_val(out, i.first, i.second, tab, textdomain);
	}

	foreach (const config::any_child &item, cfg.all_children_range())
	{
		write_open_child(out, item.key, tab);
		write_internal(item.cfg, out, textdomain, tab + 1);
		write_close_child(out, item.key, tab);
	}
}

/** The servers simple wml expects attributes to be ordered
	The client bears the cost of the simple_wml ordering expectations every time it sends data.
	@note This is a quick fix to allow the clients to ungrade their configs to unordered saving the log(n) lookup times, while
	not breaking wesnothd
	@todo a better solution would be to change the simple_table in server to an unordered_map/set and then
	both sides of the transaction are O(1)
	@todo an even better solution would be to have the server provide the clients with an integral alias to the string.
	This would 1. reduce network traffic and 2. be a direct bounds checked index into the servers vector LUT
 */
static void write_ordered_internal(config const &cfg, std::ostream &out, std::string& textdomain, size_t tab = 0)
{
	if (tab > max_recursion_levels)
		throw config::error("Too many recursion levels in config write");

	std::map<config::t_token, config::attribute_value> sorted;
	foreach (const config::attribute &i, cfg.attribute_range()) {
		sorted.insert( i ); }
	foreach (const config::attribute &i, sorted) {
		write_key_val(out, i.first, i.second, tab, textdomain);
	}

	foreach (const config::any_child &item, cfg.all_children_range())
	{
		write_open_child(out, item.key, tab);
		write_ordered_internal(item.cfg, out, textdomain, tab + 1);
		write_close_child(out, item.key, tab);
	}
}


void write(std::ostream &out, config const &cfg, unsigned int level)
{
	std::string textdomain = PACKAGE;
	///@todo remove _internal ASAP Sep10 2011.  I only did this to get multiplayer working without finding all the places that call this
	//write_internal(cfg, out, textdomain, level);
	write_ordered_internal(cfg, out, textdomain, level);
}

void write_ordered(std::ostream &out, config const &cfg, unsigned int level)
{
	std::string textdomain = PACKAGE;
	write_ordered_internal(cfg, out, textdomain, level);
}

void write_gz(std::ostream &out, config const &cfg)
{
	boost::iostreams::filtering_stream<boost::iostreams::output> filter;
	filter.push(boost::iostreams::gzip_compressor());
	filter.push(out);

	write(filter, cfg);
}
