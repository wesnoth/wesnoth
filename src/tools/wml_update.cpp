
/*
 * Copyright 2006 Dominic Bolin
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.


 * Version 1.1.2
 * Update 5
*/

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string>
#include <deque>
#include <vector>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>



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
	Level(const std::string& itag,Level* p) {tag=itag;parent=p;};
	~Level() {for(std::vector<Level*>::iterator i = data.begin(); i != data.end(); ++i) {delete *i;}};
	std::vector<Level*> data;
	std::string tag;
	Level* parent;
};

std::vector<std::string> preproc_actions;

void init_preproc_actions()
{
	preproc_actions.push_back("#define");
	preproc_actions.push_back("#ifdef");
	preproc_actions.push_back("#undef");
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
	
	if(l.data.empty()) { // tag
		if(l.tag[0] != '#') {
			TABS(outfile,tabs);
		}
		if(l.tag[0] != '#' || comments || std::find(preproc_actions.begin(),preproc_actions.end(),l.tag)!=preproc_actions.end()) {
			outfile.write(l.tag.c_str(),l.tag.size());
			outfile.put('\n');
			outfile.flush();
		}
	} else { // section
		if(l.tag != "]]]MAIN[[[") {
			TABS(outfile,tabs);
			outfile.put('[');
			outfile.write(l.tag.c_str(),l.tag.size());
			outfile.put(']');
			outfile.put('\n');
			outfile.flush();
		}
		for(int w=0;w<l.data.size();w++) {
			output_level(*l.data[w],outfile,tabs+(l.tag != "]]]MAIN[[["),comments);
		}
		if(l.tag != "]]]MAIN[[[") {
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

//make sure that we can use Mac, DOS, or Unix style text files on any system
//and they will work, by making sure the definition of whitespace is consistent
bool portable_isspace(char c)
{
	// returns true only on ASCII spaces
	if ((unsigned char)c >= 128)
		return false;
	return isnewline(c) || isspace(c);
}

//make sure we regard '\r' and '\n' as a space, since Mac, Unix, and DOS
//all consider these differently.
bool notspace(char c)
{
	return !portable_isspace(c);
}
std::string &strip(std::string &str,bool always)
{
	//if all the string contains is whitespace, then the whitespace may
	//have meaning, so don't strip it
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

void update_tree(Level& l,bool verbose)
{
	if(l.data.empty()) { // tag
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
			l.data.push_back(new_frame);
			new_frame->data.push_back(new Level("begin=-150",new_frame));
			new_frame->data.push_back(new Level("end=150",new_frame));
			new_frame->data.push_back(new Level(std::string("image=")+right_side(save_tag),new_frame));
			if(verbose)
				std::cerr << "updating image_defensive\n";
		} else if(left_side(l.tag) == "image_defensive_short" && (l.parent->tag == "unit" || l.parent->tag == "female" || l.parent->tag == "variation" || l.parent->tag == "male")) {
			std::string save_tag = l.tag;
			l.tag = "defend";
			l.data.push_back(new Level("range=melee",&l));
			Level* new_frame = new Level("frame",&l);
			l.data.push_back(new_frame);
			new_frame->data.push_back(new Level("begin=-150",new_frame));
			new_frame->data.push_back(new Level("end=150",new_frame));
			new_frame->data.push_back(new Level(std::string("image=")+right_side(save_tag),new_frame));
			if(verbose)
				std::cerr << "updating image_defensive_short\n";
		} else if((left_side(l.tag) == "image_defensive_range" || left_side(l.tag) == "image_defensive_long") && (l.parent->tag == "unit" || l.parent->tag == "female" || l.parent->tag == "variation" || l.parent->tag == "male")) {
			std::string save_tag = l.tag;
			l.tag = "defend";
			l.data.push_back(new Level("range=ranged",&l));
			Level* new_frame = new Level("frame",&l);
			l.data.push_back(new_frame);
			new_frame->data.push_back(new Level("begin=-150",new_frame));
			new_frame->data.push_back(new Level("end=150",new_frame));
			new_frame->data.push_back(new Level(std::string("image=")+right_side(save_tag),new_frame));
			if(verbose)
				std::cerr << "updating image_defensive_long\n";
		} else if (left_side(l.tag) == "image_moving" && (l.parent->tag == "unit" || l.parent->tag == "female" || l.parent->tag == "variation" || l.parent->tag == "male")) {
			std::string save_tag = l.tag;
			l.tag = "movement_anim";
			Level* new_frame = new Level("frame",&l);
			l.data.push_back(new_frame);
			new_frame->data.push_back(new Level("begin=0",new_frame));
			new_frame->data.push_back(new Level("end=150",new_frame));
			new_frame->data.push_back(new Level(std::string("image=")+right_side(save_tag),new_frame));
			if(verbose)
				std::cerr << "updating image_moving\n";
		} else if(left_side(l.tag) == "ability" && (l.parent->tag == "unit" || l.parent->tag == "female" || l.parent->tag == "variation" || l.parent->tag == "male")) {
			std::vector<std::string> abilities = split(right_side(l.tag));
			l.tag = "abilities";
			for(int a=0;a<abilities.size();a++) {
				for(int c=0;c<abilities[a].size();c++) {
					abilities[a][c]=toupper(abilities[a][c]);
				}
//				l.data.push_back(Level(std::string("{ABILITY_") + abilities[a] + (abilities[a]=="AMBUSH" ? " f" : "") + "}",&l));
				l.data.push_back(new Level(std::string("{ABILITY_") + abilities[a] + "}",&l));
			}
			if(verbose)
				std::cerr << "updating ability=\n";
		}
	} else {
		if(l.tag == "attack" || l.tag == "effect") {
			Level* n_anim = new Level("animation",&l);
			for(int s=0;s<l.data.size();s++) {
				if(l.data[s]->tag == "frame") {
					l.data[s]->parent = n_anim;
					n_anim->data.push_back(l.data[s]);
					std::vector<Level*>::iterator i= l.data.begin()+s;
					l.data.erase(i);
					s--;
				}
			}
			for(int s=0;s<l.data.size();s++) {
				if(l.data[s]->tag == "missile_frame") {
					l.data[s]->parent = n_anim;
					n_anim->data.push_back(l.data[s]);
					std::vector<Level*>::iterator i= l.data.begin()+s;
					l.data.erase(i);
					s--;
				}
			}
			for(int s=0;s<l.data.size();s++) {
				if(l.data[s]->tag == "sound") {
					l.data[s]->parent = n_anim;
					n_anim->data.push_back(l.data[s]);
					std::vector<Level*>::iterator i= l.data.begin()+s;
					l.data.erase(i);
					s--;
				}
			}
			if(n_anim->data.size()) {
				l.data.push_back(n_anim);
				if(verbose)
					std::cerr << "updating attack_anim\n";
			}
		}
		for(int w=0;w<l.data.size();w++) {
			update_tree(*l.data[w],verbose);
		}
	}
}



void update_file(const std::string& path,bool do_update,bool comments,bool verbose)
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
	
	Level* p_data = new Level("]]]MAIN[[[",NULL);
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
			current_level->data.push_back(new Level(ntag,current_level));
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
			current_level->data.push_back(new Level(new_tag,current_level));
			current_level=(current_level->data.back());
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
		} else if(idata[t] != ' ') { // tag
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
					current_level->data.push_back(new Level(ntag,current_level));
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
		update_tree(*p_data,verbose);
	}
	
	std::ofstream outfile(path.c_str());
	output_level(*p_data,outfile,0,comments);
	
	delete p_data;
	
}

int main(int argc, char *argv[])
{
	
	if(argc<2) {
		std::cerr << "Usage: filenames [options]\n\t -u : Update the WML syntax from 1.0 to 1.1\n\t -r : Process all .cfg files recursively\n\t -v : verbose output\n\t -rem : Remove comments\n";
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
	
	for(int t=1;t<argc;t++) {
		if(argv[t][0]=='-') {
			if(strcmp(argv[t],"-u")==0) {
				do_update = true;
			}	else if (strcmp(argv[t],"-r")==0) {
				recurse = true;
			} else if(strcmp(argv[t],"-rem")==0) {
				comments = false;
			} else if(strcmp(argv[t],"-v")==0) {
				verbose = true;
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
		update_file(files[0],do_update,comments,verbose);
		files.pop_front();
	}
	
	
  return 0;
}
