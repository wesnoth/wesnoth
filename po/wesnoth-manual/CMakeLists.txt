find_program(PO4A_GETTEXTTIZE_EXECUTABLE "po4a-gettextize")
find_program(PO4A_TRANSLATE_EXECUTABLE "po4a-translate")

if(ASCIIDOC_EXECUTABLE AND DOS2UNIX_EXECUTABLE AND XSLTPROC_EXECUTABLE)
  set(CAN_GENERATE_DOC TRUE)
endif()

if(PO4A_GETTEXTTIZE_EXECUTABLE AND PO4A_TRANSLATE_EXECUTABLE)
  #get domain from current dir
  file(RELATIVE_PATH REL ${CMAKE_SOURCE_DIR}/po ${CMAKE_CURRENT_SOURCE_DIR} )
  set(DOMAIN ${REL})

  ADD_PO_TARGETS(${DOMAIN} ${LINGUAS})
  
  if(EXISTS "${CMAKE_BINARY_DIR}/doc/manual/manual.en.xml" OR CAN_GENERATE_DOC)

    set(FILES "${CMAKE_BINARY_DIR}/doc/manual/manual.en.xml")
    set(MANUAL_CHARSET_default utf-8)

    ADD_PO4A_POT_TARGET("${FILES}" ${DOMAIN} docbook "-M utf-8")

    add_custom_target(update-po4a-${DOMAIN})
    set(LANG)
    ADD_PO4A_TRANSLATE_TARGET("${CMAKE_BINARY_DIR}/doc/manual/manual.en.xml" 
                              "${CMAKE_BINARY_DIR}/doc/manual/manual.@LANG@.xml"
                              "${CMAKE_BINARY_DIR}/doc/manual/manual.@LANG@.xml.stamp"
                              ${DOMAIN} docbook "MANUAL_CHARSET" "-k 80" ${LINGUAS})

    add_dependencies(update-po4a-${DOMAIN} update-po4a-manual.en.xml)
    add_dependencies(update-po4a update-po4a-${DOMAIN})

    if(CAN_GENERATE_DOC)
      add_dependencies(update-pot-${DOMAIN} manual.xml)
      add_dependencies(update-po4a-manual.en.xml manual.xml)
    endif(CAN_GENERATE_DOC)

  endif(EXISTS "${CMAKE_BINARY_DIR}/doc/manual/manual.en.xml" OR CAN_GENERATE_DOC)
endif(PO4A_GETTEXTTIZE_EXECUTABLE AND PO4A_TRANSLATE_EXECUTABLE)
