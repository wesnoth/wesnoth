find_program(PO4A_EXECUTABLE "po4a")

#get domain from current dir
file(RELATIVE_PATH REL ${CMAKE_SOURCE_DIR}/po ${CMAKE_CURRENT_SOURCE_DIR} )
set(DOMAIN ${REL})

if(PO4A_EXECUTABLE)
  set(PO4A_ARGS --rm-backups --copyright-holder "Wesnoth Development Team")

  add_custom_target(update-po4a-${DOMAIN}
                    COMMAND ${PO4A_EXECUTABLE}
                    ${PO4A_ARGS} ${DOMAIN}.cfg
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

  add_dependencies(update-po4a update-po4a-${DOMAIN})

  add_custom_target(update-pot-${DOMAIN}
                    COMMAND ${PO4A_EXECUTABLE}
                    ${PO4A_ARGS} --no-translations ${DOMAIN}.cfg
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

  #po4a creates po- and pot-files at the same time, so add it to both targets
  add_dependencies(update-pot update-pot-${DOMAIN})
  add_dependencies(update-po update-pot-${DOMAIN})

endif(PO4A_EXECUTABLE)
