/*
   Copyright (C) 2017-2018 by the Battle for Wesnoth Project https://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "playturn_network_adapter.hpp"
#include "log.hpp"

#include "utils/functional.hpp"
#include <iostream>
#include <cassert>


static lg::log_domain log_network("network");
#define LOG_NW LOG_STREAM(info, log_network)
#define ERR_NW LOG_STREAM(err, log_network)

void playturn_network_adapter::read_from_network()
{
	assert(!data_.empty());

	this->data_.emplace_back();
	config& back = data_.back();
	bool has_data = false;
	try
	{
		has_data = this->network_reader_(back);
	}
	catch(...)
	{
		//Readin from network can throw, we want to ignore the possibly corrupt packet in this case.
		this->data_.pop_back();
		throw;
	}
	//ping is handeled by network.cpp and we can ignore it.
	back.remove_attribute("ping");
	if((!has_data) || back.empty())
	{
		this->data_.pop_back();
		return;
	}
	assert(!data_.back().empty());

	if(!back.attribute_range().empty() )
	{
		ERR_NW << "found unexpected attribute:" <<back.debug() << std::endl;
		this->data_.pop_back();
		//ignore those here
	}
	assert(!data_.back().empty());
	//there should be no attributes left.
}

bool playturn_network_adapter::is_at_end()
{
	assert(!data_.empty());
	if (data_.size() > 1) return false;
	return this->next_ == data_.back().ordered_end();
}

void playturn_network_adapter::push_front(config&& cfg)
{
	data_front_.emplace_front(std::move(cfg));
}

bool playturn_network_adapter::read(config& dst)
{
	assert(dst.empty());
	if(!data_front_.empty())
	{
		dst = std::move(data_front_.back());
		data_front_.pop_back();
		return true;
	}
	if(is_at_end())
	{
		read_from_network();
	}
	if(is_at_end())
	{
		//that means we couldn't read anything from the network.
		return false;
	}
	//skip empty data.
	while(next_ == data_.begin()->ordered_end())
	{
		data_.pop_front();
		next_ = data_.front().ordered_begin();
		assert(!is_at_end());
	}
	config& child = dst.add_child(next_->key);
	config& child_old = next_->cfg;
	if(next_->key == "turn")
	{
		//split [turn] indo different [turn] for each child.
		assert(next_->cfg.all_children_count() > next_command_num_);
		config::all_children_iterator itor = child_old.ordered_begin();
		std::advance(itor, next_command_num_);
		config& childchild_old = itor->cfg;
		config& childchild = child.add_child(itor->key);
		childchild.swap(childchild_old);

		++next_command_num_;
		if(next_->cfg.all_children_count() == next_command_num_)
		{
			next_command_num_ = 0;
			++next_;
		}
		return true;
	}
	else
	{
		child.swap(child_old);
		++next_;
		return true;
	}
}

playturn_network_adapter::playturn_network_adapter(source_type source)
	: network_reader_(source)
	, data_({config()})
	, data_front_()
	, next_(data_.front().ordered_end())
	, next_command_num_(0)
	
{

}


playturn_network_adapter::~playturn_network_adapter()
{
	try {
		if(!is_at_end())
		{
			LOG_NW << "Destroying playturn_network_adapter with an non empty buffer, this means loss of network data" << std::endl;
		}
	} catch (...) {}
}

void playturn_network_adapter::set_source(source_type source)
{
	network_reader_ = source;
}


static bool read_config(config& src, config& dst)
{
	assert(dst.empty());
	if(!src.empty())
	{
		src.swap(dst);
		return true;
	}
	else
	{
		return false;
	}
}

playturn_network_adapter::source_type playturn_network_adapter::get_source_from_config(config& cfg)
{
	return std::bind(read_config, std::ref(cfg), _1);
}
