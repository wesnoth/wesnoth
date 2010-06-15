# $Id$

find_package(SDL_net REQUIRED)

include_directories( ../api )

add_executable( sdl_test sdl_with_ana.cpp )

target_link_libraries( sdl_test ana ${SDLNET_LIBRARY} )
