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

#include "config.hpp"
#include "log.hpp"
#include "filesystem.hpp"
#include "serialization/parser.hpp"
#include "serialization/binary_or_text.hpp"

#include "ban.hpp"

#include <sstream>

namespace wesnothd {

#define LOG_SERVER LOG_STREAM(info, mp_server)
#define DBG_SERVER LOG_STREAM(debug, mp_server)

	bool banned_compare::operator()(const banned_ptr a, const banned_ptr b) const
	{
		return (*a) > (*b);
	}

	banned::banned(const std::string& ip, const time_t end_time, const std::string& reason) : ip_(ip), end_time_(end_time), reason_(reason), deleted_(false)
	{
	}

	banned::banned(const config& cfg) :
		ip_(),
		end_time_(0),
		reason_(),
		deleted_(false)
	{
		read(cfg);
	}

	void banned::read(const config& cfg)
	{
		ip_ 	  = cfg["ip"];
		end_time_ = lexical_cast<time_t>(cfg["end_time"]);
		reason_	  = cfg["reason"];
		deleted_  = utils::string_bool(cfg["deleted"]);
	}

	void banned::write(config& cfg) const
	{
		std::stringstream ss;
		cfg["ip"]		= ip_;
		ss << end_time_;
		cfg["end_time"] = ss.str();
		cfg["reason"]	= reason_;
		cfg["deleted"]	= deleted_ ? "yes":"no";
	}

	std::string banned::get_human_end_time() const
	{
		if (end_time_ == 0)
		{
			return "permanent";
		}
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
	
	void ban_manager::read()
	{
		if (filename_.empty() || !file_exists(filename_))
			return;
		LOG_SERVER << "Reading bans from " <<  filename_ << "\n";
		config cfg;
		scoped_istream ban_file = istream_file(filename_);
		read_gz(cfg, *ban_file);	
		const config::child_list& bans = cfg.get_children("ban");
		for (config::child_list::const_iterator itor = bans.begin();
				itor != bans.end(); ++itor)
		{
			banned_ptr new_ban(new banned(**itor));
			if (!new_ban->is_deleted())
			{
				bans_[new_ban->get_ip()] = new_ban;
			}
			if (new_ban->get_end_time() != 0)
				time_queue_.push(new_ban);
		}
	}

	void ban_manager::write()
	{
		if (filename_.empty() || !dirty_)
			return;
		LOG_SERVER << "Writing bans to " <<  filename_ << "\n";
		dirty_ = false;
		config cfg;
		for (ban_map::const_iterator itor = bans_.begin();
				itor != bans_.end(); ++itor)
		{
			config& child = cfg.add_child("ban");
			itor->second->write(child);
		}
		scoped_ostream ban_file = ostream_file(filename_);
		config_writer writer(*ban_file, true, "");
		writer.write(cfg);
	}

	time_t ban_manager::parse_time(std::string time_in) const
	{
		time_t ret;
		ret = time(NULL);
		if (time_in.substr(0,4) == "TIME")
		{
			struct tm* loc;
			loc = localtime(&ret);

			std::string::iterator i = time_in.begin() + 4;
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
						case 'Y':
							multipler = 365*24*60*60; // a year;
							break;
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
							DBG_SERVER << "Wrong time multipler code given: '" << *i << "'. Assuming this is begin of comment.\n";
							ret = number = multipler = 0;
							break;
					}
					ret += number * multipler;
					if (multipler == 0)
						break;
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
		dirty_ = true;
		ban_map::iterator ban;
		if ((ban = bans_.find(ip)) != bans_.end())
		{
			// Already exsiting ban for ip. We have to first remove it
			ban->second->remove_ban();
			bans_.erase(ban);
		}
		banned_ptr new_ban(new banned(ip, end_time, reason));
		bans_.insert(ban_map::value_type(ip,new_ban));
		if (end_time != 0)
			time_queue_.push(new_ban);
	}

	void ban_manager::unban(std::ostringstream& os, const std::string& ip)
	{
		dirty_ = true;
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
			banned_ptr ban = time_queue_.top();

			if (ban->get_end_time() > time_now)
			{
				// No bans going to expire
				DBG_SERVER << "ban " << ban->get_ip() << " not removed. time: " << time_now << " end_time " << ban->get_end_time() << "\n";
				break;
			}

			if (ban->is_deleted())
			{
				// This was allready deleted have to free memory;
				time_queue_.pop();
				continue;
			}

			// No need to make dirty because
			// these bans will be handled correctly in next load.

			// This ban is going to expire so delete it.
			LOG_SERVER << "Remove a ban " << ban->get_ip() << ". time: " << time_now << " end_time " << ban->get_end_time() << "\n";

			bans_.erase(bans_.find(ban->get_ip()));
			time_queue_.pop();

		}
		// Save bans if there is any new ones
		write();
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
		ban_help_ = "ban <ip|nickname> [<time>] [<reason>]\nTime is give in format ‰d[‰s[‰d‰s[...]]] (where ‰s is s, m, h, D, M or Y).\nIf no time modifier is given minutes are used.\n";
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
		ban_help_ += "ban 127.0.0.1 2h20m flooded lobby\nban 127.0.0.2 medium flooded lobby again\n";
	}

	void ban_manager::load_config(const config& cfg)
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
		filename_ = cfg["ban_save_file"];
	}

	ban_manager::~ban_manager()
	{
	}
	
	ban_manager::ban_manager() : bans_(), time_queue_(), ban_times_(), ban_help_(), filename_(), dirty_(false)
	{
		init_ban_help();
	}	


}
