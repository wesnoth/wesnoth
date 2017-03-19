/*
   Copyright (C) 2004 - 2017 by Philippe Plantier <ayin@anathas.org>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef TSTRING_H_INCLUDED
#define TSTRING_H_INCLUDED

#include <memory>
#include <string>

/**
 * Helper class for translatable strings.
 */
class t_string;
class t_string_base
{
public:
	class walker
	{
	public:
		explicit walker(const t_string_base& string);

		void next()                               { begin_ = end_; update(); }
		bool eos() const                          { return begin_ == string_.size(); }
		bool last() const                         { return end_ == string_.size(); }
		bool translatable() const                 { return translatable_; }
		bool countable() const                    { return countable_; }
		int count() const                         { return count_; }
		const std::string& textdomain() const     { return textdomain_; }
		std::string::const_iterator begin() const { return string_.begin() + begin_; }
		std::string::const_iterator end() const   { return string_.begin() + end_; }
		std::string::const_iterator plural_begin() const;
		std::string::const_iterator plural_end() const;
	private:
		void update();

		const std::string& string_;
		std::string::size_type begin_;
		std::string::size_type end_;
		std::string textdomain_;
		bool translatable_, countable_;
		int count_;
	};

	friend class walker;

	t_string_base();
	/** Default implementation, but defined out-of-line for efficiency reasons. */
	~t_string_base();
	/** Default implementation, but defined out-of-line for efficiency reasons. */
	t_string_base(const t_string_base&);
	t_string_base(const std::string& string);
	t_string_base(const std::string& string, const std::string& textdomain);
	t_string_base(const std::string& sing, const std::string& pl, int count, const std::string& textdomain);
	t_string_base(const char* string);

	static t_string_base from_serialized(const std::string& string);
	std::string to_serialized() const;

	/** Default implementation, but defined out-of-line for efficiency reasons. */
	t_string_base& operator=(const t_string_base&);
	t_string_base& operator=(const std::string&);
	t_string_base& operator=(const char*);

	t_string_base operator+(const t_string_base&) const;
	t_string_base operator+(const std::string&) const;
	t_string_base operator+(const char*) const;

	t_string_base& operator+=(const t_string_base&);
	t_string_base& operator+=(const std::string&);
	t_string_base& operator+=(const char*);

	bool operator==(const t_string_base &) const;
	bool operator==(const std::string &) const;
	bool operator==(const char* string) const;

	bool operator!=(const t_string_base &that) const
	{ return !operator==(that); }
	bool operator!=(const std::string &that) const
	{ return !operator==(that); }
	bool operator!=(const char *that) const
	{ return !operator==(that); }

	bool operator<(const t_string_base& string) const;

	bool empty() const                               { return value_.empty(); }
	std::string::size_type size() const              { return str().size(); }

	operator const std::string&() const              { return str(); }
	const std::string& str() const;
	const char* c_str() const                        { return str().c_str(); }
	bool translatable() const						 { return translatable_; }
	// Warning: value() may contain platform dependent prefix bytes !
	// Consider base_str() for a more reliable untranslated string
	const std::string& value() const                 { return value_; }
	std::string base_str() const;

	size_t hash_value() const;
private:
	std::string value_;
	mutable std::string translated_value_;
	mutable unsigned translation_timestamp_;
	bool translatable_, last_untranslatable_;
};

inline size_t hash_value(const t_string_base& str) { return str.hash_value(); }
std::ostream& operator<<(std::ostream&, const t_string_base&);

class t_string
{
public:
	typedef t_string_base base;
	typedef t_string_base::walker walker;

	/** Default implementation, but defined out-of-line for efficiency reasons. */
	t_string();
	/** Default implementation, but defined out-of-line for efficiency reasons. */
	~t_string();
	/** Default implementation, but defined out-of-line for efficiency reasons. */
	t_string(const t_string &);
	/** Default implementation, but defined out-of-line for efficiency reasons. */
	t_string &operator=(const t_string &);

	t_string(const base &);
	t_string(const char *);
	t_string(const std::string &);
	t_string(const std::string &str, const std::string &textdomain);
	t_string(const std::string& sing, const std::string& pl, int count, const std::string& textdomain);

	t_string &operator=(const char *o);

	static t_string from_serialized(const std::string& string) { return t_string(base::from_serialized(string)); }
	std::string to_serialized() const { return get().to_serialized(); }

	operator const t_string_base &() const { return get(); }

	t_string operator+(const t_string& o) const { return get() + o.get(); }
	t_string operator+(const std::string& o) const { return get() + o; }
	t_string operator+(const char* o) const { return get() + o; }
private:
	template<typename T>
	void increase_impl(const T& other)
	{
		base * nw = new base(get());
		*nw += other;
		val_.reset(nw);
	}
public:
	t_string& operator+=(const t_string& o) { increase_impl(o.get()); return *this; }
	t_string& operator+=(const std::string& o) { increase_impl(o); return *this; }
	t_string& operator+=(const char* o) { increase_impl(o); return *this; }

	bool operator==(const t_string& o) const { return get() == o.get(); }
	bool operator==(const std::string& o) const { return get() == o; }
	bool operator==(const char* o) const { return get() == o; }

	bool operator!=(const t_string& o) const { return !operator==(o); }
	bool operator!=(const std::string& o) const { return !operator==(o); }
	bool operator!=(const char* o) const { return !operator==(o); }

	bool operator<(const t_string& o) const { return get() < o.get(); }

	bool empty() const { return get().empty(); }
	std::string::size_type size() const { return get().size(); }

	operator const std::string&() const { return get(); }
	const std::string& str() const { return get().str(); }
	const char* c_str() const { return get().c_str(); }
	bool translatable() const { return get().translatable(); }
	const std::string& value() const { return get().value(); }
	std::string base_str() const { return get().base_str(); }

	static void add_textdomain(const std::string &name, const std::string &path);
	static void reset_translations();

	const t_string_base& get() const { return *val_; }
	void swap(t_string& other) { val_.swap(other.val_); }
private:
	//never null
	std::shared_ptr<const t_string_base> val_;
};
inline std::ostream& operator<<(std::ostream& os, const t_string& str) { return os << str.get(); }
inline bool operator==(const std::string &a, const t_string &b)    { return b == a; }
inline bool operator==(const char *a, const t_string &b)           { return b == a; }
inline bool operator!=(const std::string &a, const t_string &b)    { return b != a; }
inline bool operator!=(const char *a, const t_string &b)           { return b != a; }
inline t_string operator+(const std::string &a, const t_string &b) { return t_string(a) + b; }
inline t_string operator+(const char *a, const t_string &b)        { return t_string(a) + b; }
#endif

