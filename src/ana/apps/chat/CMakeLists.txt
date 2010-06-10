
find_library( GETOPT getopt_pp REQUIRED )

include_directories( ../../api )

if ( GETOPT )
    add_executable( server server.cpp )

    target_link_libraries( server ana ${GETOPT} )

    add_executable( client client.cpp )

    target_link_libraries( client ana ${GETOPT} )

else ( GETOPT )
    message ( FATAL_ERROR "Install GetOpt_pp: getoptpp.googlecode.com" )
endif (GETOPT )
