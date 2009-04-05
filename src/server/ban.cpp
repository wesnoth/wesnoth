/* $Id$ */
/*
   Copyright (C) 2008 - 2009 by Pauli Nieminen <paniemin@cc.hut.fi>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "config.hpp"
#include "foreach.hpp"
#include "log.hpp"
#include "filesystem.hpp"
#include "serialization/parser.hpp"
#include "serialization/binary_or_text.hpp"

#include "ban.hpp"


#include <boost/bind.hpp>

namespace wesnothd {

#define ERR_SERVER LOG_STREAM(err, mp_server)
#define LOG_SERVER LOG_STREAM(info, mp_server)
#define DBG_SERVER LOG_STREAM(debug, mp_server)

	std::ostream& operator<<(std::ostream& o, const banned& n)
	{
	   return o << "IP: " << n.get_ip() <<
					(n.get_nick().empty() ? "" : "  nick: " + n.get_nick()) <<
					"  reason: '" << n.get_reason() << "'"
					"\nstart_time: " << n.get_human_start_time() <<
					"  end_time: " << n.get_human_end_time() <<
					"  issuer: " <<  n.get_who_banned();
	}

	bool banned_compare::operator()(const banned_ptr& a, const banned_ptr& b) const
	{
		return (*a) > (*b);
	}

	banned_compare_subnet::compare_fn banned_compare_subnet::active_ = &banned_compare_subnet::less;

	void banned_compare_subnet::set_use_subnet_mask(bool use)
	{
		if (use)
		{
			assert(active_ == &banned_compare_subnet::less);
			active_ = &banned_compare_subnet::less_with_subnet;
		}
		else
		{
			assert(active_ != &banned_compare_subnet::less);
			active_ = &banned_compare_subnet::less;
		}
	}

	bool banned_compare_subnet::operator()(const banned_ptr& a, const banned_ptr& b) const
	{
		return (this->*(active_))(a,b);
	}

	bool banned_compare_subnet::less(const banned_ptr& a, const banned_ptr& b) const
	{
		return a->get_int_ip() < b->get_int_ip();
	}

	bool banned_compare_subnet::less_with_subnet(const banned_ptr& a, const banned_ptr& b) const
	{
		return a->get_mask_ip(b->mask()) < b->get_mask_ip(a->mask());
	}

	subnet_compare_setter::subnet_compare_setter()
	{
		banned_compare_subnet::set_use_subnet_mask(true);
	}

	subnet_compare_setter::~subnet_compare_setter()
	{
		banned_compare_subnet::set_use_subnet_mask(false);
	}
	const std::string banned::who_banned_default_ = "system";

	banned_ptr banned::create_dummy(const std::string& ip)
	{
		banned_ptr dummy(new banned(ip));
		return dummy;
	}

	banned::banned(const std::string& ip) :
		ip_(0),
		mask_(0),
		ip_text_(),
		end_time_(0),
		start_time_(0),
		reason_(),
		who_banned_(who_banned_default_),
		group_(),
		nick_()
	{
		ip_mask pair = parse_ip(ip);
		ip_ = pair.first;
		mask_ = 0xFFFFFFFF;
	}

	banned::banned(const std::string& ip,
				   const time_t end_time,
				   const std::string& reason,
				   const std::string& who_banned,
				   const std::string& group,
				   const std::string& nick) :
		ip_(0),
		mask_(0),
		ip_text_(ip),
		end_time_(end_time),
		start_time_(time(0)),
		reason_(reason),
		who_banned_(who_banned),
		group_(group),
		nick_(nick)
	{
		ip_mask pair = parse_ip(ip_text_);
		ip_ = pair.first;
		mask_ = pair.second;
	}

	banned::banned(const config& cfg) :
		ip_(0),
		mask_(0),
		ip_text_(),
		end_time_(0),
		start_time_(0),
		reason_(),
		who_banned_(who_banned_default_),
		group_(),
		nick_()
	{
		read(cfg);
	}

	ip_mask parse_ip(const std::string& ip)
	{
		// We use bit operations to construct the integer
		// ip_mask is a pair: first is ip and second is mask
		ip_mask ret;
		ret.first = 0;
		ret.second = 0;
		std::vector<std::string> split_ip = utils::split(ip, '.');
		unsigned int shift = 4*8; // start shifting from the highest byte
		unsigned int mask = 0xFF000000;
		const unsigned int complite_part_mask = 0xFF;
		std::vector<std::string>::const_iterator part = split_ip.begin();
		bool wildcard = false;
		do {
			shift -= 8;
			mask >>= 8;
			if (part == split_ip.end())
			{
				if (!wildcard)
					throw banned::error("Malformed ip address given for ban: " + ip);
				// Adding 0 to ip and mask is nop
				// we can then break out of loop
				break;
			} else {
				if (*part == "*")
				{
					wildcard = true;
					// Adding 0 to ip and mask is nop
				} else {
					wildcard = false;
					unsigned int part_ip = lexical_cast_default<unsigned int>(*part, complite_part_mask + 1);
					if (part_ip > complite_part_mask)
						throw banned::error("Malformed ip address given for ban: " + ip);
					ret.first |= (part_ip << shift);
					ret.second |= (complite_part_mask << shift);
				}
			}
			++part;
		} while (shift);
		return ret;
	}

	void banned::read(const config& cfg)
	{
		{
			// parse ip and mask
			ip_text_ = cfg["ip"];
			ip_mask pair = parse_ip(ip_text_);
			ip_ = pair.first;
			mask_ = pair.second;
		}
		nick_ = cfg["nick"];
		if (cfg.has_attribute("end_time"))
			end_time_ 	= lexical_cast_default<time_t>(cfg["end_time"], 0);
		if (cfg.has_attribute("start_time"))
			start_time_ 	= lexical_cast_default<time_t>(cfg["start_time"], 0);
		reason_	  	= cfg["reason"];

		// only overwrite defaults if exists
		if (cfg.has_attribute("who_banned"))
			who_banned_ = cfg["who_banned"];
		if (cfg.has_attribute("group"))
			group_ = cfg["group"];
	}

	void banned::write(config& cfg) const
	{
		cfg["ip"]		= get_ip();
		cfg["nick"] = get_nick();
		if (end_time_ > 0)
		{
			std::stringstream ss;
			ss << end_time_;
			cfg["end_time"] = ss.str();
		}
		if (start_time_ > 0)
		{
			std::stringstream ss;
			ss << start_time_;
			cfg["start_time"] = ss.str();
		}

		cfg["reason"]	= reason_;
		if (who_banned_ != who_banned_default_)
		{
			cfg["who_banned"] = who_banned_;
		}
		if (!group_.empty())
		{
			cfg["group"] = group_;
		}
	}

	std::string banned::get_human_start_time() const
	{
		if (start_time_ == 0)
			return "unknown";
		return banned::get_human_time(start_time_);
	}

	std::string banned::get_human_time(const time_t& time)
	{
		char buf[30];
		struct tm* local;
		local = localtime(&time);
		strftime(buf,30,"%H:%M:%S %d.%m.%Y", local );
		return std::string(buf);

	}

	std::string banned::get_human_end_time() const
	{
		if (end_time_ == 0)
		{
			return "permanent";
		}
		return banned::get_human_time(end_time_);
	}

	bool banned::operator>(const banned& b) const
	{
		return end_time_ > b.get_end_time();
	}

	unsigned int banned::get_mask_ip(unsigned int mask) const
	{
		return ip_ & mask & mask_;
	}

	bool banned::match_ip(const ip_mask& pair) const {
		return (ip_ & mask_) == (pair.first & mask_);
	}

	void ban_manager::read()
	{
		if (filename_.empty() || !file_exists(filename_))
			return;
		LOG_SERVER << "Reading bans from " <<  filename_ << "\n";
		config cfg;
		scoped_istream ban_file = istream_file(filename_);
		read_gz(cfg, *ban_file);

		foreach (const config &b, cfg.child_range("ban"))
		{
			try {
				banned_ptr new_ban(new banned(b));
				assert(bans_.insert(new_ban).second);

				if (new_ban->get_end_time() != 0)
					time_queue_.push(new_ban);
			} catch (banned::error& e) {
				ERR_SERVER << e.message << " while reading bans\n";
			}
		}

		// load deleted too
		if (const config &cfg_del = cfg.child("deleted"))
		{
			foreach (const config &b, cfg_del.child_range("ban"))
			{
				try {
					banned_ptr new_ban(new banned(b));
					deleted_bans_.push_back(new_ban);
				} catch (banned::error& e) {
					ERR_SERVER << e.message << " while reading deleted bans\n";
				}
			}
		}


	}

	void ban_manager::write()
	{
		if (filename_.empty() || !dirty_)
			return;
		LOG_SERVER << "Writing bans to " <<  filename_ << "\n";
		dirty_ = false;
		config cfg;
		for (ban_set::const_iterator itor = bans_.begin();
				itor != bans_.end(); ++itor)
		{
			config& child = cfg.add_child("ban");
			(*itor)->write(child);
		}
		config& deleted = cfg.add_child("deleted");
		for (deleted_ban_list::const_iterator itor = deleted_bans_.begin();
				itor != deleted_bans_.end(); ++itor)
		{
			config& child = deleted.add_child("ban");
			(*itor)->write(child);
		}

		scoped_ostream ban_file = ostream_file(filename_);
		config_writer writer(*ban_file, true);
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
				if (is_digit(*i))
				{
					number = number * 10 + to_digit(*i);
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
							LOG_SERVER << "Invalid time modifier given: '" << *i << "'.\n";
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
			const size_t default_multipler = 60; // default to minutes
			size_t multipler = default_multipler;
			std::string::iterator i = time_in.begin();
			size_t number = 0;
			for (; i != time_in.end(); ++i)
			{
				if (is_digit(*i))
				{
					number = number * 10 + to_digit(*i);
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
							DBG_SERVER << "Invalid time modifier given: '" << *i << "'. Assuming this is the begin of the reason.\n";
							ret = number = multipler = 0;
							break;
					}
					if (multipler == 0)
						break;
					if (number == 0)
						number = 1;
					ret += number * multipler;
					multipler = default_multipler;
					number = 0;
				}
			}
			--i;
			if (is_digit(*i))
			{
					ret += number * multipler;
			}
		}
		return ret;
	}

	std::string ban_manager::ban(const std::string& ip,
								 const time_t& end_time,
								 const std::string& reason,
								 const std::string& who_banned,
								 const std::string& group,
								 const std::string& nick)
	{
		try {
			ban_set::iterator ban;
			if ((ban = bans_.find(banned::create_dummy(ip))) != bans_.end())
			{
				// Already exsiting ban for ip. We have to first remove it
				LOG_SERVER << "Overwriting ban: " << (**ban) << "\n";
				bans_.erase(ban);
			}
		} catch (banned::error& e) {
			ERR_SERVER << e.message << " while creating dummy ban for finding existing ban\n";
			return e.message;
		}
		std::ostringstream ret;
		try {
			banned_ptr new_ban(new banned(ip, end_time, reason,who_banned, group, nick));
			bans_.insert(new_ban);
			if (end_time != 0)
				time_queue_.push(new_ban);
			ret << *new_ban;
		} catch (banned::error& e) {
			ERR_SERVER << e.message << " while banning\n";
			return e.message;
		}
		dirty_ = true;
		return ret.str();
	}

	void ban_manager::unban(std::ostringstream& os, const std::string& ip)
	{
		ban_set::iterator ban;
		try {
			ban = bans_.find(banned::create_dummy(ip));
		} catch (banned::error& e) {
			ERR_SERVER << e.message << "\n";
			os << e.message;
			return;
		}

		if (ban == bans_.end())
		{
			os << "There is no ban on '" << ip << "'.";
			return;
		}
		// keep ban entry still in memory
		os << "Ban on '" << **ban << "' removed.";
		// group bans don't get saved
		if ((*ban)->get_group().empty()) deleted_bans_.push_back(*ban);
		bans_.erase(ban);
		dirty_ = true;

	}

	void ban_manager::unban_group(std::ostringstream& os, const std::string& group)
	{
		ban_set temp;
		std::insert_iterator<ban_set> temp_inserter(temp, temp.begin());
		std::remove_copy_if(bans_.begin(), bans_.end(), temp_inserter, boost::bind(&banned::match_group,boost::bind(&banned_ptr::get,_1),group));

		os << "Removed " << (bans_.size() - temp.size()) << " bans";
		bans_.swap(temp);
		dirty_ = true;
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

			// This ban is going to expire so delete it.
			LOG_SERVER << "Remove a ban " << ban->get_ip() << ". time: " << time_now << " end_time " << ban->get_end_time() << "\n";
			std::ostringstream os;
			unban(os, ban->get_ip());
			time_queue_.pop();

		}
		// Save bans if there is any new ones
		write();
	}

	void ban_manager::list_deleted_bans(std::ostringstream& out) const
	{
		if (deleted_bans_.empty())
		{
			out << "No removed bans found.";
			return;
		}

		for (deleted_ban_list::const_iterator i = deleted_bans_.begin();
				i != deleted_bans_.end();
				++i)
		{
			if (i != deleted_bans_.begin())
				out << "\n";
			out << (**i);
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
		std::set<std::string> groups;

		for (ban_set::const_iterator i = bans_.begin();
				i != bans_.end(); ++i)
		{
			if ((*i)->get_group().empty())
			{
				if (i != bans_.begin())
					out << "\n";
				out << (**i);
			} else {
				groups.insert((*i)->get_group());
			}
		}

		if (!groups.empty())
		{
			out << "\nban groups: ";

			out << *groups.begin();
			std::ostream& (*fn)(std::ostream&,const std::string&) = &std::operator<<;
			std::for_each( ++groups.begin(), groups.end(), boost::bind(fn,boost::bind(fn,boost::ref(out),std::string(", ")),_1));
		}

	}


	std::string ban_manager::is_ip_banned(const std::string& ip) const
	{
		ip_mask pair;
		try {
			pair = parse_ip(ip);
		} catch (banned::error&) {
			return "";
		}
		ban_set::const_iterator ban = std::find_if(bans_.begin(), bans_.end(), boost::bind(&banned::match_ip, boost::bind(&banned_ptr::get, _1), pair));
		if (ban == bans_.end()) return "";
		return (*ban)->get_reason();
	}

	void ban_manager::init_ban_help()
	{
		ban_help_ = "ban <ip|nickmask> [<time>] <reason>\n"
				"The time format is: %d[%s[%d[%s[...]]]] where %s is a time modifier: s (seconds), m (minutes), h (hours), D (days), M (months) or Y (years) and %d is a number.\n"
				"If no time is given then the ban is permanent.\n";
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
		ban_help_ += "ban 127.0.0.1 2h20m flooded lobby\n"
				"kban suokko 5D flooded again\n"
				"kban suokko Y One year ban for constant flooding";
	}

	void ban_manager::load_config(const config& cfg)
	{
		ban_times_.clear();
		foreach (const config &bt, cfg.child_range("ban_time")) {
			ban_times_.insert(default_ban_times::value_type(bt["name"],
				parse_time(bt["time"]) - time(NULL)));
		}
		init_ban_help();
		if (filename_ != cfg["ban_save_file"])
		{
			dirty_ = true;
			filename_ = cfg["ban_save_file"];
		}
	}

	ban_manager::~ban_manager()
	{
		write();
	}

	ban_manager::ban_manager()
		: bans_()
		, deleted_bans_()
		, time_queue_()
		, ban_times_()
		, ban_help_()
		, filename_()
		, dirty_(false)
	{
		init_ban_help();
	}


}
