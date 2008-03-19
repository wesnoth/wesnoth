#ifndef SIMPLE_WML_HPP_INCLUDED
#define SIMPLE_WML_HPP_INCLUDED

#include <string.h>

#include <iosfwd>
#include <map>
#include <string>
#include <vector>

namespace simple_wml {

struct error {
	error(const char* msg);
};

class string_span
{
public:
	string_span() : str_(NULL), size_(0)
	{}
	string_span(const char* str, int size) : str_(str), size_(size)
	{}
	string_span(const char* str) : str_(str), size_(strlen(str))
	{}

	bool operator==(const char* o) const {
		const char* i1 = str_;
		const char* i2 = str_ + size_;
		while(i1 != i2 && *o && *i1 == *o) {
			++i1;
			++o;
		}

		return i1 == i2 && *o == 0;
	}
	bool operator!=(const char* o) const {
		return !operator==(o);
	}
	bool operator==(const std::string& o) const {
		return size_ == o.size() && memcmp(str_, o.data(), size_) == 0;
	}
	bool operator!=(const std::string& o) const {
		return !operator==(o);
	}
	bool operator==(const string_span& o) const {
		return size_ == o.size_ && memcmp(str_, o.str_, size_) == 0;
	}
	bool operator!=(const string_span& o) const {
		return !operator==(o);
	}
	bool operator<(const string_span& o) const {
		const int len = size_ < o.size_ ? size_ : o.size_;
		for(int n = 0; n < len; ++n) {
			if(str_[n] != o.str_[n]) {
				if(str_[n] < o.str_[n]) {
					return true;
				} else {
					return false;
				}
			}
		}

		return size_ < o.size_;
	}

	const char* begin() const { return str_; }
	const char* end() const { return str_ + size_; }

	int size() const { return size_; }
	bool empty() const { return size_ == 0; }
	bool is_null() const { return str_ == NULL; }

	bool to_bool(bool default_value=false) const;
	int to_int() const;
	std::string to_string() const;

	//returns a duplicate of the string span in a new[] allocated buffer
	char* duplicate() const;

private:
	const char* str_;
	int size_;
};

std::ostream& operator<<(std::ostream& o, const string_span& s);

class document;

class node
{
public:
	node(document& doc, node* parent);
	node(document& doc, node* parent, const char** str, int depth=0);
	~node();

	typedef std::pair<string_span, string_span> attribute;
	typedef std::vector<node*> child_list;

	const string_span& operator[](const char* key) const;
	const string_span& attr(const char* key) const {
		return (*this)[key];
	}

	bool has_attr(const char* key) const;

	node& set_attr(const char* key, const char* value);
	node& set_attr_dup(const char* key, const char* value);
	node& set_attr_dup(const char* key, const string_span& value);
	node& set_attr_dup_key_and_value(const char* key, const char* value);

	node& set_attr_int(const char* key, int value);

	node& add_child(const char* name);
	node& add_child_at(const char* name, size_t index);
	void remove_child(const char* name, size_t index);
	void remove_child(const string_span& name, size_t index);

	node* child(const char* name);
	const node* child(const char* name) const;

	const child_list& children(const char* name) const;

	const string_span& first_child() const;

	bool is_dirty() const { return output_cache_.is_null(); }

	int output_size() const;
	void output(char*& buf);

	void copy_into(node& n) const;

	bool no_children() const { return children_.empty(); }
	bool one_child() const { return children_.size() == 1 && children_.begin()->second.size() == 1; }

	void apply_diff(const node& diff);

	void set_doc(document* doc);

	int nchildren() const;
	int nattributes_recursive() const;

private:
	node(const node&);
	void operator=(const node&);

	void set_dirty();

	void shift_buffers(int offset);

	document* doc_;

	typedef std::vector<attribute> attribute_list;
	attribute_list attr_;

	node* parent_;

	typedef std::map<string_span, child_list> child_map;
	child_map children_;

	string_span output_cache_;
};

enum INIT_BUFFER_CONTROL { INIT_TAKE_OWNERSHIP };

enum INIT_STATE { INIT_COMPRESSED, INIT_STATIC };

class document
{
public:
	document();
	explicit document(char* buf, INIT_BUFFER_CONTROL control=INIT_TAKE_OWNERSHIP);
	document(const char* buf, INIT_STATE state);
	explicit document(string_span compressed_buf);
	~document();
	const char* dup_string(const char* str);
	node& root() { if(!root_) { generate_root(); } return *root_; }
	const node& root() const { if(!root_) { const_cast<document*>(this)->generate_root(); } return *root_; }

	const char* output();
	string_span output_compressed();

	void compress();

	document* clone();

	const string_span& operator[](const char* key) const {
		return root()[key];
	}

	const string_span& attr(const char* key) const {
		return root()[key];
	}

	node* child(const char* name) {
		return root().child(name);
	}

	const node* child(const char* name) const {
		return root().child(name);
	}

	node& set_attr(const char* key, const char* value) {
		return root().set_attr(key, value);
	}

	node& set_attr_dup(const char* key, const char* value) {
		return root().set_attr_dup(key, value);
	}

	void take_ownership_of_buffer(char* buffer) {
		buffers_.push_back(buffer);
	}

	void swap(document& o);
	void clear();
	
	static std::string stats();

private:
	void generate_root();
	document(const document&);
	void operator=(const document&);

	string_span compressed_buf_;
	const char* output_;
	std::vector<char*> buffers_;
	node* root_;

	//linked list of documents for accounting purposes
	void attach_list();
	void detach_list();
	document* prev_;
	document* next_;
};

}

#endif
