
/*
   Copyright (C) 2011 - 2011 by Sytyi Nick <nsytyi@gmail.com>
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
 * This file contains objects "tag" and "key", which are used to store
 * information about tags and keys while annotation parsing.
 */

#ifndef TOOLS_SCHEMA_TAG_HPP_INCLUDED
#define TOOLS_SCHEMA_TAG_HPP_INCLUDED

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>



namespace schema_generator{
/**
  * class_key is used to save the information about one key.
  * Key has next info: name, type, default value or key is mandatory.
  */
class class_key{
public:
	class_key():name_(""),type_(""),default_(""),mandatory_(false)
	{ }
	class_key(const std::string & name,
			  const std::string &type,
			  const std::string &def,
			  bool mandatory)
	{
		name_ = name;
		type_ = type;
		default_ = def;
		mandatory_ = mandatory;
	}

	const std::string &  get_name() const{
		return name_ ;
	}
	const std::string &  get_type() const{
		return type_ ;
	}
	const std::string &  get_default() const{
		return default_;
	}
	bool  is_mandatory () const{
		return mandatory_ ;
	}

	void  set_name(const std::string& name){
		name_ = name;
	}
	void  set_type(const std::string& type){
		type_ = type;
	}
	void  set_default(const std::string& def){
		default_ = def;
		if (def.empty()){
			mandatory_ = true;
		}
	}
	void  set_mandatory(bool mandatory){
		mandatory_ = mandatory;
	}
	/** is used to print key info
 * the format is next
 *  [key]
 *      name="name"
 *      type="type"
 *      default="default"
 *      mandatory="true/false"
 *  [/key]
*/
	void  print(std::ostream& os,int level) const;


	/**
	 *Compares keys by name. Used in std::sort, i.e.
	 */
	bool  operator < ( const class_key& k) const{
		return (get_name() < k.get_name());
	}
private:
	/** Name of key*/
	std::string name_;
	/** Type of key*/
	std::string type_;
	/** Default value*/
	std::string default_;
	/** Shows, if key is a mandatory key.*/
	bool mandatory_;
};

/**
  * Stores information about tag.
  * Each tags is an element of great tag tree. This tree is close to filesystem:
  * you can use links and special include directory global/
  * Normally root is not mentioned in path.
  * Each tag has name, minimum and maximum occasions number,
  * and lists of subtags, keys and links.
  */
class class_tag{
public:
	class_tag():name_(""),min_(0),max_(0),super_("")
	{ }

	class_tag(const std::string & name,
			  int min,
			  int max
			  )
	{
		name_ = name;
		min_ = min;
		max_ = max;
	}
	class_tag(const std::string & name,
			  int min,
			  int max,
			  const std::string & super
			  )
	{
		name_ = name;
		min_ = min;
		max_ = max;
		super_ = super;
	}
	~class_tag(){  }

	/** Prints information about tag to outputstream, recursively
	 * is used to print tag info
	 * the format is next
	 *  [tag]
	 *      subtags
	 *      keys
	 *      name="name"
	 *      min="min"
	 *      max="max"
	 *  [/tag]
	*/
	void print(std::ostream& os);

	const std::string & get_name() const{
		return name_ ;
	}
	int get_min() const{
		return min_;
	}
	int get_max() const{
		return max_;
	}
	const std::string & get_super() const{
		return super_ ;
	}
	bool is_extension() const{
		return ! super_.empty();
	}

	void set_name(const std::string& name){
		name_ = name;
	}
	void set_min(int o){
		min_ = o;
	}
	void set_max(int o){
		max_ = o;
	}
	void set_min( std::string const& s){
		std::istringstream i(s);
		if (!(i >> min_)){
			min_ = 0;
		}
	}
	void set_max( std::string const & s){
		std::istringstream i(s);
		if (!(i >> max_)){
			max_ = 0;
		}
	}
	void set_super(std::string const & s){
		super_= s;
	}
	void add_key(const class_key& new_key){
		keys_.push_back(new_key);
	}
	void add_tag(const class_tag& new_tag){
		tags_.push_back(new_tag);
	}
	void add_link(const std::string & link){
		links_.push_back(link);
	}

	/**
	 * Tags are usually organized in tree.
	 * This fuction helps to add tag to his exact place in tree
	 * @param path - path in subtree to exact place of tag
	 * @param tag  - tag to add
	 * @todo make support of adding to link. Probably by returning exact path,
	 * i.e we wan add to foo/bar/example, but /foo/bar is a link to bar.
	 * Probably fuction schould return right path , i.this.e. bar/example
	 * and caller schould call one's more
	 * Path is getting shotter and shoter with each call.
	 * Path schould look like tag1/tag2/parent/ Slash at end is mandatory.
	 * @return Pointer to name of the full path.
	 * @retval NULL  add successful.
	 * @retval Not NULL  you try add a child to a link,
	 *                   here is a right path, please, use it.
	 */
	const std::string  add_tag (const std::string & path,
								const class_tag & tag);

	bool operator < ( const class_tag& t) const{
		return name_ < t.name_;
	}
	bool operator == (const class_tag & other){
		return name_ == other.name_;
	}

private:
	/** name of tag*/
	std::string name_;
	/** number of minimum occasions*/
	int min_;
	/** number of maximum occasions*/
	int max_;
	/**
	 * name of tag to extend "super-tag"
	 * Extension is smth like inheritance and is used in case
	 * when you need to use another tag with all his
	 * keys, childs, etc. But you also want to allow extra subtags of that tags,
	 * so just linking that tag wouldn't help at all.
	 */
	std::string super_;
	/** children tags*/
	std::vector<class_tag> tags_;
	/** keys*/
	std::vector<class_key> keys_;
	/** links to possible children.
	 * @todo make support of key section. which takes mandatory links usually.
	 */
	std::vector<std::string> links_;
	/**
	 * the same as class_tag::print(std::ostream&)
	 * but indents different levels with step space.
	 * @param os stream to print
	 * @param level  current level of indentation
	 * @param step   step to next indent
	 */
	void printl(std::ostream &os,int level, int step);

	void add_tags (const std::vector<class_tag> & list){
		tags_.insert(tags_.end(),list.begin(),list.end());
	}
	void add_keys (const std::vector<class_key> & list){
		keys_.insert(keys_.end(),list.begin(),list.end());
	}
};

}
#endif // TOOLS_SCHEMA_TAG_HPP_INCLUDED
