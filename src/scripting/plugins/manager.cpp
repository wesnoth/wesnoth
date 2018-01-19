/*
   Copyright (C) 2014 - 2018 by Chris Beck <render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "scripting/plugins/manager.hpp"

#include "scripting/application_lua_kernel.hpp"
#include "scripting/plugins/context.hpp"
#include <cassert>
#include <stdexcept>
#include <string>
#include <vector>

#include "log.hpp"

static lg::log_domain log_plugins("plugins");
#define DBG_PLG LOG_STREAM(debug, log_plugins)
#define LOG_PLG LOG_STREAM(info,  log_plugins)
#define WRN_PLG LOG_STREAM(warn,  log_plugins)
#define ERR_PLG LOG_STREAM(err,   log_plugins)


struct plugin {
	std::string name;
	std::string source;
	bool is_file;
	std::unique_ptr<application_lua_kernel::thread> thread;
	std::vector<plugins_manager::event> queue;
};

static plugins_manager * singleton = nullptr;

plugins_manager::plugins_manager(application_lua_kernel * kernel)
	: plugins_()
	, playing_()
	, kernel_(kernel)
{
	assert(!singleton);
	singleton = this;

	kernel_->load_core();
	add_plugin("Null Plugin", "return function() end");
	start_plugin(0);
}

plugins_manager::~plugins_manager() {}

plugins_manager * plugins_manager::get()
{
	return singleton;
}

lua_kernel_base * plugins_manager::get_kernel_base()
{
	return kernel_.get();
}

size_t plugins_manager::size() {
	return plugins_.size();
}

plugins_manager::STATUS plugins_manager::get_status(size_t idx) {
	if (idx < plugins_.size()) {
		if (!plugins_[idx].thread) {
			return plugins_manager::STATUS::NONE;
		} else {
			return plugins_[idx].thread->is_running() ? plugins_manager::STATUS::RUNNING : plugins_manager::STATUS::STOPPED;
		}
	}
	throw std::runtime_error("index out of bounds");
}

std::string plugins_manager::get_detailed_status(size_t idx) {
	if (idx < plugins_.size()) {
		if (!plugins_[idx].thread) {
			return "not loaded";
		} else {
			return plugins_[idx].thread->status();
		}
	}
	throw std::runtime_error("index out of bounds");
}

std::string plugins_manager::get_name(size_t idx) {
	if (idx < plugins_.size()) {
		return plugins_[idx].name;
	}
	throw std::runtime_error("index out of bounds");
}

void plugins_manager::start_plugin(size_t idx)
{
	DBG_PLG << "start_plugin[" << idx <<"]\n";
	if (idx < plugins_.size()) {
		if (!plugins_[idx].thread) {
			DBG_PLG << "creating thread[" << idx << "]\n";
			plugins_[idx].thread.reset(plugins_[idx].is_file ?
						kernel_->load_script_from_file(plugins_[idx].source) : kernel_->load_script_from_string(plugins_[idx].source));
			DBG_PLG << "finished [" << idx << "], status = '" << plugins_[idx].thread->status() << "'\n";
		} else {
			DBG_PLG << "thread already exists, skipping\n";
		}
		return ;
	}
	throw std::runtime_error("index out of bounds");
}

size_t plugins_manager::add_plugin(const std::string & name, const std::string & prog)
{
	size_t idx = plugins_.size();
	plugins_.push_back(new plugin);

	plugin & p = plugins_[idx];
	p.name = name;
	p.source = prog;
	p.is_file = false;

	return idx;
}

size_t plugins_manager::load_plugin(const std::string & name, const std::string & filename)
{
	size_t idx = plugins_.size();
	plugins_.push_back(new plugin);

	plugin & p = plugins_[idx];
	p.name = name;
	p.source = filename;
	p.is_file = true;

	return idx;
}

void plugins_manager::notify_event(const std::string & name, const config & data)
{
	event evt;
	evt.name = name;
	evt.data = data;

	for (size_t idx = 0; idx < size(); ++idx)
	{
		if (plugins_[idx].thread && plugins_[idx].thread->is_running()) {
			plugins_[idx].queue.push_back(evt);
		}
	}
}

void plugins_manager::play_slice(const plugins_context & ctxt)
{
	if (playing_) {
		*playing_ = false;	//this is to ensure "reentrancy" -- any previous calls to this function that never returned
					//and looped back into the plugins system, should be halted and their later requests discarded
					//this is to ensure the semantics that if a plugins context is left, then any pending requests
					//are discarded to prevent them from being executed at an improper time
	}
	playing_ = std::make_shared<bool> (true);
	std::shared_ptr<bool> local = playing_; //make a local copy of the pointer on the stack

	for (size_t idx = 0; idx < size(); ++idx)
	{
		DBG_PLG << "play_slice[" << idx << "] ... \n";
		if (plugins_[idx].thread && plugins_[idx].thread->is_running()) {
			DBG_PLG << "is running...";
			if (!*local) {			//check playing_ before each call to be sure that we should still continue
				DBG_PLG << "aborting\n";
				return;
			}

			std::vector<event> input = plugins_[idx].queue; //empty the queue to a temporary variable
			plugins_[idx].queue = std::vector<event>();

			//application_lua_kernel::requests_list requests =
			std::vector<std::function<bool(void)> > requests =
				plugins_[idx].thread->run_script(ctxt, input);

			DBG_PLG << "thread returned " << requests.size() << " requests\n";

			for (size_t j = 0; j < requests.size(); ++j) {
				if (!*local) return;		//check playing_ before each call to be sure that we should still continue
				if (!requests[j]()) {
					*local = false;
					return ; //call the function but if it returns false (error) then stop
				}
			}

			DBG_PLG << "play_slice[" << idx << "] finished.\n";
		} else if (!plugins_[idx].thread) {
			DBG_PLG << "thread ["<< idx << "] not created\n";
		} else {
			DBG_PLG << "thread ["<< idx << "] not running\n";
		}
	}
	*local = false;
}

bool plugins_manager::any_running()
{

	for (size_t i = 0; i < size(); ++i) {
		if (STATUS::RUNNING == get_status(i)) {
			return true;
		}
	}
	return false;
}
