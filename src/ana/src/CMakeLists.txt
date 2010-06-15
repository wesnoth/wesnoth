# $Id$

include_directories( ../api )

find_package( Boost 1.35 REQUIRED COMPONENTS thread system)

set ( server_srcs asio_server.cpp asio_listener.cpp )

set ( client_srcs  asio_client.cpp asio_listener.cpp asio_proxy_connection.cpp )

add_library( ana SHARED ${server_srcs} ${client_srcs} )

target_link_libraries( ana ${Boost_SYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY} )

set ( CMAKE_INSTALL_PREFIX /usr/local )

install ( TARGETS ana LIBRARY DESTINATION lib )

set ( ana_headers ../api/ana.hpp
                  ../api/common.hpp
                  ../api/timers.hpp
                  ../api/buffers.hpp
                  ../api/predicates.hpp
                  ../api/binary_streams.hpp )

install ( FILES  ${ana_headers} DESTINATION include/ana )
