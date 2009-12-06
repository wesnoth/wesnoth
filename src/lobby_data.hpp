/* $Id$ */
/*
   Copyright (C) 2009 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef INC_LOBBY_DATA
#define INC_LOBBY_DATA

#include "config.hpp"
#include "sdl_utils.hpp"

#include <set>
#include <map>
#include <deque>
#include <functional>

/** This class represenst a single stored chat message */
struct chat_message
{
	/** Create a chat message */
	chat_message(const time_t& timestamp, const std::string& user, const std::string& message);

	time_t timestamp;
	std::string user;
	std::string message;
};

/** this class memorizes a chat session. */
class chat_log
{
public:
	chat_log();

	void add_message(const time_t& timestamp, const std::string& user, const std::string& message);

	void add_message(const std::string& user, const std::string& message);

	const std::deque<chat_message>& history() const { return history_; }

	void clear();
private:
	std::deque<chat_message> history_;
};

/**
 * This class represents the information a client has about a room
 */
class room_info
{
public:
	room_info(const std::string& name);

	const std::string& name() const { return name_; }
	const std::set<std::string>& members() const { return members_; }
	bool is_member(const std::string& user) const;
	void add_member(const std::string& user);
	void remove_member(const std::string& user);
	void process_room_members(const config &data);

	const chat_log& log() const { return log_; }
	chat_log& log() { return log_; }

private:
	std::string name_;
	std::set<std::string> members_;
	chat_log log_;
};


/**
 * This class represents the information a client has about another player
 */
struct user_info
{
	user_info(const config& c);

	void update_state(int selected_game_id, const room_info* current_room = NULL);
	void update_relation();

	enum user_relation { FRIEND, ME, NEUTRAL, IGNORED };
	enum user_state    { LOBBY, SEL_ROOM, GAME, SEL_GAME };

	bool operator> (const user_info& b) const;

	std::string name;
	int game_id;
	user_relation relation;
	user_state state;
	bool registered;
	bool observing;
};

/**
 * This class represents the info a client has about a game on the server
 */
struct game_info
{
	game_info(const config& c, const config& game_config);

	bool can_join() const;
	bool can_observe() const;

	surface mini_map;
	int id;
	std::string map_data;
	std::string name;
	std::string scenario;
	bool remote_scenario;
	std::string map_info;
	std::string map_size_info;
	std::string era;
	std::string era_short;

	std::string gold;
	std::string xp;
	std::string vision;
	std::string status; //vacant slots or turn info
	std::string time_limit;
	size_t vacant_slots;

	unsigned int current_turn;
	bool reloaded;
	bool started;
	bool fog;
	bool shroud;
	bool observers;
	bool use_map_settings;
	bool verified;
	bool password_required;
	bool have_era;

	bool has_friends;
	bool has_ignored;
};

class game_filter_base : public std::unary_function<game_info, bool>
{
public:
	virtual ~game_filter_base() {}
	virtual bool match(const game_info& game) const = 0;
	bool operator()(const game_info& game) const { return match(game); }
};

template<class T>
class game_filter_not : public game_filter_base
{
public:
	game_filter_not(const T& t) : t(t) {}
	bool match(const game_info& game) const { return !t.match(game); }
	T t;
};

class game_filter_stack : public game_filter_base
{
public:
	game_filter_stack();
	virtual ~game_filter_stack();

	/**
	 * Takes ownership
	 */
	void append(game_filter_base* f);

	void clear();

	bool empty() const { return filters_.empty(); }

protected:
	std::vector<game_filter_base*> filters_;
};

class game_filter_and_stack : public game_filter_stack
{
public:
	bool match(const game_info& game) const;
};

class game_filter_or_stack : public game_filter_stack
{
public:
	bool match(const game_info& game) const;
};

template <class T, T game_info::*member, class OP = std::equal_to<T> >
class game_filter_value: public game_filter_base
{
public:
	game_filter_value(const T& value)
	: member_(member), value_(value)
	{
	}

	bool match(const game_info& game) const { return OP()(game.*member_,value_); }

private:
	T game_info::*member_;
	T value_;
};

class game_filter_string_part : public game_filter_base
{
public:
	game_filter_string_part(std::string game_info::*member, const std::string& value)
	: member_(member), value_(value)
	{
	}

	bool match(const game_info& game) const;

private:
	std::string game_info::*member_;
	std::string value_;
};

class game_filter_general_string_part : public game_filter_base
{
public:
	game_filter_general_string_part(const std::string& value)
	: value_(value)
	{
	}

	bool match(const game_info& game) const;
private:
	std::string value_;
};



/**
 * This class represents the collective information the client has
 * about the players and games on the server
 */
class lobby_info
{
public:
	lobby_info(const config& game_config);

	void process_gamelist(const config &data);
	bool process_gamelist_diff(const config &data);

	const config& gamelist() const { return gamelist_; }

	void clear_game_filter();
	void add_game_filter(game_filter_base* f);
	void set_game_filter_invert(bool value);
	void apply_game_filter();

	game_info* get_game_by_id(int id);
	const game_info* get_game_by_id(int id) const;

	void sort_users(bool by_name, bool by_relation);

	void open_room(const std::string& name);
	void close_room(const std::string& name);
	bool has_room(const std::string& name) const;
	room_info* get_room(const std::string& name);
	const room_info* get_room(const std::string& name) const;

	user_info& get_user(const std::string& name);

	chat_log& get_whisper_log(const std::string& name);

	void update_user_statuses(int game_id, const room_info* room);

	const std::vector<room_info>& rooms() const { return rooms_; }
	const std::vector<game_info>& games() const { return games_; }
	const std::vector<game_info*>& games_filtered();
	const std::vector<user_info>& users() const { return users_; }
	const std::vector<user_info*>& users_sorted();
private:
	void parse_gamelist();

	const config& game_config_;
	config gamelist_;
	bool gamelist_initialized_;
	std::vector<room_info> rooms_;
	std::vector<game_info> games_;
	std::map<int, game_info*> games_by_id_;
	std::vector<game_info*> games_filtered_;
	std::vector<user_info> users_;
	std::vector<user_info*> users_sorted_;
	std::map<std::string, chat_log> whispers_;
	game_filter_and_stack game_filter_;
	bool game_filter_invert_;
};

#endif
