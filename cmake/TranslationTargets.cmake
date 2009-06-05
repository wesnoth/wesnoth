MACRO (ADD_POT_TARGET DOMAIN)
  add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/remove-potcdate.sed
                     COMMAND sed
                     ARGS -e "'/^#/d'" remove-potcdate.sin > remove-potcdate.sed
                     MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/remove-potcdate.sin
                     WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

  set(POTFILE ${DOMAIN}.pot)

  add_custom_target(update-pot-${DOMAIN}
                    COMMAND ${CMAKE_BINARY_DIR}/po/pot-update.sh ${DOMAIN}
                    DEPENDS remove-potcdate.sed
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

  set_source_files_properties(${POTFILE} PROPERTIES GENERATED TRUE)
  add_dependencies(update-pot update-pot-${DOMAIN})

ENDMACRO (ADD_POT_TARGET DOMAIN)


MACRO (ADD_PO_TARGETS DOMAIN)
if(GETTEXT_MSGMERGE_EXECUTABLE AND GETTEXT_MSGINIT_EXECUTABLE)
  set(LINGUAS ${ARGN})

  add_custom_target(update-po-${DOMAIN})

  foreach(LANG ${LINGUAS})
    set(POFILE ${LANG}.po)
    set(UPDFILE ${LANG}.upd)

    add_custom_target(update-po-${DOMAIN}-${LANG}
                      COMMAND ${CMAKE_SOURCE_DIR}/po/po-update.sh 
                      ${DOMAIN} ${LANG} ${GETTEXT_MSGMERGE_EXECUTABLE} ${GETTEXT_MSGINIT_EXECUTABLE} 
                      DEPENDS ${DOMAIN}.pot
                      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

    add_dependencies(update-po-${DOMAIN} update-po-${DOMAIN}-${LANG})
    #add_dependencies(update-po-${DOMAIN}-${LANG} )
 
    set_source_files_properties(${POFILE} PROPERTIES GENERATED TRUE)
  endforeach(LANG ${LINGUAS})

  add_dependencies(update-po update-po-${DOMAIN})

endif(GETTEXT_MSGMERGE_EXECUTABLE AND GETTEXT_MSGINIT_EXECUTABLE)
ENDMACRO (ADD_PO_TARGETS DOMAIN)


MACRO (ADD_MO_TARGETS DOMAIN)
if(GETTEXT_MSGFMT_EXECUTABLE)

  set(LINGUAS ${ARGN})
  set(MOFILES)

  foreach(LANG ${LINGUAS})
    set(POFILE ${LANG}.po)
    set_source_files_properties(${POFILE} PROPERTIES GENERATED TRUE)

  endforeach(LANG ${LINGUAS})

endif(GETTEXT_MSGFMT_EXECUTABLE)
ENDMACRO (ADD_MO_TARGETS DOMAIN)


macro(ADD_PO4A_POT_TARGET FILES DOMAIN TYPE OPTIONS)

  set(_FILES ${FILES})
  separate_arguments(_FILES)

  set(_OPTIONS ${OPTIONS})
  separate_arguments(_OPTIONS)

  set(_FILE_OPTION)
  foreach(FILE ${_FILES})
    file(RELATIVE_PATH _REL_FILE ${CMAKE_CURRENT_SOURCE_DIR} ${FILE} )
    list(APPEND _FILE_OPTION -m ${_REL_FILE})
  endforeach(FILE ${FILES})

  add_custom_command(OUTPUT ${DOMAIN}.pot
                    COMMAND ${PO4A_GETTEXTTIZE_EXECUTABLE}
                    -f ${TYPE} ${_OPTIONS} ${_FILE_OPTION} -p ${DOMAIN}.pot
                    DEPENDS ${FILES}
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

  add_custom_target(update-pot-${DOMAIN} DEPENDS ${DOMAIN}.pot)
  add_dependencies(update-pot update-pot-${DOMAIN})
endmacro(ADD_PO4A_POT_TARGET FILES DOMAIN)


macro(ADD_PO4A_TRANSLATE_TARGET INFILE OUTFILE STAMPFILE DOMAIN FORMAT CHARSET OPTIONS)
  set(_LINGUAS ${ARGN})
  set(_OPTIONS ${OPTIONS})
  separate_arguments(_OPTIONS)
  string(REGEX REPLACE ".*/([^ /]*)$" "\\1" _FILENAME ${INFILE})

  set(_PO4AFILES)
  foreach(_LANG ${_LINGUAS})
    set(_POFILE ${_LANG}.po)
    set(LANG ${_LANG})
    string(CONFIGURE ${OUTFILE} _OUTFILE)
    string(CONFIGURE ${STAMPFILE} _STAMPFILE)
    set(LANG)

    set(_CHARSET ${${CHARSET}_${_LANG}})
    if(NOT _CHARSET)
      set(_CHARSET ${${CHARSET}_default})
      if(NOT _CHARSET)
        set(_CHARSET "utf-8")
      endif(NOT _CHARSET)
    endif(NOT _CHARSET)

    set_source_files_properties(${_POFILE} PROPERTIES GENERATED TRUE)

    add_custom_command(OUTPUT "${_STAMPFILE}"
                       COMMAND ${PO4A_TRANSLATE_EXECUTABLE}
                       -f ${FORMAT} ${_OPTIONS} -L ${_CHARSET} -m ${INFILE} -p "${_POFILE}" -l "${_OUTFILE}"
                       COMMAND ${CMAKE_COMMAND} -E touch "${_STAMPFILE}"
                       MAIN_DEPENDENCY "${_POFILE}"
                       WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
  
    list(APPEND _PO4AFILES ${_STAMPFILE})
  endforeach(_LANG ${_LINGUAS})

  add_custom_target(update-po4a-${_FILENAME} DEPENDS ${_PO4AFILES})
  
endmacro(ADD_PO4A_TRANSLATE_TARGET INFILE OUTFILE DOMAIN FORMAT OPTIONS)
