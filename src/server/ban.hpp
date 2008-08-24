/* $Id$ */
/*
   Copyright (C) 2008 by Pauli Nieminen <paniemin@cc.hut.fi>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef SERVER_GAME_HPP_INCLUDED
#define SERVER_GAME_HPP_INCLUDED
#include <map>
#include <queue>
#include <ctime>

#include <boost/shared_ptr.hpp>

class config;

namespace wesnothd {

	class banned;

	typedef boost::shared_ptr<banned> banned_ptr;

	//! We want to move the lowest value to the top
	struct banned_compare {
		bool operator()(const banned_ptr a, const banned_ptr b) const;
	};

	typedef std::map<std::string, banned_ptr> ban_map;
	typedef std::priority_queue<banned_ptr,std::vector<banned_ptr>, banned_compare> ban_time_queue;
	typedef std::map<std::string, size_t> default_ban_times;


	class banned {
		std::string ip_;
		time_t end_time_;
		std::string reason_;
		bool deleted_;

	public:
		banned(const std::string& ip, const time_t end_time, const std::string& reason);
		banned(const config&);

		void read(const config&);
		void write(config&) const;

		time_t get_end_time() const
		{ return end_time_;	}

		std::string get_human_end_time() const;

		std::string get_reason() const
		{ return reason_; }

		std::string get_ip() const 
		{ return ip_; }

		void remove_ban()
		{ deleted_ = true; }

		bool is_deleted() const
		{ return deleted_; }

		//! Notice that comparision is done wrong way to make the smallest value in top of heap
		bool operator>(const banned& b) const;
	};

	class ban_manager
	{

		ban_map bans_;
		ban_time_queue time_queue_;
		default_ban_times ban_times_;
		std::string ban_help_;
		std::string filename_;
		bool dirty_;

		bool is_number(const char& c) const
		{ return c >= '0' && c <= '9'; }
		size_t to_number(const char& c) const
		{ return c - '0'; }

		void init_ban_help();
	public:
		ban_manager();
		~ban_manager();
		
		void read();
		void write() const;

		time_t parse_time(std::string time_in) const;

		void ban(const std::string&, const time_t&, const std::string&);
		void unban(std::ostringstream& os, const std::string& ip);

		void check_ban_times(time_t time_now);

		void list_bans(std::ostringstream& out) const;

		bool is_ip_banned(std::string ip) const;
		
		const std::string& get_ban_help() const
		{ return ban_help_; }	

		void load_config(const config&);

	};
}

#endif
