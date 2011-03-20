############ Old stuff. ###########

INCLUDE(TranslationTargets)
add_custom_target(update-po4a)

############ Settings. ###########

# Available languages.
#first read in the space separated list
file(STRINGS LINGUAS LANGUAGES)
#convert all spaces into ; to have a nice "set"
separate_arguments(LANGUAGES)

# The normal domains use cpp and cfg files.
set(NORMAL_DOMAINS
    wesnoth
    wesnoth-anl
    wesnoth-aoi
    wesnoth-did
    wesnoth-dm
    wesnoth-editor
    wesnoth-ei
    wesnoth-httt
    wesnoth-l
    wesnoth-lib
    wesnoth-low
    wesnoth-multiplayer
    wesnoth-nr
    wesnoth-sof
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

############ Validation. ###########

find_program(GETTEXT_MSGINIT_EXECUTABLE msginit)
if(NOT GETTEXT_MSGINIT_EXECUTABLE)
      message(FATAL_ERROR "msginit not found")
endif(NOT GETTEXT_MSGINIT_EXECUTABLE)

find_program(GETTEXT_XGETTEXT_EXECUTABLE xgettext)
if(NOT GETTEXT_XGETTEXT_EXECUTABLE )
      message(FATAL_ERROR "xgettext not found")
endif(NOT GETTEXT_XGETTEXT_EXECUTABLE )

find_program(GETTEXT_MSGCAT_EXECUTABLE msgcat)
if(NOT GETTEXT_MSGCAT_EXECUTABLE )
      message(FATAL_ERROR "msgcat not found")
endif(NOT GETTEXT_MSGCAT_EXECUTABLE )

find_program(GETTEXT_MSGATTRIB_EXECUTABLE msgattrib)
if(NOT GETTEXT_MSGATTRIB_EXECUTABLE)
      message(FATAL_ERROR "msgattrib not found")
endif(NOT GETTEXT_MSGATTRIB_EXECUTABLE)

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
			COMMAND ${GETTEXT_XGETTEXT_EXECUTABLE}
				--force-po
				--add-comments=TRANSLATORS 
				--files-from=${PROJECT_SOURCE_DIR}/po/${DOMAIN}/POTFILES.in 
				--copyright-holder=\"Wesnoth development team\"
				--msgid-bugs-address=\"http://bugs.wesnoth.org/\"
				--from-code=UTF-8
				--sort-by-file
				--keyword=_
				--keyword=N_
				--keyword=sgettext
				--keyword=vgettext
				--keyword=VGETTEXT
				--keyword=_n:1,2
				--keyword=sngettext:1,2
				--keyword=vngettext:1,2
				--output=${PROJECT_SOURCE_DIR}/po/${DOMAIN}/${DOMAIN}.cpp.pot
			# replace the chartype
			COMMAND sed -i 
				s/charset=CHARSET/charset=UTF-8/ 
				${PROJECT_SOURCE_DIR}/po/${DOMAIN}/${DOMAIN}.cpp.pot
			# Remove some header info - Need to test whether needed.
#			COMMAND sed -i
#				-f po/remove-potcdate.sed 
#				${PROJECT_SOURCE_DIR}/po/${DOMAIN}/${DOMAIN}.cpp.pot
			DEPENDS ${PROJECT_SOURCE_DIR}/po/${DOMAIN}/POTFILES.in
			WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
			COMMENT "pot-update [${DOMAIN}]: Generated source pot file."
		)

		# Generate pot file for wml data.
#		if(EXISTS ${PROJECT_SOURCE_DIR}/po/${DOMAIN}/FINDCFG)
			add_custom_command(
				OUTPUT ${PROJECT_SOURCE_DIR}/po/${DOMAIN}/${DOMAIN}.wml.pot

				COMMAND ${WMLXGETTEXT} 
					--directory=${PROJECT_SOURCE_DIR} 
					--domain=${DOMAIN} 
					`cd ${PROJECT_SOURCE_DIR} && 
						sh ${PROJECT_SOURCE_DIR}/po/${DOMAIN}/FINDCFG`
						> ${PROJECT_SOURCE_DIR}/po/${DOMAIN}/${DOMAIN}.wml.pot
				DEPENDS ${PROJECT_SOURCE_DIR}/po/${DOMAIN}/FINDCFG
				WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
				COMMENT "pot-update [${DOMAIN}]: Generated wml pot file."
			)
#		else(EXISTS ${PROJECT_SOURCE_DIR}/po/${DOMAIN}/FINDCFG)
#			add_custom_command(
#				OUTPUT ${PROJECT_SOURCE_DIR}/po/${DOMAIN}/${DOMAIN}.wml.pot
#
#				COMMAND touch 
#						${PROJECT_SOURCE_DIR}/po/${DOMAIN}/${DOMAIN}.wml.pot
#				WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
#				COMMENT "pot-update [${DOMAIN}]: Generated dummy wml pot file."
#			)
#		endif(EXISTS ${PROJECT_SOURCE_DIR}/po/${DOMAIN}/FINDCFG)

		# Merge both pot files
		add_custom_command(
			# The old function checked for differences in the time in the header see
			# what we need to do with it.
			OUTPUT ${PROJECT_SOURCE_DIR}/po/${DOMAIN}/${DOMAIN}.pot

			COMMAND ${GETTEXT_MSGCAT_EXECUTABLE} 
				--sort-by-file 
					 ${PROJECT_SOURCE_DIR}/po/${DOMAIN}/${DOMAIN}.cpp.pot
					 ${PROJECT_SOURCE_DIR}/po/${DOMAIN}/${DOMAIN}.wml.pot
				--output  ${PROJECT_SOURCE_DIR}/po/${DOMAIN}/${DOMAIN}.pot

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
		foreach(LANG ${LANGUAGES})

			### Generate new po file.

			# For some reason CMake is rather happy to delete the po file in
			# some cases. Too avoid that problem only generate the init rule
			# if the po file doesn't exist. The case where a po file used to
			# exist and no longer exists should never occur

			if(NOT EXISTS ${PROJECT_SOURCE_DIR}/po/${DOMAIN}/${LANG}.po)
				add_custom_command(
					OUTPUT ${PROJECT_SOURCE_DIR}/po/${DOMAIN}/${LANG}.po
					COMMAND ${GETTEXT_MSGINIT_EXECUTABLE}
							--no-translator 
							--input=${DOMAIN}.pot 
							--output-file=${LANG}.po
							--locale=${LANG}
					WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/po/${DOMAIN}
					DEPENDS
						${PROJECT_SOURCE_DIR}/po/${DOMAIN}/${DOMAIN}.pot
					COMMENT 
						"pot-update [${DOMAIN}-${LANG}]: Initialized po file."
				)
			endif(NOT EXISTS ${PROJECT_SOURCE_DIR}/po/${DOMAIN}/${LANG}.po)

			### Update existing po file.
			add_custom_command(
				# The pot-update depends on a dummy file which we promise to
				# make. We don't make this dummy file but the pot-update
				# still works.
				OUTPUT ${PROJECT_SOURCE_DIR}/po/${DOMAIN}/${LANG}.po.dummy
				COMMAND ${GETTEXT_MSGMERGE_EXECUTABLE} 
						--backup=none
						-U ${LANG}.po 
						${DOMAIN}.pot
				DEPENDS
					${PROJECT_SOURCE_DIR}/po/${DOMAIN}/${DOMAIN}.pot 
					${PROJECT_SOURCE_DIR}/po/${DOMAIN}/${LANG}.po
				WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/po/${DOMAIN}
				COMMENT "pot-update [${DOMAIN}-${LANG}]: Updated po file."
			)

			SET(pot-update-SRC ${pot-update-SRC} ${DOMAIN}/${LANG}.po.dummy)

		endforeach(LANG ${LANGUAGES})

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
	foreach(LANG ${LANGUAGES})
		foreach(DOMAIN ${DOMAINS})
			add_custom_command(
				# The pot-update depends on a dummy file which we promise to
				# make. We don't make this dummy file but the pot-update
				# still works.
				OUTPUT ${PROJECT_SOURCE_DIR}/po/${DOMAIN}/${LANG}.po.lang.dummy
				# After the po file is updated it might look not entirely as
				# wanted, for example poedit reorders the file. Use msgattrib
				# to reformat the file, use a helper file to do so.
				COMMAND ${GETTEXT_MSGATTRIB_EXECUTABLE}
						--output ${LANG}.po.tmp
						${LANG}.po
				COMMAND ${CMAKE_COMMAND} -E copy
						${LANG}.po.tmp
						${LANG}.po
				COMMAND ${CMAKE_COMMAND} -E remove
						${LANG}.po.tmp
				# Now merge with the pot file.
				COMMAND ${GETTEXT_MSGMERGE_EXECUTABLE}
						--backup=none
						-U ${LANG}.po
						${DOMAIN}.pot
				WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/po/${DOMAIN}
				COMMENT "po-update [${DOMAIN}-${LANG}]: Updated po file."
			)

			set(po-update-LANG-SRC ${po-update-LANG-SRC} 
				${PROJECT_SOURCE_DIR}/po/${DOMAIN}/${LANG}.po.lang.dummy
			)

		endforeach(DOMAIN ${DOMAINS})
		add_custom_target(po-update-${LANG}
			COMMENT "po-update ${LANG}: Done."
			DEPENDS ${po-update-LANG-SRC}
		)
		set(po-update-LANG-SRC "")
	endforeach(LANG ${LANGUAGES})

endif(ENABLE_POT_UPDATE_TARGET)

if(ENABLE_NLS)

	set(GETTEXT_MSGFMT_PARAMETER --check-format --check-domain )

	if(ENABLE_POT_UPDATE_TARGET)
		# Only set the verbose flag for maintainers.
		set(GETTEXT_MSGFMT_PARAMETER -v ${GETTEXT_MSGFMT_PARAMETER} )
	endif(ENABLE_POT_UPDATE_TARGET)

	foreach(LANG ${LANGUAGES})

		foreach(DOMAIN ${DOMAINS})

			add_custom_command(
				OUTPUT ${CMAKE_SOURCE_DIR}/${LOCALEDIR}/${LANG}/LC_MESSAGES
				COMMAND ${CMAKE_COMMAND} -E make_directory 
						${CMAKE_SOURCE_DIR}/${LOCALEDIR}/${LANG}/LC_MESSAGES
				COMMENT "mo-update [${LANG}]: Creating locale directory."
			)

			add_custom_command(
				OUTPUT ${CMAKE_SOURCE_DIR}/${LOCALEDIR}/${LANG}/LC_MESSAGES/${DOMAIN}.mo
				COMMAND ${GETTEXT_MSGFMT_EXECUTABLE}
						${GETTEXT_MSGFMT_PARAMETER}
						-o ${CMAKE_SOURCE_DIR}/${LOCALEDIR}/${LANG}/LC_MESSAGES/${DOMAIN}.mo ${LANG}.po
				DEPENDS 
					${CMAKE_SOURCE_DIR}/${LOCALEDIR}/${LANG}/LC_MESSAGES
					${PROJECT_SOURCE_DIR}/po/${DOMAIN}/${LANG}.po
				WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/po/${DOMAIN}
				COMMENT "mo-update [${DOMAIN}-${LANG}]: Creating mo file."
			)

			install(FILES ${CMAKE_SOURCE_DIR}/${LOCALEDIR}/${LANG}/LC_MESSAGES/${DOMAIN}.mo
					DESTINATION ${LOCALE_INSTALL}/${LANG}/LC_MESSAGES)

			set(mo-update-SRC ${mo-update-SRC} 
				${CMAKE_SOURCE_DIR}/${LOCALEDIR}/${LANG}/LC_MESSAGES/${DOMAIN}.mo
			)
			set(mo-update-LANG-SRC ${mo-update-LANG-SRC} 
				${CMAKE_SOURCE_DIR}/${LOCALEDIR}/${LANG}/LC_MESSAGES/${DOMAIN}.mo
			)

		endforeach(DOMAIN ${DOMAINS})

		if(ENABLE_POT_UPDATE_TARGET)
			add_custom_target(mo-update-${LANG}
				COMMENT "mo-update ${LANG}: Done."
				DEPENDS ${mo-update-LANG-SRC}
			)
			set(mo-update-LANG-SRC "")
		endif(ENABLE_POT_UPDATE_TARGET)

	endforeach(LANG ${LANGUAGES})

	# The target to call
	add_custom_target(mo-update ALL
		COMMENT "mo-update: Done."
		DEPENDS ${mo-update-SRC}
	)

	set_directory_properties(properties 
		ADDITIONAL_MAKE_CLEAN_FILES ${CMAKE_SOURCE_DIR}/${LOCALEDIR}
	)

endif(ENABLE_NLS)

