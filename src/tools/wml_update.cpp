/* $Id$ */
/*
 * Copyright 2006 - 2008 Dominic Bolin
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.


 * Version 1.2
 * Update 1
*/

//! @file tools/wml_update.cpp
//! Standalone-Utility to update the WML-syntax from 1.0 to 1.1.2, and 1.1.2 to 1.2.
//! See also: data/tools/wmllint

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <deque>
#include <vector>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFBase.h>
#endif



struct Level;

typedef std::deque<Level*> child_list;

template<typename To, typename From>
To lexical_cast_default(From a, To def=To())
{
	To res;
	std::stringstream str;

	if(!(str << a && str >> res)) {
		return def;
	} else {
		return res;
	}
}


std::string left_side(std::string& s)
{
	return s.substr(0,s.find('='));
}
std::string right_side(std::string& s)
{
	return s.substr(s.find('=')+1,s.size());
}
bool isnewline(char c)
{
	return c == '\r' || c == '\n';
}

int pattern_match(const std::string& pat,const std::string& mat)
{
	int ppos=0;
	int mpos=0;
	while(1) {
		if(ppos>=pat.size()) {
			if(mpos<mat.size()) {
				return false;
			} else {
				return true;
			}
		}
		if(mpos>=mat.size()) {
			if(ppos<pat.size()) {
				return false;
			} else {
				return true;
			}
		}
		if(pat[ppos] == '*') {
			ppos++;
			if(ppos>=pat.size()) {
				return true;
			}
			while(mat[mpos] != pat[ppos]) {
				mpos++;
				if(mpos>=mat.size()) {
					return false;
				}
			}
		} else if(pat[ppos] != '&') {
			if(mat[mpos] != pat[ppos]) {
				return false;
			}
		}
		ppos++;
		mpos++;
	}
}


enum FILE_NAME_MODE { ENTIRE_FILE_PATH, FILE_NAME_ONLY };

void list_directory(const std::string& directory,
                      std::deque<std::string>* files,
                      std::deque<std::string>* dirs,
                      FILE_NAME_MODE mode,
					  const std::string& suffix)
{
		DIR* dir = opendir(directory.c_str());

		if(dir == NULL) {
			return;
		}

		struct dirent* entry;
		while((entry = readdir(dir)) != NULL) {
			if(entry->d_name[0] == '.')
				continue;
#ifdef __APPLE__
		  /* HFS Mac OS X decompose filenames using combining unicode
		  characters. Try to get the precomposed form.
		  */
		  char filename[MAXNAMLEN+1];
		  CFStringRef cstr = CFStringCreateWithCString(NULL, entry->d_name,
							       kCFStringEncodingUTF8);
		  CFMutableStringRef mut_str = CFStringCreateMutableCopy(NULL, 0, cstr);
		  CFStringNormalize(mut_str, kCFStringNormalizationFormC);
		  CFStringGetCString(mut_str,filename,sizeof(filename)-1,kCFStringEncodingUTF8);
		  CFRelease(cstr);
		  CFRelease(mut_str);
#else
		  char *filename = entry->d_name;
#endif
			const std::string name((directory + "/") + filename);

			struct stat st;
			if (::stat(name.c_str(), &st) != -1) {
				if (S_ISREG(st.st_mode)) {
					if (files != NULL) {
						if (mode == ENTIRE_FILE_PATH && name.substr(name.size()-suffix.size()) == suffix) {
							files->push_back(name);
						} else if(name.substr(name.size()-suffix.size()) == suffix) {
							files->push_back(filename);
						}
					}
				} else if (S_ISDIR(st.st_mode)) {
					if (dirs != NULL) {
						if (mode == ENTIRE_FILE_PATH) {
							dirs->push_back(name);
						} else if (true) {
							dirs->push_back(filename);
						}
					}
				}
			}
		}
	closedir(dir);

	if(files != NULL)
		std::sort(files->begin(),files->end());

	if(dirs != NULL)
		std::sort(dirs->begin(),dirs->end());
}


struct Level {
	Level(const std::string& itag,Level* p) {tag=itag;parent=p;is_tag=false;};
	~Level() {for(child_list::iterator i = data.begin(); i != data.end(); ++i) {delete *i;}};

	Level* add_child(Level* l)
	{
		data.push_back(l);
		l->parent=this;
		is_tag = true;
		return l;
	}

	Level* set(Level& l)
	{
		tag=l.tag;
		parent=l.parent;
		is_tag=l.is_tag;
		for(child_list::iterator i = l.data.begin(); i != l.data.end(); ++i) {
			add_child((new Level((*i)->tag,this))->set(**i));
		}
		return this;
	}

	Level* child(const std::string& key)
	{
		for(child_list::iterator i = data.begin(); i != data.end(); ++i) {
			if((*i)->tag == key) {
				return *i;
			}
		}
		return NULL;
	}
	void delete_child(const std::string& key)
	{
		for(child_list::iterator i = data.begin(); i != data.end(); ++i) {
			if((*i)->tag == key) {
					delete *i;
					data.erase(i);
					return;
			}
		}
	}
	void delete_lchild(const std::string& key)
	{
		for(child_list::iterator i = data.begin(); i != data.end(); ++i) {
			if(left_side((*i)->tag) == key) {
					delete *i;
					data.erase(i);
					return;
			}
		}
	}

	Level* insert(int index,const std::string& key,Level* l)
	{
		for(child_list::iterator i = data.begin(); i != data.end(); ++i) {
			if((*i)->tag == key) {
				if(index == 0) {
					data.insert(i,l);
					return l;
				} else {
					index--;
				}
			}
		}
		add_child(l);
		return l;
	}

	std::string operator [](const std::string& key)
	{
		for(child_list::iterator i = data.begin(); i != data.end(); ++i) {
			if(left_side((*i)->tag) == key) {
				return right_side((*i)->tag);
			}
		}
		return "";
	}
	void replace_child(const std::string& key,const std::string value)
	{
		for(child_list::iterator i = data.begin(); i != data.end(); ++i) {
			if(left_side((*i)->tag) == key) {
				(*i)->tag = key + value;
				return;
			}
		}
	}

	child_list get_children(const std::string& key)
	{
		child_list ret;
		for(child_list::iterator i = data.begin(); i != data.end(); ++i) {
			if((*i)->tag == key) {
				ret.push_back(*i);
			}
		}
		return ret;
	}

	std::string tag;
	bool is_tag;
	child_list data;
	Level* parent;
};









std::vector<std::string> preproc_actions;

void init_preproc_actions()
{
	preproc_actions.push_back("#define");
	preproc_actions.push_back("#ifdef");
	preproc_actions.push_back("#undef");
	preproc_actions.push_back("#textdomain");
}

#define CEND(s,p) if(p >= s.size()) {std::cerr << "Error! Unexpected EOF! @ " << __LINE__ << "\n"; return;}
#define CENDB(s,p) if(p >= s.size()) {break;}
#define TABS(s,n) for(int t=0;t<n;t++) {s.put('\t');}

void output_level(Level& l,std::ofstream& outfile,int tabs,bool comments)
{
	if(!outfile) {
		std::cerr << "Error! Could not open file for writing!\n";
		return;
	}

	if(!l.is_tag && l.data.empty()) { // key
		if(l.tag[0] != '#') {
			TABS(outfile,tabs);
		}
		if(l.tag[0] != '#' || comments || std::find(preproc_actions.begin(),preproc_actions.end(),l.tag)!=preproc_actions.end()) {
			outfile.write(l.tag.c_str(),l.tag.size());
			outfile.put('\n');
			outfile.flush();
		}
	} else { // section
		if(l.tag != "]]]{MAIN}[[[") {
			TABS(outfile,tabs);
			outfile.put('[');
			outfile.write(l.tag.c_str(),l.tag.size());
			outfile.put(']');
			outfile.put('\n');
			outfile.flush();
		}
		for(int w=0;w<l.data.size();w++) {
			output_level(*l.data[w],outfile,tabs+(l.tag != "]]]{MAIN}[[["),comments);
		}
		if(l.tag != "]]]{MAIN}[[[") {
			TABS(outfile,tabs);
			outfile.put('[');
			outfile.put('/');
			int s=0;
			if(l.tag[0] == '+' || l.tag[0] == '-') {
				s=1;
			}
			outfile.write(l.tag.c_str()+s,l.tag.size()-s);
			outfile.put(']');
			outfile.put('\n');
			outfile.flush();
		}
	}
}

// Make sure that we can use Mac, DOS, or Unix style text files
// on any system and they will work, by making sure
// the definition of whitespace is consistent
bool portable_isspace(char c)
{
	// Returns true only on ASCII spaces
	if ((unsigned char)c >= 128)
		return false;
	return isnewline(c) || isspace(c);
}

// Make sure we regard '\r' and '\n' as a space, since Mac, Unix,
// and DOS all consider these differently.
bool notspace(char c)
{
	return !portable_isspace(c);
}
std::string &strip(std::string &str,bool always)
{
	// If all the string contains is whitespace,
	// then the whitespace may have meaning, so don't strip it
	std::string::iterator it = std::find_if(str.begin(), str.end(), notspace);
	if (it == str.end() && !always)
		return str;

	str.erase(str.begin(), it);
	str.erase(std::find_if(str.rbegin(), str.rend(), notspace).base(), str.end());

	return str;
}
enum { REMOVE_EMPTY = 0x01, STRIP_SPACES = 0x02 };
std::vector< std::string > split(std::string const &val, char c = ',', int flags = REMOVE_EMPTY | STRIP_SPACES)
{
	std::vector< std::string > res;

	std::string::const_iterator i1 = val.begin();
	std::string::const_iterator i2 = val.begin();

	while (i2 != val.end()) {
		if (*i2 == c) {
			std::string new_val(i1, i2);
			if (flags & STRIP_SPACES)
				strip(new_val,false);
			if (!(flags & REMOVE_EMPTY) || !new_val.empty())
				res.push_back(new_val);
			++i2;
			if (flags & STRIP_SPACES) {
				while (i2 != val.end() && *i2 == ' ')
					++i2;
			}

			i1 = i2;
		} else {
			++i2;
		}
	}

	std::string new_val(i1, i2);
	if (flags & STRIP_SPACES)
		strip(new_val,false);
	if (!(flags & REMOVE_EMPTY) || !new_val.empty())
		res.push_back(new_val);

	return res;
}

void update_tree_1_1_2(Level& l,bool verbose);

bool level_compare(Level* a,Level* b)
{
	if(a->data.empty()) {
		if(b->data.empty()) {
			if(a->tag < b->tag) {
				return true;
			} else {
				return false;
			}
		} else {
			return true;
		}
	} else {
		if(b->data.empty()) {
			return false;
		} else {
			if(a->tag < b->tag) {
				return true;
			} else {
				return false;
			}
		}
	}
}

void reorder_tree(Level& l)
{
	std::sort(l.data.begin(),l.data.end(),level_compare);
	for(int w=0;w<l.data.size();w++) {
		reorder_tree(*l.data[w]);
	}
}

bool integrity_check(Level& l,bool verbose,Level* p)
{
	bool success = true;
	if(l.parent != p) {
		std::cerr << "Internal Error: Parent integrity check failed! (" << l.parent->tag << " : " << p->tag << ")\n";
		return false;
	}
	if(!l.is_tag && l.data.empty()) { // key
		if(l.tag.find('=') == std::string::npos && l.tag.find('{') == std::string::npos && l.tag.find('#') == std::string::npos) {
			std::cerr << "Internal Error: Key integrity check failed! (" << l.tag << ")\n";
			return false;
		}
	} else {
		if(l.tag.find('=') != std::string::npos) {
			std::cerr << "Internal Error: Tag integrity check failed! (" << l.tag << ")\n";
			return false;
		}
		for(int w=0;w<l.data.size();w++) {
			success = success && integrity_check(*l.data[w],verbose,&l);
		}
	}
	return success;
}


void resolve_tree_1_1_2(Level& l,bool verbose)
{
	if(l.data.empty()) { // key
	} else {
		if(l.tag == "animation" && (l.parent->tag == "attack" || l.parent->tag == "effect")) {
			child_list frame_tags = l.get_children("frame");
			for(child_list::iterator i = frame_tags.begin(); i != frame_tags.end(); ++i) {
				l.data.erase(std::find(l.data.begin(),l.data.end(),*i));
			}
			while(frame_tags.size()) {
				int first = 10000;
				child_list::iterator j;
				for(child_list::iterator i = frame_tags.begin(); i != frame_tags.end(); ++i) {
					if(lexical_cast_default<int>((**i)["begin"]) < first) {
						j=i;
						first = lexical_cast_default<int>((**i)["begin"]);
					}
				}
				l.add_child(*j);
				frame_tags.erase(j);
			}
			bool dirty = false;
			frame_tags = l.get_children("frame");
			for(child_list::iterator i = frame_tags.begin(); i != frame_tags.end(); ++i) {
				if(i+1 != frame_tags.end()) {
					if(lexical_cast_default<int>((**i)["end"]) > lexical_cast_default<int>((**(i+1))["begin"])) {
						if(lexical_cast_default<int>((**i)["end"]) > lexical_cast_default<int>((**(i+1))["end"])) {
							Level* nframe = (new Level("frame",&l))->set(**i);
							nframe->replace_child("begin","="+(**(i+1))["end"]);
							nframe->replace_child("end","="+(**i)["end"]);
							nframe->delete_lchild("sound");
							l.add_child(nframe);
							dirty = true;
							if(verbose) {
								std::cerr << "Splitting frame\n";
							}
						}
						if(verbose) {
							std::cerr << "Changing frame end to next begin (" << (**i)["end"] << " -> " << (**(i+1))["begin"] << "\n";
						}
						(*i)->replace_child("end","="+(**(i+1))["begin"]);
					}
				}
			}
			if(dirty) {
				resolve_tree_1_1_2(l,verbose);
			}
		}

		for(int w=0;w<l.data.size();w++) {
			resolve_tree_1_1_2(*l.data[w],verbose);
		}
	}
}

bool pre_update_1_1_2(Level& l,bool verbose) // split things here
{
	bool need_update = false;
	if(l.data.empty()) { // key
	} else {
		if(l.tag == "animation" && (l.parent->tag == "attack" || l.parent->tag == "effect")) {
			// Split sound_hit and sound_miss
			child_list sound_tags = l.get_children("sound");
			Level* miss_anim = (new Level(l.tag,l.parent))->set(l);
			Level* hit_anim = (new Level(l.tag,l.parent))->set(l);

			bool use_miss = false;
			bool use_hit = false;
			for(child_list::iterator i = sound_tags.begin(); i != sound_tags.end(); ++i) {
				if((**i)["sound_miss"] != "") {
					use_miss = true;
				}
				if((**i)["sound"] != "" && (**i)["sound_miss"] != "") {
					use_hit = true;
				}
			}
			if(use_miss && use_hit) { // needs splitting; maybe always split if sound_miss exists?
				miss_anim->data.push_front(new Level("hits=no",miss_anim));
				hit_anim->data.push_front(new Level("hits=yes",hit_anim));

				Level* p = l.parent;
				child_list::iterator f = std::find(p->data.begin(),p->data.end(),&l);
				delete &l;
				p->data.erase(f);

				update_tree_1_1_2(*miss_anim,verbose);
				update_tree_1_1_2(*hit_anim,verbose);

				miss_anim->parent = p;
				hit_anim->parent = p;
				p->add_child(miss_anim);
				p->add_child(hit_anim);

				if(verbose)
					std::cerr << "splitting sound_miss and sound\n";
				return true;
			}

			bool hits = l.child("hits=no") == NULL;
			std::string sound_suffix((hits ? "" : "_miss"));

			while(l.child("sound")) {
				// Needs to find/make a frame with sound=
				Level* i = l.child("sound");
				child_list frame_tags = l.get_children("frame");
				bool create = true;
				// Find a frame with equal time if possible
				for(child_list::iterator j = frame_tags.begin(); j != frame_tags.end(); ++j) {
					if(lexical_cast_default<int>((**j)["begin"],0) == lexical_cast_default<int>((*i)["time"],0)) {
						if((*i)["sound_miss"] == "" && !hits) {
							(*j)->add_child(new Level("sound=" + (*i)[std::string("sound")],*j));
						} else {
							(*j)->add_child(new Level("sound=" + (*i)[std::string("sound") + sound_suffix],*j));
						}
						if(verbose)
							std::cerr << "merging frame for sound\n";
						create = false;
						break;
					}
				}
				if(create) {
					// Create new frame
					int index=0;
					for(child_list::iterator j = frame_tags.begin(); j != frame_tags.end(); ++j) {
						if(lexical_cast_default<int>((**j)["begin"],0) > lexical_cast_default<int>((*i)["time"],0)) {
							Level* nframe = new Level("frame",&l);
							if((*i)["sound_miss"] == "" && !hits) {
								nframe->add_child(new Level("sound=" + (*i)[std::string("sound")],nframe));
							} else {
								nframe->add_child(new Level("sound=" + (*i)[std::string("sound") + sound_suffix],nframe));
							}
							nframe->add_child(new Level("begin="+(*i)["time"],nframe));
							for(child_list::iterator k = i->data.begin(); k != i->data.end(); ++k) {
								if(pattern_match("halo*=*",(*k)->tag)) {
									nframe->add_child((new Level((*k)->tag,nframe))->set(**k));
								}
							}
							nframe->add_child(new Level("end="+lexical_cast_default<std::string>(lexical_cast_default<int>((*i)["time"],0)+1),nframe));
							for(child_list::iterator k = frame_tags.begin(); k != frame_tags.end(); ++k) {
								if((lexical_cast_default<int>((**k)["end"]) >= lexical_cast_default<int>((*i)["time"])) && (lexical_cast_default<int>((**k)["begin"]) <= lexical_cast_default<int>((*i)["time"]))) {
									for(child_list::iterator y = (*k)->data.begin(); y != (*k)->data.end(); ++y) {
										if(pattern_match("halo*=*",(*y)->tag)) {
											nframe->add_child(new Level((*y)->tag,nframe));
										}
									}
								}
							}
							l.insert(index,"frame",nframe);

							if(j != frame_tags.begin()) { // not beginning
								if(j+1 != frame_tags.end()) { // middle
//									(*(j-1))->replace_child("end",(*i)["time"]);
//									nframe->add_child(new Level("end="+(**(j+1))["end"],nframe));
									nframe->add_child(new Level("image="+(**(j-1))["image"],nframe));
									if(verbose)
										std::cerr << "adding new frame for sound\n";
									create = false;
									break;
								} else { // end
//									nframe->add_child(new Level("end="+lexical_cast_default<std::string>(lexical_cast_default<int>((*i)["time"],0)+1),nframe));
									nframe->add_child(new Level("image="+(**(j-1))["image"],nframe));
									if(verbose)
										std::cerr << "adding new frame for sound\n";
									create = false;
									break;
								}
							} else { // beginning
//									nframe->add_child(new Level("end="+lexical_cast_default<std::string>(lexical_cast_default<int>((*i)["time"],0)+1),nframe));
									if(verbose)
										std::cerr << "adding new frame for sound\n";
									create = false;
									break;
							}

						}
						index++;
					}
					if(create) {
						Level* nframe = new Level("frame",&l);
						if((*i)["sound_miss"] == "" && !hits) {
							nframe->add_child(new Level("sound=" + (*i)[std::string("sound")],nframe));
						} else {
							nframe->add_child(new Level("sound=" + (*i)[std::string("sound") + sound_suffix],nframe));
						}
						nframe->add_child(new Level("begin="+(*i)["time"],nframe));
						nframe->add_child(new Level("end="+lexical_cast_default<std::string>(lexical_cast_default<int>((*i)["time"],0)+1),nframe));
						for(child_list::iterator k = frame_tags.begin(); k != frame_tags.end(); ++k) {
								if((lexical_cast_default<int>((**k)["end"]) >= lexical_cast_default<int>((*i)["time"])) && (lexical_cast_default<int>((**k)["begin"]) <= lexical_cast_default<int>((*i)["time"]))) {
									for(child_list::iterator y = (*k)->data.begin(); y != (*k)->data.end(); ++y) {
									if(pattern_match("halo*=*",(*y)->tag)) {
										nframe->add_child(new Level((*y)->tag,nframe));
									}
								}
							}
						}
						if(frame_tags.size()) {
							nframe->add_child(new Level("image="+(**(frame_tags.end()-1))["image"],nframe));
							l.data.insert(std::find(l.data.begin(),l.data.end(),(frame_tags.back())),nframe);
						} else {
							l.add_child(nframe);
						}
						if(verbose)
							std::cerr << "adding new frame for sound\n";
					}
				}
				std::cerr << "deleting [sound]\n";
				l.delete_child("sound");
				need_update = true;
			}
		}
		for(int w=0;w<l.data.size();w++) {
			need_update |= pre_update_1_1_2(*l.data[w],verbose);
		}
	}
	return need_update;
}

void update_tree_1_1_2(Level& l,bool verbose)
{
	if(l.data.empty()) { // key
		if(l.tag == "range=short" && (l.parent->tag == "attack" || l.parent->tag == "defend" || l.parent->tag == "effect")) {
			l.tag = "range=melee";
			if(verbose)
				std::cerr << "updating range=short\n";
		} else if(l.tag == "range=long" && (l.parent->tag == "attack" || l.parent->tag == "defend" || l.parent->tag == "effect")) {
			l.tag = "range=ranged";
			if(verbose)
				std::cerr << "updating range=long\n";
//		} else if (l.tag == "{ABILITY_AMBUSH}" && l.parent->tag == "abilities") {
//			l.tag = "{ABILITY_AMBUSH f}";
//			if(verbose)
//				std::cerr << "updating {ABILITY_AMBUSH}\n";
		} else if(left_side(l.tag) == "image_defensive" && (l.parent->tag == "unit" || l.parent->tag == "female" || l.parent->tag == "variation" || l.parent->tag == "male")) {
			std::string save_tag = l.tag;
			l.tag = "defend";
			Level* new_frame = new Level("frame",&l);
			l.add_child(new_frame);
			new_frame->add_child(new Level("begin=-150",new_frame));
			new_frame->add_child(new Level("end=150",new_frame));
			new_frame->add_child(new Level(std::string("image=")+right_side(save_tag),new_frame));
			if(verbose)
				std::cerr << "updating image_defensive\n";
		} else if(left_side(l.tag) == "image_defensive_short" && (l.parent->tag == "unit" || l.parent->tag == "female" || l.parent->tag == "variation" || l.parent->tag == "male")) {
			std::string save_tag = l.tag;
			l.tag = "defend";
			l.add_child(new Level("range=melee",&l));
			Level* new_frame = new Level("frame",&l);
			l.add_child(new_frame);
			new_frame->add_child(new Level("begin=-150",new_frame));
			new_frame->add_child(new Level("end=150",new_frame));
			new_frame->add_child(new Level(std::string("image=")+right_side(save_tag),new_frame));
			if(verbose)
				std::cerr << "updating image_defensive_short\n";
		} else if((left_side(l.tag) == "image_defensive_range" || left_side(l.tag) == "image_defensive_long") && (l.parent->tag == "unit" || l.parent->tag == "female" || l.parent->tag == "variation" || l.parent->tag == "male")) {
			std::string save_tag = l.tag;
			l.tag = "defend";
			l.add_child(new Level("range=ranged",&l));
			Level* new_frame = new Level("frame",&l);
			l.add_child(new_frame);
			new_frame->add_child(new Level("begin=-150",new_frame));
			new_frame->add_child(new Level("end=150",new_frame));
			new_frame->add_child(new Level(std::string("image=")+right_side(save_tag),new_frame));
			if(verbose)
				std::cerr << "updating image_defensive_long\n";
		} else if (left_side(l.tag) == "image_moving" && (l.parent->tag == "unit" || l.parent->tag == "female" || l.parent->tag == "variation" || l.parent->tag == "male")) {
			std::string save_tag = l.tag;
			l.tag = "movement_anim";
			Level* new_frame = new Level("frame",&l);
			l.add_child(new_frame);
			new_frame->add_child(new Level("begin=0",new_frame));
			new_frame->add_child(new Level("end=150",new_frame));
			new_frame->add_child(new Level(std::string("image=")+right_side(save_tag),new_frame));
			if(verbose)
				std::cerr << "updating image_moving\n";
		} else if(left_side(l.tag) == "ability" && (l.parent->tag == "unit" || l.parent->tag == "female" || l.parent->tag == "variation" || l.parent->tag == "male")) {
			std::vector<std::string> abilities = split(right_side(l.tag));
			l.tag = "abilities";
			for(int a=0;a<abilities.size();a++) {
				for(int c=0;c<abilities[a].size();c++) {
					abilities[a][c]=toupper(abilities[a][c]);
				}
//				l.add_child(Level(std::string("{ABILITY_") + abilities[a] + (abilities[a]=="AMBUSH" ? " f" : "") + "}",&l));
				l.add_child(new Level(std::string("{ABILITY_") + abilities[a] + "}",&l));
			}
			if(verbose)
				std::cerr << "updating ability=\n";
		}
	} else {
		if(l.tag == "animation") {
			if(l.child("hits=no")) {
				child_list sounds = l.get_children("sound");
				for(child_list::iterator i = sounds.begin(); i != sounds.end(); ++i) {
					if(((**i)["sound"] != "") && ((**i)["sound_miss"] != "")) {
						Level* to_del = (*i)->child("sound=" + (**i)["sound"]);
						if(to_del) {
							(*i)->data.erase(std::find((*i)->data.begin(),(*i)->data.end(),to_del));
							delete to_del;
						}
					}
				}
			} else if(l.child("hits=yes")) {
				child_list sounds = l.get_children("sound");
				for(child_list::iterator i = sounds.begin(); i != sounds.end(); ++i) {
					if((**i)["sound_miss"] != "") {
						Level* to_del = (*i)->child("sound_miss=" + (**i)["sound_miss"]);
						if(to_del) {
							(*i)->data.erase(std::find((*i)->data.begin(),(*i)->data.end(),to_del));
							delete to_del;
						}
					}
				}
			}
		} else if(l.tag == "attack" || l.tag == "effect") {
			Level* n_anim = new Level("animation",&l);
			child_list frames = l.get_children("frame");
			child_list mframes = l.get_children("missile_frame");
			child_list sframes = l.get_children("sound");
			for(child_list::iterator i = frames.begin(); i != frames.end(); ++i) {
				n_anim->add_child(*i);
				l.data.erase(std::find(l.data.begin(),l.data.end(),*i));
			}
			for(child_list::iterator i = mframes.begin(); i != mframes.end(); ++i) {
				n_anim->add_child(*i);
				l.data.erase(std::find(l.data.begin(),l.data.end(),*i));
			}
			for(child_list::iterator i = sframes.begin(); i != sframes.end(); ++i) {
				n_anim->add_child(*i);
				l.data.erase(std::find(l.data.begin(),l.data.end(),*i));
			}
			if(n_anim->data.size()) {
				l.add_child(n_anim);
				if(verbose)
					std::cerr << "updating attack_anim\n";
			}
		}
		for(int w=0;w<l.data.size();w++) {
			update_tree_1_1_2(*l.data[w],verbose);
		}
	}
}


















void resolve_tree_1_2(Level& l,bool verbose)
{
	if(l.data.empty()) { // key
	} else {
		for(int w=0;w<l.data.size();w++) {
			resolve_tree_1_2(*l.data[w],verbose);
		}
	}
}

bool pre_update_1_2(Level& l,bool verbose)
{
	bool need_update = false;
	if(l.data.empty()) { // key
	} else {
		for(int w=0;w<l.data.size();w++) {
			need_update |= pre_update_1_2(*l.data[w],verbose);
		}
	}
	return need_update;
}


void update_tree_1_2(Level& l,bool verbose)
{
	if(l.data.empty()) { // key
		if(left_side(l.tag)=="special" && l.parent->tag=="attack" || l.parent->tag=="effect") {
			std::string special_macro;
			if(right_side(l.tag).find('(') == std::string::npos) {
				special_macro = "{WEAPON_SPECIAL_" + right_side(l.tag) + "}";
			} else {
				std::string spec = right_side(l.tag);
				special_macro = "{WEAPON_SPECIAL_" + spec.substr(0,6) + "_TYPE " + spec.substr(6) + "}";
			}
			l.tag = "specials";
			for(int c = 0; c < special_macro.size(); ++c) {
				special_macro[c] = toupper(special_macro[c]);
			}
			l.add_child(new Level(special_macro,&l));
			if(verbose) {
				std::cerr << "updating special...\n";
			}
		} else if(l.tag == "{ABILITY_LEADERSHIP}") {
			std::string level = (*l.parent->parent)["level"];
			l.tag = "{ABILITY_LEADERSHIP_LEVEL_" + level + "}";
			if(verbose) {
				std::cerr << "updating leadership...\n";
			}
		}
	} else {
		child_list frames = l.get_children("frame");
		if(frames.size()) {
			int ii = 0;
			for(child_list::iterator i = frames.begin(); i != frames.end();) {
				if(i+1 != frames.end()) {
					int st = lexical_cast_default<int>((**i)["begin"]);
					int end = lexical_cast_default<int>((**i)["end"]);
					std::string snd;
					if(end-st == 1) {
						std::cerr << "deleting frame\n";
						snd = (**i)["sound"];
						l.data.erase(std::find(l.data.begin(),l.data.end(),*i));
						delete *i;
						i = frames.erase(i);
					}
					if(snd.size()) {
						std::cerr << "merging frame\n";
						(*i)->add_child(new Level("sound="+snd,(*i)));
						Level* bg = (*i)->child("begin="+(**i)["begin"]);
						if(bg) {
							int st = lexical_cast_default<int>(right_side(bg->tag));
							st--;
							bg->tag = "begin=" + lexical_cast_default<std::string>(st);
						}
					}
				}
				++i;
				++ii;
			}
		}
		for(int w=0;w<l.data.size();w++) {
			update_tree_1_2(*l.data[w],verbose);
		}
	}
}

void update_file(const std::string& path,bool do_update,bool comments,bool verbose,bool reorder,int version)
{
	std::cerr << "Processing " << path << "... \n";
	std::ifstream infile(path.c_str());
	if(! infile) {
		std::cerr << "Error! Could not open file for reading!\n";
		return;
	}
	std::string idata;
	char c;
	while(! infile.eof()) {
		c=infile.get();
		if(c != '\r' && c != '\t') {
			idata += c;
		}
	}

	idata=idata.substr(0,idata.size()-1);
	if(idata[idata.size()-1] != '\n' && idata[idata.size()-2] != '\n') {
		idata += "\n";
	}

	Level* p_data = new Level("]]]{MAIN}[[[",NULL);
	Level* current_level=p_data;

	int t=0;
	while(t < idata.size()-1) {
		if(idata[t] == '#') {
			std::string ntag;
			while(idata[t]!= '\n') {
				ntag += idata[t];
				t++;
				CEND(idata,t)
			}
			current_level->add_child(new Level(ntag,current_level));
			t++;
		} else if(idata[t] == '[' && idata[t+1] != '/') { // enter new level
			t++;
			CEND(idata,t)
			std::string new_tag;
			while(idata[t] != ']') {
				new_tag += idata[t];
				t++;
				CEND(idata,t)
			}
			t++;
			CENDB(idata,t)
			new_tag=strip(new_tag,true);
			current_level->add_child(new Level(new_tag,current_level));
			current_level=(current_level->data.back());
			current_level->is_tag = true;
		} else if(idata[t] == '[' && idata[t+1] == '/') { // close level
			t += 2;
			CEND(idata,t)
			std::string close_tag;
			while(idata[t] != ']') {
				close_tag += idata[t];
				t++;
				CEND(idata,t)
			}
			t++;
			CENDB(idata,t)
			close_tag=strip(close_tag,true);
			int s=0;
			if(current_level->tag[0] == '+' || current_level->tag[0] == '-') {
				s=1;
			}
			if(close_tag != current_level->tag.substr(s)) {
				std::cerr << "Syntax error: Expected [/" << current_level->tag << "], found [/" << close_tag << "]\n";
				return;
			}
			if(current_level->parent == NULL) {
				std::cerr << "Syntax error: closing tag [/" << close_tag << "] without starting tag\n";
			}
			current_level = current_level->parent;
		} else if(idata[t] != ' ') { // key
			std::string ntag;
			while(idata[t] != '\n') {
				if(idata[t] != '\"') {
					ntag += idata[t];
					t++;
					CENDB(idata,t)
				} else {
					ntag += idata[t];
					t++;
					CENDB(idata,t)
					while(idata[t] != '\"') {
						ntag += idata[t];
						t++;
						CENDB(idata,t)
					}
					ntag += idata[t];
					t++;
				}
				CENDB(idata,t)
			}
			int lsize = ntag.size();
			do {
				lsize = ntag.size();
				ntag=strip(ntag,true);
				if(!comments) {
					int cc=0;
					int qq=0;
					while(cc<ntag.size()) {
						if(ntag[cc] == '\"') {
							qq = !qq;
							cc++;
						} else {
							if(ntag[cc] == '#' && qq == 0 && cc !=0) {
								ntag=ntag.substr(0,cc);
								break;
							}
							cc++;
						}
					}
				}
			} while(ntag.size() != lsize);
			if(!ntag.empty()) {
				if(ntag[0] == '+' || (current_level->data.size() && current_level->data.back()->tag[current_level->data.back()->tag.size()-1]=='+' )) {
					current_level->data.back()->tag += " " + ntag;
				} else {
					current_level->add_child(new Level(ntag,current_level));
				}
			}
			t++;
		} else {
			while(idata[t] == ' ') {
				t++;
				if(t>=idata.size()) {
					goto finish;
				}
			}
		}
	}
	finish:

	if(do_update) {
		if(version == 112) {
			while(pre_update_1_1_2(*p_data,verbose));
			update_tree_1_1_2(*p_data,verbose);
			resolve_tree_1_1_2(*p_data,verbose);
		} else if(version == 12) {
			while(pre_update_1_2(*p_data,verbose));
			update_tree_1_2(*p_data,verbose);
			resolve_tree_1_2(*p_data,verbose);
		}
//		if(reorder) {
//			reorder_tree(*p_data);
//		}
	}
	if(integrity_check(*p_data,verbose,NULL)) {
		std::ofstream outfile(path.c_str());
		output_level(*p_data,outfile,0,comments);
	}

	delete p_data;

}

int main(int argc, char *argv[])
{
	if(argc<2) {
		std::cerr << "Usage: filenames [options]\n\t -u : Update the WML syntax from 1.0 to 1.1.2\n\t -u1 : Update the WML syntax from 1.1.2 to 1.2\n\t -r : Process all .cfg files recursively\n\t -v : verbose output\n\t -rem : Remove comments\n\t\n";
		return 0;
	}

	init_preproc_actions();

	std::string update_path = argv[1];
	std::deque<std::string> files;
	std::deque<std::string> dirs;
	bool do_update = false;
	bool recurse = false;
	bool comments = true;
	bool verbose = false;
	bool reorder = false;
	int version = 112;

	for(int t=1;t<argc;t++) {
		if(argv[t][0]=='-') {
			if(strcmp(argv[t],"-u")==0) {
				do_update = true;
			} else if (strcmp(argv[t],"-u1")==0) {
				do_update = true;
				version = 12;
			} else if (strcmp(argv[t],"-r")==0) {
				recurse = true;
			} else if(strcmp(argv[t],"-rem")==0) {
				comments = false;
			} else if(strcmp(argv[t],"-v")==0) {
				verbose = true;
			} else if(strcmp(argv[t],"--reorder")==0) {
				reorder = true;
			} else {
				std::cerr << "Unknown option '" << argv[t] << "'\n";
			}
		} else {
			files.push_back(argv[t]);
		}
	}

	int found = 0;

	if(recurse) {
		list_directory(getcwd(NULL,1024),&files,&dirs,ENTIRE_FILE_PATH,".cfg");
		while(dirs.size() && recurse) {
			list_directory(dirs[0],&files,&dirs,ENTIRE_FILE_PATH,".cfg");
			dirs.pop_front();
			if(files.size() != found) {
				std::cerr << files.size() << " files found...\n";
				found = files.size();
			}
		}
	}

	while(files.size()) {
		update_file(files[0],do_update,comments,verbose,reorder,version);
		files.pop_front();
	}


  return 0;
}
