/*
Copyright (C) 2006 - 2017 by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
wesnoth playturn Copyright (C) 2003 by David White <dave@whitevine.net>
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
#include "config.hpp"
#include "serialization/string_utils.hpp"
#include "formula/string_utils.hpp"
#include "gettext.hpp"

#include <boost/algorithm/string.hpp>

namespace events {

//simple command args parser, separated from command_handler for clarity.
//a word begins with a nonspace
//n-th arg is n-th word up to the next space
//n-th data is n-th word up to the end
//cmd is 0-th arg, begins at 0 always.
class cmd_arg_parser
{
public:
	cmd_arg_parser() :
		str_(""),
		args(1, 0),
		args_end(false)
	{
	}

	explicit cmd_arg_parser(const std::string& str) :
		str_(str),
		args(1, 0),
		args_end(false)
	{
	}

	void parse(const std::string& str)
	{
		str_ = str;
		args.clear();
		args.push_back(0);
		args_end = false;
	}

	const std::string& get_str() const
	{
		return str_;
	}
	std::string get_arg(unsigned n) const
	{
		advance_to_arg(n);
		if (n < args.size()) {
			return std::string(str_, args[n], str_.find(' ', args[n]) - args[n]);
		}
		else {
			return "";
		}
	}
	std::string get_data(unsigned n) const
	{
		advance_to_arg(n);
		if (n < args.size()) {
			std::string data(str_, args[n]);
			boost::trim(data);
			return data;
		}
		else {
			return "";
		}
	}
	std::string get_cmd() const
	{
		return get_arg(0);
	}
private:
	cmd_arg_parser& operator=(const cmd_arg_parser&);
	cmd_arg_parser(const cmd_arg_parser&);
	void advance_to_arg(unsigned n) const
	{
		while (n < args.size() && !args_end) {
			size_t first_space = str_.find_first_of(' ', args.back());
			size_t next_arg_begin = str_.find_first_not_of(' ', first_space);
			if (next_arg_begin != std::string::npos) {
				args.push_back(next_arg_begin);
			}
			else {
				args_end = true;
			}
		}
	}
	std::string str_;
	mutable std::vector<size_t> args;
	mutable bool args_end;
};


//A helper class template with a slim public interface
//This represents a map of strings to void()-member-function-of-Worker-pointers
//with all the common functionality like general help, command help and aliases
//Usage (of a derived class): Derived(specific-arguments) d; d.dispatch(command);
//Derived classes should override virtual functions where noted.
//The template parameter currently must be the dervived class itself,
//i.e. class X : public map_command_handler<X>
//To add a new command in a derived class:
//  * add a new private void function() to the derived class
//  * add it to the function map in init_map there, setting flags like
//    "D" for debug only (checking the flag is also done in the derived class)
//  * remember to add some help and/or usage information in init_map()
template <class Worker>
class map_command_handler
{
public:
	typedef void (Worker::*command_handler)();
	struct command
	{
		command_handler handler;
		std::string help; //long help text
		std::string usage; //only args info
		std::string flags;
		explicit command(command_handler h, const std::string help = "",
			const std::string& usage = "", const std::string flags = "")
			: handler(h), help(help), usage(usage), flags(flags)
		{
		}
		bool has_flag(const char f) const
		{
			return flags.find(f) != flags.npos;
		}
		command& add_flag(const char f)
		{
			flags += f;
			return *this;
		}
	};
	typedef std::map<std::string, command> command_map;
	typedef std::map<std::string, std::string> command_alias_map;

	map_command_handler() : cap_("")
	{
	}

	virtual ~map_command_handler() {}

	bool empty() const
	{
		return command_map_.empty();
	}
	//actual work function
	void dispatch(std::string cmd)
	{
		if (empty()) {
			init_map_default();
			init_map();
		}

		// We recursively resolve alias (100 max to avoid infinite recursion)
		for (int i = 0; i < 100; ++i) {
			parse_cmd(cmd);
			std::string actual_cmd = get_actual_cmd(get_cmd());
			if (actual_cmd == get_cmd())
				break;
			std::string data = get_data(1);
			// translate the command and add space + data if any
			cmd = actual_cmd + (data.empty() ? "" : " ") + data;
		}

		if (get_cmd().empty()) {
			return;
		}

		if (const command* c = get_command(get_cmd())) {
			if (is_enabled(*c)) {
				(static_cast<Worker*>(this)->*(c->handler))();
			}
			else {
				print(get_cmd(), _("This command is currently unavailable."));
			}
		}
		else if (help_on_unknown_) {
			utils::string_map symbols;
			symbols["command"] = get_cmd();
			symbols["help_command"] = cmd_prefix_ + "help";
			print("help", VGETTEXT("Unknown command '$command', try $help_command "
				"for a list of available commands.", symbols));
		}
	}

	std::vector<std::string> get_commands_list() const
	{
		std::vector<std::string> res;
		for (typename command_map::value_type i : command_map_) {
			res.push_back(i.first);
		}
		return res;
	}
	//command error reporting shorthands
	void command_failed(const std::string& message, bool = false)
	{
		print(get_cmd(), _("Error:") + std::string(" ") + message);
	}
protected:
	void init_map_default()
	{
		register_command("help", &map_command_handler<Worker>::help,
			_("Available commands list and command-specific help. "
				"Use \"help all\" to include currently unavailable commands."),
			_("do not translate the 'all'^[all|<command>]"));
	}
	//derived classes initialize the map overriding this function
	virtual void init_map() = 0;
	//overridden in derived classes to actually print the messages somwehere
	virtual void print(const std::string& title, const std::string& message) = 0;
	//should be overridden in derived classes if the commands have flags
	//this should return a string describing what all the flags mean
	virtual std::string get_flags_description() const
	{
		return "";
	}
	//this should return a string describing the flags of the given command
	virtual std::string get_command_flags_description(const command& /*c*/) const
	{
		return "";
	}
	//this should be overridden if e.g. flags are used to control command
	//availability. Return false if the command should not be executed by dispatch()
	virtual bool is_enabled(const command& /*c*/) const
	{
		return true;
	}
	virtual void parse_cmd(const std::string& cmd_string)
	{
		cap_.parse(cmd_string);
	}
	//safe n-th argunment getter
	virtual std::string get_arg(unsigned argn) const
	{
		return cap_.get_arg(argn);
	}
	//"data" is n-th arg and everything after it
	virtual std::string get_data(unsigned argn = 1) const
	{
		return cap_.get_data(argn);
	}
	virtual std::string get_cmd() const
	{
		return cap_.get_cmd();
	}
	void command_failed_need_arg(int argn)
	{
		utils::string_map symbols;
		symbols["arg_id"] = std::to_string(argn);
		command_failed(VGETTEXT("Missing argument $arg_id", symbols));
	}
	void print_usage()
	{
		help_command(get_cmd());
	}
	//take aliases into account
	std::string get_actual_cmd(const std::string& cmd) const
	{
		command_alias_map::const_iterator i = command_alias_map_.find(cmd);
		return i != command_alias_map_.end() ? i->second : cmd;
	}
	const command* get_command(const std::string& cmd) const
	{
		typename command_map::const_iterator i = command_map_.find(cmd);
		return i != command_map_.end() ? &i->second : 0;
	}
	command* get_command(const std::string& cmd)
	{
		typename command_map::iterator i = command_map_.find(cmd);
		return i != command_map_.end() ? &i->second : 0;
	}
	void help()
	{
		//print command-specific help if available, otherwise list commands
		if (help_command(get_arg(1))) {
			return;
		}
		std::stringstream ss;
		bool show_unavail = show_unavailable_ || get_arg(1) == "all";
		for (typename command_map::value_type i : command_map_) {
			if (show_unavail || is_enabled(i.second)) {
				ss << i.first;
				//if (!i.second.usage.empty()) {
				//	ss << " " << i.second.usage;
				//}
				//uncomment the above to display usage information in command list
				//which might clutter it somewhat
				if (!i.second.flags.empty()) {
					ss << " (" << i.second.flags << ") ";
				}
				ss << "; ";
			}
		}
		utils::string_map symbols;
		symbols["flags_description"] = get_flags_description();
		symbols["list_of_commands"] = ss.str();
		symbols["help_command"] = cmd_prefix_ + "help";
		print(_("help"), VGETTEXT("Available commands $flags_description:\n$list_of_commands", symbols));
		print(_("help"), VGETTEXT("Type $help_command <command> for more info.", symbols));
	}
	//returns true if the command exists.
	bool help_command(const std::string& acmd)
	{
		std::string cmd = get_actual_cmd(acmd);
		const command* c = get_command(cmd);
		if (c) {
			std::stringstream ss;
			ss << cmd_prefix_ << cmd;
			if (c->help.empty() && c->usage.empty()) {
				ss << _(" No help available.");
			}
			else {
				ss << " - " << c->help;
			}
			if (!c->usage.empty()) {
				ss << " " << _("Usage:") << " " << cmd_prefix_ << cmd << " " << c->usage;
			}
			ss << get_command_flags_description(*c);
			const std::vector<std::string> l = get_aliases(cmd);
			if (!l.empty()) {
				ss << " (" << _("aliases:") << " " << utils::join(l, " ") << ")";
			}
			print(_("help"), ss.str());
		}
		return c != 0;
	}
	cmd_arg_parser cap_;
protected:
	//show a "try help" message on unknown command?
	static void set_help_on_unknown(bool value)
	{
		help_on_unknown_ = value;
	}
	//this is display-only
	static void set_cmd_prefix(std::string value)
	{
		cmd_prefix_ = value;
	}
	virtual void register_command(const std::string& cmd,
		command_handler h, const std::string& help = "",
		const std::string& usage = "", const std::string& flags = "")
	{
		command c = command(h, help, usage, flags);
		std::pair<typename command_map::iterator, bool> r;
		r = command_map_.insert(typename command_map::value_type(cmd, c));
		if (!r.second) { //overwrite if exists
			r.first->second = c;
		}
	}

	virtual void register_alias(const std::string& to_cmd,
		const std::string& cmd)
	{
		command_alias_map_[cmd] = to_cmd;
	}

	//get all aliases of a command.
	static const std::vector<std::string> get_aliases(const std::string& cmd)
	{
		std::vector<std::string> aliases;
		typedef command_alias_map::value_type p;
		for (p i : command_alias_map_) {
			if (i.second == cmd) {
				aliases.push_back(i.first);
			}
		}
		return aliases;
	}
private:
	static command_map command_map_;
	static command_alias_map command_alias_map_;
	static bool help_on_unknown_;
	static bool show_unavailable_;
	static std::string cmd_prefix_;
};

//static member definitions
template <class Worker>
typename map_command_handler<Worker>::command_map map_command_handler<Worker>::command_map_;

template <class Worker>
typename map_command_handler<Worker>::command_alias_map map_command_handler<Worker>::command_alias_map_;

template <class Worker>
bool map_command_handler<Worker>::help_on_unknown_ = true;

template <class Worker>
bool map_command_handler<Worker>::show_unavailable_ = false;

template <class Worker>
std::string map_command_handler<Worker>::cmd_prefix_;

}
