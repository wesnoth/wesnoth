set_directory_properties(PROPERTIES CLEAN_NO_CUSTOM true)

install(FILES manual.en.html DESTINATION "${CMAKE_INSTALL_DOCDIR}/html")
install(DIRECTORY images styles
	DESTINATION "${CMAKE_INSTALL_DOCDIR}/html"
	PATTERN ".git" EXCLUDE
)

if(ENABLE_NLS)
	foreach(LANG ${LINGUAS})
		install(FILES manual.${LANG}.html DESTINATION "${CMAKE_INSTALL_DOCDIR}/html" OPTIONAL)
	endforeach(LANG ${LINGUAS})
endif(ENABLE_NLS)


if(ENABLE_POT_UPDATE_TARGET)

	add_custom_command(
		OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/manual.en.xml
		COMMAND ${CMAKE_COMMAND} -E remove manual.en.xml
		COMMAND ${CMAKE_COMMAND} -E remove manual.en.html
		COMMAND ${ASCIIDOC_EXECUTABLE} ${ASCIIDOC_OPTIONS}
				-o manual.en.xml
				manual.txt
		COMMAND ${DOS2UNIX_EXECUTABLE} manual.en.xml
		COMMAND ${XSLTPROC_EXECUTABLE} ${XSLTPROC_OPTIONS}
				-o manual.en.html
				${ASCIIDOC_DOCBOOK_XSL_PATH}/xhtml.xsl
				manual.en.xml
		DEPENDS manual.txt
		COMMENT "[update-po4a-manual] Building manual.xml."
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)

	add_custom_command(
		OUTPUT ${PROJECT_SOURCE_DIR}/po/wesnoth-manual/wesnoth-manual.pot
		COMMAND ${PO4A-GETTEXTIZE_EXECUTABLE} ${PO4A-GETTEXTIZE_OPTIONS}
				-m ${CMAKE_CURRENT_SOURCE_DIR}/manual.en.xml
				-p wesnoth-manual.pot
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/manual.en.xml
		COMMENT "[update-po4a-manual] Building wesnoth-manual.pot."
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/po/wesnoth-manual
	)

	set(UPDATE-PO4A-MANUAL_DEPENDENCIES)
	foreach(LINGUA ${LINGUAS})

		add_custom_command(
			OUTPUT ${PROJECT_SOURCE_DIR}/po/wesnoth-manual/${LINGUA}.po
			COMMAND ${PO4A-UPDATEPO_EXECUTABLE} ${PO4A-UPDATEPO_OPTIONS}
					-f docbook
					-m ${CMAKE_CURRENT_SOURCE_DIR}/manual.en.xml
					-p ${LINGUA}.po
			DEPENDS ${PROJECT_SOURCE_DIR}/po/wesnoth-manual/wesnoth-manual.pot
			COMMENT "[update-po4a-manual ${LINGUA}] Building ${LINGUA}.po."
			WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/po/wesnoth-manual
		)

		# Create the .xml file if enough strings are translated.
		add_custom_command(
			OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/manual.${LINGUA}.xml
			COMMAND ${PO4A-TRANSLATE_EXECUTABLE} ${PO4A-TRANSLATE_OPTIONS}
					-m ${CMAKE_CURRENT_SOURCE_DIR}/manual.en.xml
					-p ${LINGUA}.po
					-l ${CMAKE_CURRENT_SOURCE_DIR}/manual.${LINGUA}.xml
			DEPENDS ${PROJECT_SOURCE_DIR}/po/wesnoth-manual/${LINGUA}.po
			COMMENT "[update-po4a-manual ${LINGUA}] Building ${LINGUA}.xml."
			WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/po/wesnoth-manual
		)


		# Only create a .html if there is an .xml file. Since it's not
		# possible to only execute cmake code if the file exists directly,
		# build the command here. Then send it to a small script that only
		# executes if the .html file exists.
		set(CMD "${XSLTPROC_EXECUTABLE} ${XSLTPROC_OPTIONS}")
		set(CMD "${CMD} -o manual.${LINGUA}.html")
		set(CMD "${CMD} ${ASCIIDOC_DOCBOOK_XSL_PATH}/xhtml.xsl")
		set(CMD "${CMD} manual.${LINGUA}.xml")
		add_custom_command(
			OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/manual.${LINGUA}.html
			COMMAND ${CMAKE_COMMAND} -E remove manual.${LINGUA}.html
			COMMAND ${CMAKE_COMMAND}
				-DSOURCE="manual.${LINGUA}.xml"
				-DCMD="${CMD}"
				-P "${CMAKE_SOURCE_DIR}/cmake/po4a-manual.cmake"
			COMMAND ${CMAKE_COMMAND} -E remove manual.${LINGUA}.xml
			DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/manual.${LINGUA}.xml
			COMMENT "[update-po4a-manual ${LINGUA}] Building ${LINGUA}.html."
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		)

		set(UPDATE-PO4A-MANUAL_DEPENDENCIES
			${UPDATE-PO4A-MANUAL_DEPENDENCIES}
			${CMAKE_CURRENT_SOURCE_DIR}/manual.${LINGUA}.html
		)
	endforeach(LINGUA ${LINGUAS})

	add_custom_target(update-po4a-manual
		COMMAND ${CMAKE_COMMAND} -E remove manual.en.xml
		DEPENDS
			${UPDATE-PO4A-MANUAL_DEPENDENCIES}
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)

endif(ENABLE_POT_UPDATE_TARGET)
