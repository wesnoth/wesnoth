/*
	Copyright (C) 2008 - 2024
	by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include <sstream>

#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/bzip2.hpp>
#include <boost/iostreams/filter/counter.hpp>
#include <boost/iostreams/filter/gzip.hpp>

#include "server/common/simple_wml.hpp"

#include "log.hpp"
#include "utils/general.hpp"

static lg::log_domain log_config("config");
#define ERR_SWML LOG_STREAM(err, log_config)
#define LOG_SWML LOG_STREAM(info, log_config)

namespace simple_wml {

std::size_t document::document_size_limit = 40000000;

namespace {

void debug_delete(node* n) {
	delete n;
}

char* uncompress_buffer(const string_span& input, string_span* span)
{
	int nalloc = input.size();
	int state = 0;
	try {
		std::istringstream stream(std::string(input.begin(), input.end()));
		state = 1;
		boost::iostreams::filtering_stream<boost::iostreams::input> filter;
		state = 2;
		if (!span->empty() && *span->begin() == 'B') {
			filter.push(boost::iostreams::bzip2_decompressor());
		} else {
			filter.push(boost::iostreams::gzip_decompressor());
		}
		filter.push(stream);
		state = 3;

		const std::size_t chunk_size = input.size() * 10;
		nalloc = chunk_size;
		std::vector<char> buf(chunk_size);
		state = 4;
		std::size_t len = 0;
		std::size_t pos = 0;
		while(filter.good() && (len = filter.read(&buf[pos], chunk_size).gcount()) == chunk_size) {
			if(pos + chunk_size > document::document_size_limit) {
				throw error("WML document exceeded size limit during decompression");
			}

			pos += len;
			buf.resize(pos + chunk_size);
			len = 0;
		}

		if(!filter.eof() && !filter.good()) {
			throw error("failed to uncompress");
		}

		pos += len;
		state = 5;
		nalloc = pos;

		buf.resize(pos);
		state = 6;

		char* small_out = new char[pos+1];
		memcpy(small_out, &buf[0], pos);
		state = 7;

		small_out[pos] = 0;

		*span = string_span(small_out, pos);
		state = 8;
		return small_out;
	} catch (const std::bad_alloc& e) {
		ERR_SWML << "ERROR: bad_alloc caught in uncompress_buffer() state "
		<< state << " alloc bytes " << nalloc << " with input: '"
		<< input << "' " << e.what();
		throw error("Bad allocation request in uncompress_buffer().");
	}
}

char* compress_buffer(const char* input, string_span* span, bool bzip2)
{
	int nalloc = strlen(input);
	int state = 0;
	try {
		std::string in(input);
		state = 1;
		std::istringstream istream(in);
		state = 2;
		boost::iostreams::filtering_stream<boost::iostreams::output> filter;
		state = 3;
		if (bzip2) {
			filter.push(boost::iostreams::bzip2_compressor());
		} else {
			filter.push(boost::iostreams::gzip_compressor());
		}
		state = 4;
		nalloc = in.size()*2 + 80;
		std::vector<char> buf(nalloc);
		boost::iostreams::array_sink out(&buf[0], buf.size());
		filter.push(boost::iostreams::counter());
		filter.push(out);

		state = 5;

		boost::iostreams::copy(istream, filter, buf.size());
		const int len = filter.component<boost::iostreams::counter>(1)->characters();
		assert(len < 128*1024*1024);
		if((!filter.eof() && !filter.good()) || len == static_cast<int>(buf.size())) {
			throw error("failed to compress");
		}
		state = 6;
		nalloc = len;

		buf.resize(len);
		state = 7;

		char* small_out = new char[len];
		memcpy(small_out, &buf[0], len);
		state = 8;

		*span = string_span(small_out, len);
		assert(*small_out == (bzip2 ? 'B' : 31));
		state = 9;
		return small_out;
	} catch (const std::bad_alloc& e) {
		ERR_SWML << "ERROR: bad_alloc caught in compress_buffer() state "
		<< state << " alloc bytes " << nalloc << " with input: '"
		<< input << "' " << e.what();
		throw error("Bad allocation request in compress_buffer().");
	}
}

}  // namespace

bool string_span::to_bool(bool default_value) const
{
	if(empty()) {
		return default_value;
	}

	if (operator==("no") || operator==("off") || operator==("false") || operator==("0") || operator==("0.0"))
		return false;

	return true;
}

int string_span::to_int() const
{
	const int buf_size = 64;
	if(size() >= buf_size) {
		return 0;
	}
	char buf[64];
	memcpy(buf, begin(), size());
	buf[size()] = 0;
	return atoi(buf);
}

std::string string_span::to_string() const
{
	return std::string(begin(), end());
}

char* string_span::duplicate() const
{
	char* buf = new char[size() + 1];
	memcpy(buf, begin(), size());
	buf[size()] = 0;
	return buf;
}

error::error(const char* msg)
  : game::error(msg)
{
	ERR_SWML << "ERROR: '" << msg << "'";
}

std::ostream& operator<<(std::ostream& o, const string_span& s)
{
	o << std::string(s.begin(), s.end());
	return o;
}

node::node(document& doc, node* parent) :
	doc_(&doc),
	attr_(),
	parent_(parent),
	children_(),
	ordered_children_(),
	output_cache_()
{
}

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4706)
#endif
node::node(document& doc, node* parent, const char** str, int depth) :
	doc_(&doc),
	attr_(),
	parent_(parent),
	children_(),
	ordered_children_(),
	output_cache_()
{
	if(depth >= 1000) {
		throw error("elements nested too deep");
	}

	const char*& s = *str;

	const char* const begin = s;
	while(*s) {
		switch(*s) {
		case '[': {
			if(s[1] == '/') {
				output_cache_ = string_span(begin, s - begin);
				s = strchr(s, ']');
				if(s == nullptr) {
					throw error("end element unterminated");
				}

				++s;
				return;
			}

			++s;
			const char* end = strchr(s, ']');
			if(end == nullptr) {
				throw error("unterminated element");
			}

			const int list_index = get_children(string_span(s, end - s));
			check_ordered_children();

			s = end + 1;

			children_[list_index].second.push_back(new node(doc, this, str, depth+1));
			ordered_children_.emplace_back(list_index, children_[list_index].second.size() - 1);
			check_ordered_children();

			break;
		}
		case ' ':
		case '\t':
		case '\n':
			++s;
			break;
		case '#':
			s = strchr(s, '\n');
			if(s == nullptr) {
				throw error("did not find newline after '#'");
			}
			break;
		default: {
			const char* end = strchr(s, '=');
			if(end == nullptr) {
				ERR_SWML << "attribute: " << s;
				throw error("did not find '=' after attribute");
			}

			string_span name(s, end - s);
			s = end + 1;
			if(*s == '_') {
				s = strchr(s, '"');
				if(s == nullptr) {
					throw error("did not find '\"' after '_'");
				}
			}

			if (*s != '"') {
				end = strchr(s, '\n');
				if (!end) {
					ERR_SWML << "ATTR: '" << name << "' (((" << s << ")))";
					throw error("did not find end of attribute");
				}
				if (memchr(s, '"', end - s))
					throw error("found stray quotes in unquoted value");
				goto read_attribute;
			}
			end = s;
			while(true)
			{
				// Read until the first single double quote.
				while((end = strchr(end+1, '"')) && end[1] == '"') {
#ifdef _MSC_VER
#pragma warning (pop)
#endif
					++end;
				}
				if(end == nullptr)
					throw error("did not find end of attribute");

				// Stop if newline.
				const char *endline = end + 1;
				while (*endline == ' ') ++endline;
				if (*endline == '\n') break;

				// Read concatenation marker.
				if (*(endline++) != '+')
					throw error("did not find newline after end of attribute");
				if (*(endline++) != '\n')
					throw error("did not find newline after '+'");

				// Read textdomain marker.
				if (*endline == '#') {
					endline = strchr(endline + 1, '\n');
					if (!endline)
						throw error("did not find newline after '#'");
					++endline;
				}

				// Read indentation and start of string.
				while (*endline == '\t') ++endline;
				if (*endline == '_') ++endline;
				if (*endline != '"')
					throw error("did not find quotes after '+'");
				end = endline;
			}

			++s;

			read_attribute:
			string_span value(s, end - s);
			if(attr_.empty() == false && !(attr_.back().key < name)) {
				ERR_SWML << "attributes: '" << attr_.back().key << "' < '" << name << "'";
				throw error("attributes not in order");
			}

			s = end + 1;

			attr_.emplace_back(name, value);
		}
		}
	}

	output_cache_ = string_span(begin, s - begin);
	check_ordered_children();
}

node::~node()
{
	for(child_map::iterator i = children_.begin(); i != children_.end(); ++i) {
		for(child_list::iterator j = i->second.begin(); j != i->second.end(); ++j) {
			debug_delete(*j);
		}
	}
}

namespace {
struct string_span_pair_comparer
{
	bool operator()(const string_span& a, const node::attribute& b) const {
		return a < b.key;
	}

	bool operator()(const node::attribute& a, const string_span& b) const {
		return a.key < b;
	}

	bool operator()(const node::attribute& a,
	                const node::attribute& b) const {
		return a.key < b.key;
	}
};
}

const string_span& node::operator[](const char* key) const
{
	static string_span empty("");
	string_span span(key);
	std::pair<attribute_list::const_iterator,
	          attribute_list::const_iterator> range = std::equal_range(attr_.begin(), attr_.end(), span, string_span_pair_comparer());
	if(range.first != range.second) {
		return range.first->value;
	}

	return empty;
}

bool node::has_attr(const char* key) const
{
	string_span span(key);
	std::pair<attribute_list::const_iterator,
	          attribute_list::const_iterator> range = std::equal_range(attr_.begin(), attr_.end(), span, string_span_pair_comparer());
	return range.first != range.second;
}

node& node::set_attr(const char* key, const char* value)
{
	set_dirty();

	string_span span(key);
	std::pair<attribute_list::iterator,
	          attribute_list::iterator> range = std::equal_range(attr_.begin(), attr_.end(), span, string_span_pair_comparer());
	if(range.first != range.second) {
		range.first->value = string_span(value);
	} else {
		attr_.insert(range.first, attribute(span, string_span(value)));
	}

	return *this;
}

node& node::set_attr_dup(const char* key, const char* value)
{
	return set_attr(key, doc_->dup_string(value));
}

node& node::set_attr_dup(const char* key, const string_span& value)
{
	char* buf = value.duplicate();
	doc_->take_ownership_of_buffer(buf);
	return set_attr(key, buf);
}

node& node::set_attr_int(const char* key, int value)
{
	std::string temp = std::to_string(value);
	return set_attr_dup(key, temp.c_str());
}

node& node::add_child_at(const char* name, std::size_t index)
{
	set_dirty();

	const int list_index = get_children(name);
	child_list& list = children_[list_index].second;
	if(index > list.size()) {
		index = list.size();
	}

	check_ordered_children();
	list.insert(list.begin() + index, new node(*doc_, this));
	insert_ordered_child(list_index, index);

	check_ordered_children();
	return *list[index];
}


node& node::add_child(const char* name)
{
	set_dirty();

	const int list_index = get_children(name);
	check_ordered_children();
	child_list& list = children_[list_index].second;
	list.push_back(new node(*doc_, this));
	ordered_children_.emplace_back(list_index, list.size() - 1);
	check_ordered_children();
	return *list.back();
}

void node::remove_child(const string_span& name, std::size_t index)
{
	set_dirty();

	//if we don't already have a vector for this item we don't want to add one.
	child_map::iterator itor = find_in_map(children_, name);
	if(itor == children_.end()) {
		return;
	}

	child_list& list = itor->second;
	if(index >= list.size()) {
		return;
	}

	remove_ordered_child(std::distance(children_.begin(), itor), index);

	debug_delete(list[index]);
	list.erase(list.begin() + index);

	if(list.empty()) {
		remove_ordered_child_list(std::distance(children_.begin(), itor));
		children_.erase(itor);
	}
}

void node::insert_ordered_child(int child_map_index, int child_list_index)
{
	bool inserted = false;
	std::vector<node_pos>::iterator i = ordered_children_.begin();
	while(i != ordered_children_.end()) {
		if(i->child_map_index == child_map_index && i->child_list_index > child_list_index) {
			i->child_list_index++;
		} else if(i->child_map_index == child_map_index && i->child_list_index == child_list_index) {
			inserted = true;
			i->child_list_index++;
			i = ordered_children_.insert(i, node_pos(child_map_index, child_list_index));
			++i;
		}

		++i;
	}

	if(!inserted) {
		ordered_children_.emplace_back(child_map_index, child_list_index);
	}
}

void node::remove_ordered_child(int child_map_index, int child_list_index)
{
	int erase_count = 0;
	std::vector<node_pos>::iterator i = ordered_children_.begin();
	while(i != ordered_children_.end()) {
		if(i->child_map_index == child_map_index && i->child_list_index == child_list_index) {
			i = ordered_children_.erase(i);
			++erase_count;
		} else {
			if(i->child_map_index == child_map_index && i->child_list_index > child_list_index) {
				i->child_list_index--;
			}
			++i;
		}
	}

	assert(erase_count == 1);
}

void node::insert_ordered_child_list(int child_map_index)
{
	std::vector<node_pos>::iterator i = ordered_children_.begin();
	while(i != ordered_children_.end()) {
		if(i->child_map_index >= child_map_index) {
			i->child_map_index++;
		}
	}
}

void node::remove_ordered_child_list(int child_map_index)
{
	std::vector<node_pos>::iterator i = ordered_children_.begin();
	while(i != ordered_children_.end()) {
		if(i->child_map_index == child_map_index) {
			assert(false);
			i = ordered_children_.erase(i);
		} else {
			if(i->child_map_index > child_map_index) {
				i->child_map_index--;
			}

			++i;
		}
	}
}

void node::check_ordered_children() const
{
// only define this symbol in debug mode to work out child ordering.
#ifdef CHECK_ORDERED_CHILDREN
	std::vector<node_pos>::const_iterator i = ordered_children_.begin();
	while(i != ordered_children_.end()) {
		assert(i->child_map_index < children_.size());
		assert(i->child_list_index < children_[i->child_map_index].second.size());
		++i;
	}

	for(child_map::const_iterator j = children_.begin(); j != children_.end(); ++j) {
		const unsigned short child_map_index = j - children_.begin();
		for(child_list::const_iterator k = j->second.begin(); k != j->second.end(); ++k) {
			const unsigned short child_list_index = k - j->second.begin();
			bool found = false;
			for(int n = 0; n != ordered_children_.size(); ++n) {
				if(ordered_children_[n].child_map_index == child_map_index &&
				   ordered_children_[n].child_list_index == child_list_index) {
					found = true;
					break;
				}
			}

			assert(found);
		}
	}
#endif // CHECK_ORDERED_CHILDREN
}

void node::remove_child(const char* name, std::size_t index)
{
	remove_child(string_span(name), index);
}

node* node::child(const char* name)
{
	for(child_map::iterator i = children_.begin(); i != children_.end(); ++i) {
		if(i->first == name) {
			assert(i->second.empty() == false);
			return i->second.front();
		}
	}

	return nullptr;
}

const node* node::child(const char* name) const
{
	for(child_map::const_iterator i = children_.begin(); i != children_.end(); ++i) {
		if(i->first == name) {
			if(i->second.empty()) {
				return nullptr;
			} else {
				return i->second.front();
			}
		}
	}

	return nullptr;
}

node& node::child_or_add(const char* name)
{
	if(node* res = child(name)) {
		return *res;
	}
	return add_child(name);
}

const node::child_list& node::children(const char* name) const
{
	for(child_map::const_iterator i = children_.begin(); i != children_.end(); ++i) {
		if(i->first == name) {
			return i->second;
		}
	}

	static const node::child_list empty;
	return empty;
}

int node::get_children(const char* name)
{
	return get_children(string_span(name));
}

int node::get_children(const string_span& name)
{
	for(child_map::iterator i = children_.begin(); i != children_.end(); ++i) {
		if(i->first == name) {
			return std::distance(children_.begin(), i);
		}
	}

	children_.emplace_back(string_span(name), child_list());
	return children_.size() - 1;
}

node::child_map::const_iterator node::find_in_map(const child_map& m, const string_span& attr)
{
	child_map::const_iterator i = m.begin();
	for(; i != m.end(); ++i) {
		if(i->first == attr) {
			break;
		}
	}

	return i;
}

node::child_map::iterator node::find_in_map(child_map& m, const string_span& attr)
{
	child_map::iterator i = m.begin();
	for(; i != m.end(); ++i) {
		if(i->first == attr) {
			break;
		}
	}

	return i;
}

const string_span& node::first_child() const
{
	if(children_.empty()) {
		static const string_span empty;
		return empty;
	}

	return children_.begin()->first;
}

int node::output_size() const
{
	check_ordered_children();
	if(output_cache_.empty() == false) {
		return output_cache_.size();
	}

	int res = 0;
	for(attribute_list::const_iterator i = attr_.begin(); i != attr_.end(); ++i) {
		res += i->key.size() + i->value.size() + 4;
	}

	std::size_t count_children = 0;
	for(child_map::const_iterator i = children_.begin(); i != children_.end(); ++i) {
		for(child_list::const_iterator j = i->second.begin(); j != i->second.end(); ++j) {
			res += i->first.size()*2 + 7;
			res += (*j)->output_size();
			++count_children;
		}
	}

	assert(count_children == ordered_children_.size());

	return res;
}

void node::shift_buffers(ptrdiff_t offset)
{
	if(!output_cache_.empty()) {
		output_cache_ = string_span(output_cache_.begin() + offset, output_cache_.size());
	}

	for(std::vector<attribute>::iterator i = attr_.begin(); i != attr_.end(); ++i) {
		i->key = string_span(i->key.begin() + offset, i->key.size());
		i->value = string_span(i->value.begin() + offset, i->value.size());
	}

	for(child_map::iterator i = children_.begin(); i != children_.end(); ++i) {
		string_span& key = i->first;
		key = string_span(key.begin() + offset, key.size());
		for(child_list::iterator j = i->second.begin(); j != i->second.end(); ++j) {
			(*j)->shift_buffers(offset);
		}
	}
}

void node::output(char*& buf, CACHE_STATUS cache_status)
{
	if(output_cache_.empty() == false) {
		memcpy(buf, output_cache_.begin(), output_cache_.size());
		if(cache_status == REFRESH_CACHE) {
			shift_buffers(buf - output_cache_.begin());
		}
		buf += output_cache_.size();
		return;
	}

	char* begin = buf;

	for(std::vector<attribute>::iterator i = attr_.begin(); i != attr_.end(); ++i) {
		memcpy(buf, i->key.begin(), i->key.size());
		if(cache_status == REFRESH_CACHE) {
			i->key = string_span(buf, i->key.size());
		}
		buf += i->key.size();
		*buf++ = '=';
		*buf++ = '"';
		memcpy(buf, i->value.begin(), i->value.size());
		if(cache_status == REFRESH_CACHE) {
			i->value = string_span(buf, i->value.size());
		}
		buf += i->value.size();
		*buf++ = '"';
		*buf++ = '\n';
	}

	for(std::vector<node_pos>::const_iterator i = ordered_children_.begin();
	    i != ordered_children_.end(); ++i) {
		assert(i->child_map_index < children_.size());
		assert(i->child_list_index < children_[i->child_map_index].second.size());
		string_span& attr = children_[i->child_map_index].first;
		*buf++ = '[';
		memcpy(buf, attr.begin(), attr.size());
		if(cache_status == REFRESH_CACHE) {
			attr = string_span(buf, attr.size());
		}
		buf += attr.size();
		*buf++ = ']';
		*buf++ = '\n';
		children_[i->child_map_index].second[i->child_list_index]->output(buf, cache_status);
		*buf++ = '[';
		*buf++ = '/';
		memcpy(buf, attr.begin(), attr.size());
		buf += attr.size();
		*buf++ = ']';
		*buf++ = '\n';
	}

	if(cache_status == REFRESH_CACHE) {
		output_cache_ = string_span(begin, buf - begin);
	}
}

std::string node_to_string(const node& n)
{
	//calling output with status=DO_NOT_MODIFY_CACHE really doesn't modify the
	//node, so we can do it safely
	node& mutable_node = const_cast<node&>(n);
	std::vector<char> v(mutable_node.output_size());
	char* ptr = &v[0];
	mutable_node.output(ptr, node::DO_NOT_MODIFY_CACHE);
	assert(ptr == &v[0] + v.size());
	return std::string(v.begin(), v.end());
}

void node::copy_into(node& n) const
{
	n.set_dirty();
	for(attribute_list::const_iterator i = attr_.begin(); i != attr_.end(); ++i) {
		char* key = i->key.duplicate();
		char* value = i->value.duplicate();
		n.doc_->take_ownership_of_buffer(key);
		n.doc_->take_ownership_of_buffer(value);
		n.set_attr(key, value);
	}

	for(std::vector<node_pos>::const_iterator i = ordered_children_.begin();
	    i != ordered_children_.end(); ++i) {
		assert(i->child_map_index < children_.size());
		assert(i->child_list_index < children_[i->child_map_index].second.size());
		char* buf = children_[i->child_map_index].first.duplicate();
		n.doc_->take_ownership_of_buffer(buf);
		children_[i->child_map_index].second[i->child_list_index]->copy_into(n.add_child(buf));
	}
}

void node::apply_diff(const node& diff)
{
	set_dirty();
	const node* inserts = diff.child("insert");
	if(inserts != nullptr) {
		for(attribute_list::const_iterator i = inserts->attr_.begin(); i != inserts->attr_.end(); ++i) {
			char* name = i->key.duplicate();
			char* value = i->value.duplicate();
			set_attr(name, value);
			doc_->take_ownership_of_buffer(name);
			doc_->take_ownership_of_buffer(value);
		}
	}

	const node* deletes = diff.child("delete");
	if(deletes != nullptr) {
		for(attribute_list::const_iterator i = deletes->attr_.begin(); i != deletes->attr_.end(); ++i) {
			std::pair<attribute_list::iterator,
	                  attribute_list::iterator> range = std::equal_range(attr_.begin(), attr_.end(), i->key, string_span_pair_comparer());
			if(range.first != range.second) {
				attr_.erase(range.first);
			}
		}
	}

	const child_list& child_changes = diff.children("change_child");
	for(child_list::const_iterator i = child_changes.begin(); i != child_changes.end(); ++i) {
		const std::size_t index = (**i)["index"].to_int();
		for(child_map::const_iterator j = (*i)->children_.begin(); j != (*i)->children_.end(); ++j) {
			const string_span& name = j->first;
			for(child_list::const_iterator k = j->second.begin(); k != j->second.end(); ++k) {
				child_map::iterator itor = find_in_map(children_, name);
				if(itor != children_.end()) {
					if(index < itor->second.size()) {
						itor->second[index]->apply_diff(**k);
					}
				}
			}
		}
	}

	const child_list& child_inserts = diff.children("insert_child");
	for(child_list::const_iterator i = child_inserts.begin(); i != child_inserts.end(); ++i) {
		const std::size_t index = (**i)["index"].to_int();
		for(child_map::const_iterator j = (*i)->children_.begin(); j != (*i)->children_.end(); ++j) {
			const string_span& name = j->first;
			for(child_list::const_iterator k = j->second.begin(); k != j->second.end(); ++k) {
				char* buf = name.duplicate();
				doc_->take_ownership_of_buffer(buf);
				(*k)->copy_into(add_child_at(buf, index));
			}
		}
	}

	const child_list& child_deletes = diff.children("delete_child");
	for(child_list::const_iterator i = child_deletes.begin(); i != child_deletes.end(); ++i) {
		const std::size_t index = (**i)["index"].to_int();
		for(child_map::const_iterator j = (*i)->children_.begin(); j != (*i)->children_.end(); ++j) {
			if(j->second.empty()) {
				continue;
			}

			const string_span& name = j->first;
			remove_child(name, index);
		}
	}
}

void node::set_doc(document* doc)
{
	doc_ = doc;

	for(child_map::iterator i = children_.begin(); i != children_.end(); ++i) {
		for(child_list::iterator j = i->second.begin(); j != i->second.end(); ++j) {
			(*j)->set_doc(doc);
		}
	}
}

int node::nchildren() const
{
	int res = 0;
	for(child_map::const_iterator i = children_.begin(); i != children_.end(); ++i) {
		for(child_list::const_iterator j = i->second.begin(); j != i->second.end(); ++j) {
			++res;
			res += (*j)->nchildren();
		}
	}

	return res;
}

int node::nattributes_recursive() const
{
	int res = attr_.capacity();
	for(child_map::const_iterator i = children_.begin(); i != children_.end(); ++i) {
		for(child_list::const_iterator j = i->second.begin(); j != i->second.end(); ++j) {
			res += (*j)->nattributes_recursive();
		}
	}

	return res;
}

void node::set_dirty()
{
	for(node* n = this; n != nullptr && n->output_cache_.is_null() == false; n = n->parent_) {
		n->output_cache_ = string_span();
	}
}

document::document() :
	compressed_buf_(),
	output_(nullptr),
	buffers_(),
	root_(new node(*this, nullptr)),
	prev_(nullptr),
	next_(nullptr)
{
	attach_list();
}

document::document(char* buf, INIT_BUFFER_CONTROL control) :
	compressed_buf_(),
	output_(buf),
	buffers_(),
	root_(nullptr),
	prev_(nullptr),
	next_(nullptr)
{
	if(control == INIT_TAKE_OWNERSHIP) {
		buffers_.push_back(buf);
	}
	const char* cbuf = buf;
	root_ = new node(*this, nullptr, &cbuf);

	attach_list();
}

document::document(const char* buf, INIT_STATE state) :
	compressed_buf_(),
	output_(buf),
	buffers_(),
	root_(nullptr),
	prev_(nullptr),
	next_(nullptr)
{
	if(state == INIT_COMPRESSED) {
		output_compressed();
		output_ = nullptr;
	} else {
		root_ = new node(*this, nullptr, &buf);
	}

	attach_list();
}

document::document(string_span compressed_buf) :
	compressed_buf_(compressed_buf),
	output_(nullptr),
	buffers_(),
	root_(nullptr),
	prev_(nullptr),
	next_(nullptr)
{
	string_span uncompressed_buf;
	buffers_.push_back(uncompress_buffer(compressed_buf, &uncompressed_buf));
	output_ = uncompressed_buf.begin();
	const char* cbuf = output_;
	try {
		root_ = new node(*this, nullptr, &cbuf);
	} catch(...) {
		ERR_SWML << "Caught exception creating a new simple_wml node: " << utils::get_unknown_exception_type();
		delete [] buffers_.front();
		buffers_.clear();
		throw;
	}

	attach_list();
}

document::~document()
{
	for(std::vector<char*>::iterator i = buffers_.begin(); i != buffers_.end(); ++i) {
		delete [] *i;
	}

	buffers_.clear();
	debug_delete(root_);

	detach_list();
}

const char* document::dup_string(const char* str)
{
	const int len = strlen(str);
	char* res = new char[len+1];
	memcpy(res, str, len + 1);
	buffers_.push_back(res);
	return res;
}

const char* document::output()
{
	if(output_ && (!root_ || root_->is_dirty() == false)) {
		return output_;
	}
	if(!root_) {
		assert(compressed_buf_.empty() == false);
		string_span uncompressed_buf;
		buffers_.push_back(uncompress_buffer(compressed_buf_, &uncompressed_buf));
		output_ = uncompressed_buf.begin();
		return output_;
	}

	//we're dirty, so the compressed buf must also be dirty; clear it.
	compressed_buf_ = string_span();

	std::vector<char*> bufs;
	bufs.swap(buffers_);

	const int buf_size = root_->output_size() + 1;
	char* buf;
	try {
		buf = new char[buf_size];
	} catch (const std::bad_alloc& e) {
		ERR_SWML << "ERROR: Trying to allocate " << buf_size << " bytes. "
		<< e.what();
		throw error("Bad allocation request in output().");
	}
	buffers_.push_back(buf);
	output_ = buf;

	root_->output(buf, node::REFRESH_CACHE);
	*buf++ = 0;
	assert(buf == output_ + buf_size);

	for(std::vector<char*>::iterator i = bufs.begin(); i != bufs.end(); ++i) {
		delete [] *i;
	}

	bufs.clear();

	return output_;
}

string_span document::output_compressed(bool bzip2)
{
	if(compressed_buf_.empty() == false &&
	   (root_ == nullptr || root_->is_dirty() == false)) {
		assert(*compressed_buf_.begin() == (bzip2 ? 'B' : 31));
		return compressed_buf_;
	}

	buffers_.push_back(compress_buffer(output(), &compressed_buf_, bzip2));
	assert(*compressed_buf_.begin() == (bzip2 ? 'B' : 31));

	return compressed_buf_;
}

void document::compress()
{
	output_compressed();
	debug_delete(root_);
	root_ = nullptr;
	output_ = nullptr;
	std::vector<char*> new_buffers;
	for(std::vector<char*>::iterator i = buffers_.begin(); i != buffers_.end(); ++i) {
		if(*i != compressed_buf_.begin()) {
			delete [] *i;
		} else {
			new_buffers.push_back(*i);
		}
	}

	buffers_.swap(new_buffers);
	assert(buffers_.size() == 1);
}

void document::generate_root()
{
	if(output_ == nullptr) {
		assert(compressed_buf_.empty() == false);
		string_span uncompressed_buf;
		buffers_.push_back(uncompress_buffer(compressed_buf_, &uncompressed_buf));
		output_ = uncompressed_buf.begin();
	}

	assert(root_ == nullptr);
	const char* cbuf = output_;
	root_ = new node(*this, nullptr, &cbuf);
}

std::unique_ptr<document> document::clone()
{
	char* buf = new char[strlen(output())+1];
	strcpy(buf, output());
	return std::make_unique<document>(buf);
}

void document::swap(document& o)
{
	std::swap(compressed_buf_, o.compressed_buf_);
	std::swap(output_, o.output_);
	buffers_.swap(o.buffers_);
	std::swap(root_, o.root_);

	root_->set_doc(this);
	o.root_->set_doc(&o);
}

void document::clear()
{
	compressed_buf_ = string_span();
	output_ = nullptr;
	debug_delete(root_);
	root_ = new node(*this, nullptr);
	for(std::vector<char*>::iterator i = buffers_.begin(); i != buffers_.end(); ++i) {
		delete [] *i;
	}

	buffers_.clear();
}

namespace {
document* head_doc = nullptr;
}

void document::attach_list()
{
	prev_ = nullptr;
	next_ = head_doc;

	if(next_) {
		next_->prev_ = this;
	}
	head_doc = this;
}

void document::detach_list()
{
	if(head_doc == this) {
		head_doc = next_;
	}

	if(next_) {
		next_->prev_ = prev_;
	}

	if(prev_) {
		prev_->next_ = next_;
	}
	next_ = prev_ = nullptr;
}

std::string document::stats()
{
	std::ostringstream s;
	int ndocs = 0;
	int ncompressed = 0;
	int compressed_size = 0;
	int ntext = 0;
	int text_size = 0;
	int nbuffers = 0;
	int nnodes = 0;
	int ndirty = 0;
	int nattributes = 0;
	for(document* d = head_doc; d != nullptr; d = d->next_) {
		ndocs++;
		nbuffers += d->buffers_.size();

		if(d->compressed_buf_.is_null() == false) {
			++ncompressed;
			compressed_size += d->compressed_buf_.size();
		}

		if(d->output_) {
			++ntext;
			text_size += strlen(d->output_);
		}

		if(d->root_) {
			nnodes += 1 + d->root_->nchildren();
			nattributes += d->root_->nattributes_recursive();
		}

		if(d->root_ && d->root_->is_dirty()) {
			++ndirty;
		}
	}

	const int nodes_alloc = nnodes*(sizeof(node) + 12);
	const int attr_alloc = nattributes*(sizeof(string_span)*2);
	const int total_alloc = compressed_size + text_size + nodes_alloc + attr_alloc;

	s << "WML documents: " << ndocs << "\n"
	  << "Dirty: " << ndirty << "\n"
	  << "With compression: " << ncompressed << " (" << compressed_size
	  << " bytes)\n"
	  << "With text: " << ntext << " (" << text_size
	  << " bytes)\n"
	  << "Nodes: " << nnodes << " (" << nodes_alloc << " bytes)\n"
	  << "Attr: " << nattributes << " (" << attr_alloc << " bytes)\n"
	  << "Buffers: " << nbuffers << "\n"
	  << "Total allocation: " << total_alloc << " bytes\n";

	return s.str();
}

void swap(document& lhs, document& rhs)
{
	lhs.swap(rhs);
}

}
