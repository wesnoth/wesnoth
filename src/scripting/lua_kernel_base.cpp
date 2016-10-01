/*
   Copyright (C) 2014 - 2016 by Chris Beck <render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "scripting/lua_kernel_base.hpp"

#include "global.hpp"

#include "exceptions.hpp"
#include "game_config.hpp"
#include "game_errors.hpp"
#include "log.hpp"
#include "lua_jailbreak_exception.hpp"  // for tlua_jailbreak_exception
#include "seed_rng.hpp"

#ifdef DEBUG_LUA
#include "scripting/debug_lua.hpp"
#endif

#include "scripting/lua_common.hpp"
#include "scripting/lua_cpp_function.hpp"
#include "scripting/lua_fileops.hpp"
#include "scripting/lua_formula_bridge.hpp"
#include "scripting/lua_gui2.hpp"
#include "scripting/lua_map_location_ops.hpp"
#include "scripting/lua_rng.hpp"
#include "scripting/push_check.hpp"

#include "version.hpp"                  // for do_version_check, etc

#include "serialization/string_utils.hpp"
#include "utils/functional.hpp"
#include "utils/name_generator.hpp"
#include "utils/markov_generator.hpp"
#include "utils/context_free_grammar_generator.hpp"

#include <cstring>
#include <exception>
#include <new>
#include <string>
#include <sstream>
#include <vector>

#include "lua/lauxlib.h"
#include "lua/lua.h"
#include "lua/lualib.h"

static lg::log_domain log_scripting_lua("scripting/lua");
#define DBG_LUA LOG_STREAM(debug, log_scripting_lua)
#define LOG_LUA LOG_STREAM(info, log_scripting_lua)
#define WRN_LUA LOG_STREAM(warn, log_scripting_lua)
#define ERR_LUA LOG_STREAM(err, log_scripting_lua)

// Registry key for metatable
static const char * Gen = "name generator";

// Callback implementations

/**
 * Compares 2 version strings - which is newer.
 * - Args 1,3: version strings
 * - Arg 2: comparison operator (string)
 * - Ret 1: comparison result
 */
static int intf_compare_versions(lua_State* L)
{
	char const *v1 = luaL_checkstring(L, 1);

	const VERSION_COMP_OP vop = parse_version_op(luaL_checkstring(L, 2));
	if(vop == OP_INVALID) return luaL_argerror(L, 2, "unknown version comparison operator - allowed are ==, !=, <, <=, > and >=");

	char const *v2 = luaL_checkstring(L, 3);

	const bool result = do_version_check(version_info(v1), vop, version_info(v2));
	lua_pushboolean(L, result);

	return 1;
}

/**
 * Replacement print function -- instead of printing to std::cout, print to the command log.
 * Intended to be bound to this' command_log at registration time.
 */
int lua_kernel_base::intf_print(lua_State* L)
{
	DBG_LUA << "intf_print called:\n";
	size_t nargs = lua_gettop(L);

	for (size_t i = 1; i <= nargs; ++i) {
		const char * str = lua_tostring(L,i);
		if (!str) {
			str = "";
		}
		if (i > 1) {
			cmd_log_ << "\t"; //separate multiple args with tab character
		}
		cmd_log_ << str;
		DBG_LUA << "'" << str << "'\n";
	}

	cmd_log_ << "\n";
	DBG_LUA << "\n";

	return 0;
}

template<lua_kernel_base::video_function callback>
int video_dispatch(lua_State *L) {
	return lua_kernel_base::get_lua_kernel<lua_kernel_base>(L).video_dispatch_impl(L, callback);
}

// The show-dialog call back is here implemented as a method of lua kernel, since it needs a pointer to external object CVideo
int lua_kernel_base::video_dispatch_impl(lua_State* L, lua_kernel_base::video_function callback)
{
	if (!video_) {
		ERR_LUA << "Cannot show dialog, no video object is available to this lua kernel.";
		lua_error(L);
		return 0;
	}

	return callback(L, *video_);
}

// The show lua console callback is similarly a method of lua kernel
int lua_kernel_base::intf_show_lua_console(lua_State *L)
{
	if (!video_) {
		ERR_LUA << "Cannot show dialog, no video object is available to this lua kernel.";
		lua_error(L);
		return 0;
	}

	if (cmd_log_.external_log_ != nullptr) {
		std::string message = "There is already an external logger attached to this lua kernel, you cannot open the lua console right now.";
		log_error(message.c_str());
		cmd_log_ << message << "\n";
		return 0;
	}

	return lua_gui2::show_lua_console(L, *video_, this);
}

static int impl_name_generator_call(lua_State *L)
{
	name_generator* gen = static_cast<name_generator*>(lua_touserdata(L, 1));
	lua_pushstring(L, gen->generate().c_str());
	return 1;
}

static int impl_name_generator_collect(lua_State *L)
{
	name_generator* gen = static_cast<name_generator*>(lua_touserdata(L, 1));
	gen->~name_generator();
	return 0;
}

static int intf_name_generator(lua_State *L)
{
	std::string type = luaL_checkstring(L, 1);
	name_generator* gen = nullptr;
	try {
		if(type == "markov" || type == "markov_chain") {
			std::vector<std::string> input;
			if(lua_istable(L, 2)) {
				input = lua_check<std::vector<std::string>>(L, 2);
			} else {
				input = utils::parenthetical_split(luaL_checkstring(L, 2), ',');
			}
			int chain_sz = luaL_optinteger(L, 3, 2);
			int max_len = luaL_optinteger(L, 4, 12);
			gen = new(L) markov_generator(input, chain_sz, max_len);
			// Ensure the pointer didn't change when cast
			assert(static_cast<void*>(gen) == dynamic_cast<markov_generator*>(gen));
		} else if(type == "context_free" || type == "cfg" || type == "CFG") {
			if(lua_istable(L, 2)) {
				std::map<std::string, std::vector<std::string>> data;
				for(lua_pushnil(L); lua_next(L, 2); lua_pop(L, 1)) {
					if(!lua_isstring(L, -2)) {
						lua_pushstring(L, "CFG generator: invalid nonterminal name (must be a string)");
						return lua_error(L);
					}
					if(lua_isstring(L, -1)) {
						data[lua_tostring(L,-2)] = utils::split(lua_tostring(L,-1),'|');
					} else if(lua_istable(L, -1)) {
						data[lua_tostring(L,-2)] = lua_check<std::vector<std::string>>(L, -1);
					} else {
						lua_pushstring(L, "CFG generator: invalid noterminal value (must be a string or list of strings)");
						return lua_error(L);
					}
				}
				if(!data.empty()) {
					gen = new(L) context_free_grammar_generator(data);
				}
			} else {
				gen = new(L) context_free_grammar_generator(luaL_checkstring(L, 2));
			}
			if(gen) {
				assert(static_cast<void*>(gen) == dynamic_cast<context_free_grammar_generator*>(gen));
			}
		} else {
			return luaL_argerror(L, 1, "should be either 'markov_chain' or 'context_free'");
		}
	}
	catch (const name_generator_invalid_exception& ex) {
		lua_pushstring(L, ex.what());
		return lua_error(L);
	}

	// We set the metatable now, even if the generator is invalid, so that it
	// will be properly collected if it was invalid.
	luaL_getmetatable(L, Gen);
	lua_setmetatable(L, -2);

	return 1;
}

// End Callback implementations

// Template which allows to push member functions to the lua kernel base into lua as C functions, using a shim
typedef int (lua_kernel_base::*member_callback)(lua_State *L);

template <member_callback method>
int dispatch(lua_State *L) {
	return ((lua_kernel_base::get_lua_kernel<lua_kernel_base>(L)).*method)(L);
}

extern void push_error_handler(lua_State *L);

// Ctor, initialization
lua_kernel_base::lua_kernel_base(CVideo * video)
 : mState(luaL_newstate())
 , video_(video)
 , cmd_log_()
{
	get_lua_kernel_base_ptr(mState) = this;
	lua_State *L = mState;

	cmd_log_ << "Initializing " << my_name() << "...\n";

	// Open safe libraries.
	// Debug and OS are not, but most of their functions will be disabled below.
	cmd_log_ << "Adding standard libs...\n";

	static const luaL_Reg safe_libs[] = {
		{ "",       luaopen_base   },
		{ "table",  luaopen_table  },
		{ "string", luaopen_string },
		{ "math",   luaopen_math   },
		{ "coroutine",   luaopen_coroutine   },
		{ "debug",  luaopen_debug  },
		{ "os",     luaopen_os     },
		{ "bit32",  luaopen_bit32  }, // added in Lua 5.2
		{ nullptr, nullptr }
	};
	for (luaL_Reg const *lib = safe_libs; lib->func; ++lib)
	{
		luaL_requiref(L, lib->name, lib->func, 1);
		lua_pop(L, 1);  /* remove lib */
	}

	// Disable functions from os which we don't want.
	lua_getglobal(L, "os");
	lua_pushnil(L);
	while(lua_next(L, -2) != 0) {
		lua_pop(L, 1);
		char const* function = lua_tostring(L, -1);
		if(strcmp(function, "clock") == 0 || strcmp(function, "date") == 0
			|| strcmp(function, "time") == 0 || strcmp(function, "difftime") == 0) continue;
		lua_pushnil(L);
		lua_setfield(L, -3, function);
	}
	lua_pop(L, 1);

	// Disable functions from debug which we don't want.
	lua_getglobal(L, "debug");
	lua_pushnil(L);
	while(lua_next(L, -2) != 0) {
		lua_pop(L, 1);
		char const* function = lua_tostring(L, -1);
		if(strcmp(function, "traceback") == 0 || strcmp(function, "getinfo") == 0) continue;	//traceback is needed for our error handler
		lua_pushnil(L);										//getinfo is needed for ilua strict mode
		lua_setfield(L, -3, function);
	}
	lua_pop(L, 1);

	// Delete dofile and loadfile.
	lua_pushnil(L);
	lua_setglobal(L, "dofile");
	lua_pushnil(L);
	lua_setglobal(L, "loadfile");

	// Store the error handler.
	cmd_log_ << "Adding error handler...\n";
	push_error_handler(L);

	// Create the gettext metatable.
	cmd_log_ << lua_common::register_gettext_metatable(L);

	// Create the tstring metatable.
	cmd_log_ << lua_common::register_tstring_metatable(L);


	lua_settop(L, 0);

	// Define the CPP_function metatable ( so we can override print to point to a C++ member function, add "show_dialog" for this kernel, etc. )
	cmd_log_ << "Adding boost function proxy...\n";

	lua_cpp::register_metatable(L);

	// Add some callback from the wesnoth lib
	cmd_log_ << "Registering basic wesnoth API...\n";

	static luaL_Reg const callbacks[] = {
		{ "compare_versions",         &intf_compare_versions         		},
		{ "have_file",                &lua_fileops::intf_have_file          },
		{ "read_file",                &lua_fileops::intf_read_file          },
		{ "textdomain",               &lua_common::intf_textdomain   		},
		{ "tovconfig",                &lua_common::intf_tovconfig		},
		{ "get_dialog_value",         &lua_gui2::intf_get_dialog_value		},
		{ "set_dialog_active",        &lua_gui2::intf_set_dialog_active		},
		{ "set_dialog_visible",       &lua_gui2::intf_set_dialog_visible    },
		{ "add_dialog_tree_node",     &lua_gui2::intf_add_dialog_tree_node	},
		{ "set_dialog_callback",      &lua_gui2::intf_set_dialog_callback	},
		{ "set_dialog_canvas",        &lua_gui2::intf_set_dialog_canvas		},
		{ "set_dialog_focus",         &lua_gui2::intf_set_dialog_focus      },
		{ "set_dialog_markup",        &lua_gui2::intf_set_dialog_markup		},
		{ "set_dialog_value",         &lua_gui2::intf_set_dialog_value		},
		{ "remove_dialog_item",       &lua_gui2::intf_remove_dialog_item    },
		{ "dofile", 		      &dispatch<&lua_kernel_base::intf_dofile>           },
		{ "require", 		      &dispatch<&lua_kernel_base::intf_require>          },
		{ "show_dialog",	      &video_dispatch<lua_gui2::show_dialog>   },
		{ "show_menu",               &video_dispatch<lua_gui2::show_menu>  },
		{ "show_message_dialog",     &video_dispatch<lua_gui2::show_message_dialog> },
		{ "show_popup_dialog",       &video_dispatch<lua_gui2::show_popup_dialog>   },
		{ "show_lua_console",	      &dispatch<&lua_kernel_base::intf_show_lua_console> },
		{ "compile_formula",          &lua_formula_bridge::intf_compile_formula},
		{ "eval_formula",             &lua_formula_bridge::intf_eval_formula},
		{ "name_generator",           &intf_name_generator           },
		{ nullptr, nullptr }
	};

	lua_getglobal(L, "wesnoth");
	if (!lua_istable(L,-1)) {
		lua_newtable(L);
	}
	luaL_setfuncs(L, callbacks, 0);
	//lua_cpp::set_functions(L, cpp_callbacks, 0);
	lua_setglobal(L, "wesnoth");

	// Override the print function
	cmd_log_ << "Redirecting print function...\n";

	lua_getglobal(L, "print");
	lua_setglobal(L, "std_print"); //storing original impl as 'std_print'
	lua_settop(L, 0); //clear stack, just to be sure

	lua_pushcfunction(L, &dispatch<&lua_kernel_base::intf_print>);
	lua_setglobal(L, "print");

	// Create the package table.
	lua_getglobal(L, "wesnoth");
	lua_newtable(L);
	lua_setfield(L, -2, "package");
	lua_pop(L, 1);

	// Get some callbacks for map locations
	cmd_log_ << "Adding map_location table...\n";

	static luaL_Reg const map_callbacks[] = {
		{ "get_direction",		&lua_map_location::intf_get_direction         		},
		{ "vector_sum",			&lua_map_location::intf_vector_sum			},
		{ "vector_negation",		&lua_map_location::intf_vector_negation			},
		{ "zero",			&lua_map_location::intf_vector_zero			},
		{ "rotate_right_around_center",	&lua_map_location::intf_rotate_right_around_center	},
		{ "tiles_adjacent",		&lua_map_location::intf_tiles_adjacent			},
		{ "get_adjacent_tiles",		&lua_map_location::intf_get_adjacent_tiles		},
		{ "distance_between",		&lua_map_location::intf_distance_between		},
		{ "get_in_basis_N_NE",		&lua_map_location::intf_get_in_basis_N_NE		},
		{ "get_relative_dir",		&lua_map_location::intf_get_relative_dir		},
		{ "parse_direction",		&lua_map_location::intf_parse_direction			},
		{ "write_direction",		&lua_map_location::intf_write_direction			},
		{ nullptr, nullptr }
	};

	// Create the map_location table.
	lua_getglobal(L, "wesnoth");
	lua_newtable(L);
	luaL_setfuncs(L, map_callbacks, 0);
	lua_setfield(L, -2, "map_location");
	lua_pop(L, 1);

	// Add mersenne twister rng wrapper
	cmd_log_ << "Adding rng tables...\n";
	lua_rng::load_tables(L);

	cmd_log_ << "Adding name generator metatable...\n";
	luaL_newmetatable(L, Gen);
	static luaL_Reg const generator[] = {
		{ "__call", &impl_name_generator_call},
		{ "__gc", &impl_name_generator_collect},
		{ nullptr, nullptr}
	};
	luaL_setfuncs(L, generator, 0);

	// Create formula bridge metatables
	cmd_log_ << lua_formula_bridge::register_metatables(L);

	// Loading ilua:
	cmd_log_ << "Loading ilua...\n";

	lua_settop(L, 0);
	lua_pushstring(L, "lua/ilua.lua");
	int result = intf_require(L);
	if (result == 1) {
		//run "ilua.set_strict()"
		lua_pushstring(L, "set_strict");
		lua_gettable(L, -2);
		if (!protected_call(0,0, std::bind(&lua_kernel_base::log_error, this, _1, _2))) {
			cmd_log_ << "Failed to activate strict mode.\n";
		} else {
			cmd_log_ << "Activated strict mode.\n";
		}

		lua_setglobal(L, "ilua"); //save ilua table as a global
	} else {
		cmd_log_ << "Error: failed to load ilua.\n";
	}
	lua_settop(L, 0);
}

lua_kernel_base::~lua_kernel_base()
{
	lua_close(mState);
}

void lua_kernel_base::log_error(char const * msg, char const * context)
{
	ERR_LUA << context << ": " << msg;
}

void lua_kernel_base::throw_exception(char const * msg, char const * context)
{
	throw game::lua_error(msg, context);
}

bool lua_kernel_base::protected_call(int nArgs, int nRets)
{
	error_handler eh = std::bind(&lua_kernel_base::log_error, this, _1, _2 );
	return protected_call(nArgs, nRets, eh);
}

bool lua_kernel_base::load_string(char const * prog)
{
	error_handler eh = std::bind(&lua_kernel_base::log_error, this, _1, _2 );
	return load_string(prog, eh);
}

bool lua_kernel_base::protected_call(int nArgs, int nRets, error_handler e_h)
{
	return protected_call(mState, nArgs, nRets, e_h);
}

extern int luaW_pcall_internal(lua_State *L, int nArgs, int nRets);

bool lua_kernel_base::protected_call(lua_State * L, int nArgs, int nRets, error_handler e_h)
{
	int errcode = luaW_pcall_internal(L, nArgs, nRets);

	if (errcode != LUA_OK) {
		char const * msg = lua_tostring(L, -1);
		std::string message = msg ? msg : "null string";

		std::string context = "When executing, ";
		if (errcode == LUA_ERRRUN) {
			context += "Lua runtime error";
		} else if (errcode == LUA_ERRERR) {
			context += "Lua error in attached debugger";
		} else if (errcode == LUA_ERRMEM) {
			context += "Lua out of memory error";
		} else if (errcode == LUA_ERRGCMM) {
			context += "Lua error in garbage collection metamethod";
		} else {
			context += "unknown lua error";
		}

		lua_pop(L, 1);

		e_h(message.c_str(), context.c_str());

		return false;
	}

	return true;
}

bool lua_kernel_base::load_string(char const * prog, error_handler e_h)
{
	int errcode = luaL_loadstring(mState, prog);
	if (errcode != LUA_OK) {
		char const * msg = lua_tostring(mState, -1);
		std::string message = msg ? msg : "null string";

		std::string context = "When parsing a string to lua, ";

		if (errcode == LUA_ERRSYNTAX) {
			context += " a syntax error";
		} else if(errcode == LUA_ERRMEM){
			context += " a memory error";
		} else if(errcode == LUA_ERRGCMM) {
			context += " an error in garbage collection metamethod";
		} else {
			context += " an unknown error";
		}

		lua_pop(mState, 1);

		e_h(message.c_str(), context.c_str());

		return false;
	}
	return true;
}

void lua_kernel_base::run_lua_tag(const config& cfg)
{
	int nArgs = 0;
	if (const config& args = cfg.child("args")) {
		luaW_pushconfig(this->mState, args);
		++nArgs;
	}
	run(cfg["code"].str().c_str(), nArgs);
}
// Call load_string and protected call. Make them throw exceptions.
//
void lua_kernel_base::throwing_run(const char * prog, int nArgs)
{
	cmd_log_ << "$ " << prog << "\n";
	error_handler eh = std::bind(&lua_kernel_base::throw_exception, this, _1, _2 );
	load_string(prog, eh);
	lua_insert(mState, -nArgs - 1);
	protected_call(nArgs, 0, eh);
}

// Do a throwing run, but if we catch a lua_error, reformat it with signature for this function and log it.
void lua_kernel_base::run(const char * prog, int nArgs)
{
	try {
		throwing_run(prog, nArgs);
	} catch (game::lua_error & e) {
		cmd_log_ << e.what() << "\n";
		lua_kernel_base::log_error(e.what(), "In function lua_kernel::run()");
	}
}

// Tests if a program resolves to an expression, and pretty prints it if it is, otherwise it runs it normally. Throws exceptions.
void lua_kernel_base::interactive_run(char const * prog) {
	std::string experiment = "ilua._pretty_print(";
	experiment += prog;
	experiment += ")";

	error_handler eh = std::bind(&lua_kernel_base::throw_exception, this, _1, _2 );

	try {
		// Try to load the experiment without syntax errors
		load_string(experiment.c_str(), eh);
	} catch (game::lua_error &) {
		throwing_run(prog, 0);	// Since it failed, fall back to the usual throwing_run, on the original input.
		return;
	}
	// experiment succeeded, now run but log normally.
	cmd_log_ << "$ " << prog << "\n";
	protected_call(0, 0, eh);
}
/**
 * Loads and executes a Lua file.
 * - Arg 1: string containing the file name.
 * - Ret *: values returned by executing the file body.
 */
int lua_kernel_base::intf_dofile(lua_State* L)
{
	if (lua_fileops::load_file(L) != 1) return 0;
	//^ should end with the file contents loaded on the stack. actually it will call lua_error otherwise, the return 0 is redundant.

	error_handler eh = std::bind(&lua_kernel_base::log_error, this, _1, _2 );
	protected_call(0, LUA_MULTRET, eh);
	return lua_gettop(L);
}

/**
 * Loads and executes a Lua file, if there is no corresponding entry in wesnoth.package.
 * Stores the result of the script in wesnoth.package and returns it.
 * - Arg 1: string containing the file name.
 * - Ret 1: value returned by the script.
 */
int lua_kernel_base::intf_require(lua_State* L)
{
	const char * m = luaL_checkstring(L, 1);
	if (!m) {
		luaL_argerror(L, 1, "found a null string argument to wesnoth require");
	}

	// Check if there is already an entry.

	lua_getglobal(L, "wesnoth");
	lua_pushstring(L, "package");
	lua_rawget(L, -2);
	lua_pushvalue(L, 1);
	lua_rawget(L, -2);
	if (!lua_isnil(L, -1) && !game_config::debug_lua) return 1;
	lua_pop(L, 1);
	lua_pushvalue(L, 1);
	// stack is now [packagename] [wesnoth] [package] [packagename]

	if (lua_fileops::load_file(L) != 1) return 0;
	//^ should end with the file contents loaded on the stack. actually it will call lua_error otherwise, the return 0 is redundant.
	// stack is now [packagename] [wesnoth] [package] [chunk]
	DBG_LUA << "require: loaded a file, now calling it\n";

	if (!protected_call(L, 0, 1, std::bind(&lua_kernel_base::log_error, this, _1, _2))) return 0;
	//^ historically if wesnoth.require fails it just yields nil and some logging messages, not a lua error
	// stack is now [packagename] [wesnoth] [package] [results]

	lua_pushvalue(L, 1);
	lua_pushvalue(L, -2);
	// stack is now [packagename] [wesnoth] [package] [results] [packagename] [results]
	// Add the return value to the table.

	lua_settable(L, -4);
	// stack is now [packagename] [wesnoth] [package] [results]
	return 1;
}
/**
 * Loads the "package" package into the Lua environment.
 * This action is inherently unsafe, as Lua scripts will now be able to
 * load C libraries on their own, hence granting them the same privileges
 * as the Wesnoth binary itsef.
 */
void lua_kernel_base::load_package()
{
	lua_State *L = mState;
	lua_pushcfunction(L, luaopen_package);
	lua_pushstring(L, "package");
	lua_call(L, 1, 0);
}

/**
 * Gets all the global variable names in the Lua environment. This is useful for tab completion.
 */
std::vector<std::string> lua_kernel_base::get_global_var_names()
{
	std::vector<std::string> ret;

	lua_State *L = mState;

	int idx = lua_gettop(L);
	lua_getglobal(L, "_G");
	lua_pushnil(L);

	while (lua_next(L, idx+1) != 0) {
		if (lua_isstring(L, -2)) {
			ret.push_back(lua_tostring(L,-2));
		}
		lua_pop(L,1);
	}
	lua_settop(L, idx);
	return ret;
}

/**
 * Gets all attribute names of an extended variable name. This is useful for tab completion.
 */
std::vector<std::string> lua_kernel_base::get_attribute_names(const std::string & input)
{
	std::vector<std::string> ret;
	std::string var_path = input; // it's convenient to make a copy, even if it's a little slower

	lua_State *L = mState;

	int base = lua_gettop(L);
	lua_getglobal(L, "_G");

	size_t idx = var_path.find('.');
	size_t last_dot = 0;
	while (idx != std::string::npos ) {
		last_dot += idx + 1; // Since idx was not npos, add it to the "last_dot" idx, so that last_dot keeps track of indices in input string
		lua_pushstring(L, var_path.substr(0, idx).c_str()); //push the part of the path up to the period
		lua_rawget(L, -2);

		if (!lua_istable(L,-1) && !lua_isuserdata(L,-1)) {
			lua_settop(L, base);
			return ret; //if we didn't get a table or userdata we can't proceed
		}

		var_path = var_path.substr(idx+1); // chop off the part of the path we just dereferenced
		idx = var_path.find('.'); // find the next .
	}

	std::string prefix = input.substr(0, last_dot);

	lua_pushnil(L);
	while (lua_next(L, -2) != 0) {
		if (lua_isstring(L, -2)) {
			ret.push_back(prefix + lua_tostring(L,-2));
		}
		lua_pop(L,1);
	}
	lua_settop(L, base);
	return ret;
}

lua_kernel_base*& lua_kernel_base::get_lua_kernel_base_ptr(lua_State *L)
{
	return *reinterpret_cast<lua_kernel_base**>(reinterpret_cast<char*>(L) - LUA_KERNEL_BASE_OFFSET);
}

uint32_t lua_kernel_base::get_random_seed()
{
	return seed_rng::next_seed();
}
