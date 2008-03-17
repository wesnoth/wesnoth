#include <assert.h>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <zlib.h>

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>

#include "SDL.h"
#include "../config.hpp"
#include "../serialization/binary_wml.hpp"
#include "simple_wml.hpp"

namespace simple_wml {

namespace {

char* uncompress_buffer(const string_span& input, string_span* span)
{
	std::istringstream stream(std::string(input.begin(), input.end()));
	boost::iostreams::filtering_stream<boost::iostreams::input> filter;
	filter.push(boost::iostreams::gzip_decompressor());
	filter.push(stream);

	const int chunk_size = input.size() * 10;
	std::vector<char> buf(chunk_size);
	int len = 0;
	int pos = 0;
	while(filter.good() && (len = filter.read(&buf[pos], chunk_size).gcount()) == chunk_size) {
		pos += len;
		buf.resize(pos + chunk_size);
		len = 0;
	}

	if(!filter.eof() && !filter.good()) {
		throw error("failed to uncompress");
	}

	pos += len;
	buf.resize(pos);

	char* small_out = new char[pos+1];
	memcpy(small_out, &buf[0], pos);
	small_out[pos] = 0;

	*span = string_span(small_out, pos);
	return small_out;
}

char* compress_buffer(const char* input, string_span* span)
{
	std::string in(input);
	std::istringstream stream(in);
	boost::iostreams::filtering_stream<boost::iostreams::input> filter;
	filter.push(boost::iostreams::gzip_compressor());
	filter.push(stream);

	std::vector<char> buf(in.size()*2 + 80);
	const int len = filter.read(&buf[0], buf.size()).gcount();
	if(!filter.eof() && !filter.good() || len == buf.size()) {
		throw error("failed to compress");
	}

	buf.resize(len);

	char* small_out = new char[len];
	memcpy(small_out, &buf[0], len);

	*span = string_span(small_out, len);
	assert(*small_out == 31);
	return small_out;
}

}  // namespace

bool string_span::to_bool(bool default_value) const
{
	if(empty()) {
		return default_value;
	}

	return operator==("yes") || operator==("on") || operator==("true") || operator==("1");
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
{
	std::cerr << "ERROR: '" << msg << "'\n";
}

std::ostream& operator<<(std::ostream& o, const string_span& s)
{
	o << std::string(s.begin(), s.end());
	return o;
}

node::node(document& doc, node* parent)
  : doc_(&doc), parent_(parent)
{}

node::node(document& doc, node* parent, const char** str, int depth)
  : doc_(&doc), parent_(parent)
{
	if(depth >= 30) {
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
				if(s == NULL) {
					throw error("end element unterminated");
				}

				++s;
				return;
			}

			++s;
			const char* end = strchr(s, ']');
			if(end == NULL) {
				throw error("unterminated element");
			}

			child_list& list = children_[string_span(s, end - s)];

			s = end + 1;

			list.push_back(new node(doc, this, str, depth+1));

			break;
		}
		case ' ':
		case '\t':
		case '\n':
			++s;
			break;
		case '#':
			s = strchr(s, '\n');
			if(s == NULL) {
				throw error("could not find newline after #");
			}
			break;
		default: {
			const char* end = strchr(s, '=');
			if(end == NULL) {
				std::cerr << "attribute: " << s << "\n";
				throw error("could not find '=' after attribute");
			}

			string_span name(s, end - s);
			s = end + 1;
			if(*s == '_') {
				s = strchr(s, '"');
				if(s == NULL) {
					throw error("could not find '\"' after _");
				}
			}

			if(*s != '"') {
				std::cerr << "no quotes for attribute '" << name << "'\n";
				throw error("did not find quotes around attribute");
			}

			end = s;

			for(;;) {
				while((end = strchr(end+1, '"')) && end[1] == '"') {
					++end;
				}

				if(end == NULL) {
					std::cerr << "ATTR: '" << name << "' (((" << s << ")))\n";
					throw error("did not find end of attribute");
				}

				const char* endline = end;
				while(*endline && *endline != '\n' && *endline != '+') {
					++endline;
				}

				if(*endline != '+') {
					break;
				}

				end = strchr(endline, '"');
				if(end == NULL) {
					throw error("did not find quotes after +");
				}
			}

			++s;

			string_span value(s, end - s);
			if(attr_.empty() == false && !(attr_.back().first < name)) {
				std::cerr << "attributes: '" << attr_.back().first << "' < '" << name << "'\n";
				throw error("attributes not in order");
			}

			s = end + 1;

			attr_.push_back(attribute(name, value));
		}
		}
	}

	output_cache_ = string_span(begin, s - begin);
}

node::~node()
{
	for(child_map::iterator i = children_.begin(); i != children_.end(); ++i) {
		for(child_list::iterator j = i->second.begin(); j != i->second.end(); ++j) {
			delete *j;
		}	
	}
}

namespace {
struct string_span_pair_comparer
{
	bool operator()(const string_span& a, const node::attribute& b) const {
		return a < b.first;
	}

	bool operator()(const node::attribute& a, const string_span& b) const {
		return a.first < b;
	}

	bool operator()(const node::attribute& a,
	                const node::attribute& b) const {
		return a.first < b.first;
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
		return range.first->second;
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
		range.first->second = string_span(value);
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

node& node::set_attr_dup_key_and_value(const char* key, const char* value)
{
	return set_attr(doc_->dup_string(key), doc_->dup_string(value));
}

node& node::set_attr_int(const char* key, int value)
{
	char buf[64];
	sprintf(buf, "%d", value);
	return set_attr_dup(key, buf);
}

node& node::add_child_at(const char* name, size_t index)
{
	set_dirty();

	child_list& list = children_[string_span(name)];
	if(index > list.size()) {
		index = list.size();
	}

	list.insert(list.begin() + index, new node(*doc_, this));
	return *list[index];
}


node& node::add_child(const char* name)
{
	set_dirty();

	child_list& list = children_[string_span(name)];
	list.push_back(new node(*doc_, this));
	return *list.back();
}

void node::remove_child(const string_span& name, size_t index)
{
	set_dirty();

	child_list& list = children_[name];
	if(index >= list.size()) {
		return;
	}

	delete list[index];
	list.erase(list.begin() + index);
}

void node::remove_child(const char* name, size_t index)
{
	remove_child(string_span(name), index);
}

node* node::child(const char* name)
{
	child_map::iterator itor = children_.find(string_span(name));
	if(itor == children_.end() || itor->second.empty()) {
		return NULL;
	}

	return itor->second.front();
}

const node* node::child(const char* name) const
{
	child_map::const_iterator itor = children_.find(string_span(name));
	if(itor == children_.end() || itor->second.empty()) {
		return NULL;
	}

	return itor->second.front();
}

const node::child_list& node::children(const char* name) const
{
	static const node::child_list empty;
	child_map::const_iterator itor = children_.find(string_span(name));
	if(itor == children_.end()) {
		return empty;
	}

	return itor->second;
}

int node::output_size() const
{
/*
	if(output_cache_.empty() == false) {
		return output_cache_.size();
	}
	*/

	int res = 0;
	for(attribute_list::const_iterator i = attr_.begin(); i != attr_.end(); ++i) {
		res += i->first.size() + i->second.size() + 4;
	}

	for(child_map::const_iterator i = children_.begin(); i != children_.end(); ++i) {
		for(child_list::const_iterator j = i->second.begin(); j != i->second.end(); ++j) {
			res += i->first.size()*2 + 7;
			res += (*j)->output_size();
		}
	}

	return res;
}

void node::shift_buffers(int offset)
{
	if(!output_cache_.empty()) {
		output_cache_ = string_span(output_cache_.begin() + offset, output_cache_.size());
	}

	for(std::vector<attribute>::iterator i = attr_.begin(); i != attr_.end(); ++i) {
		i->first = string_span(i->first.begin() + offset, i->first.size());
		i->second = string_span(i->second.begin() + offset, i->second.size());
	}

	for(child_map::iterator i = children_.begin(); i != children_.end(); ++i) {
		string_span& key = const_cast<string_span&>(i->first);
		key = string_span(key.begin() + offset, key.size());
		for(child_list::iterator j = i->second.begin(); j != i->second.end(); ++j) {
			(*j)->shift_buffers(offset);
		}
	}
}

void node::output(char*& buf)
{
/*
	if(output_cache_.empty() == false) {
		memcpy(buf, output_cache_.begin(), output_cache_.size());
		shift_buffers(buf - output_cache_.begin());
		buf += output_cache_.size();
		return;
	}
*/
	char* begin = buf;

	for(std::vector<attribute>::iterator i = attr_.begin(); i != attr_.end(); ++i) {
		memcpy(buf, i->first.begin(), i->first.size());
		i->first = string_span(buf, i->first.size());
		buf += i->first.size();
		*buf++ = '=';
		*buf++ = '"';
		memcpy(buf, i->second.begin(), i->second.size());
		i->second = string_span(buf, i->second.size());
		buf += i->second.size();
		*buf++ = '"';
		*buf++ = '\n';
	}

	for(child_map::iterator i = children_.begin(); i != children_.end(); ++i) {
		for(child_list::iterator j = i->second.begin(); j != i->second.end(); ++j) {
			*buf++ = '[';
			memcpy(buf, i->first.begin(), i->first.size());
			const_cast<string_span&>(i->first) = string_span(buf, i->first.size());
			buf += i->first.size();
			*buf++ = ']';
			*buf++ = '\n';
			(*j)->output(buf);
			*buf++ = '[';
			*buf++ = '/';
			memcpy(buf, i->first.begin(), i->first.size());
			buf += i->first.size();
			*buf++ = ']';
			*buf++ = '\n';
		}
	}

	output_cache_ = string_span(begin, buf - begin);
}

void node::copy_into(node& n) const
{
	n.set_dirty();
	for(attribute_list::const_iterator i = attr_.begin(); i != attr_.end(); ++i) {
		char* key = i->first.duplicate();
		char* value = i->second.duplicate();
		n.doc_->take_ownership_of_buffer(key);
		n.doc_->take_ownership_of_buffer(value);
		n.set_attr(key, value);
	}

	for(child_map::const_iterator i = children_.begin(); i != children_.end(); ++i) {
		if(i->second.empty()) {
			continue;
		}

		char* buf = i->first.duplicate();
		n.doc_->take_ownership_of_buffer(buf);

		for(child_list::const_iterator j = i->second.begin(); j != i->second.end(); ++j) {
			(*j)->copy_into(n.add_child(buf));
		}
	}
}

void node::apply_diff(const node& diff)
{
	set_dirty();
	const node* inserts = diff.child("insert");
	if(inserts != NULL) {
		for(attribute_list::const_iterator i = inserts->attr_.begin(); i != inserts->attr_.end(); ++i) {
			char* name = i->first.duplicate();
			char* value = i->second.duplicate();
			set_attr(name, value);
			doc_->take_ownership_of_buffer(name);
			doc_->take_ownership_of_buffer(value);
		}
	}

	const node* deletes = diff.child("delete");
	if(deletes != NULL) {
		for(attribute_list::const_iterator i = deletes->attr_.begin(); i != deletes->attr_.end(); ++i) {
			std::pair<attribute_list::iterator,
	                  attribute_list::iterator> range = std::equal_range(attr_.begin(), attr_.end(), i->first, string_span_pair_comparer());
			if(range.first != range.second) {
				attr_.erase(range.first);
			}
		}
	}

	const child_list& child_changes = diff.children("change_child");
	for(child_list::const_iterator i = child_changes.begin(); i != child_changes.end(); ++i) {
		const size_t index = (**i)["index"].to_int();
		for(child_map::const_iterator j = (*i)->children_.begin(); j != (*i)->children_.end(); ++j) {
			const string_span& name = j->first;
			for(child_list::const_iterator k = j->second.begin(); k != j->second.end(); ++k) {
				child_map::iterator itor = children_.find(name);
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
		const size_t index = (**i)["index"].to_int();
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
		const size_t index = (**i)["index"].to_int();
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
	for(node* n = this; n != NULL && n->output_cache_.is_null() == false; n = n->parent_) {
		n->output_cache_ = string_span();
	}
}

document::document() : output_(NULL),
					   root_(new node(*this, NULL))
{
	attach_list();
}

document::document(char* buf, INIT_BUFFER_CONTROL control) : output_(buf), root_(NULL)
{
	if(control == INIT_TAKE_OWNERSHIP) {
		buffers_.push_back(buf);
	}
	const char* cbuf = buf;
	root_ = new node(*this, NULL, &cbuf);

	attach_list();
}

document::document(const char* buf, INIT_STATE state) : output_(NULL),
                                                        root_(NULL)
{
	output_ = buf;
	output_compressed();
	output_ = NULL;

	attach_list();
}

document::document(string_span compressed_buf)
  : compressed_buf_(compressed_buf),
    output_(NULL),
	root_(NULL)
{
	int ticks = SDL_GetTicks();
	string_span uncompressed_buf;
	buffers_.push_back(uncompress_buffer(compressed_buf, &uncompressed_buf));
	std::cerr << "UNCOMPRESSED: " << (SDL_GetTicks() - ticks) << "\n";
	ticks = SDL_GetTicks();
	output_ = uncompressed_buf.begin();
	const char* cbuf = output_;
	try {
		root_ = new node(*this, NULL, &cbuf);
	} catch(...) {
		delete [] buffers_.front();
		buffers_.clear();
		throw;
	}
	std::cerr << "PARSED: " << (SDL_GetTicks() - ticks) << "\n";

	attach_list();
}

document::~document()
{
	for(std::vector<char*>::iterator i = buffers_.begin(); i != buffers_.end(); ++i) {
		delete [] *i;
	}

	buffers_.clear();
	delete root_;

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
	char* buf = new char[buf_size];
	buffers_.push_back(buf);
	output_ = buf;

	root_->output(buf);
	*buf++ = 0;
	assert(buf == output_ + buf_size);

	for(std::vector<char*>::iterator i = bufs.begin(); i != bufs.end(); ++i) {
		delete [] *i;
	}

	bufs.clear();

	return output_;
}

string_span document::output_compressed()
{
	if(compressed_buf_.empty() == false &&
	   (root_ == NULL || root_->is_dirty() == false)) {
		assert(*compressed_buf_.begin() == 31);
		return compressed_buf_;
	}

	buffers_.push_back(compress_buffer(output(), &compressed_buf_));
	assert(*compressed_buf_.begin() == 31);

	return compressed_buf_;
}

void document::compress()
{
	output_compressed();
	delete root_;
	root_ = NULL;
	output_ = NULL;
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
	if(output_ == NULL) {
		assert(compressed_buf_.empty() == false);
		string_span uncompressed_buf;
		buffers_.push_back(uncompress_buffer(compressed_buf_, &uncompressed_buf));
		output_ = uncompressed_buf.begin();
	}

	const char* cbuf = output_;
	root_ = new node(*this, NULL, &cbuf);
}

document* document::clone()
{
	char* buf = new char[strlen(output())+1];
	strcpy(buf, output());
	return new document(buf);
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
	output_ = NULL;
	delete root_;
	root_ = new node(*this, NULL);
	for(std::vector<char*>::iterator i = buffers_.begin(); i != buffers_.end(); ++i) {
		delete [] *i;
	}

	buffers_.clear();
}

namespace {
document* head_doc = NULL;
}

void document::attach_list()
{
	prev_ = NULL;
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
	next_ = prev_ = NULL;
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
	int nhas_nodes = 0;
	int ndirty = 0;
	int nattributes = 0;
	for(document* d = head_doc; d != NULL; d = d->next_) {
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
			++nhas_nodes;
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

}

#ifdef UNIT_TEST_SIMPLE_WML

int main(int argc, char** argv)
{
	char* doctext = strdup(
"[test]\n"
"a=\"blah\"\n"
"b=\"blah\"\n"
"c=\"\\\\\"\n"
"d=\"\\\"\"\n"
"[/test]");
	std::cerr << doctext << "\n";
	simple_wml::document doc(doctext);

	simple_wml::node& node = doc.root();
	simple_wml::node* test_node = node.child("test");
	assert(test_node);
	assert((*test_node)["a"] == "blah");
	assert((*test_node)["b"] == "blah");
	assert((*test_node)["c"] == "\\\\");
	assert((*test_node)["d"] == "\\\"");

	node.set_attr("blah", "blah");
	test_node->set_attr("e", "f");
	std::cerr << doc.output();
}

#endif
