struct lua_State;
struct luaL_Reg {
    const char* name;
    int (*callback)(lua_State*);
};

extern "C" {
int luaL_newmetatable(lua_State*, const char*);
void luaL_setfuncs(lua_State*, const luaL_Reg*, int);
void lua_pushcfunction(lua_State*, int (*)(lua_State*));
void lua_pushstring(lua_State*, const char*);
void lua_setfield(lua_State*, int, const char*);
}

void lua_newtable(lua_State*);

static int callback(lua_State*) { return 0; }
static const luaL_Reg callbacks[] = {
    {"__index", callback},
    {nullptr, nullptr}
};

// Example of an unprotected metatable registered with a callback array.
static void exposed_array(lua_State* L) {
    luaL_newmetatable(L, "exposed array");
    luaL_setfuncs(L, callbacks, 0);
}

// Safe example: a non-nil `__metatable` value protects the callback array.
static void protected_array(lua_State* L) {
    luaL_newmetatable(L, "protected array");
    luaL_setfuncs(L, callbacks, 0);
    lua_pushstring(L, "protected array");
    lua_setfield(L, -2, "__metatable");
}

// Example of a vulnerability where the protection write can be skipped.
static void conditionally_protected(lua_State* L, bool protect) {
    luaL_newmetatable(L, "conditional protection");
    luaL_setfuncs(L, callbacks, 0);
    if (protect) {
        lua_pushstring(L, "conditional protection");
        lua_setfield(L, -2, "__metatable");
    }
}

// Example of a vulnerability where `__metatable` is set to Lua nil.
static void nil_protection(lua_State* L) {
    luaL_newmetatable(L, "nil protection");
    luaL_setfuncs(L, callbacks, 0);
    lua_pushstring(L, nullptr);
    lua_setfield(L, -2, "__metatable");
}

// Example of a vulnerability where a different Lua state receives the protection.
static void wrong_state_protection(lua_State* L, lua_State* other) {
    luaL_newmetatable(L, "wrong state");
    luaL_setfuncs(L, callbacks, 0);
    lua_pushstring(other, "wrong state");
    lua_setfield(other, -2, "__metatable");
}

// Example of an unprotected native method registered with a pushed callback.
static void exposed_split(lua_State* L) {
    luaL_newmetatable(L, "exposed split");
    lua_pushcfunction(L, callback);
    lua_setfield(L, -2, "named_method");
}

// Safe example: a pushed callback is followed by a non-nil protection value.
static void protected_split(lua_State* L) {
    luaL_newmetatable(L, "protected split");
    lua_pushcfunction(L, callback);
    lua_setfield(L, -2, "__index");
    lua_pushstring(L, "protected split");
    lua_setfield(L, -2, "__metatable");
}

// Example of a vulnerability where the protection key and value are reversed.
static void flipped_protection(lua_State* L) {
    luaL_newmetatable(L, "flipped protection");
    luaL_setfuncs(L, callbacks, 0);
    lua_pushstring(L, "__metatable");
    lua_setfield(L, -2, "flipped protection");
}

// Example of a vulnerability where `__metatable` protects a different table.
static void protection_on_another_table(lua_State* L) {
    luaL_newmetatable(L, "wrong target");
    luaL_setfuncs(L, callbacks, 0);
    lua_newtable(L);
    lua_pushstring(L, "wrong target");
    lua_setfield(L, -2, "__metatable");
}

// Safe example: every branch supplies a non-nil protection value.
static void branch_protection(lua_State* L, bool first) {
    luaL_newmetatable(L, "branch protection");
    luaL_setfuncs(L, callbacks, 0);
    if (first) {
        lua_pushstring(L, "first");
    } else {
        lua_pushstring(L, "second");
    }
    lua_setfield(L, -2, "__metatable");
}

void lua_pushvalue(lua_State*, int);

// Safe example: `lua_pushvalue` copies the non-nil protection value.
static void copied_protection_value(lua_State* L) {
    luaL_newmetatable(L, "copied protection");
    luaL_setfuncs(L, callbacks, 0);
    lua_pushstring(L, "copied protection");
    lua_pushvalue(L, -1);
    lua_setfield(L, -3, "__metatable");
}

// Example of an unprotected callback pushed along either control-flow branch.
static void exposed_branch_registration(lua_State* L, bool first) {
    luaL_newmetatable(L, "branch registration");
    if (first) {
        lua_pushcfunction(L, callback);
    } else {
        lua_pushcfunction(L, callback);
    }
    lua_setfield(L, -2, "__index");
}

// Safe example: the protection write occurs before callback registration.
static void protected_before_registration(lua_State* L) {
    luaL_newmetatable(L, "protected before registration");
    lua_pushstring(L, "protected before registration");
    lua_setfield(L, -2, "__metatable");
    luaL_setfuncs(L, callbacks, 0);
}

// Example of a vulnerability where an earlier protection write can be skipped.
static void conditionally_protected_before_registration(lua_State* L, bool protect) {
    luaL_newmetatable(L, "conditional before registration");
    if (protect) {
        lua_pushstring(L, "conditional before registration");
        lua_setfield(L, -2, "__metatable");
    }
    luaL_setfuncs(L, callbacks, 0);
}

// Example of a vulnerability where an expression evaluates to a different field name.
static void expression_key_not_protection(lua_State* L) {
    luaL_newmetatable(L, "expression key");
    luaL_setfuncs(L, callbacks, 0);
    lua_pushstring(L, "protected");
    lua_setfield(L, -2, false ? "__metatable" : "other");
}

namespace std {
class string_view {
public:
    string_view(const char*);
};
}

template<typename T>
static void luaW_table_set(lua_State*, int, std::string_view, const T&) {}

// Safe example: Wesnoth's table helper sets a non-nil `__metatable` value.
static void protected_with_helper(lua_State* L) {
    luaL_newmetatable(L, "protected with helper");
    luaL_setfuncs(L, callbacks, 0);
    luaW_table_set(L, -1, "__metatable", "protected");
}

void lua_createtable(lua_State*, int, int);
void luaW_getglobal(lua_State*, const char*, const char*);

// Safe example: protection still applies after building a separate `__index` table.
static void protected_after_index_table(lua_State* L) {
    luaL_newmetatable(L, "protected after index table");
    luaL_setfuncs(L, callbacks, 0);
    lua_createtable(L, 0, 1);
    luaW_getglobal(L, "string", "format");
    lua_setfield(L, -2, "format");
    luaW_getglobal(L, "stringx", "vformat");
    lua_setfield(L, -2, "vformat");
    lua_setfield(L, -2, "__index");
    lua_pushstring(L, "protected after index table");
    lua_setfield(L, -2, "__metatable");
}

