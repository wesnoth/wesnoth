# FindThreads override for Emscripten builds.
# Emscripten provides pthreads via the -pthread compiler/linker flag rather
# than a separate library, so CMake's standard FindThreads tests fail.
if(EMSCRIPTEN)
    set(CMAKE_THREAD_LIBS_INIT "-pthread" CACHE STRING "" FORCE)
    set(CMAKE_HAVE_THREADS_LIBRARY 1 CACHE BOOL "" FORCE)
    set(CMAKE_USE_PTHREADS_INIT 1 CACHE BOOL "" FORCE)
    set(Threads_FOUND TRUE CACHE BOOL "" FORCE)

    if(NOT TARGET Threads::Threads)
        add_library(Threads::Threads INTERFACE IMPORTED)
        set_target_properties(Threads::Threads PROPERTIES
            INTERFACE_COMPILE_OPTIONS "-pthread"
            INTERFACE_LINK_LIBRARIES "-pthread"
        )
    endif()
else()
    # Fall through to the real FindThreads for non-Emscripten builds.
    list(REMOVE_ITEM CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}")
    find_package(Threads ${Threads_FIND_VERSION}
        ${Threads_FIND_QUIETLY_ARG} ${Threads_FIND_REQUIRED_ARG})
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}")
endif()
