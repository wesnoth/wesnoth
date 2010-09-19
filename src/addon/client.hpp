/* $Id$ */
/*
   Copyright (C) 2010 by Maciej Pawlowski <>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/


#ifndef ADDON_CLIENT_HPP_INCLUDED
#define ADDON_CLIENT_HPP_INCLUDED

#include "config.hpp"
#include "network_progress_data.hpp"

#include <boost/function.hpp>
#include <boost/thread/thread.hpp>
#include <stdexcept>
#include <string>
#include <vector>
#include <map>
#include <curl/curl.h>

namespace network {

/*
exception thrown by various unsafe methods of addon_client
*/
class addon_client_error :
	public std::runtime_error
{
public:
	addon_client_error(const std::string& why);

	~addon_client_error() throw() {}
};

/*
HTTP client to access remote addon server
example usage:

network::addon_client ac;
ac.set_base_url("http://localhost:8000/addons");
std::cout<<ac.get_addon_list()<<std::endl;
std::cout<<ac.get_addon_description(2)<<std::endl;

*/
class addon_client
{
public:
	addon_client();
	~addon_client();

	//include 'http://' in base_url, ex. http://wesnoth.org/addons/
	void set_base_url(const std::string& base_url);

	//simple versions, will block
	std::string get_addon_description(unsigned int addon_id);
	std::string get_addon_list();
	config get_addon_list_cfg();
	std::string get_addon(const std::string& addon_name);
	config get_addon_cfg(unsigned int addon_id);
	config get_addon_cfg(const std::string& addon_name);
	bool is_addon_valid(
		const config& pbl, 
		const std::string& login,
		const std::string& pass,
		std::string& error_message);

	std::string publish_addon(
		const config& addon, 
		const std::string& login,
		const std::string& pass);

	std::string delete_remote_addon(
		const std::string& addon_name,
		const std::string& login,
		const std::string& pass);
	
	/*
	Async versions, won't block and can be aborted. 
	Will report progress through progress_data. 
	Get results by callingget_async_response() when progress_data::running() == false.
	Abort by calling progress_data::set_abort(true)
	*/
	void async_get_addon_list(progress_data& pd);
	void async_get_addon(progress_data& pd, const std::string& name);
	void async_publish_addon(
		progress_data& pd, 
		const config& addon, 
		const std::string& login,
		const std::string& pass);
	void async_delete_remote_addon(
		progress_data& pd, 
		const std::string& addon_name,
		const std::string& login,
		const std::string& pass);

	//waits for the last async_* operation to finish
	//returns immediately if it's already finished
	void async_wait();

	//returns string that should be parsable wml unless there was
	//a weird server error or something
	std::string get_async_response();

protected:
	std::string base_url_;
	CURL* handle_;
	char error_buffer_[CURL_ERROR_SIZE];
	std::string async_response_buffer_;
	boost::thread thread_;
	void flush(const std::string& buffer);

	static size_t default_recv_callback(
		void* buffer, 
		size_t size, 
		size_t nmemb, 
		void* userp);
	static int upload_progress_callback(
		void *clientp,
		double dltotal,
		double dlnow,
		double ultotal,
		double ulnow);
	static int download_progress_callback(
		void *clientp,
		double dltotal,
		double dlnow,
		double ultotal,
		double ulnow);

	void async_entry(
		progress_data* pd,
		bool download,
		boost::function<std::string (void)> blocking_fun);

		void async_wrap(progress_data& pd, bool down, boost::function<std::string()>);

		typedef std::map<std::string, std::string> string_map_t;

	std::string get_response(std::string url,
		const string_map_t& arguments,
		bool post = false);

	std::string get_response(const std::string& url);

	std::string url_encode(const std::string& raw_string) const;
};

}

#endif
