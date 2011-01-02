file(STRINGS ${PROJECT_SOURCE_DIR}/po/LINGUAS LANGUAGES)
separate_arguments(LANGUAGES)

find_program(ASCIIDOC_EXECUTABLE "asciidoc")
find_program(DOS2UNIX_EXECUTABLE "dos2unix")
find_program(PO4A-TRANSLATE_EXECUTABLE "po4a-translate")
find_program(PO4A-UPDATEPO_EXECUTABLE "po4a-updatepo")
find_program(PO4A-GETTEXTIZE_EXECUTABLE "po4a-gettextize")
find_program(PO4A "po4a")
find_program(XSLTPROC_EXECUTABLE "xsltproc")

set(XSLTOPTS 
	--nonet
	--stringparam callout.graphics 0
	--stringparam navig.graphics 0
	--stringparam admon.textlabel 1
	--stringparam admon.graphics 0
	--stringparam html.stylesheet "./styles/manual.css"
)

set(PO4A-TRANSLATE_EXECUTABLE_options
	-f docbook
	-k 80
	-M utf-8
	-L utf-8)

# Remove old files
execute_process(COMMAND ${CMAKE_COMMAND} -E remove manual.en.xml)
execute_process(COMMAND ${CMAKE_COMMAND} -E remove manual.en.html)

foreach(LANG ${LANGUAGES})
	execute_process(COMMAND ${CMAKE_COMMAND} -E remove manual.${LANG}.html) 
	execute_process(COMMAND ${CMAKE_COMMAND} -E remove manual.${LANG}.xml)
endforeach(LANG ${LANGUAGES})

# Recreate master files
execute_process(COMMAND ${ASCIIDOC_EXECUTABLE}
		-b docbook
		-d book
		-n
		-a toc
		-o manual.en.xml
		manual.txt)
execute_process(COMMAND ${DOS2UNIX_EXECUTABLE} manual.en.xml)

execute_process(COMMAND ${XSLTPROC_EXECUTABLE}
		${XSLTOPTS}
		/etc/asciidoc/docbook-xsl/xhtml.xsl 
		manual.en.xml
		OUTPUT_FILE manual.en.html)

# Pot update
message("[update-po4a-manual] Generate pot file.")
execute_process(COMMAND ${PO4A-GETTEXTIZE_EXECUTABLE}
	--copyright-holder "Wesnoth Development Team"
	-f docbook
	-M utf-8
	-L utf-8
	-m ../../doc/manual/manual.en.xml
	-p wesnoth-manual.pot
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/po/wesnoth-manual)

# Recreate translations
foreach(LANG ${LANGUAGES})

	message("[update-po4a-manual ${LANG}] Update po file.")
	execute_process(COMMAND ${PO4A-UPDATEPO_EXECUTABLE}
			${PO4A-UPDATEPO_EXECUTABLE_options}
			-f docbook
			-M utf-8
			-m ../../doc/manual/manual.en.xml
			-p ${LANG}.po
			WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/po/wesnoth-manual)
	
	message("[update-po4a-manual ${LANG}] create xml file.")
	execute_process(COMMAND ${PO4A-TRANSLATE_EXECUTABLE}
			${PO4A-TRANSLATE_EXECUTABLE_options}
			-m ../../doc/manual/manual.en.xml
			-p ${LANG}.po
			-l ../../doc/manual/manual.${LANG}.xml
			WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/po/wesnoth-manual)

	if(EXISTS manual.${LANG}.xml)
		message("[update-po4a-manual ${LANG}] create manual file.")
		execute_process(COMMAND ${XSLTPROC_EXECUTABLE}
				${XSLTOPTS}
				/etc/asciidoc/docbook-xsl/xhtml.xsl 
				manual.${LANG}.xml
				OUTPUT_FILE manual.${LANG}.html)
	endif(EXISTS manual.${LANG}.xml)

endforeach(LANG ${LANGUAGES})

