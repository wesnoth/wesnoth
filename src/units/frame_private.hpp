/*
   Copyright (C) 2006 - 2017 by Jeremy Rosen <jeremy.rosen@enst-bretagne.fr>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef UNIT_FRAME_PRIVATE_HPP_INCLUDED
#define UNIT_FRAME_PRIVATE_HPP_INCLUDED

#include "lexical_cast.hpp"
#include "serialization/string_utils.hpp"
#include "utils/general.hpp"

#include <vector>

namespace image { class locator; }

template<typename T, typename D>
class progressive_base
{
public:
	using data_t = std::vector<std::pair<D, int>>;

	progressive_base(const std::string& input)
		: data_()
		, input_(input)
	{}

	virtual const T get_current_element(int current_time, T default_val) const = 0;

	virtual bool does_not_change() const
	{
		return data_.size() <= 1;
	}

	int duration() const
	{
		int total = 0;
		for(const auto& entry : data_) {
			total += entry.second;
		}

		return total;
	}

	std::string get_original() const
	{
		return input_;
	}

	data_t& data()
	{
		return data_;
	}

	const data_t& data() const
	{
		return data_;
	}

	virtual ~progressive_base() {}

private:
	data_t data_;
	std::string input_;
};

template<typename T>
class progressive_pair : public progressive_base<T, std::pair<T, T>>
{
public:
	progressive_pair(const std::string& input = "", int duration = 0)
		: progressive_base<T, std::pair<T, T>>(input)
	{
		auto& base_data = progressive_pair_base_type::data();

		const int split_flag = utils::REMOVE_EMPTY; // useless to strip spaces

		const std::vector<std::string> comma_split = utils::split(input, ',', split_flag);
		const int time_chunk = std::max<int>(1, duration / std::max<int>(comma_split.size(), 1));

		for(const auto& entry : comma_split) {
			std::vector<std::string> colon_split = utils::split(entry, ':', split_flag);
			int time = 0;

			try {
				time = (colon_split.size() > 1) ? std::stoi(colon_split[1]) : time_chunk;
			} catch(std::invalid_argument) {
				//ERR_NG << "Invalid time in unit animation: " << colon_split[1] << "\n";
			}

			try {
				std::vector<std::string> range = utils::split(colon_split[0],'~',split_flag);
				T range0 = lexical_cast<T>(range[0]);
				T range1 = (range.size() > 1) ? lexical_cast<T>(range[1]) : range0;

				base_data.push_back({{range0, range1}, time});
			} catch(bad_lexical_cast) {}
		}
	}

	virtual const T get_current_element(int current_time, T default_val = T()) const override
	{
		const auto& base_data = progressive_pair_base_type::data();
		const int& base_duration = progressive_pair_base_type::duration();

		if(base_data.empty()) {
			return default_val;
		}

		int time = 0;
		unsigned int i = 0;
		const int searched_time = utils::clamp(current_time, 0, base_duration);

		while(time < searched_time && i < base_data.size()) {
			time += base_data[i].second;
			++i;
		}

		if(i != 0) {
			i--;
			time -= base_data[i].second;
		}

		const T first  = base_data[i].first.first;
		const T second = base_data[i].first.second;

		return T((
			static_cast<double>(searched_time - time) /
			static_cast<double>(base_data[i].second)
		) * (second - first) + first);
	}

	bool does_not_change() const override
	{
		const auto& base_data = progressive_pair_base_type::data();
		return base_data.empty() || (base_data.size() == 1 && base_data[0].first.first == base_data[0].first.second);
	}

private:
	using progressive_pair_base_type = progressive_base<T, std::pair<T, T>>;
};

template<typename T>
class progressive_single : public progressive_base<T, T>
{
public:
	progressive_single(const std::string& input = "", int duration = 0)
		: progressive_base<T, T>(input)
	{
		auto& base_data = progressive_single_base_type::data();

		const std::vector<std::string> first_pass = utils::square_parenthetical_split(input);
		int time_chunk = std::max<int>(duration, 1);

		if(duration > 1 && !first_pass.empty()) {
			// If duration specified, divide evenly the time for items with unspecified times
			int total_specified_time = 0;

			for(const std::string& fp_string : first_pass) {
				std::vector<std::string> second_pass = utils::split(fp_string, ':');
				if(second_pass.size() > 1) {
					try {
						total_specified_time += std::stoi(second_pass[1]);
					} catch(std::invalid_argument) {
						//ERR_NG << "Invalid time in unit animation: " << second_pass[1] << "\n";
					}
				}
			}

			time_chunk = std::max<int>((duration - total_specified_time) / first_pass.size(), 1);
		}

		for(const std::string& fp_string : first_pass) {
			std::vector<std::string> second_pass = utils::split(fp_string, ':');
			if(second_pass.size() > 1) {
				try {
					base_data.push_back({std::move(second_pass[0]), std::stoi(second_pass[1])});
				} catch(std::invalid_argument) {
					//ERR_NG << "Invalid time in unit animation: " << second_pass[1] << "\n";
				}
			} else {
				base_data.push_back({std::move(second_pass[0]) ,time_chunk});
			}
		}
	}

	virtual const T get_current_element(int current_time, T default_val = T()) const override
	{
		const auto& base_data = progressive_single_base_type::data();

		if(base_data.empty()) {
			return default_val;
		}

		int time = 0;
		unsigned int i = 0;

		while(time < current_time && i < base_data.size()) {
			time += base_data[i].second;
			++i;
		}

		// TODO: what is this for?
		if(i) {
			i--;
		}

		return base_data[i].first;
	}

private:
	using progressive_single_base_type = progressive_base<T, T>;
};

// Common types used by the unit frame code.
using progressive_int = progressive_pair<int>;
using progressive_double = progressive_pair<double>;

using progressive_string = progressive_single<std::string>;
using progressive_image = progressive_single<image::locator>;

#endif
