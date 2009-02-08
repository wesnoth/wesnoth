find_program(PO4A_GETTEXTTIZE_EXECUTABLE "po4a-gettextize")
find_program(PO4A_TRANSLATE_EXECUTABLE "po4a-translate")

if(PO4A_GETTEXTTIZE_EXECUTABLE AND PO4A_TRANSLATE_EXECUTABLE)
  #get domain from current dir
  file(RELATIVE_PATH REL ${CMAKE_SOURCE_DIR}/po ${CMAKE_CURRENT_SOURCE_DIR} )
  set(DOMAIN ${REL})

  set(FILES "${CMAKE_SOURCE_DIR}/doc/man/wesnoth.6"
            "${CMAKE_SOURCE_DIR}/doc/man/wesnothd.6")

  set(CHARSET_default utf8)
#everything below is not needed anymore, just using utf8 is best (as with autotools)...
#  set(CHARSET_bg cp1251)
#  set(CHARSET_ca_ES@valencia iso-8859-15)
#  set(CHARSET_cs iso-8859-2)
#  set(CHARSET_el iso-8859-7)
#  set(CHARSET_he iso-8859-8)
#  set(CHARSET_hr utf8)
#  set(CHARSET_hu iso-8859-2)
#  set(CHARSET_ja euc-jp)
#  set(CHARSET_ko euc-kr)
#  set(CHARSET_lt iso-8859-13)
#  set(CHARSET_pl iso-8859-2)
#  set(CHARSET_racv iso-8859-15)
#  set(CHARSET_ro iso-8859-2)
#  set(CHARSET_ru iso-8859-5)
#  set(CHARSET_sk iso-8859-2)
#  set(CHARSET_sl iso-8859-2)
#  set(CHARSET_sr utf8)
#  set(CHARSET_sr@latin utf8)
#  set(CHARSET_tr iso-8859-9)
#  set(CHARSET_zh_CN gb2312)

  ADD_PO4A_POT_TARGET("${FILES}" ${DOMAIN} man "")
  ADD_PO_TARGETS(${DOMAIN} ${LINGUAS})

  set(LANG)
  ADD_PO4A_TRANSLATE_TARGET("${CMAKE_SOURCE_DIR}/doc/man/wesnoth.6" 
                            "${CMAKE_SOURCE_DIR}/doc/man/@LANG@/wesnoth.6"
                            "${CMAKE_BINARY_DIR}/doc/man/wesnoth.6.@LANG@.stamp" ${DOMAIN} man "CHARSET" "-k 80" ${LINGUAS})
  ADD_PO4A_TRANSLATE_TARGET("${CMAKE_SOURCE_DIR}/doc/man/wesnothd.6" 
                            "${CMAKE_SOURCE_DIR}/doc/man/@LANG@/wesnothd.6"
                            "${CMAKE_BINARY_DIR}/doc/man/wesnothd.6.@LANG@.stamp" ${DOMAIN} man "CHARSET" "-k 80" ${LINGUAS})
  add_custom_target(update-po4a-${DOMAIN})
  add_dependencies(update-po4a-${DOMAIN} update-po4a-wesnothd.6 update-po4a-wesnoth.6) 
  add_dependencies(update-po4a update-po4a-${DOMAIN})


endif(PO4A_GETTEXTTIZE_EXECUTABLE AND PO4A_TRANSLATE_EXECUTABLE)
