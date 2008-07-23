set(XSLTOPTS 
   --nonet
  --stringparam callout.graphics 0
  --stringparam navig.graphics 0
  --stringparam admon.textlabel 1
  --stringparam admon.graphics 0
  --stringparam html.stylesheet "./styles/manual.css"
)

find_program(ASCIIDOC_EXECUTABLE "asciidoc")
find_program(DOS2UNIX_EXECUTABLE "dos2unix")
find_program(XSLTPROC_EXECUTABLE "xsltproc")

if(ASCIIDOC_EXECUTABLE AND DOS2UNIX_EXECUTABLE AND XSLTPROC_EXECUTABLE)
  add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/manual.en.xml
                     COMMAND ${ASCIIDOC_EXECUTABLE} -b docbook -d book -n -a toc -o manual.en.xml ${CMAKE_CURRENT_SOURCE_DIR}/manual.txt
                     COMMAND ${DOS2UNIX_EXECUTABLE} manual.en.xml
                     MAIN_DEPENDENCY manual.txt)
  add_custom_target(manual.xml DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/manual.en.xml)

  add_custom_command(OUTPUT manual.en.html
                     COMMAND ${XSLTPROC_EXECUTABLE} ${XSLTOPTS} /etc/asciidoc/docbook-xsl/xhtml.xsl 
                             manual.en.xml > ${CMAKE_CURRENT_SOURCE_DIR}/manual.en.html
                     MAIN_DEPENDENCY ${CMAKE_CURRENT_BINARY_DIR}/manual.en.xml)


  if(ENABLE_NLS)
    set(TRANSLATED)
    foreach(LANG ${LINGUAS})
      set(INFILE manual.${LANG}.xml)
      set(OUTFILE ${CMAKE_CURRENT_SOURCE_DIR}/manual.${LANG}.html)
      set(STAMPFILE ${CMAKE_CURRENT_BINARY_DIR}/manual.${LANG}.xml.stamp)

      #only build the manual if the xml file for that language is really present
      set_source_files_properties(${STAMPFILE} PROPERTIES GENERATED TRUE)
      add_custom_command(OUTPUT ${OUTFILE}
                         COMMAND test -f ${INFILE} && 
                         ${XSLTPROC_EXECUTABLE} ${XSLTOPTS} /etc/asciidoc/docbook-xsl/xhtml.xsl 
                         ${INFILE} > ${OUTFILE} || true
                         MAIN_DEPENDENCY ${STAMPFILE})

      list(APPEND TRANSLATED ${OUTFILE})
    endforeach(LANG ${LINGUAS})

    add_custom_target(docs DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/manual.en.html ${TRANSLATED})
    #add translated documents as dependency
    find_program(PO4A_TRANSLATE_EXECUTABLE "po4a-translate")
    if(PO4A_TRANSLATE_EXECUTABLE)
      add_dependencies(docs update-po4a-wesnoth-manual)
    endif(PO4A_TRANSLATE_EXECUTABLE)
  else()
    add_custom_target(docs DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/manual.en.html)
  endif(ENABLE_NLS)

endif(ASCIIDOC_EXECUTABLE AND DOS2UNIX_EXECUTABLE AND XSLTPROC_EXECUTABLE)

install(FILES manual.en.html DESTINATION "${DOCDIR}")
install(DIRECTORY images styles DESTINATION "${DOCDIR}"
          PATTERN ".svn" EXCLUDE )

if(ENABLE_NLS)
  foreach(LANG ${LINGUAS})
    install(FILES manual.${LANG}.html DESTINATION "${DOCDIR}" OPTIONAL)
  endforeach(LANG ${LINGUAS})
endif(ENABLE_NLS)
