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

#include "../log.hpp"
#include "../config.hpp"

#include "ban.hpp"
#include <sstream>

namespace wesnothd {

#define LOG_SERVER LOG_STREAM(info, mp_server)
#define DBG_SERVER LOG_STREAM(debug, mp_server)

	bool banned_compare::operator()(const banned* a, const banned* b) const
	{
		return (*a) > (*b);
	}

	banned::banned(const std::string& ip, const time_t end_time, const std::string& reason) : ip_(ip), end_time_(end_time), reason_(reason), deleted_(false)
	{
	}

	std::string banned::get_human_end_time() const
	{
		char buf[30];
		struct tm* local;
		local = localtime(&end_time_);
		strftime(buf,30,"%H:%M:%S %d.%m.%Y", local );
		return std::string(buf);
	}
	
	bool banned::operator>(const banned& b) const
	{
		return end_time_ > b.get_end_time();
	}

	time_t ban_manager::parse_time(std::string time_in) const
	{
		time_t ret;
		ret = time(NULL);
		if (time_in.substr(0,3) == "UTC")
		{
			struct tm* loc;
			loc = localtime(&ret);

			std::string::iterator i = time_in.begin() + 3;
			size_t number = 0;
			for (; i != time_in.end(); ++i)
			{
				if (is_number(*i))
				{
					number = number*10 + to_number(*i);
				}
				else
				{
					switch(*i)
					{
						case 'Y':
							loc->tm_year = number;
							break;
						case 'M':
							loc->tm_mon = number;
							break;
						case 'D':
							loc->tm_mday = number;
							break;
						case 'h':
							loc->tm_hour = number;
							break;
						case 'm':
							loc->tm_min = number;
							break;
						case 's':
							loc->tm_sec = number;
							break;
						default:
							LOG_SERVER << "Wrong time code for ban: " << *i << "\n";
							break;
					}
					number = 0;
				}
			}
			return mktime(loc);
		}
		default_ban_times::const_iterator time_itor = ban_times_.find(time_in);	
		if (time_itor != ban_times_.end())
			ret += time_itor->second;
		else
		{
			size_t multipler = 60; // default minutes
			std::string::iterator i = time_in.begin();
			size_t number = 0;
			for (; i != time_in.end(); ++i)
			{
				if (is_number(*i))
				{
					number = number * 10  + to_number(*i);
				} else {
					switch(*i)
					{
						case 'M':
							multipler = 30*24*60*60; // 30 days
							break;
						case 'D':
							multipler = 24*60*60;
							break;
						case 'h':
							multipler = 60*60;
							break;
						case 'm':
							multipler = 60;
							break;
						case 's':
							multipler = 1;
							break;
						default:
							LOG_SERVER << "Wrong time multipler code given: " << *i << "\n";
							break;
					}
					ret += number * multipler;
				}
			}
			--i;
			if (is_number(*i))
			{
					ret += number * multipler;
			}
		}
		return ret;
	}

	void ban_manager::ban(const std::string& ip, const time_t& end_time, const std::string& reason)
	{
		ban_map::iterator ban;
		if ((ban = bans_.find(ip)) != bans_.end())
		{
			// Already exsiting ban for ip. We have to first remove it
			ban->second->remove_ban();
			bans_.erase(ban);
		}
		banned *new_ban = new banned(ip, end_time, reason);
		bans_.insert(ban_map::value_type(ip,new_ban));
		time_queue_.push(new_ban);
	}

	void ban_manager::unban(std::ostringstream& os, const std::string& ip)
	{
		ban_map::iterator ban = bans_.find(ip);
		if (ban == bans_.end())
		{
			os << "There is no ban on '" << ip << "'.";
			return;
		}
		ban->second->remove_ban();
		bans_.erase(ban);

		os << "Ban on '" << ip << "' removed.";
	}

	void ban_manager::check_ban_times(time_t time_now)
	{
		while (!time_queue_.empty())
		{
			banned* ban = time_queue_.top();

			if (ban->get_end_time() > time_now)
			{
				// No bans going to expire
				LOG_SERVER << "ban " << ban->get_ip() << " not removed. time: " << time_now << " end_time " << ban->get_end_time() << "\n";
				break;
			}

			if (ban->is_deleted())
			{
				// This was allready deleted have to free memory;
				time_queue_.pop();
				delete ban;
				continue;
			}

			// This ban is going to expire so delete it.
			LOG_SERVER << "Remove a ban " << ban->get_ip() << ". time: " << time_now << " end_time " << ban->get_end_time() << "\n";

			bans_.erase(bans_.find(ban->get_ip()));
			time_queue_.pop();
			delete ban;

		}
	}

	void ban_manager::list_bans(std::ostringstream& out) const
	{
		if (bans_.empty()) 
		{ 
			out << "No bans set.";
			return;
		}

		out << "BAN LIST\n";
		for (ban_map::const_iterator i = bans_.begin();
				i != bans_.end(); ++i)
		{
			out << "IP: '" << i->second->get_ip() << 
				"' end_time: '" << i->second->get_human_end_time() <<
				"' reason: '" << i->second->get_reason() << "'\n";
		}

	}

	bool ban_manager::is_ip_banned(std::string ip) const 
	{
		for (ban_map::const_iterator i = bans_.begin(); i != bans_.end(); ++i) {
			if (utils::wildcard_string_match(ip, i->first)) {
				DBG_SERVER << "Comparing ban '" << i->first << "' vs '..." << ip << "'\t" << "banned.\n";
				return true;
			}
			DBG_SERVER << "Comparing ban '" << i->first << "' vs '..." << ip << "'\t" << "not banned.\n";
		}
		return false;
	}
	
	void ban_manager::init_ban_help()
	{
		ban_help_ = "ban <ip|nickname> <time> [<reason>]\nTime is give in formar ‰d[‰s‰d‰s...] (where ‰sis s, m, h, D or M).\nIf no time modifier is given minutes are used.\n";
		default_ban_times::iterator itor = ban_times_.begin();
		if (itor != ban_times_.end())
		{
			ban_help_ += "You can also use " + itor->first;
			++itor;
		}
		for (; itor != ban_times_.end(); ++itor)
		{
			ban_help_ += std::string(", ") + itor->first;
		}
		if (!ban_times_.empty())
		{
			ban_help_ += " for standard ban times.\n";
		}
		ban_help_ += "ban 127.0.0.1 2H20m flooded lobby\nban 127.0.0.2 medium flooded lobby again\n";
	}

	void ban_manager::set_default_ban_times(const config& cfg)
	{
		ban_times_.clear();
		const config::child_list& times = cfg.get_children("ban_time");
		for (config::child_list::const_iterator itor = times.begin();
				itor != times.end(); ++itor)
		{
			ban_times_.insert(default_ban_times::value_type((**itor)["name"],
						parse_time((**itor)["time"])-time(NULL)));
		}
		init_ban_help();
	}

	ban_manager::~ban_manager()
	{
		bans_.clear();
		while(!time_queue_.empty())
		{
			banned* ban = time_queue_.top();
			delete ban;
			time_queue_.pop();
		}
	}
	
	ban_manager::ban_manager() : bans_(), time_queue_(), ban_times_(), ban_help_()
	{
		init_ban_help();
	}	


}
