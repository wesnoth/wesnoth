#pragma once

#if defined(__clang__)
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wold-style-cast"
#elif defined(__GNUG__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wold-style-cast"
#endif

#if defined(HAVE_SYSTEM_LUA)
    #include "lauxlib.h"
#else
    #include "modules/lua/lauxlib.h"
#endif

constexpr int luaL_buffersize = LUAL_BUFFERSIZE;

#if defined(__clang__)
    #pragma clang diagnostic pop
#elif defined(__GNUG__)
    #pragma GCC diagnostic pop
#endif
