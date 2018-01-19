/*
   Copyright (C) 2008 - 2018 by Pauli Nieminen <paniemin@cc.hut.fi>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "exceptions.hpp"

#include <set>
#include <map>
#include <list>
#include <queue>
#include <ctime>

class config;

namespace wesnothd {

	class banned;

	std::ostream& operator<<(std::ostream& o, const banned& n);

	typedef std::shared_ptr<banned> banned_ptr;

	/** We want to move the lowest value to the top. */
	struct banned_compare {
		bool operator()(const banned_ptr& a, const banned_ptr& b) const;
	};

	struct banned_compare_subnet {
		bool operator()(const banned_ptr& a, const banned_ptr& b) const;
		private:
		bool less(const banned_ptr& a, const banned_ptr& b) const;
		typedef bool (banned_compare_subnet::*compare_fn)(const banned_ptr& a, const banned_ptr& b) const;
		static compare_fn active_;
	};

	typedef std::set<banned_ptr,banned_compare_subnet > ban_set;
	typedef std::list<banned_ptr> deleted_ban_list;
	typedef std::priority_queue<banned_ptr,std::vector<banned_ptr>, banned_compare> ban_time_queue;
	typedef std::map<std::string, size_t> default_ban_times;
	typedef std::pair<unsigned int, unsigned int> ip_mask;

	ip_mask parse_ip(const std::string&);

	class banned {
		unsigned int ip_;
		unsigned int mask_;
		std::string ip_text_;
		time_t end_time_;
		time_t start_time_;
		std::string reason_;
		std::string who_banned_;
		std::string group_;
		std::string nick_;
		static const std::string who_banned_default_;

		banned(const std::string& ip);

	public:
		banned(const std::string& ip, const time_t end_time, const std::string& reason, const std::string& who_banned=who_banned_default_, const std::string& group="", const std::string& nick="");
		banned(const config&);

		void read(const config&);
		void write(config&) const;

		time_t get_end_time() const
		{ return end_time_;	}

		std::string get_human_end_time() const;
		std::string get_human_start_time() const;
		std::string get_human_time_span() const;
		static std::string get_human_time(const time_t&);

		std::string get_reason() const
		{ return reason_; }

		std::string get_ip() const
		{ return ip_text_; }
		std::string get_group() const
		{ return group_; }

		std::string get_who_banned() const
		{ return who_banned_; }

		std::string get_nick() const
		{ return nick_; }

		bool match_group(const std::string& group) const
		{ return group_ == group; }

		bool match_ip(const ip_mask& ip) const;
		bool match_ipmask(const ip_mask& ip) const;

		unsigned int get_mask_ip(unsigned int) const;
		unsigned int get_int_ip() const
		{ return ip_; }

		unsigned int mask() const
		{ return mask_; }

		static banned_ptr create_dummy(const std::string& ip);

		bool operator>(const banned& b) const;

		struct error : public ::game::error {
			error(const std::string& message) : ::game::error(message) {}
		};
	};

	class ban_manager
	{

		ban_set bans_;
		deleted_ban_list deleted_bans_;
		ban_time_queue time_queue_;
		default_ban_times ban_times_;
		std::string ban_help_;
		std::string filename_;
		bool dirty_;

		bool is_digit(const char& c) const
		{ return c >= '0' && c <= '9'; }
		size_t to_digit(const char& c) const
		{ return c - '0'; }

		void init_ban_help();
		void check_ban_times(time_t time_now);
		inline void expire_bans() {
			check_ban_times(time(nullptr));
		}
	public:
		ban_manager();
		~ban_manager();

		void read();
		void write();

		/**
		 * Parses the given duration and adds it to *time except if the
		 * duration is '0' or 'permanent' in which case *time will be set to '0'.
		 * @returns false if an invalid time modifier is encountered.
		 * *time is undefined in that case.
		 */
		bool parse_time(const std::string& duration, time_t* time) const;

		std::string ban(const std::string&, const time_t&, const std::string&, const std::string&, const std::string&, const std::string& = "");
		void unban(std::ostringstream& os, const std::string& ip, bool immediate_write=true);
		void unban_group(std::ostringstream& os, const std::string& group);


		void list_deleted_bans(std::ostringstream& out, const std::string& mask = "*") const;
		void list_bans(std::ostringstream& out, const std::string& mask = "*");

		std::string is_ip_banned(const std::string& ip);

		const std::string& get_ban_help() const
		{ return ban_help_; }

		void load_config(const config&);

	};
}
