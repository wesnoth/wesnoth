#pragma once

#if defined(__clang__)
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wold-style-cast"
    #include "modules/lua/lauxlib.h"
    constexpr int luaL_buffersize = LUAL_BUFFERSIZE;
    #pragma clang diagnostic pop
#elif defined(__GNUG__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wold-style-cast"
    #include "modules/lua/lauxlib.h"
    constexpr int luaL_buffersize = LUAL_BUFFERSIZE;
    #pragma GCC diagnostic pop
#elif defined(_MSC_VER)
    #include "modules/lua/lauxlib.h"
    constexpr int luaL_buffersize = LUAL_BUFFERSIZE;
#endif
