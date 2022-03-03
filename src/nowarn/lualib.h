#if defined(__clang__)
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wold-style-cast"
    #include "lua/lualib.h"
    #pragma clang diagnostic pop
#elif defined(__GNUG__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wold-style-cast"
    #include "lua/lualib.h"
    #pragma GCC diagnostic pop
#elif defined(_MSC_VER)
    #include "lua/lualib.h"
#endif
