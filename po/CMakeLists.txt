############ Settings. ###########
set_directory_properties(PROPERTIES CLEAN_NO_CUSTOM true)

# The normal domains use cpp and cfg files.
set(NORMAL_DOMAINS
	wesnoth
	wesnoth-ai
	wesnoth-anl
	wesnoth-aoi
	wesnoth-did
	wesnoth-dm
	wesnoth-dw
	wesnoth-editor
	wesnoth-ei
	wesnoth-help
	wesnoth-httt
	wesnoth-l
	wesnoth-lib
	wesnoth-low
	wesnoth-multiplayer
	wesnoth-nr
	wesnoth-sof
	wesnoth-sota
	wesnoth-sotbe
	wesnoth-tb
	wesnoth-test
	wesnoth-thot
	wesnoth-trow
	wesnoth-tsg
	wesnoth-tutorial
	wesnoth-units
	wesnoth-utbs
)

# All available domains.
set(DOMAINS
	${NORMAL_DOMAINS}
	wesnoth-manpages
	wesnoth-manual
)

# If a source file has not GETTEXT_DOMAIN it belongs to this domain.
set(DEFAULT_DOMAIN wesnoth)

set(WMLXGETTEXT ${PROJECT_SOURCE_DIR}/utils/wmlxgettext)

############ pot update. ###########

if(ENABLE_POT_UPDATE_TARGET)

	foreach(DOMAIN ${NORMAL_DOMAINS})

		# Update the source file dependencies.
		include(update_pot_source_dependencies)

		# Generate pot file for c++ data.
		add_custom_command(
			# misses bug address
			OUTPUT ${PROJECT_SOURCE_DIR}/po/${DOMAIN}/${DOMAIN}.cpp.pot
			# create the pot file make sure we always get output.
			COMMAND ${GETTEXT_XGETTEXT_EXECUTABLE} ${GETTEXT_XGETTEXT_OPTIONS}
					--files-from=${PROJECT_SOURCE_DIR}/po/${DOMAIN}/POTFILES.in
					--output=${PROJECT_SOURCE_DIR}/po/${DOMAIN}/${DOMAIN}.cpp.pot
			# replace the chartype
			COMMAND sed -i
				s/charset=CHARSET/charset=UTF-8/
				${PROJECT_SOURCE_DIR}/po/${DOMAIN}/${DOMAIN}.cpp.pot
			# Remove some header info - Need to test whether needed.
			DEPENDS ${PROJECT_SOURCE_DIR}/po/${DOMAIN}/POTFILES.in
			WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
			COMMENT "pot-update [${DOMAIN}]: Generated source pot file."
		)

		# Generate pot file for wml data.
		add_custom_command(
			OUTPUT ${PROJECT_SOURCE_DIR}/po/${DOMAIN}/${DOMAIN}.wml.pot

			COMMAND ${WMLXGETTEXT}
				--directory=${PROJECT_SOURCE_DIR}
				--domain=${DOMAIN}
				`cd ${PROJECT_SOURCE_DIR} &&
					sh ${PROJECT_SOURCE_DIR}/po/${DOMAIN}/FINDCFG`
					-o ${PROJECT_SOURCE_DIR}/po/${DOMAIN}/${DOMAIN}.wml.pot
			DEPENDS ${PROJECT_SOURCE_DIR}/po/${DOMAIN}/FINDCFG
			WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
			COMMENT "pot-update [${DOMAIN}]: Generated wml pot file."
		)

		# Merge both pot files
		add_custom_command(
			# The old function checked for differences in the time in the header see
			# what we need to do with it.
			OUTPUT ${PROJECT_SOURCE_DIR}/po/${DOMAIN}/${DOMAIN}.pot

			COMMAND ${GETTEXT_MSGCAT_EXECUTABLE}
				--sort-by-file
					 ${PROJECT_SOURCE_DIR}/po/${DOMAIN}/${DOMAIN}.cpp.pot
					 ${PROJECT_SOURCE_DIR}/po/${DOMAIN}/${DOMAIN}.wml.pot
				--output ${PROJECT_SOURCE_DIR}/po/${DOMAIN}/${DOMAIN}.pot

			COMMAND ${CMAKE_COMMAND} -E remove
					 ${PROJECT_SOURCE_DIR}/po/${DOMAIN}/${DOMAIN}.cpp.pot
					 ${PROJECT_SOURCE_DIR}/po/${DOMAIN}/${DOMAIN}.wml.pot

			WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
			DEPENDS
					${PROJECT_SOURCE_DIR}/po/${DOMAIN}/${DOMAIN}.cpp.pot
					${PROJECT_SOURCE_DIR}/po/${DOMAIN}/${DOMAIN}.wml.pot
			COMMENT "pot-update [${DOMAIN}]: Generated pot file."
		)

		# Update / generate the po files for all languages
		foreach(LINGUA ${LINGUAS})

			### Generate new po file.

			# For some reason CMake is rather happy to delete the po file in
			# some cases. Too avoid that problem only generate the init rule
			# if the po file doesn't exist. The case where a po file used to
			# exist and no longer exists should never occur

			if(NOT EXISTS ${PROJECT_SOURCE_DIR}/po/${DOMAIN}/${LINGUA}.po)
				add_custom_command(
					OUTPUT ${PROJECT_SOURCE_DIR}/po/${DOMAIN}/${LINGUA}.po
					COMMAND ${GETTEXT_MSGINIT_EXECUTABLE}
							--no-translator
							--input=${DOMAIN}.pot
							--output-file=${LINGUA}.po
							--locale=${LINGUA}
					WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/po/${DOMAIN}
					DEPENDS
						${PROJECT_SOURCE_DIR}/po/${DOMAIN}/${DOMAIN}.pot
					COMMENT
						"pot-update [${DOMAIN}-${LINGUA}]: Initialized po file."
				)
			endif(NOT EXISTS ${PROJECT_SOURCE_DIR}/po/${DOMAIN}/${LINGUA}.po)

			### Update existing po file.
			add_custom_command(
				# The pot-update depends on a dummy file which we promise to
				# make. We don't make this dummy file but the pot-update
				# still works.
				OUTPUT ${PROJECT_SOURCE_DIR}/po/${DOMAIN}/${LINGUA}.po.dummy
				COMMAND ${GETTEXT_MSGMERGE_EXECUTABLE}
						--backup=none
						--previous
						-U ${LINGUA}.po
						${DOMAIN}.pot
				DEPENDS
					${PROJECT_SOURCE_DIR}/po/${DOMAIN}/${DOMAIN}.pot
					${PROJECT_SOURCE_DIR}/po/${DOMAIN}/${LINGUA}.po
				WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/po/${DOMAIN}
				COMMENT "pot-update [${DOMAIN}-${LINGUA}]: Updated po file."
			)

			SET(pot-update-SRC ${pot-update-SRC} ${DOMAIN}/${LINGUA}.po.dummy)

		endforeach(LINGUA ${LINGUAS})

		# Add to target list
		SET(pot-update-SRC
			${pot-update-SRC}
			${PROJECT_SOURCE_DIR}/po/${DOMAIN}/${DOMAIN}.pot # should depend on languages
		)

	endforeach(DOMAIN ${NORMAL_DOMAINS})

	# The target to call
	add_custom_target(pot-update
		COMMENT "pot-update: Done."
		DEPENDS ${pot-update-SRC}
	)


	##### po update per language #####
	foreach(LINGUA ${LINGUAS})
		foreach(DOMAIN ${DOMAINS})
			add_custom_command(
				# The pot-update depends on a dummy file which we promise to
				# make. We don't make this dummy file but the pot-update
				# still works.
				OUTPUT ${PROJECT_SOURCE_DIR}/po/${DOMAIN}/${LINGUA}.po.lang.dummy
				# After the po file is updated it might look not entirely as
				# wanted, for example poedit reorders the file. Use msgattrib
				# to reformat the file, use a helper file to do so.
				COMMAND ${GETTEXT_MSGATTRIB_EXECUTABLE}
						--output ${LINGUA}.po.tmp
						${LINGUA}.po
				COMMAND ${CMAKE_COMMAND} -E copy
						${LINGUA}.po.tmp
						${LINGUA}.po
				COMMAND ${CMAKE_COMMAND} -E remove
						${LINGUA}.po.tmp
				# Now merge with the pot file.
				COMMAND ${GETTEXT_MSGMERGE_EXECUTABLE}
						--backup=none
						--previous
						-U ${LINGUA}.po
						${DOMAIN}.pot
				WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/po/${DOMAIN}
				COMMENT "po-update [${DOMAIN}-${LINGUA}]: Updated po file."
			)

			set(po-update-LINGUA-SRC ${po-update-LINGUA-SRC}
				${PROJECT_SOURCE_DIR}/po/${DOMAIN}/${LINGUA}.po.lang.dummy
			)

		endforeach(DOMAIN ${DOMAINS})
		add_custom_target(po-update-${LINGUA}
			COMMENT "po-update ${LINGUA}: Done."
			DEPENDS ${po-update-LINGUA-SRC}
		)
		set(po-update-LINGUA-SRC "")
	endforeach(LINGUA ${LINGUAS})

endif(ENABLE_POT_UPDATE_TARGET)

if(ENABLE_NLS)

	set(GETTEXT_MSGFMT_PARAMETER --check-domain )

	if(ENABLE_POT_UPDATE_TARGET)
		# Only set the verbose flag for maintainers.
		set(GETTEXT_MSGFMT_PARAMETER -v ${GETTEXT_MSGFMT_PARAMETER} )
	endif(ENABLE_POT_UPDATE_TARGET)

	foreach(LINGUA ${LINGUAS})

		foreach(DOMAIN ${DOMAINS})

			add_custom_command(
				OUTPUT ${CMAKE_SOURCE_DIR}/${LOCALEDIR}/${LINGUA}/LC_MESSAGES
				COMMAND ${CMAKE_COMMAND} -E make_directory
						${CMAKE_SOURCE_DIR}/${LOCALEDIR}/${LINGUA}/LC_MESSAGES
				COMMENT "mo-update [${LINGUA}]: Creating locale directory."
			)

			add_custom_command(
				OUTPUT ${CMAKE_SOURCE_DIR}/${LOCALEDIR}/${LINGUA}/LC_MESSAGES/${DOMAIN}.mo
				COMMAND ${GETTEXT_MSGFMT_EXECUTABLE}
						${GETTEXT_MSGFMT_PARAMETER}
						-o ${CMAKE_SOURCE_DIR}/${LOCALEDIR}/${LINGUA}/LC_MESSAGES/${DOMAIN}.mo ${LINGUA}.po
				DEPENDS
					${CMAKE_SOURCE_DIR}/${LOCALEDIR}/${LINGUA}/LC_MESSAGES
					${PROJECT_SOURCE_DIR}/po/${DOMAIN}/${LINGUA}.po
				WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/po/${DOMAIN}
				COMMENT "mo-update [${DOMAIN}-${LINGUA}]: Creating mo file."
			)

			install(FILES ${CMAKE_SOURCE_DIR}/${LOCALEDIR}/${LINGUA}/LC_MESSAGES/${DOMAIN}.mo
					DESTINATION ${LOCALE_INSTALL}/${LINGUA}/LC_MESSAGES)

			set(mo-update-SRC ${mo-update-SRC}
				${CMAKE_SOURCE_DIR}/${LOCALEDIR}/${LINGUA}/LC_MESSAGES/${DOMAIN}.mo
			)
			set(mo-update-LINGUA-SRC ${mo-update-LINGUA-SRC}
				${CMAKE_SOURCE_DIR}/${LOCALEDIR}/${LINGUA}/LC_MESSAGES/${DOMAIN}.mo
			)

		endforeach(DOMAIN ${DOMAINS})

		if(ENABLE_POT_UPDATE_TARGET)
			add_custom_target(mo-update-${LINGUA}
				COMMENT "mo-update ${LINGUA}: Done."
				DEPENDS ${mo-update-LINGUA-SRC}
			)
			set(mo-update-LINGUA-SRC "")
		endif(ENABLE_POT_UPDATE_TARGET)

	endforeach(LINGUA ${LINGUAS})

	# The target to call
	add_custom_target(mo-update ALL
		COMMENT "mo-update: Done."
		DEPENDS ${mo-update-SRC}
	)

	set_directory_properties(properties
		ADDITIONAL_MAKE_CLEAN_FILES ${CMAKE_SOURCE_DIR}/${LOCALEDIR}
	)

endif(ENABLE_NLS)
