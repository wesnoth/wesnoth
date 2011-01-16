# $Id$

include_directories(../api)

find_package(Boost 1.35 REQUIRED COMPONENTS thread system)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -ggdb3")

set(common_srcs asio_listener.cpp asio_sender.cpp asio_timers.cpp)

set(server_srcs asio_server.cpp)

set(client_srcs asio_client.cpp asio_proxy_connection.cpp)

add_library( ana SHARED ${server_srcs} ${client_srcs} ${common_srcs})
add_library( ana-static STATIC ${server_srcs} ${client_srcs} ${common_srcs})

target_link_libraries( ana ${Boost_SYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY})
target_link_libraries( ana-static ${Boost_SYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY})

set(CMAKE_INSTALL_PREFIX /usr/local)

if(NOT MSVC)
	set(CMAKE_INSTALL_PREFIX /usr/local)
	install(TARGETS ana LIBRARY DESTINATION lib)
	install(TARGETS ana-static ARCHIVE DESTINATION lib)
endif(NOT MSVC)

set(ana_headers
	../api/ana.hpp
	../api/common.hpp
	../api/timers.hpp
	../api/buffers.hpp
	../api/predicates.hpp
	../api/binary_streams.hpp
)

if(NOT MSVC)
	install(FILES ${ana_headers} DESTINATION include/ana)
endif(NOT MSVC)
