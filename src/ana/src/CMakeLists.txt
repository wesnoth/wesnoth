include_directories( ../api )

find_package( Boost 1.37 REQUIRED COMPONENTS thread system)

set ( server_srcs asio_server.cpp asio_listener.cpp )

set ( client_srcs  asio_client.cpp asio_listener.cpp asio_proxy_connection.cpp )

add_library( ana SHARED ${server_srcs} ${client_srcs} )

target_link_libraries( ana ${Boost_SYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY} )

set ( CMAKE_INSTALL_PREFIX /usr/local )

install ( TARGETS ana LIBRARY DESTINATION lib )

install ( FILES ../api/ana.hpp ../api/detail.hpp ../api/binary_streams.hpp DESTINATION include/ana )
