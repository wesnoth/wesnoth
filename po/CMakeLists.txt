############ Settings. ###########
set_directory_properties(PROPERTIES CLEAN_NO_CUSTOM true)

# The normal domains use cfg, cpp, lua, py files.
set(NORMAL_DOMAINS
	wesnoth
	wesnoth-anl
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
	wesnoth-thot
	wesnoth-tools
	wesnoth-trow
	wesnoth-tsg
	wesnoth-tutorial
	wesnoth-units
	wesnoth-utbs
	wesnoth-wc
	wesnoth-wof
)

# All available domains.
set(DOMAINS
	${NORMAL_DOMAINS}
	wesnoth-manpages
	wesnoth-manual
)

# If a source file has not GETTEXT_DOMAIN it belongs to this domain.
set(DEFAULT_DOMAIN wesnoth)

set(WMLXGETTEXT ${PROJECT_SOURCE_DIR}/data/tools/wmlxgettext)

############ pot update. ###########

if(ENABLE_POT_UPDATE_TARGET)

	# The variable pot-update-SRC accumulates a list of the dummy-targets
	# declared in the following loops. Depending on it will cause all of the
	# custom commands to run.

	foreach(DOMAIN ${NORMAL_DOMAINS})

		# Update the source file dependencies.
		include(update_pot_source_dependencies)

		# Generate pot file for C++ strings.
		add_custom_command(
			# misses bug address
			OUTPUT ${PROJECT_SOURCE_DIR}/po/${DOMAIN}/${DOMAIN}.cpp.pot
			# create the pot file, make sure we always get output and replace the charset
			COMMAND ${GETTEXT_XGETTEXT_EXECUTABLE} ${GETTEXT_XGETTEXT_OPTIONS}
					--files-from=${PROJECT_SOURCE_DIR}/po/${DOMAIN}/POTFILES_CPP.in
					--output=- | sed s/charset=CHARSET/charset=UTF-8/ > ${PROJECT_SOURCE_DIR}/po/${DOMAIN}/${DOMAIN}.cpp.pot
			DEPENDS ${PROJECT_SOURCE_DIR}/po/${DOMAIN}/POTFILES_CPP.in
			WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
			COMMENT "pot-update [${DOMAIN}]: Generating source pot file."
		)

		# Generate pot file for Python strings.
		add_custom_command(
			OUTPUT ${PROJECT_SOURCE_DIR}/po/${DOMAIN}/${DOMAIN}.py.pot
			# create the pot file, make sure we always get output and replace the charset
			COMMAND ${GETTEXT_XGETTEXT_EXECUTABLE} ${GETTEXT_XGETTEXT_OPTIONS}
					--language=Python
					--files-from=${PROJECT_SOURCE_DIR}/po/${DOMAIN}/POTFILES_PY.in
					--output=- | sed s/charset=CHARSET/charset=UTF-8/ > ${PROJECT_SOURCE_DIR}/po/${DOMAIN}/${DOMAIN}.py.pot
			DEPENDS ${PROJECT_SOURCE_DIR}/po/${DOMAIN}/POTFILES_PY.in
			WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
			COMMENT "pot-update [${DOMAIN}]: Generating python pot file."
		)
		# Generate pot file for WML/Lua strings.
		add_custom_command(
			OUTPUT ${PROJECT_SOURCE_DIR}/po/${DOMAIN}/${DOMAIN}.wml.pot

			COMMAND ${Python_EXECUTABLE} ${WMLXGETTEXT}
				--force-po
				--domain=${DOMAIN}
				`cd ${PROJECT_SOURCE_DIR} &&
					sh ${PROJECT_SOURCE_DIR}/po/${DOMAIN}/FINDCFG`
					-o ${PROJECT_SOURCE_DIR}/po/${DOMAIN}/${DOMAIN}.wml.pot
			DEPENDS ${PROJECT_SOURCE_DIR}/po/${DOMAIN}/FINDCFG
			WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
			COMMENT "pot-update [${DOMAIN}]: Generating wml pot file."
		)

		# Merge pot files
		add_custom_command(
			# The old function checked for differences in the time in the header see
			# what we need to do with it.
			OUTPUT ${PROJECT_SOURCE_DIR}/po/${DOMAIN}/${DOMAIN}.pot

			COMMAND ${GETTEXT_MSGCAT_EXECUTABLE}
				--sort-by-file
					 ${PROJECT_SOURCE_DIR}/po/${DOMAIN}/${DOMAIN}.cpp.pot
					 ${PROJECT_SOURCE_DIR}/po/${DOMAIN}/${DOMAIN}.py.pot
					 ${PROJECT_SOURCE_DIR}/po/${DOMAIN}/${DOMAIN}.wml.pot
				--output ${PROJECT_SOURCE_DIR}/po/${DOMAIN}/${DOMAIN}.pot

			COMMAND ${CMAKE_COMMAND} -E remove
					 ${PROJECT_SOURCE_DIR}/po/${DOMAIN}/${DOMAIN}.cpp.pot
					 ${PROJECT_SOURCE_DIR}/po/${DOMAIN}/${DOMAIN}.py.pot
					 ${PROJECT_SOURCE_DIR}/po/${DOMAIN}/${DOMAIN}.wml.pot

			WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
			DEPENDS
					${PROJECT_SOURCE_DIR}/po/${DOMAIN}/${DOMAIN}.cpp.pot
					${PROJECT_SOURCE_DIR}/po/${DOMAIN}/${DOMAIN}.py.pot
					${PROJECT_SOURCE_DIR}/po/${DOMAIN}/${DOMAIN}.wml.pot
			COMMENT "pot-update [${DOMAIN}]: Generating pot file."
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

	# Generate the stats based on completeness of all the .po files
	add_custom_command(
			# Same idea with a dummy file as in pot-update: we never make it, and run the command each time.
			OUTPUT ${PROJECT_SOURCE_DIR}/po/po_stat.dummy
			COMMAND python3 utils/po_stat.py
			--update-cfg
			--textdomains=wesnoth,wesnoth-editor,wesnoth-help,wesnoth-lib,wesnoth-multiplayer,wesnoth-tutorial,wesnoth-units,wesnoth-anl
			WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
			COMMENT "pot-update Updated data/languages/*.cfg files."
			# Needs to depend on the subset of .po files matching po_stat.py's --textdomains argument,
			# but for simplicity just depend on all the targets to update all .po files.
			DEPENDS ${pot-update-SRC}
	)

	SET(pot-update-SRC ${pot-update-SRC} ${PROJECT_SOURCE_DIR}/po/po_stat.dummy)

	# The target to call
	add_custom_target(pot-update
		COMMENT "pot-update: Done."
		DEPENDS ${pot-update-SRC}
	)


	##### po update per language #####
	# This is used to implement `make po-update-en_GB` etc. It started as a
	# copy of code from the main pot-update, no idea why commit
	# 93591845568158e00441094a2ede03e6bab8e285 changed only this copy.
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
# newer cmake minimum versions forbid the "@" symbol in certain targets
# this causes problems for translation targets such as "sr@latin"
		string(REPLACE "@" "_" target_name ${LINGUA})
		add_custom_target(po-update-${target_name} COMMENT "po-update ${LINGUA} as ${target_name}: Done." DEPENDS ${po-update-LINGUA-SRC})
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

			file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/${LOCALEDIR}/${LINGUA}/LC_MESSAGES)

			add_custom_command(
				OUTPUT ${CMAKE_SOURCE_DIR}/${LOCALEDIR}/${LINGUA}/LC_MESSAGES/${DOMAIN}.mo
				COMMAND ${GETTEXT_MSGFMT_EXECUTABLE}
						${GETTEXT_MSGFMT_PARAMETER}
						-o ${CMAKE_SOURCE_DIR}/${LOCALEDIR}/${LINGUA}/LC_MESSAGES/${DOMAIN}.mo ${LINGUA}.po
				DEPENDS
					${PROJECT_SOURCE_DIR}/po/${DOMAIN}/${LINGUA}.po
				WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/po/${DOMAIN}
				COMMENT "mo-update [${DOMAIN}-${LINGUA}]: Creating mo file."
			)

			list(FIND NORMAL_DOMAINS ${DOMAIN} IS_NORMAL_DOMAIN)
			if(IS_NORMAL_DOMAIN GREATER -1)
				install(FILES ${CMAKE_SOURCE_DIR}/${LOCALEDIR}/${LINGUA}/LC_MESSAGES/${DOMAIN}.mo
					DESTINATION ${LOCALE_INSTALL}/${LINGUA}/LC_MESSAGES)
			endif()

			set(mo-update-SRC ${mo-update-SRC}
				${CMAKE_SOURCE_DIR}/${LOCALEDIR}/${LINGUA}/LC_MESSAGES/${DOMAIN}.mo
			)
			set(mo-update-LINGUA-SRC ${mo-update-LINGUA-SRC}
				${CMAKE_SOURCE_DIR}/${LOCALEDIR}/${LINGUA}/LC_MESSAGES/${DOMAIN}.mo
			)

		endforeach(DOMAIN ${DOMAINS})

		if(ENABLE_POT_UPDATE_TARGET)
# newer cmake minimum versions forbid the "@" symbol in certain targets
# this causes problems for translation targets such as "sr@latin"
			string(REPLACE "@" "_" target_name ${LINGUA})
			add_custom_target(mo-update-${target_name} COMMENT "mo-update ${LINGUA} as ${target_name}: Done." DEPENDS ${mo-update-LINGUA-SRC})
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
