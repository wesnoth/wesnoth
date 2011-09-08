/* $Id$ */
/*
   Copyright (C) 2004 - 2011 by Philippe Plantier <ayin@anathas.org>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef UTILS_COUNT_LOGGER_H_INCLUDED
#define UTILS_COUNT_LOGGER_H_INCLUDED

/** @file */

#include <string>
#include <vector>
#include <iosfwd>

///@todo when C++0x is supported switch to #include <unordered_map>
#include <boost/unordered_map.hpp>

//debug
#include <iostream> //std::cerr

//#include <utility>
//using namespace std::rel_ops; //conflicts with other source files i.e. tree_view.cpp

namespace n_count_logger {



/**
   @class t_count_logger
   @brief t_count_logger accumulates counts of some event and spits out the total on std::cerr when its destroyed.
 */
template <typename T>
struct t_count_logger{
	typedef boost::unordered_map<T, unsigned long> t_map;
	t_map counts;

	std::string name_;
	int outcount_;

	t_count_logger(std::string const & n, int outcount=7) ;
	unsigned long inc(T const & x);

	~t_count_logger();
};

template <typename T>
struct sortf {
	bool operator()(std::pair<T, unsigned long>  const &a, std::pair<T, unsigned long> const &b){
		return a.second > b.second;
	}
};

template <typename T>
t_count_logger<T>::t_count_logger(std::string const & n, int outcount) : name_(n), outcount_(outcount){}

template <typename T>
unsigned long t_count_logger<T>::inc(T const & x){
	unsigned long &v = counts[x];
	return ++v;
}


template <typename T>
t_count_logger<T>::~t_count_logger(){
	sortf<T> ff;

	std::vector<std::pair<T, unsigned long> > all;
	typename t_map::iterator j(t_count_logger::counts.begin());
	typename t_map::iterator jend(t_count_logger::counts.end());
	for(;j != jend ; ++j){ all.push_back(*j); }
	std::sort(all.begin(), all.end(), ff);


	std::cerr<<name_<<" \n";
	int i=0, iend(all.size());
	for(;i<outcount_ && i!=iend;++i){
		std::cerr<<" Key : count ="<< all[i].first << " : " <<all[i].second<<"\n"; }
}

}
#endif
