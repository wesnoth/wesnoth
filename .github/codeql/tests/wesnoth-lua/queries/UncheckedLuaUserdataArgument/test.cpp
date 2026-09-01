struct lua_State;
struct luaL_Reg {
    const char* name;
    int (*callback)(lua_State*);
};

extern "C" {
void* lua_touserdata(lua_State*, int);
void* luaL_checkudata(lua_State*, int, const char*);
void* luaL_testudata(lua_State*, int, const char*);
int luaL_newmetatable(lua_State*, const char*);
void luaL_setfuncs(lua_State*, const luaL_Reg*, int);
void lua_pushcfunction(lua_State*, int (*)(lua_State*));
void lua_pushlightuserdata(lua_State*, void*);
void lua_pushcclosure(lua_State*, int (*)(lua_State*), int);
void lua_rawset(lua_State*, int);
void lua_setfield(lua_State*, int, const char*);
}

struct Box {
    int value;
    int read() const { return value; }
};

// Example of an unchecked userdata receiver exposed through a metatable callback array.
static int unsafe_index(lua_State* L) {
    auto* box = static_cast<Box*>(lua_touserdata(L, 1));
    return box->value;
}

// Example of an unchecked callback stored in a metatable field and using its receiver as a C++ object.
static int unsafe_split(lua_State* L) {
    auto* box = static_cast<Box*>(lua_touserdata(L, 1));
    return box->value;
}

// Safe example: `luaL_checkudata` verifies the receiver before its C++ layout is used.
static int checked_index(lua_State* L) {
    luaL_checkudata(L, 1, "checked");
    auto* box = static_cast<Box*>(lua_touserdata(L, 1));
    return box->value;
}

// Safe example: the C++ layout is used only when `luaL_testudata` returns non-null.
static int guarded_index(lua_State* L) {
    if (luaL_testudata(L, 1, "guarded") != nullptr) {
        auto* box = static_cast<Box*>(lua_touserdata(L, 1));
        return box->value;
    }
    return 0;
}

// Example of a vulnerability where the `luaL_testudata` result is ignored.
static int ignored_test_index(lua_State* L) {
    luaL_testudata(L, 1, "ignored");
    auto* box = static_cast<Box*>(lua_touserdata(L, 1));
    return box->value;
}

// Example of a vulnerability where the check accepts a different userdata type.
static int wrong_type_index(lua_State* L) {
    luaL_checkudata(L, 1, "different");
    auto* box = static_cast<Box*>(lua_touserdata(L, 1));
    return box->value;
}

extern bool should_check();

// Example of a vulnerability where the type check can be skipped.
static int conditional_check_index(lua_State* L) {
    if (should_check()) {
        luaL_checkudata(L, 1, "conditional");
    }
    auto* box = static_cast<Box*>(lua_touserdata(L, 1));
    return box->value;
}

// Example of a vulnerability where a helper receives the Lua state and uses the receiver.
static int forwarding_helper(lua_State* L) {
    auto* box = static_cast<Box*>(lua_touserdata(L, 1));
    return box->value;
}

static int forwarding_callback(lua_State* L) {
    return forwarding_helper(L);
}

// Example of a vulnerability hidden behind Wesnoth's template callback adapter.
template<int (*Callback)(lua_State*)>
static int dispatch(lua_State* L) {
    return Callback(L);
}

static int dispatched_method(lua_State* L) {
    auto* box = static_cast<Box*>(lua_touserdata(L, 1));
    return box->value;
}

static void register_unsafe(lua_State* L) {
    luaL_newmetatable(L, "unsafe");
    static const luaL_Reg methods[] = {{"__index", unsafe_index}, {nullptr, nullptr}};
    luaL_setfuncs(L, methods, 0);
}

static void register_checked(lua_State* L) {
    luaL_newmetatable(L, "checked");
    static const luaL_Reg methods[] = {{"__index", checked_index}, {nullptr, nullptr}};
    luaL_setfuncs(L, methods, 0);
}

static void register_guarded(lua_State* L) {
    luaL_newmetatable(L, "guarded");
    static const luaL_Reg methods[] = {{"__index", guarded_index}, {nullptr, nullptr}};
    luaL_setfuncs(L, methods, 0);
}

static void register_ignored(lua_State* L) {
    luaL_newmetatable(L, "ignored");
    static const luaL_Reg methods[] = {{"__index", ignored_test_index}, {nullptr, nullptr}};
    luaL_setfuncs(L, methods, 0);
}

static void register_wrong_type(lua_State* L) {
    luaL_newmetatable(L, "expected");
    static const luaL_Reg methods[] = {{"__index", wrong_type_index}, {nullptr, nullptr}};
    luaL_setfuncs(L, methods, 0);
}

static void register_conditional(lua_State* L) {
    luaL_newmetatable(L, "conditional");
    static const luaL_Reg methods[] = {
        {"__index", conditional_check_index},
        {nullptr, nullptr}
    };
    luaL_setfuncs(L, methods, 0);
}

static void register_forwarding(lua_State* L) {
    luaL_newmetatable(L, "forwarding");
    static const luaL_Reg methods[] = {{"__index", forwarding_callback}, {nullptr, nullptr}};
    luaL_setfuncs(L, methods, 0);
}

static void register_dispatched(lua_State* L) {
    luaL_newmetatable(L, "dispatched");
    static const luaL_Reg methods[] = {
        {"__index", &dispatch<&dispatched_method>},
        {nullptr, nullptr}
    };
    luaL_setfuncs(L, methods, 0);
}

static void register_split(lua_State* L) {
    luaL_newmetatable(L, "split");
    lua_pushcfunction(L, unsafe_split);
    lua_setfield(L, -2, "__index");
}

extern "C" {
void lua_newtable(lua_State*);
void lua_pushstring(lua_State*, const char*);
void lua_setglobal(lua_State*, const char*);
}

// Safe example: a protected `__index` callback receives its own userdata as argument 1.
static int protected_unsafe(lua_State* L) {
    auto* box = static_cast<Box*>(lua_touserdata(L, 1));
    return box->value;
}

// Example of an unchecked userdata argument passed to a regular module function.
static int module_unsafe(lua_State* L) {
    auto* box = static_cast<Box*>(lua_touserdata(L, 1));
    return box->value;
}

// Safe example: the function checks its argument, then reads a native value it pushed at `-1`.
static int module_checked(lua_State* L) {
    luaL_checkudata(L, 1, "box");
    auto* box = static_cast<Box*>(lua_touserdata(L, 1));
    static Box internal_box{0};
    lua_pushlightuserdata(L, &internal_box);
    auto* internal = static_cast<Box*>(lua_touserdata(L, -1));
    return box->value + internal->value;
}

// Example of a vulnerability where `-1` still names the last script-supplied argument.
static int module_negative_index_argument(lua_State* L) {
    auto* box = static_cast<Box*>(lua_touserdata(L, -1));
    return box->value;
}

// Example of an unchecked table callback calling a C++ member through its userdata argument.
static int direct_unsafe(lua_State* L) {
    auto* box = static_cast<Box*>(lua_touserdata(L, 1));
    return box->read();
}

// Example of an unchecked global callback dereferencing its userdata argument as a C++ object.
static int global_unsafe(lua_State* L) {
    auto* box = static_cast<Box*>(lua_touserdata(L, 1));
    return (*box).value;
}

static void register_protected_unsafe(lua_State* L) {
    luaL_newmetatable(L, "protected unsafe");
    static const luaL_Reg methods[] = {{"__index", protected_unsafe}, {nullptr, nullptr}};
    luaL_setfuncs(L, methods, 0);
    lua_pushstring(L, "protected");
    lua_setfield(L, -2, "__metatable");
}

static void register_module(lua_State* L) {
    lua_newtable(L);
    static const luaL_Reg functions[] = {
        {"do_my_thing", module_unsafe},
        {"checked", module_checked},
        {"negative_index_argument", module_negative_index_argument},
        {nullptr, nullptr}
    };
    luaL_setfuncs(L, functions, 0);
}

static void register_direct(lua_State* L) {
    lua_newtable(L);
    lua_pushcfunction(L, direct_unsafe);
    lua_setfield(L, -2, "direct");
}

static void register_global(lua_State* L) {
    lua_pushcfunction(L, global_unsafe);
    lua_setglobal(L, "global_unsafe");
}

// Example of an unchecked userdata vulnerability registered with `lua_rawset`.
// `lua_pushcclosure` consumes one upvalue before it pushes the callback.
static int raw_closure_unsafe(lua_State* L) {
    auto* box = static_cast<Box*>(lua_touserdata(L, 1));
    return box->value;
}

static void register_raw_closure(lua_State* L) {
    luaL_newmetatable(L, "raw closure");
    lua_pushstring(L, "__index");
    lua_pushlightuserdata(L, L);
    auto callback = raw_closure_unsafe;
    lua_pushcclosure(L, callback, 1);
    lua_rawset(L, -3);
}

// Safe example: a protected `__gc` callback receives only its own userdata.
static int protected_collect(lua_State* L) {
    auto* box = static_cast<Box*>(lua_touserdata(L, 1));
    return box->value;
}

static void register_protected_collect(lua_State* L) {
    luaL_newmetatable(L, "protected collect");
    lua_pushcfunction(L, protected_collect);
    lua_setfield(L, -2, "__gc");
    lua_pushstring(L, "protected");
    lua_setfield(L, -2, "__metatable");
}

// Example of a protected `__newindex` callback trusting the script-supplied value.
static int protected_newindex(lua_State* L) {
    auto* box = static_cast<Box*>(lua_touserdata(L, 3));
    return box->value;
}

static void register_protected_newindex(lua_State* L) {
    luaL_newmetatable(L, "protected newindex");
    lua_pushcfunction(L, protected_newindex);
    lua_setfield(L, -2, "__newindex");
    lua_pushstring(L, "protected");
    lua_setfield(L, -2, "__metatable");
}

// Example of a protected `__index` callback trusting the script-supplied key.
static int protected_index_key(lua_State* L) {
    auto* box = static_cast<Box*>(lua_touserdata(L, 2));
    return box->value;
}

// Example of a protected `__index` callback reading the script-supplied key through `-1`.
static int protected_negative_index_key(lua_State* L) {
    auto* box = static_cast<Box*>(lua_touserdata(L, -1));
    return box->value;
}

static void register_protected_negative_index_key(lua_State* L) {
    luaL_newmetatable(L, "protected negative index key");
    lua_pushcfunction(L, protected_negative_index_key);
    lua_setfield(L, -2, "__index");
    lua_pushstring(L, "protected");
    lua_setfield(L, -2, "__metatable");
}

// Example of a protected binary metamethod trusting both script-supplied operands.
static int protected_add_operands(lua_State* L) {
    auto* left = static_cast<Box*>(lua_touserdata(L, 1));
    auto* right = static_cast<Box*>(lua_touserdata(L, 2));
    return left->value + right->value;
}

// Example of a protected `__call` callback trusting an extra script-supplied argument.
static int protected_call_argument(lua_State* L) {
    auto* box = static_cast<Box*>(lua_touserdata(L, 2));
    return box->value;
}

// Example of a protected `__close` callback trusting the script-controlled error value.
static int protected_close_error(lua_State* L) {
    auto* box = static_cast<Box*>(lua_touserdata(L, 2));
    return box->value;
}

// Safe example: Lua duplicates a unary operand, so argument 2 is not independently supplied.
static int protected_unary_duplicate(lua_State* L) {
    auto* box = static_cast<Box*>(lua_touserdata(L, 2));
    return box->value;
}

static void register_protected_dispatch_families(lua_State* L) {
    luaL_newmetatable(L, "protected dispatch families");
    static const luaL_Reg methods[] = {
        {"__index", protected_index_key},
        {"__add", protected_add_operands},
        {"__call", protected_call_argument},
        {"__close", protected_close_error},
        {"__unm", protected_unary_duplicate},
        {nullptr, nullptr}
    };
    luaL_setfuncs(L, methods, 0);
    lua_pushstring(L, "protected");
    lua_setfield(L, -2, "__metatable");
}

// Safe example: the protected `__newindex` callback checks its script-supplied value.
static int protected_checked_newindex(lua_State* L) {
    luaL_checkudata(L, 3, "value box");
    auto* box = static_cast<Box*>(lua_touserdata(L, 3));
    return box->value;
}

static void register_protected_checked_newindex(lua_State* L) {
    luaL_newmetatable(L, "protected checked newindex");
    lua_pushcfunction(L, protected_checked_newindex);
    lua_setfield(L, -2, "__newindex");
    lua_pushstring(L, "protected");
    lua_setfield(L, -2, "__metatable");
}

static int read_box(Box* box) {
    return box->value;
}

// Example of an unchecked userdata pointer passed to a helper before its C++ layout is used.
static int interprocedural_unsafe(lua_State* L) {
    auto* box = static_cast<Box*>(lua_touserdata(L, 1));
    return read_box(box);
}

static void register_interprocedural_unsafe(lua_State* L) {
    lua_newtable(L);
    static const luaL_Reg functions[] = {
        {"interprocedural_unsafe", interprocedural_unsafe},
        {nullptr, nullptr}
    };
    luaL_setfuncs(L, functions, 0);
}

// Safe example: the userdata is checked before its pointer is passed to a helper.
static int interprocedural_checked(lua_State* L) {
    luaL_checkudata(L, 1, "interprocedural checked");
    auto* box = static_cast<Box*>(lua_touserdata(L, 1));
    return read_box(box);
}

static void register_interprocedural_checked(lua_State* L) {
    lua_newtable(L);
    static const luaL_Reg functions[] = {
        {"interprocedural_checked", interprocedural_checked},
        {nullptr, nullptr}
    };
    luaL_setfuncs(L, functions, 0);
}

