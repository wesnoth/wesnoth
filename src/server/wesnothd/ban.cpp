/*
	Copyright (C) 2008 - 2022
	by Pauli Nieminen <paniemin@cc.hut.fi>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "config.hpp"
#include "filesystem.hpp"
#include "lexical_cast.hpp"
#include "log.hpp"
#include "serialization/binary_or_text.hpp"
#include "serialization/parser.hpp"
#include "serialization/string_utils.hpp"
#include "serialization/unicode.hpp"
#include "utils/general.hpp"

#include "server/wesnothd/ban.hpp"

#include <functional>

namespace wesnothd
{
static lg::log_domain log_server("server");
#define ERR_SERVER LOG_STREAM(err, log_server)
#define LOG_SERVER LOG_STREAM(info, log_server)
#define DBG_SERVER LOG_STREAM(debug, log_server)

std::ostream& operator<<(std::ostream& o, const banned& n)
{
	return o << "IP: " << n.get_ip() <<
		(n.get_nick().empty() ? "" : "  nick: " + n.get_nick()) <<
		"  reason: '" << n.get_reason() << "'"
		"  start_time: " << n.get_human_start_time() <<
		"  end_time: " << n.get_human_end_time() <<
		"  issuer: " <<  n.get_who_banned();
}

bool banned_compare::operator()(const banned_ptr& a, const banned_ptr& b) const
{
	return (*a) > (*b);
}

banned_compare_subnet::compare_fn banned_compare_subnet::active_ = &banned_compare_subnet::less;

bool banned_compare_subnet::operator()(const banned_ptr& a, const banned_ptr& b) const
{
	return (this->*(active_))(a,b);
}

bool banned_compare_subnet::less(const banned_ptr& a, const banned_ptr& b) const
{
	return a->get_int_ip() < b->get_int_ip();
}

const std::string banned::who_banned_default_ = "system";

banned::banned(const std::string& ip)
	: ip_(0)
	, mask_(0)
	, ip_text_()
	, end_time_(0)
	, start_time_(0)
	, reason_()
	, who_banned_(who_banned_default_)
	, group_()
	, nick_()
{
	ip_mask pair = parse_ip(ip);
	ip_ = pair.first;
	mask_ = 0xFFFFFFFF;
}

banned::banned(const std::string& ip,
		const std::time_t end_time,
		const std::string& reason,
		const std::string& who_banned,
		const std::string& group,
		const std::string& nick)
	: ip_(0)
	, mask_(0)
	, ip_text_(ip)
	, end_time_(end_time)
	, start_time_(std::time(0))
	, reason_(reason)
	, who_banned_(who_banned)
	, group_(group)
	, nick_(nick)
{
	ip_mask pair = parse_ip(ip_text_);
	ip_ = pair.first;
	mask_ = pair.second;
}

banned::banned(const config& cfg)
	: ip_(0)
	, mask_(0)
	, ip_text_()
	, end_time_(0)
	, start_time_(0)
	, reason_()
	, who_banned_(who_banned_default_)
	, group_()
	, nick_()
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
	if (split_ip.size() > 4) throw banned::error("Malformed ip address: " + ip);

	unsigned int shift = 4*8; // start shifting from the highest byte
	//unsigned int mask = 0xFF000000;
	const unsigned int complete_part_mask = 0xFF;
	auto part = split_ip.begin();
	bool wildcard = false;
	do {
		shift -= 8;
		//mask >>= 8;
		if(part == split_ip.end()) {
			if(!wildcard)
				throw banned::error("Malformed ip address: '" + ip + "'");
			// Adding 0 to ip and mask is nop
			// we can then break out of loop
			break;
		} else {
			if(*part == "*") {
				wildcard = true;
				// Adding 0 to ip and mask is nop
			} else {
				// wildcard = false;
				unsigned int part_ip = lexical_cast_default<unsigned int>(*part, complete_part_mask + 1);
				if(part_ip > complete_part_mask) {
					throw banned::error("Malformed ip address: '" + ip + "'");
				}

				ret.first |= (part_ip << shift);
				ret.second |= (complete_part_mask << shift);
			}
		}
		++part;
	} while(shift);

	return ret;
}

void banned::read(const config& cfg)
{
	{
		// parse ip and mask
		ip_text_ = cfg["ip"].str();
		ip_mask pair = parse_ip(ip_text_);
		ip_ = pair.first;
		mask_ = pair.second;
	}

	nick_ = cfg["nick"].str();

	if(cfg.has_attribute("end_time")) {
		end_time_ = cfg["end_time"].to_time_t(0);
	}

	if(cfg.has_attribute("start_time")) {
		start_time_ = cfg["start_time"].to_time_t(0);
	}

	reason_ = cfg["reason"].str();

	// only overwrite defaults if exists
	if(cfg.has_attribute("who_banned")) {
		who_banned_ = cfg["who_banned"].str();
	}

	if(cfg.has_attribute("group")) {
		group_ = cfg["group"].str();
	}
}

void banned::write(config& cfg) const
{
	cfg["ip"] = get_ip();
	cfg["nick"] = get_nick();

	if(end_time_ > 0) {
		std::stringstream ss;
		ss << end_time_;
		cfg["end_time"] = ss.str();
	}

	if(start_time_ > 0) {
		std::stringstream ss;
		ss << start_time_;
		cfg["start_time"] = ss.str();
	}

	cfg["reason"] = reason_;

	if(who_banned_ != who_banned_default_) {
		cfg["who_banned"] = who_banned_;
	}

	if(!group_.empty()) {
		cfg["group"] = group_;
	}
}

std::string banned::get_human_start_time() const
{
	if(start_time_ == 0) {
		return "unknown";
	}

	return lg::get_timestamp(start_time_);
}

std::string banned::get_human_end_time() const
{
	if(end_time_ == 0) {
		return "permanent";
	}

	return lg::get_timestamp(end_time_);
}

std::string banned::get_human_time_span() const
{
	if(end_time_ == 0) {
		return "permanent";
	}

	return lg::get_timespan(end_time_ - std::time(nullptr));
}

bool banned::operator>(const banned& b) const
{
	return end_time_ > b.get_end_time();
}

unsigned int banned::get_mask_ip(unsigned int mask) const
{
	return ip_ & mask & mask_;
}

bool banned::match_ip(const ip_mask& pair) const
{
	return (ip_ & mask_) == (pair.first & mask_);
}

// Unlike match_ip this function takes both masks into account.
bool banned::match_ipmask(const ip_mask& pair) const
{
	return (ip_ & mask_ & pair.second) == (pair.first & pair.second & mask_);
}

void ban_manager::read()
{
	if(filename_.empty() || !filesystem::file_exists(filename_)) {
		return;
	}

	LOG_SERVER << "Reading bans from " <<  filename_;
	config cfg;
	dirty_ = false;
	filesystem::scoped_istream ban_file = filesystem::istream_file(filename_);
	read_gz(cfg, *ban_file);

	for(const config& b : cfg.child_range("ban")) {
		try {
			auto new_ban = std::make_shared<banned>(b);
			assert(bans_.insert(new_ban).second);

			if (new_ban->get_end_time() != 0)
				time_queue_.push(new_ban);
		} catch(const banned::error& e) {
			ERR_SERVER << e.message << " while reading bans";
		}
	}

	// load deleted too
	if(const config& cfg_del = cfg.child("deleted")) {
		for(const config& b : cfg_del.child_range("ban")) {
			try {
				auto new_ban = std::make_shared<banned>(b);
				deleted_bans_.push_back(new_ban);
			} catch(const banned::error& e) {
				ERR_SERVER << e.message << " while reading deleted bans";
			}
		}
	}
}

void ban_manager::write()
{
	if(filename_.empty() || !dirty_) {
		return;
	}

	LOG_SERVER << "Writing bans to " << filename_;
	dirty_ = false;

	config cfg;
	for(const auto& b : bans_) {
		config& child = cfg.add_child("ban");
		b->write(child);
	}

	config& deleted = cfg.add_child("deleted");
	for(const auto& db : deleted_bans_) {
		config& child = deleted.add_child("ban");
		db->write(child);
	}

	filesystem::scoped_ostream ban_file = filesystem::ostream_file(filename_);
	config_writer writer(*ban_file, true);
	writer.write(cfg);
}

bool ban_manager::parse_time(const std::string& duration, std::time_t* time) const
{
	if (!time) return false;

	if(duration.substr(0, 4) == "TIME") {
		std::tm* loc;
		loc = std::localtime(time);

		std::size_t number = 0;
		for(auto i = duration.begin() + 4; i != duration.end(); ++i) {
			if(is_digit(*i)) {
				number = number * 10 + to_digit(*i);
			} else {
				switch(*i) {
				case 'Y':
					loc->tm_year = static_cast<int>(number);
					break;
				case 'M':
					loc->tm_mon = static_cast<int>(number);
					break;
				case 'D':
					loc->tm_mday = static_cast<int>(number);
					break;
				case 'h':
					loc->tm_hour = static_cast<int>(number);
					break;
				case 'm':
					loc->tm_min = static_cast<int>(number);
					break;
				case 's':
					loc->tm_sec = static_cast<int>(number);
					break;
				default:
					LOG_SERVER << "Invalid time modifier given: '" << *i << "'.";
					break;
				}
				number = 0;
			}
		}
		*time = mktime(loc);
		return true;
	}

	const auto time_itor = ban_times_.find(duration);

	std::string dur_lower;
	try {
		dur_lower = utf8::lowercase(duration);
	} catch(const utf8::invalid_utf8_exception& e) {
		ERR_SERVER << "While parsing ban command duration string, caught an invalid utf8 exception: " << e.what();
		return false;
	}

	if(dur_lower == "permanent" || duration == "0") {
		*time = 0;
	} else if(ban_times_.find(duration) != ban_times_.end()) {
		*time += time_itor->second;
	} else {
		std::string::const_iterator i = duration.begin();
		int number = -1;
		for (std::string::const_iterator d_end = duration.end(); i != d_end; ++i) {
			if (is_digit(*i))
			{
				if (number == -1) number = 0;
				number = static_cast<int>(number * 10 + to_digit(*i));
			} else {
				if (number == -1) number = 1;
				switch(*i)
				{
					case 'Y':
					case 'y':
						if (++i != d_end && tolower(*i) == 'e'
						&&  ++i != d_end && tolower(*i) == 'a'
						&&  ++i != d_end && tolower(*i) == 'r'
						&&  ++i != d_end && tolower(*i) == 's') {
						} else --i;
						*time += number * 365*24*60*60; // a year;
						break;
					case 'M':
						if (++i != d_end && tolower(*i) == 'i') {
							if (++i != d_end && tolower(*i) == 'n'
							&&  ++i != d_end && tolower(*i) == 'u'
							&&  ++i != d_end && tolower(*i) == 't'
							&&  ++i != d_end && tolower(*i) == 'e'
							&&  ++i != d_end && tolower(*i) == 's') {
							} else --i;
							*time += number * 60;
							break;
						}
						--i;
						if (++i != d_end && tolower(*i) == 'o'
						&&  ++i != d_end && tolower(*i) == 'n'
						&&  ++i != d_end && tolower(*i) == 't'
						&&  ++i != d_end && tolower(*i) == 'h'
						&&  ++i != d_end && tolower(*i) == 's') {
						} else --i;
						*time += number * 30*24*60*60; // 30 days
						break;
					case 'D':
					case 'd':
						if (++i != d_end && tolower(*i) == 'a'
						&&  ++i != d_end && tolower(*i) == 'y'
						&&  ++i != d_end && tolower(*i) == 's') {
						} else --i;
						*time += number * 24*60*60;
						break;
					case 'H':
					case 'h':
						if (++i != d_end && tolower(*i) == 'o'
						&&  ++i != d_end && tolower(*i) == 'u'
						&&  ++i != d_end && tolower(*i) == 'r'
						&&  ++i != d_end && tolower(*i) == 's') {
						} else --i;
						*time += number * 60*60;
						break;
					case 'm':
						if (++i != d_end && tolower(*i) == 'o') {
							if (++i != d_end && tolower(*i) == 'n'
							&&  ++i != d_end && tolower(*i) == 't'
							&&  ++i != d_end && tolower(*i) == 'h'
							&&  ++i != d_end && tolower(*i) == 's') {
							} else --i;
							*time += number * 30*24*60*60; // 30 days
							break;
						}
						--i;
						if (++i != d_end && tolower(*i) == 'i'
						&&  ++i != d_end && tolower(*i) == 'n'
						&&  ++i != d_end && tolower(*i) == 'u'
						&&  ++i != d_end && tolower(*i) == 't'
						&&  ++i != d_end && tolower(*i) == 'e'
						&&  ++i != d_end && tolower(*i) == 's') {
						} else --i;
						*time += number * 60;
						break;
					case 'S':
					case 's':
						if (++i != d_end && tolower(*i) == 'e'
						&&  ++i != d_end && tolower(*i) == 'c'
						&&  ++i != d_end && tolower(*i) == 'o'
						&&  ++i != d_end && tolower(*i) == 'n'
						&&  ++i != d_end && tolower(*i) == 'd'
						&&  ++i != d_end && tolower(*i) == 's') {
						} else --i;
						*time += number;
						break;
					default:
						return false;
						break;
				}
				number = -1;
			}
		}

		if(is_digit(*--i)) {
			*time += number * 60; // default to minutes
		}
	}

	return true;
}

std::string ban_manager::ban(const std::string& ip,
	const std::time_t& end_time,
	const std::string& reason,
	const std::string& who_banned,
	const std::string& group,
	const std::string& nick)
{
	std::ostringstream ret;
	try {
		ban_set::iterator ban;
		if((ban = bans_.find(std::make_shared<banned>(ip))) != bans_.end()) {
			// Already exsiting ban for ip. We have to first remove it
			ret << "Overwriting ban: " << (**ban) << "\n";
			bans_.erase(ban);
		}
	} catch(const banned::error& e) {
		ERR_SERVER << e.message << " while creating dummy ban for finding existing ban";
		return e.message;
	}

	try {
		auto new_ban = std::make_shared<banned>(ip, end_time, reason, who_banned, group, nick);
		bans_.insert(new_ban);
		if(end_time != 0) {
			time_queue_.push(new_ban);
		}
		ret << *new_ban;
	} catch(const banned::error& e) {
		ERR_SERVER << e.message << " while banning";
		return e.message;
	}

	dirty_ = true;
	write();
	return ret.str();
}

void ban_manager::unban(std::ostringstream& os, const std::string& ip, bool immediate_write)
{
	ban_set::iterator ban;
	try {
		ban = bans_.find(std::make_shared<banned>(ip));
	} catch (const banned::error& e) {
		ERR_SERVER << e.message;
		os << e.message;
		return;
	}

	if(ban == bans_.end()) {
		os << "There is no ban on '" << ip << "'.";
		return;
	}

	// keep ban entry still in memory
	os << "Ban on '" << **ban << "' removed.";
	// group bans don't get saved
	if ((*ban)->get_group().empty()) deleted_bans_.push_back(*ban);
	bans_.erase(ban);
	dirty_ = true;
	if(immediate_write) {
		write();
	}
}

void ban_manager::unban_group(std::ostringstream& os, const std::string& group)
{
	ban_set temp;
	std::insert_iterator<ban_set> temp_inserter(temp, temp.begin());
	std::remove_copy_if(bans_.begin(), bans_.end(), temp_inserter, [&group](const banned_ptr& p) { return p->match_group(group); });

	os << "Removed " << (bans_.size() - temp.size()) << " bans";
	bans_.swap(temp);
	dirty_ = true;
	write();
}

void ban_manager::check_ban_times(std::time_t time_now)
{
	while(!time_queue_.empty()) {
		banned_ptr ban = time_queue_.top();

		if(ban->get_end_time() > time_now) {
			// No bans going to expire
			DBG_SERVER << "ban " << ban->get_ip() << " not removed. time: " << time_now << " end_time "
					   << ban->get_end_time();
			break;
		}

		// This ban is going to expire so delete it.
		LOG_SERVER << "Remove a ban " << ban->get_ip() << ". time: " << time_now << " end_time " << ban->get_end_time();
		std::ostringstream os;
		unban(os, ban->get_ip(), false);
		time_queue_.pop();
	}

	// Save bans if there is any new ones
	write();
}

void ban_manager::list_deleted_bans(std::ostringstream& out, const std::string& mask) const
{
	if(deleted_bans_.empty()) {
		out << "No removed bans found.";
		return;
	}

	ip_mask pair;
	try {
		pair = parse_ip(mask);
	} catch(const banned::error& e) {
		out << "parse error: " << e.message;
		return;
	}

	out << "DELETED BANS LIST";
	for(deleted_ban_list::const_iterator i = deleted_bans_.begin(); i != deleted_bans_.end(); ++i) {
		if((*i)->match_ipmask(pair)) {
			out << "\n" << (**i);
		}
	}
}

void ban_manager::list_bans(std::ostringstream& out, const std::string& mask)
{
	expire_bans();
	if(bans_.empty()) {
		out << "No bans set.";
		return;
	}

	ip_mask pair;
	try {
		pair = parse_ip(mask);
	} catch(const banned::error& e) {
		out << "parse error: " << e.message;
		return;
	}

	out << "BAN LIST";
	std::set<std::string> groups;

	for(const auto& b : bans_) {
		if(b->get_group().empty()) {
			if(b->match_ipmask(pair)) {
				out << "\n" << *b;
			}
		} else {
			groups.insert(b->get_group());
		}
	}

	// Don't list ban groups when looking for specific bans.
	if(!groups.empty() && mask == "*") {
		out << "\nban groups: ";

		out << *groups.begin();
		std::ostream& (*fn)(std::ostream&, const std::string&) = &std::operator<<;
		std::for_each(++groups.begin(), groups.end(),
			std::bind(fn, std::bind(fn, std::ref(out), std::string(", ")), std::placeholders::_1));
	}
}

std::string ban_manager::is_ip_banned(const std::string& ip)
{
	expire_bans();
	ip_mask pair;
	try {
		pair = parse_ip(ip);
	} catch (const banned::error&) {
		return "";
	}

	auto ban = std::find_if(bans_.begin(), bans_.end(), [pair](const banned_ptr& p) { return p->match_ip(pair); });
	if (ban == bans_.end()) return "";
	const std::string& nick = (*ban)->get_nick();
	return (*ban)->get_reason() + (nick.empty() ? "" : " (" + nick + ")") + " (Remaining ban duration: " + (*ban)->get_human_time_span() + ")";
}

void ban_manager::init_ban_help()
{
	ban_help_ = "ban <mask> <time> <reason>\n"
			"The time format is: %d[%s[%d[%s[...]]]] where %s is a time"
			" modifier: s or S (seconds), m (minutes), h or H (hours), d"
			" or D (days), M (months) or y or Y (years) and %d is a number.\n"
			"Permanent bans can be set with 'permanent' or '0' as the time"
			" argument.\n";
	auto itor = ban_times_.begin();
	if(itor != ban_times_.end()) {
		ban_help_ += "You can also use " + itor->first;
		++itor;
	}
	for(; itor != ban_times_.end(); ++itor) {
		ban_help_ += std::string(", ") + itor->first;
	}
	if(!ban_times_.empty()) {
		ban_help_ += " for standard ban times. (not combinable)\n";
	}
	ban_help_ += "ban 127.0.0.1 2h20m flooded lobby\n"
			"kban suokko 5D flooded again\n"
			"kban suokko Y One year ban for constant flooding";
}

void ban_manager::load_config(const config& cfg)
{
	ban_times_.clear();
	for(const config& bt : cfg.child_range("ban_time")) {
		std::time_t duration = 0;
		if(parse_time(bt["time"], &duration)) {
			ban_times_.emplace(bt["name"], duration);
		}
	}

	init_ban_help();
	if(filename_ != cfg["ban_save_file"]) {
		dirty_ = true;
		filename_ = cfg["ban_save_file"].str();
	}
}

ban_manager::~ban_manager()
{
	try {
		write();
	} catch(...) {
		DBG_SERVER << "Caught exception in ban_manager destructor: " << utils::get_unknown_exception_type();
	}
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

} // namespace wesnothd
