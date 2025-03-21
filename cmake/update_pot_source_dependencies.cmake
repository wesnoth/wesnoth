# Update the source file dependencies of the pot file.
#
# This globs all files cpp in the src directory and looks for the text domain
# definition in that file and outputs these dependencies in POTFILES_CPP.in.
# py, pyw are listed in POTFILES_PY.in

# Remove the old input file.
# Dummy target with a non existing (and not created file) is always executed.
add_custom_command(
	OUTPUT ${PROJECT_SOURCE_DIR}/po/${DOMAIN}/POTFILES.in.dummy
	# remove the old file.
	COMMAND ${CMAKE_COMMAND}
			-E remove ${PROJECT_SOURCE_DIR}/po/${DOMAIN}/POTFILES*.in 
	COMMENT "pot-update [${DOMAIN}]: Removing existing POTFILES*.in."
)

add_custom_command(
	OUTPUT ${PROJECT_SOURCE_DIR}/po/${DOMAIN}/POTFILES_CPP.in

	# Write list of matching files to POTFILES_CPP.in.
	COMMAND ${Python_EXECUTABLE} ${PROJECT_SOURCE_DIR}/po/FINDCPP ${DOMAIN} --initialdomain ${DEFAULT_DOMAIN} >|
				${PROJECT_SOURCE_DIR}/po/${DOMAIN}/POTFILES_CPP.in

	DEPENDS ${PROJECT_SOURCE_DIR}/po/${DOMAIN}/POTFILES.in.dummy
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
	COMMENT "pot-update [${DOMAIN}]: Creating POTFILES_CPP.in."
)
add_custom_command(
	OUTPUT ${PROJECT_SOURCE_DIR}/po/${DOMAIN}/POTFILES_PY.in

	# Write list of matching files to PY.in.
	COMMAND ${Python_EXECUTABLE} ${PROJECT_SOURCE_DIR}/po/FINDPY ${DOMAIN} >|
				${PROJECT_SOURCE_DIR}/po/${DOMAIN}/POTFILES_PY.in

	DEPENDS ${PROJECT_SOURCE_DIR}/po/${DOMAIN}/POTFILES.in.dummy
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
	COMMENT "pot-update [${DOMAIN}]: Creating POTFILES_PY.in."
)
