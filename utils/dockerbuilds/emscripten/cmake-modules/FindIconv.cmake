# FindIconv override for Emscripten builds.
# Emscripten provides iconv in its sysroot (musl libc), so the standard
# FindIconv try_compile test should succeed, but it fails under
# cross-compilation.  Short-circuit it.
if(EMSCRIPTEN)
    set(Iconv_FOUND TRUE CACHE BOOL "" FORCE)
    set(Iconv_IS_BUILT_IN TRUE CACHE BOOL "" FORCE)
    set(Iconv_INCLUDE_DIR "" CACHE PATH "" FORCE)
    set(Iconv_LIBRARIES "" CACHE STRING "" FORCE)

    if(NOT TARGET Iconv::Iconv)
        add_library(Iconv::Iconv INTERFACE IMPORTED)
    endif()
else()
    list(REMOVE_ITEM CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}")
    find_package(Iconv ${ARGV})
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}")
endif()
