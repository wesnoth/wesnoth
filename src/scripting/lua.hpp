#ifndef SCRIPTING_LUA_HPP
#define SCRIPTING_LUA_HPP

#include "game_events.hpp"

struct lua_State;

class LuaKernel
{
	lua_State *mState;
	void execute(char const *, int, int);
public:
	LuaKernel();
	~LuaKernel();
	void run_event(char const *, game_events::queued_event const &,
	               game_events::event_handler *, unit_map *);
	void run(char const *prog);
};

#endif
