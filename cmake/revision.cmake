if (EXISTS revision.hpp)
	file(READ revision.hpp OLD_VERSION)
endif (EXISTS revision.hpp)

execute_process(COMMAND ${SVNVERSION_EXECUTABLE} -n ${SRC_DIR}
	OUTPUT_VARIABLE SVN_VERSION)

if(SVN_VERSION MATCHES [0-9]+.*)

	if(NOT OLD_VERSION MATCHES ".*\"${SVN_VERSION}\".*")
		file(WRITE revision.hpp "#define REVISION \"${SVN_VERSION}\"\n")
	endif(NOT OLD_VERSION MATCHES ".*\"${SVN_VERSION}\".*")

endif(SVN_VERSION MATCHES [0-9]+.*)
