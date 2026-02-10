/*
	Copyright (C) 2024 - 2025
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "serialization/markup.hpp"

#include "config.hpp"
#include "font/pango/escape.hpp"
#include "formatter.hpp"
#include "game_config.hpp"
#include "gettext.hpp"
#include "serialization/string_utils.hpp"
#include "serialization/unicode.hpp"
#include "serialization/unicode_cast.hpp"  // for unicode_cast
#include "utils/general.hpp"

#include <algorithm>
#include <sstream>

namespace markup {

std::string make_link(const std::string& text, const std::string& dst)
{
	// some sorting done on list of links may rely on the fact that text is first
	return formatter() << "<ref dst='" << dst << "'>" << text << "</ref>";
}

std::string img(const std::string& src, const std::string& align, bool floating)
{
	return formatter()
		<< "<img src='" << src << "' "
		<< "float='" << std::boolalpha << floating << "' "
		<< "align='" << align << "' "
		<< "/>";
}

//
// Markup Parser
//

/*

Here's a little mini-grammar of the markup language:

DOCUMENT ::= (TEXT | TAG)*
TEXT ::= ([^<&\] | ENTITY | ESCAPE)*
ESCAPE ::= '\' [:unicode-char:]
ENTITY ::= '&' '#' [0-9]+ ';'
ENTITY ::= '&' 'x' [0-9a-fA-F]+ ';'
ENTITY ::= '&' NAME ';'
TAG ::= '<' NAME ATTRIBUTE* '/' '>'
TAG ::= '<' NAME ATTRIBUTE* '>' DOCUMENT '<' '/' NAME '>' ## NB: the names must match!
TAG ::= '<' NAME '>' ATTRIBUTE* TEXT? '<' '/' NAME '>' ## NB: the names must match!
ATTRIBUTE ::= NAME
ATTRIBUTE ::= NAME '=' [^'" ]*
ATTRIBUTE ::= NAME '=' "'" TEXT "'"
ATTRIBUTE ::= NAME '=' '"' TEXT '"'
NAME ::= [_0-9a-zA-Z]+

Notes:
* Entities and the first two tag formats are Pango-style. The tags can be nested inside each other.
* Escapes and the third tag format are for compatibility with the old help markup. Tags cannot be nested.
* This mostly doesn't attempt to define the meaning of specific tags or entity names. It does however substitute numeric entities, as well as some very basic named entities: lt, gt, amp, quot, apos.
* The definition of TEXT is left a bit nebulous, but just think of it as "non-greedy"
* Attributes without a value are only supported in Pango-style tags
* Some restrictions may apply beyond what the grammar specifies. For example, arbitrary named entities are not supported in attribute values (numeric ones and the 5 special ones work though).

------

The result of the parsing is represented in the format of a WML config.
Text spans are represented as a [text] tag, and character entities as a [character_entity] tag.
All other tags are represented by a tag of the same name.
Any attributes on a tag become key-value pairs within the tag.
Old-style help markup tags with text at the end put the text in a "text" key in the tag.
The same approach is used for new-style Pango tags, but only if there are no nested tags or entities.
If there ARE nested tags or entities, the contents of the tag is broken down into spans as subtags of the parent tag.
Thus, a tag with content has EITHER a text attribute OR some subtags.

Note: Only unrecognized named entities count for the above purposes!
Numerical entities and the special five lt, gt, amp, apos, quot are directly substituted in-place.

Also, text spans will be broken up on paragraph breaks (double newlines).
This means that adjacent [text] tags should be rendered with a paragraph break between them.
However, no paragraph break should be used when [text] is followed by something else.
It is possible to have empty text spans in some cases, for example given a run of more than 2 newlines,
or a character entity directly followed by a paragraph break.

*/

namespace
{
using namespace std::string_literals;
const std::array old_style_tags{ "bold"s, "italic"s, "header"s, "format"s, "img"s, "ref"s, "jump"s };
const std::array old_style_attr{ /*ref*/ "dst"s, "text"s, "force"s, /*jump*/ "to"s, "amount"s, /*img*/ "src"s, "align"s, "float"s, /*format*/ "bold"s, "italic"s, "color"s, "font_size"s };
}

static std::string position_info(const std::string::const_iterator& text_start, const std::string::const_iterator& error_position)
{
	// line numbers start from 1
	int lines = std::count(text_start, error_position, '\n') + 1;
	// Find the start position of the line where the current position is.
	// We do this by searching in reverse from cursor_position toward text_start.
	auto pos = error_position;
	for(; pos != text_start && *pos != '\n'; pos--);
	return formatter()
		<< "line " << lines
		<< ", character " << utf8::size(pos, error_position);
}

static config parse_entity(std::string::const_iterator& beg, std::string::const_iterator end)
{
	config entity;
	std::stringstream s;
	enum { UNKNOWN, NAMED, HEX, DECIMAL } type = UNKNOWN;
	assert(*beg == '&');
	++beg;
	for(; beg != end && *beg != ';'; ++beg) {
		switch(type) {
		case UNKNOWN:
			if(*beg == '#') {
				type = DECIMAL;
			} else if(isalnum(*beg) || *beg == '_') {
				type = NAMED;
				s << *beg;
			} else {
				throw parse_error(beg, "invalid entity: unexpected characters after '&', alphanumeric characters, '#' or '_' expected.");
			}
			break;
		case NAMED:
			if(!isalnum(*beg)) {
				throw parse_error(beg, "invalid entity: non-alphanumeric characters after '&'.");
			}
			s << *beg;
			break;
		case DECIMAL:
			if(*beg == 'x') {
				type = HEX;
			} else if(isdigit(*beg)) {
				s << *beg;
			} else {
				throw parse_error(beg, "invalid entity: unexpected characters after '&#', numbers or 'x' expected.");
			}
			break;
		case HEX:
			if(isxdigit(*beg)) {
				s << *beg;
			} else {
				throw parse_error(beg, "invalid entity: unexpected characters after '&#x', hexadecimal digits expected.");
			}
			break;
		}
	}
	if(type == NAMED) {
		std::string name = s.str();
		entity["name"] = name;
		if(name == "lt") {
			entity["code_point"] = '<';
		} else if(name == "gt") {
			entity["code_point"] = '>';
		} else if(name == "apos") {
			entity["code_point"] = '\'';
		} else if(name == "quot") {
			entity["code_point"] = '"';
		} else if(name == "amp") {
			entity["code_point"] = '&';
		}
	} else {
		s.seekg(0);
		if(type == HEX) {
			s >> std::hex;
		}
		int n;
		s >> n;
		entity["code_point"] = n;
	}
	return entity;
}

static char parse_escape(std::string::const_iterator& beg, std::string::const_iterator end)
{
	assert(*beg == '\\');
	// An escape at the end of stream is just treated as a literal.
	// Otherwise, take the next character as a literal and be done with it.
	if((beg + 1) != end) {
		++beg;
	}
	return *beg;
}

static config parse_text_until(std::string::const_iterator& beg, std::string::const_iterator end, char close)
{
	// In practice, close will be one of < ' "
	// Parsing will go until either close or end of stream, and will emit one or more text and character_entity tags.
	// However, recognized character entities will be collapsed into the text tags.
	std::ostringstream s;
	bool saw_newline = false;
	config res;
	for(; beg != end && *beg != close; ++beg) {
		if(*beg == '&') {
			auto entity = parse_entity(beg, end);
			if(beg == end) {
				throw parse_error(beg, "unexpected end of stream after entity");
			}
			if(entity.has_attribute("code_point")) {
				s << unicode_cast<std::string>(entity["code_point"].to_int());
			} else {
				// TODO: Adding the text here seems wrong in the case that the stream BEGINS with an entity...
				res.add_child("text", config("text", s.str()));
				res.add_child("character_entity", entity);
				s.str("");
			}
		} else if(*beg == '\\') {
			s << parse_escape(beg, end);
		} else if(*beg == '\n') {
			if(saw_newline) {
				res.add_child("text", config("text", s.str()));
				s.str("");
			} else {
				saw_newline = true;
				continue;
			}
		} else {
			if(saw_newline) {
				s << '\n';
			}
			s << *beg;
		}
		saw_newline = false;
	}
	// If the span ended in a newline, preserve it
	if(saw_newline) {
		s << '\n';
	}
	res.add_child("text", config("text", s.str()));
	assert(beg == end || *beg == close);
	return res;
}

static std::string parse_name(std::string::const_iterator& beg, std::string::const_iterator end)
{
	std::ostringstream s;
	for(; beg != end && (isalnum(*beg) || *beg == '_'); ++beg) {
		s << *beg;
	}
	return s.str();
}

static std::pair<std::string, std::string> parse_attribute(std::string::const_iterator& beg, std::string::const_iterator end, bool old_style)
{
	std::string attr = parse_name(beg, end);
	if(attr.empty()) {
		throw parse_error(beg, "missing attribute name");
	}
	if(old_style && !utils::contains(old_style_attr, attr)) {
		throw parse_error(beg, "dummy error: not an old-style attribute name"); // old-style=true caller ignores parse errors
	}

	while(isspace(*beg)) ++beg;

	if(*beg != '=') {
		if(old_style) {
			throw parse_error(beg, "attribute missing value in old-style tag");
		} else {
			// The caller expects beg to point to the last character of the attribute upon return.
			// But in this path, we're now pointing to the character AFTER that.
			--beg;
			return {attr, ""};
		}
	}
	++beg;
	while(isspace(*beg)) ++beg;

	std::string value;
	if(*beg == '\'' || *beg == '"') {
		config res = parse_text_until(beg, end, *beg++);
		if(res.has_child("character_entity")) {
			throw parse_error(beg, "unsupported entity in attribute value");
		} else if(res.all_children_count() > 1) {
			throw parse_error(beg, "paragraph break in attribute value");
		}
		if(auto t = res.optional_child("text")) {
			value = t["text"].str();
		}
	} else {
		std::ostringstream s;
		bool found_slash = false;
		for(; beg != end && *beg != '>' && *beg != '<' && !isspace(*beg); ++beg) {
			if(*beg == '&') {
				auto entity = parse_entity(beg, end);
				if(beg == end) {
					throw parse_error(beg, "unexpected end of stream after entity");
				}
				if(entity.has_attribute("code_point")) {
					s << unicode_cast<std::string>(entity["code_point"].to_int());
				} else {
					throw parse_error(beg, "unsupported entity in attribute value");
				}
			} else if(*beg == '\\') {
				s << parse_escape(beg, end);
			} else if(*beg == '/') {
				found_slash = true;
			} else {
				if(found_slash) {
					s << '/';
					found_slash = false;
				}
				s << *beg;
			}
		}
		value = s.str();
		// The caller expects beg to point to the last character of the attribute upon return.
		// But in this path, we're now pointing to the character AFTER that.
		--beg;
		if(found_slash) --beg;
	}
	return {attr, value};
}

static void check_closing_tag(std::string::const_iterator& beg, std::string::const_iterator end, std::string_view tag_name)
{
	std::size_t remaining = end - beg;
	assert(remaining >= 2 && *beg == '<' && *(beg + 1) == '/');
	if(remaining < tag_name.size() + 3) {
		throw parse_error(beg, "Unexpected end of stream in closing tag");
	}
	beg += 2;
	if(!std::equal(tag_name.begin(), tag_name.end(), beg)) {
		throw parse_error(beg, "Mismatched closing tag " + std::string(tag_name));
	}
	beg += tag_name.size();
	if(*beg != '>') {
		throw parse_error(beg, "Unterminated closing tag " + std::string(tag_name));
	}
	++beg;
}

static std::pair<std::string, config> parse_tag(std::string::const_iterator& beg, std::string::const_iterator end);
static config parse_tag_contents(std::string::const_iterator& beg, std::string::const_iterator end, std::string_view tag_name, bool check_for_attributes)
{
	assert(*beg == '>');
	++beg;

	if(!utils::contains(old_style_tags, tag_name)) {
		check_for_attributes = false;
	}

	// This also parses the matching closing tag!
	config res;
	for(; check_for_attributes && beg != end && *beg != '<'; ++beg) {
		if(isspace(*beg)) continue;
		auto save_beg = beg;
		try {
			auto [key, val] = parse_attribute(beg, end, true);
			res[key] = val;
		} catch(parse_error&) {
			beg = save_beg;
			while(beg != end && isspace(*beg)) ++beg;
			break;
		}
	}
	if(res.has_attribute("text")) {
		if(beg == end || *beg != '<' || (beg + 1) == end || *(beg + 1) != '/') {
			throw parse_error(beg, "Extra text at the end of old-style tag with explicit 'text' attribute");
		}
		check_closing_tag(beg, end, tag_name);
		return res;
	} else if(res.attribute_count() > 0) {
		config text = parse_text_until(beg, end, '<');
		if(beg == end || *beg != '<' || (beg + 1) == end || *(beg + 1) != '/') {
			throw parse_error(beg, "Extra text at the end of old-style tag with explicit 'text' attribute");
		}
		if(text.all_children_count() == 1 && text.has_child("text")) {
			res["text"] = text.mandatory_child("text")["text"];
		} else {
			res.append_children(text);
		}
		check_closing_tag(beg, end, tag_name);
		return res;
	}
	while(true) {
		config text = parse_text_until(beg, end, '<');
		if(beg == end || beg + 1 == end) {
			throw parse_error(beg, "Missing closing tag for " + std::string(tag_name));
		}
		res.append_children(text);
		if(*(beg + 1) == '/') {
			check_closing_tag(beg, end, tag_name);
			break;
		}
		auto [tag, contents] = parse_tag(beg, end);
		res.add_child(tag, contents);
	}
	if(res.all_children_count() == 1 && res.has_child("text")) {
		return res.mandatory_child("text");
	}
	return res;
}

static std::pair<std::string, config> parse_tag(std::string::const_iterator& beg, std::string::const_iterator end)
{
	assert(*beg == '<');
	++beg;
	std::string tag_name = parse_name(beg, end);
	if(tag_name.empty()) {
		throw parse_error(beg, "missing tag name");
	}
	bool auto_closed = false;
	config elem;
	for(; beg != end && *beg != '>'; ++beg) {
		if(isspace(*beg)) continue;
		if(*beg == '/' && (beg + 1) != end && *(beg + 1) == '>') {
			auto_closed = true;
		} else if(isalnum(*beg) || *beg == '_') {
			const auto& [key, value] = parse_attribute(beg, end, false);
			if(beg == end) {
				throw parse_error(beg, "unexpected end of stream following attribute");
			}
			elem[key] = value;
		}
	}
	if(auto_closed) {
		assert(*beg == '>');
		++beg;
	} else {
		config contents = parse_tag_contents(beg, end, tag_name, elem.attribute_count() == 0);
		if(contents.all_children_count() == 0 && contents.attribute_count() == 1 && contents.has_attribute("text")) {
			elem["text"] = contents["text"];
		} else {
			elem.append(contents);
		}
	}
	return {tag_name, elem};
}

config parse_text(const std::string& text)
{
	config res;
	auto beg = text.begin(), end = text.end();
	try {
		while(beg != end) {
			if(*beg == '<') {
				auto [tag, contents] = parse_tag(beg, end);
				res.add_child(tag, contents);
			} else {
				config text = parse_text_until(beg, end, '<');
				res.append_children(text);
			}
		}
	} catch(parse_error& e) {
		// NOTE: The text.begin() itor is in scope here, so we add the error location info
		// to the error message here and rethrow. Both itors used in the call to
		// position_info below can go out of scope otherwise.
		e.message = position_info(text.begin(), e.error_location()) + ": " + e.message;
		throw e;
	}
	return res;
}

static std::string config_to_pango_markup(const std::string& orig_tagname, const config& cfg) {
	std::stringstream text;

	std::string tagname;
	if (orig_tagname == "bold" || orig_tagname == "b") {
		tagname = "b";
	} else if (orig_tagname == "italic" || orig_tagname == "i") {
		tagname = "i";
	} else if (orig_tagname == "underline" || orig_tagname == "u" || orig_tagname == "ref") {
		tagname = "u"; // <ref> tags will be shown as pango underline
	} else if (orig_tagname == "format" || orig_tagname == "header" || orig_tagname == "h") {
		tagname = "span";
	} else if (orig_tagname == "img") {
		return ""; // ignore
	} else if (orig_tagname == "table") {
		text << "\n";
		for (const auto& row : cfg.child_range("row")) {
			for (const auto& cell : row.child_range("col")) {
				text << config_to_pango_markup("col", cell) << " ";
			}
			text << "\n";
		}
		return text.str();
	} else {
		// Anything that does not match any preceding if blocks
		// gets its inner text extracted and returned, no special handling.
		// This also handles plain text [text] blocks.
		return font::escape_text(cfg["text"].str());
	}

	// Inner text content
	if(cfg.has_attribute("text")) {
		text << font::escape_text(cfg["text"].str());
	}

	// Tag specific formatting attributes
	tag_attributes attrs;
	if (orig_tagname == "span" || orig_tagname == "format") {
		for(const auto& [key, val] : cfg.attribute_range()) {
			attrs.emplace_back(key, val.str());
		}
	} else if (orig_tagname == "header" || orig_tagname == "h") {
		attrs.emplace_back("weight", "heavy");
		attrs.emplace_back("color", "white");
		attrs.emplace_back("size", "large");
	}

	// Nested tags
	for(const auto& [tagname, cfg] : cfg.all_children_view()) {
		text << config_to_pango_markup(tagname, cfg);
	}

	return markup::tag_attr(tagname, attrs, text.str());
}

std::string help_to_pango_markup(const std::string& help_markup) {
	const config& help_cfg = parse_text(help_markup);
	std::stringstream pango_text;
	std::string prev_tagname = "";
	for(const auto& [tagname, child_cfg] : help_cfg.all_children_view()) {
		// Two consecutive [text] blocks mean two paragraphs of text.
		// Reinserting the paragraph break.
		if(prev_tagname == "text" && tagname == "text") {
			pango_text << "\n";
		}

		pango_text << config_to_pango_markup(tagname, child_cfg);

		prev_tagname = tagname;
	}
	return pango_text.str();
}

}
