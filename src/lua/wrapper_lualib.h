#if defined(__clang__)
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wold-style-cast"
#elif defined(__GNUG__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wold-style-cast"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if defined(HAVE_SYSTEM_LUA)
    #include "lualib.h"
#else
    #include "modules/lua/lualib.h"
#endif

#ifdef __cplusplus
}
#endif

#if defined(__clang__)
    #pragma clang diagnostic pop
#elif defined(__GNUG__)
    #pragma GCC diagnostic pop
#endif
