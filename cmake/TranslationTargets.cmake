MACRO (ADD_POT_TARGET DOMAIN)
  add_custom_command(OUTPUT remove-potcdate.sed
                     COMMAND sed
                     ARGS -e "'/^#/d'" remove-potcdate.sin > remove-potcdate.sed
                     MAIN_DEPENDENCY remove-potcdate.sin )

  set(POTFILE ${DOMAIN}.pot)

  add_custom_target(update-pot-${DOMAIN}
                    COMMAND ${CMAKE_SOURCE_DIR}/po/pot-update.sh ${DOMAIN}
                    DEPENDS remove-potcdate.sed
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
#  add_custom_command(OUTPUT ${POTFILE}
#                    COMMAND ${CMAKE_SOURCE_DIR}/po/pot-update.sh ${DOMAIN}
#                    DEPENDS remove-potcdate.sed
#                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
#  add_custom_target(update-pot-${DOMAIN} DEPENDS ${POTFILE})

  set_source_files_properties(${POTFILE} PROPERTIES GENERATED TRUE)
  add_dependencies(update-pot update-pot-${DOMAIN})

ENDMACRO (ADD_POT_TARGET DOMAIN)


MACRO (ADD_PO_TARGETS DOMAIN)
  set(LINGUAS ${ARGN})
  set(POFILES)

  foreach(LANG ${LINGUAS})
    set(POFILE ${LANG}.po)
    set(UPDFILE ${LANG}.upd)

    add_custom_command(OUTPUT ${UPDFILE}
                       COMMAND ${CMAKE_SOURCE_DIR}/po/po-update.sh 
                       ARGS ${DOMAIN} ${LANG} ${GETTEXT_MSGMERGE_EXECUTABLE} ${GETTEXT_MSGINIT_EXECUTABLE} 
                       MAIN_DEPENDENCY ${DOMAIN}.pot )
 
    set_source_files_properties(${POFILE} PROPERTIES GENERATED TRUE)
    set(POFILES ${POFILES} ${UPDFILE})
  endforeach(LANG ${LINGUAS})

  add_custom_target(update-po-${DOMAIN} DEPENDS ${POFILES})
  add_dependencies(update-po update-po-${DOMAIN})

ENDMACRO (ADD_PO_TARGETS DOMAIN)


MACRO (ADD_MO_TARGETS DOMAIN)
  set(LINGUAS ${ARGN})
  set(MOFILES)

  foreach(LANG ${LINGUAS})
    set(POFILE ${LANG}.po)
    set(MOFILE ${LANG}.gmo)

    add_custom_command(OUTPUT ${MOFILE} 
                       COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} 
                       ARGS -c -o ${MOFILE} ${POFILE} 
                       MAIN_DEPENDENCY ${POFILE} )

    install(FILES ${MOFILE} DESTINATION ${LOCALE_INSTALL}/${LANG}/LC_MESSAGES/ RENAME ${DOMAIN}.mo)

    set(MOFILES ${MOFILES} ${MOFILE})

  endforeach(LANG ${LINGUAS})

  add_custom_target(update-gmo-${DOMAIN} ALL DEPENDS ${MOFILES})
  add_dependencies(update-gmo update-gmo-${DOMAIN})
ENDMACRO (ADD_MO_TARGETS DOMAIN)
