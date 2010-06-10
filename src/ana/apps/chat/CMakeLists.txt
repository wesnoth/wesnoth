find_library( ANA ana REQUIRED )

find_library( GETOPT getopt_pp REQUIRED )

if ( ANA )
    if ( GETOPT )
        add_executable( server server.cpp )

        target_link_libraries( server ${ANA} ${GETOPT} )

        add_executable( client client.cpp )

        target_link_libraries( client ${ANA} ${GETOPT} )

    else ( GETOPT )
        message ( FATAL_ERROR "Install GetOpt_pp: getoptpp.googlecode.com" )
    endif (GETOPT )
else ( ANA )
    message ( FATAL_ERROR "Install the ana library." )
endif ( ANA )