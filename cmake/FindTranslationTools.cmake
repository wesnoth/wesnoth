# - Find the tools needed for updating the potfiles and translations

set(TRANSLATION_TOOLS_FOUND true)

find_program(GETTEXT_MSGINIT_EXECUTABLE msginit)
if(NOT GETTEXT_MSGINIT_EXECUTABLE)
	message("msginit not found")
	set(TRANSLATION_TOOLS_FOUND false)
endif(NOT GETTEXT_MSGINIT_EXECUTABLE)

find_program(GETTEXT_XGETTEXT_EXECUTABLE xgettext)
if(NOT GETTEXT_XGETTEXT_EXECUTABLE )
	message("xgettext not found")
	set(TRANSLATION_TOOLS_FOUND false)
endif(NOT GETTEXT_XGETTEXT_EXECUTABLE )

find_program(GETTEXT_MSGCAT_EXECUTABLE msgcat)
if(NOT GETTEXT_MSGCAT_EXECUTABLE )
	message("msgcat not found")
	set(TRANSLATION_TOOLS_FOUND false)
endif(NOT GETTEXT_MSGCAT_EXECUTABLE )

find_program(GETTEXT_MSGATTRIB_EXECUTABLE msgattrib)
if(NOT GETTEXT_MSGATTRIB_EXECUTABLE)
	message("msgattrib not found")
	set(TRANSLATION_TOOLS_FOUND false)
endif(NOT GETTEXT_MSGATTRIB_EXECUTABLE)

if(NOT TRANSLATION_TOOLS_FOUND)
	if(TranslationTools_FIND_REQUIRED)
		message(FATAL_ERROR "Not all translation tools are found")
	endif(TranslationTools_FIND_REQUIRED)
endif(NOT TRANSLATION_TOOLS_FOUND)

