# $Id$

include_directories( ../../api )

add_executable( server server.cpp )

target_link_libraries( server ana )

add_executable( client client.cpp )

target_link_libraries( client ana )

