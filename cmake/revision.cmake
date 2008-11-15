if (EXISTS revision-stamp)
  file(READ revision-stamp OLD_VERSION)
endif (EXISTS revision-stamp)

execute_process(COMMAND ${SVNVERSION_EXECUTABLE} -n ${SRC_DIR}
                OUTPUT_VARIABLE SVN_VERSION)

if(SVN_VERSION MATCHES [0-9]+.*)

  if(NOT OLD_VERSION MATCHES ".*\"${SVN_VERSION}\".*")
    file(WRITE revision-stamp "#define REVISION \"${SVN_VERSION}\"\n")
  endif(NOT OLD_VERSION MATCHES ".*\"${SVN_VERSION}\".*")

endif(SVN_VERSION MATCHES [0-9]+.*)
