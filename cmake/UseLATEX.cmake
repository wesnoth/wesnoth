# File: UseLATEX.cmake
# CMAKE commands to actually use the LaTeX compiler
# Version: 1.7.3
# Author: Kenneth Moreland (kmorel at sandia dot gov)
#
# Copyright 2004 Sandia Corporation.
# Under the terms of Contract DE-AC04-94AL85000, there is a non-exclusive
# license for use of this work by or on behalf of the
# U.S. Government. Redistribution and use in source and binary forms, with
# or without modification, are permitted provided that this Notice and any
# statement of authorship are reproduced on all copies.
#
# The following MACROS are defined:
#
# ADD_LATEX_DOCUMENT(<tex_file>
#                       [BIBFILES <bib_files>]
#                       [INPUTS <input_tex_files>]
#                       [IMAGE_DIRS] <image_directories>
#                       [IMAGES] <image_files>
#                       [CONFIGURE] <tex_files>
#                       [DEPENDS] <tex_files>
#                       [USE_INDEX] [USE_GLOSSARY]
#                       [DEFAULT_PDF] [MANGLE_TARGET_NAMES])
#       Adds targets that compile <tex_file>.  The latex output is placed
#       in LATEX_OUTPUT_PATH or CMAKE_CURRENT_BINARY_DIR if the former is
#       not set.  The latex program is picky about where files are located,
#       so all input files are copied from the source directory to the
#       output directory.  This includes the target tex file, any tex file
#       listed with the INPUTS option, the bibliography files listed with
#       the BIBFILES option, and any .cls, .bst, and .clo files found in
#       the current source directory.  Images found in the IMAGE_DIRS
#       directories or listed by IMAGES are also copied to the output
#       directory and coverted to an appropriate format if necessary.  Any
#       tex files also listed with the CONFIGURE option are also processed
#       with the CMake CONFIGURE_FILE command (with the @ONLY flag.  Any
#       file listed in CONFIGURE but not the target tex file or listed with
#       INPUTS has no effect. DEPENDS can be used to specify generated files
#       that are needed to compile the latex target.
#
#       The following targets are made:
#               dvi: Makes <name>.dvi
#               pdf: Makes <name>.pdf using pdflatex.
#               safepdf: Makes <name>.pdf using ps2pdf.  If using the default
#                       program arguments, this will ensure all fonts are
#                       embedded and no lossy compression has been performed
#                       on images.
#               ps: Makes <name>.ps
#               html: Makes <name>.html
#               auxclean: Deletes <name>.aux.  This is sometimes necessary
#                       if a LaTeX error occurs and writes a bad aux file.
#
#       The dvi target is added to the ALL.  That is, it will be the target
#       built by default.  If the DEFAULT_PDF argument is given, then the
#       pdf target will be the default instead of dvi.
#
#       If the argument MANGLE_TARGET_NAMES is given, then each of the
#       target names above will be mangled with the <tex_file> name.  This
#       is to make the targets unique if ADD_LATEX_DOCUMENT is called for
#       multiple documents.  If the argument USE_INDEX is given, then
#       commands to build an index are made.  If the argument USE_GLOSSARY
#       is given, then commands to build a glossary are made.
#
# History:
#
# 1.7.3 Fix some issues with interactions between makeglossaries and bibtex
#       (thanks to Mark de Wever).
#
# 1.7.2 Use ps2pdf to convert eps to pdf to get around the problem with
#       ImageMagick dropping the bounding box (thanks to Lukasz Lis).
#
# 1.7.1 Fixed some dependency issues.
#
# 1.7.0 Added DEPENDS options (thanks to Theodore Papadopoulo).
#
# 1.6.1 Ported the makeglossaries command to CMake and embedded the port
#       into UseLATEX.cmake.
#
# 1.6.0 Allow the use of the makeglossaries command.  Thanks to Oystein
#       S. Haaland for the patch.
#
# 1.5.0 Allow any type of file in the INPUTS lists, not just tex file
#       (suggested by Eric Noulard).  As a consequence, the ability to
#       specify tex files without the .tex extension is removed.  The removed
#       function is of dubious value anyway.
#
#       When copying input files, skip over any file that exists in the
#       binary directory but does not exist in the source directory with the
#       assumption that these files were added by some other mechanism.  I
#       find this useful when creating large documents with multiple
#       chapters that I want to build separately (for speed) as I work on
#       them.  I use the same boilerplate as the starting point for all
#       and just copy it with different configurations.  This was what the
#       separate ADD_LATEX_DOCUMENT method was supposed to originally be for.
#       Since its external use is pretty much deprecated, I removed that
#       documentation.
#
# 1.4.1 Copy .sty files along with the other class and package files.
#
# 1.4.0 Added a MANGLE_TARGET_NAMES option that will mangle the target names.
#
#       Fixed problem with copying bib files that became apparent with
#       CMake 2.4.
#
# 1.3.0 Added a LATEX_OUTPUT_PATH variable that allows you or the user to
#       specify where the built latex documents to go.  This is especially
#       handy if you want to do in-source builds.
#
#       Removed the ADD_LATEX_IMAGES macro and absorbed the functionality
#       into ADD_LATEX_DOCUMENT.  The old interface was always kind of
#       clunky anyway since you had to specify the image directory in both
#       places.  It also made supporting LATEX_OUTPUT_PATH problematic.
#
#       Added support for jpeg files.
#
# 1.2.0 Changed the configuration options yet again.  Removed the NO_CONFIGURE
#       Replaced it with a CONFIGURE option that lists input files for which
#       configure should be run.
#
#       The pdf target no longer depends on the dvi target.  This allows you
#       to build latex documents that require pdflatex.  Also added an option
#       to make the pdf target the default one.
#
# 1.1.1 Added the NO_CONFIGURE option.  The @ character can be used when
#       specifying table column separators.  If two or more are used, then
#       will incorrectly substitute them.
#
# 1.1.0 Added ability include multiple bib files.  Added ability to do copy
#       sub-tex files for multipart tex files.
#
# 1.0.0 If both ps and pdf type images exist, just copy the one that
#       matches the current render mode.  Replaced a bunch of STRING
#       commands with GET_FILENAME_COMPONENT commands that were made to do
#       the desired function.
#
# 0.4.0 First version posted to CMake Wiki.
#

#############################################################################
# Find the location of myself while originally executing.  If you do this
# inside of a macro, it will recode where the macro was invoked.
#############################################################################
SET(LATEX_USE_LATEX_LOCATION ${CMAKE_CURRENT_LIST_FILE}
  CACHE INTERNAL "Location of UseLATEX.cmake file." FORCE
  )

#############################################################################
# Generic helper macros
#############################################################################

# Helpful list macros.
MACRO(LATEX_CAR var)
  SET(${var} ${ARGV1})
ENDMACRO(LATEX_CAR)
MACRO(LATEX_CDR var junk)
  SET(${var} ${ARGN})
ENDMACRO(LATEX_CDR)

MACRO(LATEX_LIST_CONTAINS var value)
  SET(${var})
  FOREACH (value2 ${ARGN})
    IF (${value} STREQUAL ${value2})
      SET(${var} TRUE)
    ENDIF (${value} STREQUAL ${value2})
  ENDFOREACH (value2)
ENDMACRO(LATEX_LIST_CONTAINS)

# Parse macro arguments.
MACRO(LATEX_PARSE_ARGUMENTS prefix arg_names option_names)
  SET(DEFAULT_ARGS)
  FOREACH(arg_name ${arg_names})
    SET(${prefix}_${arg_name})
  ENDFOREACH(arg_name)
  FOREACH(option ${option_names})
    SET(${prefix}_${option})
  ENDFOREACH(option)

  SET(current_arg_name DEFAULT_ARGS)
  SET(current_arg_list)
  FOREACH(arg ${ARGN})
    LATEX_LIST_CONTAINS(is_arg_name ${arg} ${arg_names})
    IF (is_arg_name)
      SET(${prefix}_${current_arg_name} ${current_arg_list})
      SET(current_arg_name ${arg})
      SET(current_arg_list)
    ELSE (is_arg_name)
      LATEX_LIST_CONTAINS(is_option ${arg} ${option_names})
      IF (is_option)
        SET(${prefix}_${arg} TRUE)
      ELSE (is_option)
        SET(current_arg_list ${current_arg_list} ${arg})
      ENDIF (is_option)
    ENDIF (is_arg_name)
  ENDFOREACH(arg)
  SET(${prefix}_${current_arg_name} ${current_arg_list})
ENDMACRO(LATEX_PARSE_ARGUMENTS)

# Match the contents of a file to a regular expression.
MACRO(LATEX_FILE_MATCH variable filename regexp default)
  # The FILE STRINGS command would be a bit better, but it's not supported on
  # older versions of CMake.
  FILE(READ ${filename} file_contents)
  STRING(REGEX MATCHALL "${regexp}"
    ${variable} ${file_contents}
    )
  IF (NOT ${variable})
    SET(${variable} "${default}")
  ENDIF (NOT ${variable})
ENDMACRO(LATEX_FILE_MATCH)

#############################################################################
# Macros that perform processing during a LaTeX build.
#############################################################################
MACRO(LATEX_MAKEGLOSSARIES)
  MESSAGE("**************************** In makeglossaries")
  IF (NOT LATEX_TARGET)
    MESSAGE(SEND_ERROR "Need to define LATEX_TARGET")
  ENDIF (NOT LATEX_TARGET)

  IF (NOT MAKEINDEX_COMPILER)
    MESSAGE(SEND_ERROR "Need to define MAKEINDEX_COMPILER")
  ENDIF (NOT MAKEINDEX_COMPILER)

  SET(aux_file ${LATEX_TARGET}.aux)

  IF (NOT EXISTS ${aux_file})
    MESSAGE(SEND_ERROR "${aux_file} does not exist.  Run latex on your target file.")
  ENDIF (NOT EXISTS ${aux_file})

  LATEX_FILE_MATCH(newglossary_lines ${aux_file}
    "@newglossary[ \t]*{([^}]*)}{([^}]*)}{([^}]*)}{([^}]*)}"
    "@newglossary{main}{glg}{gls}{glo}"
    )

  LATEX_FILE_MATCH(istfile_line ${aux_file}
    "@istfilename[ \t]*{([^}]*)}"
    "@istfilename{${LATEX_TARGET}.ist}"
    )
  STRING(REGEX REPLACE "@istfilename[ \t]*{([^}]*)}" "\\1"
    istfile ${istfile_line}
    )

  FOREACH(newglossary ${newglossary_lines})
    STRING(REGEX REPLACE
      "@newglossary[ \t]*{([^}]*)}{([^}]*)}{([^}]*)}{([^}]*)}"
      "\\1" glossary_name ${newglossary}
      )
    STRING(REGEX REPLACE
      "@newglossary[ \t]*{([^}]*)}{([^}]*)}{([^}]*)}{([^}]*)}"
      "${LATEX_TARGET}.\\2" glossary_log ${newglossary}
      )
    STRING(REGEX REPLACE
      "@newglossary[ \t]*{([^}]*)}{([^}]*)}{([^}]*)}{([^}]*)}"
      "${LATEX_TARGET}.\\3" glossary_out ${newglossary}
      )
    STRING(REGEX REPLACE
      "@newglossary[ \t]*{([^}]*)}{([^}]*)}{([^}]*)}{([^}]*)}"
      "${LATEX_TARGET}.\\4" glossary_in ${newglossary}
      )
    MESSAGE("${MAKEINDEX_COMPILER} ${MAKEGLOSSARIES_COMPILER_FLAGS} -s ${istfile} -t ${glossary_log} -o ${glossary_out} ${glossary_in}")
    EXEC_PROGRAM(${MAKEINDEX_COMPILER} ARGS ${MAKEGLOSSARIES_COMPILER_FLAGS}
      -s ${istfile} -t ${glossary_log} -o ${glossary_out} ${glossary_in}
      )
  ENDFOREACH(newglossary)
ENDMACRO(LATEX_MAKEGLOSSARIES)

#############################################################################
# Helper macros for establishing LaTeX build.
#############################################################################

MACRO(LATEX_NEEDIT VAR NAME)
  IF (NOT ${VAR})
    MESSAGE(SEND_ERROR "I need the ${NAME} command.")
  ENDIF(NOT ${VAR})
ENDMACRO(LATEX_NEEDIT)

MACRO(LATEX_WANTIT VAR NAME)
  IF (NOT ${VAR})
    MESSAGE(STATUS "I could not find the ${NAME} command.")
  ENDIF(NOT ${VAR})
ENDMACRO(LATEX_WANTIT)

MACRO(LATEX_SETUP_VARIABLES)
  SET(LATEX_OUTPUT_PATH "${LATEX_OUTPUT_PATH}"
    CACHE PATH "If non empty, specifies the location to place LaTeX output."
    )

  FIND_PACKAGE(LATEX)

  MARK_AS_ADVANCED(CLEAR
    LATEX_COMPILER
    PDFLATEX_COMPILER
    BIBTEX_COMPILER
    MAKEINDEX_COMPILER
    DVIPS_CONVERTER
    PS2PDF_CONVERTER
    LATEX2HTML_CONVERTER
    )

  LATEX_NEEDIT(LATEX_COMPILER latex)
  LATEX_WANTIT(PDFLATEX_COMPILER pdflatex)
  LATEX_NEEDIT(BIBTEX_COMPILER bibtex)
  LATEX_NEEDIT(MAKEINDEX_COMPILER makeindex)
  LATEX_WANTIT(DVIPS_CONVERTER dvips)
  LATEX_WANTIT(PS2PDF_CONVERTER ps2pdf)
  LATEX_WANTIT(LATEX2HTML_CONVERTER latex2html)

  SET(LATEX_COMPILER_FLAGS "-interaction=nonstopmode"
    CACHE STRING "Flags passed to latex.")
  SET(PDFLATEX_COMPILER_FLAGS ${LATEX_COMPILER_FLAGS}
    CACHE STRING "Flags passed to pdflatex.")
  SET(BIBTEX_COMPILER_FLAGS ""
    CACHE STRING "Flags passed to bibtex.")
  SET(MAKEINDEX_COMPILER_FLAGS ""
    CACHE STRING "Flags passed to makeindex.")
  SET(MAKEGLOSSARIES_COMPILER_FLAGS ""
    CACHE STRING "Flags passed to makeglossaries.")
  SET(DVIPS_CONVERTER_FLAGS "-Ppdf -G0 -t letter"
    CACHE STRING "Flags passed to dvips.")
  SET(PS2PDF_CONVERTER_FLAGS "-dMaxSubsetPct=100 -dCompatibilityLevel=1.3 -dSubsetFonts=true -dEmbedAllFonts=true -dAutoFilterColorImages=false -dAutoFilterGrayImages=false -dColorImageFilter=/FlateEncode -dGrayImageFilter=/FlateEncode -dMonoImageFilter=/FlateEncode"
    CACHE STRING "Flags passed to ps2pdf.")
  SET(LATEX2HTML_CONVERTER_FLAGS ""
    CACHE STRING "Flags passed to latex2html.")
  MARK_AS_ADVANCED(
    LATEX_COMPILER_FLAGS
    PDFLATEX_COMPILER_FLAGS
    BIBTEX_COMPILER_FLAGS
    MAKEINDEX_COMPILER_FLAGS
    MAKEGLOSSARIES_COMPILER_FLAGS
    DVIPS_CONVERTER_FLAGS
    PS2PDF_CONVERTER_FLAGS
    LATEX2HTML_CONVERTER_FLAGS
    )
  SEPARATE_ARGUMENTS(LATEX_COMPILER_FLAGS)
  SEPARATE_ARGUMENTS(PDFLATEX_COMPILER_FLAGS)
  SEPARATE_ARGUMENTS(BIBTEX_COMPILER_FLAGS)
  SEPARATE_ARGUMENTS(MAKEINDEX_COMPILER_FLAGS)
  SEPARATE_ARGUMENTS(MAKEGLOSSARIES_COMPILER_FLAGS)
  SEPARATE_ARGUMENTS(DVIPS_CONVERTER_FLAGS)
  SEPARATE_ARGUMENTS(PS2PDF_CONVERTER_FLAGS)
  SEPARATE_ARGUMENTS(LATEX2HTML_CONVERTER_FLAGS)

  FIND_PROGRAM(IMAGEMAGICK_CONVERT convert
    DOC "The convert program that comes with ImageMagick (available at http://www.imagemagick.org)."
    )
  IF (NOT IMAGEMAGICK_CONVERT)
    MESSAGE(SEND_ERROR "Could not find convert program.  Please download ImageMagick from http://www.imagemagick.org and install.")
  ENDIF (NOT IMAGEMAGICK_CONVERT)

  OPTION(LATEX_SMALL_IMAGES
    "If on, the raster images will be converted to 1/6 the original size.  This is because papers usually require 600 dpi images whereas most monitors only require at most 96 dpi.  Thus, smaller images make smaller files for web distributation and can make it faster to read dvi files."
    OFF)
  IF (LATEX_SMALL_IMAGES)
    SET(LATEX_RASTER_SCALE 16)
    SET(LATEX_OPPOSITE_RASTER_SCALE 100)
  ELSE (LATEX_SMALL_IMAGES)
    SET(LATEX_RASTER_SCALE 100)
    SET(LATEX_OPPOSITE_RASTER_SCALE 16)
  ENDIF (LATEX_SMALL_IMAGES)

  # Just holds extensions for known image types.  They should all be lower case.
  SET(LATEX_DVI_VECTOR_IMAGE_EXTENSIONS .eps)
  SET(LATEX_DVI_RASTER_IMAGE_EXTENSIONS)
  SET(LATEX_DVI_IMAGE_EXTENSIONS
    ${LATEX_DVI_VECTOR_IMAGE_EXTENSIONS} ${LATEX_DVI_RASTER_IMAGE_EXTENSIONS})
  SET(LATEX_PDF_VECTOR_IMAGE_EXTENSIONS .pdf)
  SET(LATEX_PDF_RASTER_IMAGE_EXTENSIONS .png .jpeg .jpg)
  SET(LATEX_PDF_IMAGE_EXTENSIONS
    ${LATEX_PDF_VECTOR_IMAGE_EXTENSIONS} ${LATEX_PDF_RASTER_IMAGE_EXTENSIONS})
  SET(LATEX_IMAGE_EXTENSIONS
    ${LATEX_DVI_IMAGE_EXTENSIONS} ${LATEX_PDF_IMAGE_EXTENSIONS})
ENDMACRO(LATEX_SETUP_VARIABLES)

MACRO(LATEX_GET_OUTPUT_PATH var)
  SET(${var})
  IF (LATEX_OUTPUT_PATH)
    IF ("${LATEX_OUTPUT_PATH}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
      MESSAGE(SEND_ERROR "You cannot set LATEX_OUTPUT_PATH to the same directory that contains LaTeX input files.")
    ELSE ("${LATEX_OUTPUT_PATH}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
      SET(${var} "${LATEX_OUTPUT_PATH}")
    ENDIF ("${LATEX_OUTPUT_PATH}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
  ELSE (LATEX_OUTPUT_PATH)
    IF ("${CMAKE_CURRENT_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
      MESSAGE(SEND_ERROR "LaTeX files must be built out of source or you must set LATEX_OUTPUT_PATH.")
    ELSE ("${CMAKE_CURRENT_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
      SET(${var} "${CMAKE_CURRENT_BINARY_DIR}")
    ENDIF ("${CMAKE_CURRENT_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
  ENDIF (LATEX_OUTPUT_PATH)
ENDMACRO(LATEX_GET_OUTPUT_PATH)

MACRO(LATEX_ADD_CONVERT_COMMAND output_path input_path output_extension
        input_extension flags)
  SET (converter ${IMAGEMAGICK_CONVERT})
  SET (convert_flags "")
  # ImageMagick has broken eps to pdf conversion
  # use ps2pdf instead
  IF (${input_extension} STREQUAL ".eps" AND ${output_extension} STREQUAL ".pdf")
    IF (PS2PDF_CONVERTER)
      SET (converter ${PS2PDF_CONVERTER})
      SET (convert_flags "-dEPSCrop ${flags}")
    ELSE (PS2PDF_CONVERTER)
      MESSAGE(SEND_ERROR "Using postscript files with pdflatex requires ps2pdf for conversion.")
    ENDIF (PS2PDF_CONVERTER)
  ELSE (${input_extension} STREQUAL ".eps" AND ${output_extension} STREQUAL ".pdf")
    SET (convert_flags ${flags})
  ENDIF (${input_extension} STREQUAL ".eps" AND ${output_extension} STREQUAL ".pdf")

  ADD_CUSTOM_COMMAND(OUTPUT ${output_path}
    COMMAND ${converter}
      ARGS ${convert_flags} ${input_path} ${output_path}
    DEPENDS ${input_path}
    )
ENDMACRO(LATEX_ADD_CONVERT_COMMAND)

# Makes custom commands to convert a file to a particular type.
MACRO(LATEX_CONVERT_IMAGE output_files input_file output_extension convert_flags
    output_extensions other_files)
  SET(input_dir ${CMAKE_CURRENT_SOURCE_DIR})
  LATEX_GET_OUTPUT_PATH(output_dir)

  GET_FILENAME_COMPONENT(extension "${input_file}" EXT)

  STRING(REGEX REPLACE "\\.[^.]*\$" ${output_extension} output_file
    "${input_file}")

  LATEX_LIST_CONTAINS(is_type ${extension} ${output_extensions})
  IF (is_type)
    IF (convert_flags)
      LATEX_ADD_CONVERT_COMMAND(${output_dir}/${output_file}
        ${input_dir}/${input_file} ${output_extension} ${extension}
        "${convert_flags}")
      SET(${output_files} ${${output_files}} ${output_dir}/${output_file})
    ELSE (convert_flags)
      # As a shortcut, we can just copy the file.
      ADD_CUSTOM_COMMAND(OUTPUT ${output_dir}/${input_file}
        COMMAND ${CMAKE_COMMAND}
        ARGS -E copy ${input_dir}/${input_file} ${output_dir}/${input_file}
        DEPENDS ${input_dir}/${input_file}
        )
      SET(${output_files} ${${output_files}} ${output_dir}/${input_file})
    ENDIF (convert_flags)
  ELSE (is_type)
    SET(do_convert TRUE)
    # Check to see if there is another input file of the appropriate type.
    FOREACH(valid_extension ${output_extensions})
      STRING(REGEX REPLACE "\\.[^.]*\$" ${output_extension} try_file
        "${input_file}")
      LATEX_LIST_CONTAINS(has_native_file "${try_file}" ${other_files})
      IF (has_native_file)
        SET(do_convert FALSE)
      ENDIF (has_native_file)
    ENDFOREACH(valid_extension)

    # If we still need to convert, do it.
    IF (do_convert)
      LATEX_ADD_CONVERT_COMMAND(${output_dir}/${output_file}
        ${input_dir}/${input_file} ${output_extension} ${extension}
        "${convert_flags}")
      SET(${output_files} ${${output_files}} ${output_dir}/${output_file})
    ENDIF (do_convert)
  ENDIF (is_type)
ENDMACRO(LATEX_CONVERT_IMAGE)

# Adds custom commands to process the given files for dvi and pdf builds.
# Adds the output files to the given variables (does not replace).
MACRO(LATEX_PROCESS_IMAGES dvi_outputs pdf_outputs)
  LATEX_GET_OUTPUT_PATH(output_dir)
  FOREACH(file ${ARGN})
    IF (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${file}")
      GET_FILENAME_COMPONENT(extension "${file}" EXT)
      SET(convert_flags)

      # Check to see if we need to downsample the image.
      LATEX_LIST_CONTAINS(is_raster extension
        ${LATEX_DVI_RASTER_IMAGE_EXTENSIONS}
        ${LATEX_PDF_RASTER_IMAGE_EXTENSIONS})
      IF (LATEX_SMALL_IMAGES)
        IF (is_raster)
          SET(convert_flags -resize ${LATEX_RASTER_SCALE}%)
        ENDIF (is_raster)
      ENDIF (LATEX_SMALL_IMAGES)

      # Make sure the output directory exists.
      GET_FILENAME_COMPONENT(path "${output_dir}/${file}" PATH)
      MAKE_DIRECTORY("${path}")

      # Do conversions for dvi.
      LATEX_CONVERT_IMAGE(${dvi_outputs} "${file}" .eps "${convert_flags}"
        "${LATEX_DVI_IMAGE_EXTENSIONS}" "${ARGN}")

      # Do conversions for pdf.
      IF (is_raster)
        LATEX_CONVERT_IMAGE(${pdf_outputs} "${file}" .png "${convert_flags}"
          "${LATEX_PDF_IMAGE_EXTENSIONS}" "${ARGN}")
      ELSE (is_raster)
        LATEX_CONVERT_IMAGE(${pdf_outputs} "${file}" .pdf "${convert_flags}"
          "${LATEX_PDF_IMAGE_EXTENSIONS}" "${ARGN}")
      ENDIF (is_raster)
    ELSE (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${file}")
      MESSAGE("Could not find file \"${CMAKE_CURRENT_SOURCE_DIR}/${file}\"")
    ENDIF (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${file}")
  ENDFOREACH(file)
ENDMACRO(LATEX_PROCESS_IMAGES)

MACRO(ADD_LATEX_IMAGES)
  MESSAGE("The ADD_LATEX_IMAGES macro is deprecated.  Image directories are specified with LATEX_ADD_DOCUMENT.")
ENDMACRO(ADD_LATEX_IMAGES)

MACRO(LATEX_COPY_GLOBBED_FILES pattern dest)
  FILE(GLOB file_list ${pattern})
  FOREACH(in_file ${file_list})
    GET_FILENAME_COMPONENT(out_file ${in_file} NAME)
    CONFIGURE_FILE(${in_file} ${dest}/${out_file} COPYONLY)
  ENDFOREACH(in_file)
ENDMACRO(LATEX_COPY_GLOBBED_FILES)

MACRO(LATEX_COPY_INPUT_FILE file)
  LATEX_GET_OUTPUT_PATH(output_dir)

  IF (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${file})
    GET_FILENAME_COMPONENT(path ${file} PATH)
    FILE(MAKE_DIRECTORY ${output_dir}/${path})

    LATEX_LIST_CONTAINS(use_config ${file} ${LATEX_CONFIGURE})
    IF (use_config)
      CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/${file}
        ${output_dir}/${file}
        @ONLY
        )
      ADD_CUSTOM_COMMAND(OUTPUT ${output_dir}/${file}
        COMMAND ${CMAKE_COMMAND}
        ARGS ${CMAKE_BINARY_DIR}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${file}
        )
    ELSE (use_config)
      ADD_CUSTOM_COMMAND(OUTPUT ${output_dir}/${file}
        COMMAND ${CMAKE_COMMAND}
        ARGS -E copy ${CMAKE_CURRENT_SOURCE_DIR}/${file} ${output_dir}/${file}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${file}
        )
    ENDIF (use_config)
  ELSE (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${file})
    IF (EXISTS ${output_dir}/${file})
      # Special case: output exists but input does not.  Assume that it was
      # created elsewhere and skip the input file copy.
    ELSE (EXISTS ${output_dir}/${file})
      MESSAGE("Could not find input file ${CMAKE_CURRENT_SOURCE_DIR}/${file}")
    ENDIF (EXISTS ${output_dir}/${file})
  ENDIF (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${file})
ENDMACRO(LATEX_COPY_INPUT_FILE)

#############################################################################
# Commands provided by the UseLATEX.cmake "package"
#############################################################################

MACRO(LATEX_USAGE command message)
  MESSAGE(SEND_ERROR
    "${message}\nUsage: ${command}(<tex_file>\n           [BIBFILES <bib_file> <bib_file> ...]\n           [INPUTS <tex_file> <tex_file> ...]\n           [IMAGE_DIRS <directory1> <directory2> ...]\n           [IMAGES <image_file1> <image_file2>\n           [CONFIGURE <tex_file> <tex_file> ...]\n           [DEPENDS <tex_file> <tex_file> ...]\n           [USE_INDEX] [USE_GLOSSARY] [DEFAULT_PDF] [MANGLE_TARGET_NAMES])"
    )
ENDMACRO(LATEX_USAGE command message)

# Parses arguments to ADD_LATEX_DOCUMENT and ADD_LATEX_TARGETS and sets the
# variables LATEX_TARGET, LATEX_IMAGE_DIR, LATEX_BIBFILES, LATEX_DEPENDS, and
# LATEX_INPUTS.
MACRO(PARSE_ADD_LATEX_ARGUMENTS command)
  LATEX_PARSE_ARGUMENTS(
    LATEX
    "BIBFILES;INPUTS;IMAGE_DIRS;IMAGES;CONFIGURE;DEPENDS"
    "USE_INDEX;USE_GLOSSARY;USE_GLOSSARIES;DEFAULT_PDF;MANGLE_TARGET_NAMES"
    ${ARGN}
    )

  # The first argument is the target latex file.
  IF (LATEX_DEFAULT_ARGS)
    LATEX_CAR(LATEX_MAIN_INPUT ${LATEX_DEFAULT_ARGS})
    LATEX_CDR(LATEX_DEFAULT_ARGS ${LATEX_DEFAULT_ARGS})
    GET_FILENAME_COMPONENT(LATEX_TARGET ${LATEX_MAIN_INPUT} NAME_WE)
  ELSE (LATEX_DEFAULT_ARGS)
    LATEX_USAGE(${command} "No tex file target given to ${command}.")
  ENDIF (LATEX_DEFAULT_ARGS)

  IF (LATEX_DEFAULT_ARGS)
    LATEX_USAGE(${command} "Invalid or depricated arguments: ${LATEX_DEFAULT_ARGS}")
  ENDIF (LATEX_DEFAULT_ARGS)

  # Backward compatibility between 1.6.0 and 1.6.1.
  IF (LATEX_USE_GLOSSARIES)
    SET(LATEX_USE_GLOSSARY TRUE)
  ENDIF (LATEX_USE_GLOSSARIES)
ENDMACRO(PARSE_ADD_LATEX_ARGUMENTS)

MACRO(ADD_LATEX_TARGETS)
  LATEX_GET_OUTPUT_PATH(output_dir)
  PARSE_ADD_LATEX_ARGUMENTS(ADD_LATEX_TARGETS ${ARGV})

  # Set up target names.
  IF (LATEX_MANGLE_TARGET_NAMES)
    SET(dvi_target      ${LATEX_TARGET}_dvi)
    SET(pdf_target      ${LATEX_TARGET}_pdf)
    SET(ps_target       ${LATEX_TARGET}_ps)
    SET(safepdf_target  ${LATEX_TARGET}_safepdf)
    SET(html_target     ${LATEX_TARGET}_html)
    SET(auxclean_target ${LATEX_TARGET}_auxclean)
  ELSE (LATEX_MANGLE_TARGET_NAMES)
    SET(dvi_target      dvi)
    SET(pdf_target      pdf)
    SET(ps_target       ps)
    SET(safepdf_target  safepdf)
    SET(html_target     html)
    SET(auxclean_target auxclean)
  ENDIF (LATEX_MANGLE_TARGET_NAMES)

  # For each directory in LATEX_IMAGE_DIRS, glob all the image files and
  # place them in LATEX_IMAGES.
  FOREACH(dir ${LATEX_IMAGE_DIRS})
    FOREACH(extension ${LATEX_IMAGE_EXTENSIONS})
      FILE(GLOB files ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/*${extension})
      FOREACH(file ${files})
        GET_FILENAME_COMPONENT(filename ${file} NAME)
        SET(LATEX_IMAGES ${LATEX_IMAGES} ${dir}/${filename})
      ENDFOREACH(file)
    ENDFOREACH(extension)
  ENDFOREACH(dir)

  SET(dvi_images)
  SET(pdf_images)
  LATEX_PROCESS_IMAGES(dvi_images pdf_images ${LATEX_IMAGES})

  SET(make_dvi_command
    ${CMAKE_COMMAND} -E chdir ${output_dir}
    ${LATEX_COMPILER} ${LATEX_COMPILER_FLAGS} ${LATEX_MAIN_INPUT})
  SET(make_pdf_command
    ${CMAKE_COMMAND} -E chdir ${output_dir}
    ${PDFLATEX_COMPILER} ${PDFLATEX_COMPILER_FLAGS} ${LATEX_MAIN_INPUT})

  SET(make_dvi_depends ${LATEX_DEPENDS} ${dvi_images})
  SET(make_pdf_depends ${LATEX_DEPENDS} ${pdf_images})
  FOREACH(input ${LATEX_MAIN_INPUT} ${LATEX_INPUTS})
    SET(make_dvi_depends ${make_dvi_depends} ${output_dir}/${input})
    SET(make_pdf_depends ${make_pdf_depends} ${output_dir}/${input})
  ENDFOREACH(input)

  IF (LATEX_USE_GLOSSARY)
    FOREACH(dummy 0 1)   # Repeat these commands twice.
      SET(make_dvi_command ${make_dvi_command}
        COMMAND ${CMAKE_COMMAND} -E chdir ${output_dir}
        ${CMAKE_COMMAND}
        -D LATEX_BUILD_COMMAND=makeglossaries
        -D LATEX_TARGET=${LATEX_TARGET}
        -D MAKEINDEX_COMPILER=${MAKEINDEX_COMPILER}
        -D MAKEGLOSSARIES_COMPILER_FLAGS=${MAKEGLOSSARIES_COMPILER_FLAGS}
        -P ${LATEX_USE_LATEX_LOCATION}
        COMMAND ${CMAKE_COMMAND} -E chdir ${output_dir}
        ${LATEX_COMPILER} ${LATEX_COMPILER_FLAGS} ${LATEX_MAIN_INPUT}
        )
      SET(make_pdf_command ${make_pdf_command}
        COMMAND ${CMAKE_COMMAND} -E chdir ${output_dir}
        ${CMAKE_COMMAND}
        -D LATEX_BUILD_COMMAND=makeglossaries
        -D LATEX_TARGET=${LATEX_TARGET}
        -D MAKEINDEX_COMPILER=${MAKEINDEX_COMPILER}
        -D MAKEGLOSSARIES_COMPILER_FLAGS=${MAKEGLOSSARIES_COMPILER_FLAGS}
        -P ${LATEX_USE_LATEX_LOCATION}
        COMMAND ${CMAKE_COMMAND} -E chdir ${output_dir}
        ${PDFLATEX_COMPILER} ${PDFLATEX_COMPILER_FLAGS} ${LATEX_MAIN_INPUT}
        )
    ENDFOREACH(dummy)
  ENDIF (LATEX_USE_GLOSSARY)

  IF (LATEX_BIBFILES)
    SET(make_dvi_command ${make_dvi_command}
      COMMAND ${CMAKE_COMMAND} -E chdir ${output_dir}
      ${BIBTEX_COMPILER} ${BIBTEX_COMPILER_FLAGS} ${LATEX_TARGET})
    SET(make_pdf_command ${make_pdf_command}
      COMMAND ${CMAKE_COMMAND} -E chdir ${output_dir}
      ${BIBTEX_COMPILER} ${BIBTEX_COMPILER_FLAGS} ${LATEX_TARGET})
    FOREACH (bibfile ${LATEX_BIBFILES})
      SET(make_dvi_depends ${make_dvi_depends} ${output_dir}/${bibfile})
      SET(make_pdf_depends ${make_pdf_depends} ${output_dir}/${bibfile})
    ENDFOREACH (bibfile ${LATEX_BIBFILES})
  ENDIF (LATEX_BIBFILES)

  IF (LATEX_USE_INDEX)
    SET(make_dvi_command ${make_dvi_command}
      COMMAND ${CMAKE_COMMAND} -E chdir ${output_dir}
      ${LATEX_COMPILER} ${LATEX_COMPILER_FLAGS} ${LATEX_MAIN_INPUT}
      COMMAND ${CMAKE_COMMAND} -E chdir ${output_dir}
      ${MAKEINDEX_COMPILER} ${MAKEINDEX_COMPILER_FLAGS} ${LATEX_TARGET}.idx)
    SET(make_pdf_command ${make_pdf_command}
      COMMAND ${CMAKE_COMMAND} -E chdir ${output_dir}
      ${PDFLATEX_COMPILER} ${PDFLATEX_COMPILER_FLAGS} ${LATEX_MAIN_INPUT}
      COMMAND ${CMAKE_COMMAND} -E chdir ${output_dir}
      ${MAKEINDEX_COMPILER} ${MAKEINDEX_COMPILER_FLAGS} ${LATEX_TARGET}.idx)
  ENDIF (LATEX_USE_INDEX)

  SET(make_dvi_command ${make_dvi_command}
    COMMAND ${CMAKE_COMMAND} -E chdir ${output_dir}
    ${LATEX_COMPILER} ${LATEX_COMPILER_FLAGS} ${LATEX_MAIN_INPUT}
    COMMAND ${CMAKE_COMMAND} -E chdir ${output_dir}
    ${LATEX_COMPILER} ${LATEX_COMPILER_FLAGS} ${LATEX_MAIN_INPUT})
  SET(make_pdf_command ${make_pdf_command}
    COMMAND ${CMAKE_COMMAND} -E chdir ${output_dir}
    ${PDFLATEX_COMPILER} ${PDFLATEX_COMPILER_FLAGS} ${LATEX_MAIN_INPUT}
    COMMAND ${CMAKE_COMMAND} -E chdir ${output_dir}
    ${PDFLATEX_COMPILER} ${PDFLATEX_COMPILER_FLAGS} ${LATEX_MAIN_INPUT})

  # Add commands and targets for building dvi outputs.
  ADD_CUSTOM_COMMAND(OUTPUT ${output_dir}/${LATEX_TARGET}.dvi
    COMMAND ${make_dvi_command}
    DEPENDS ${make_dvi_depends}
    )
  IF (LATEX_DEFAULT_PDF)
    ADD_CUSTOM_TARGET(${dvi_target}
      DEPENDS ${output_dir}/${LATEX_TARGET}.dvi)
  ELSE (LATEX_DEFAULT_PDF)
    ADD_CUSTOM_TARGET(${dvi_target} ALL
      DEPENDS ${output_dir}/${LATEX_TARGET}.dvi)
  ENDIF (LATEX_DEFAULT_PDF)

  # Add commands and targets for building pdf outputs (with pdflatex).
  IF (PDFLATEX_COMPILER)
    ADD_CUSTOM_COMMAND(OUTPUT ${output_dir}/${LATEX_TARGET}.pdf
      COMMAND ${make_pdf_command}
      DEPENDS ${make_pdf_depends}
      )
    IF (LATEX_DEFAULT_PDF)
      ADD_CUSTOM_TARGET(${pdf_target} ALL
        DEPENDS ${output_dir}/${LATEX_TARGET}.pdf)
    ELSE (LATEX_DEFAULT_PDF)
      ADD_CUSTOM_TARGET(${pdf_target}
        DEPENDS ${output_dir}/${LATEX_TARGET}.pdf)
    ENDIF (LATEX_DEFAULT_PDF)
  ENDIF (PDFLATEX_COMPILER)

  IF (DVIPS_CONVERTER)
    ADD_CUSTOM_COMMAND(OUTPUT ${output_dir}/${LATEX_TARGET}.ps
      COMMAND ${CMAKE_COMMAND} -E chdir ${output_dir}
        ${DVIPS_CONVERTER} ${DVIPS_CONVERTER_FLAGS} -o ${LATEX_TARGET}.ps ${LATEX_TARGET}.dvi
      DEPENDS ${output_dir}/${LATEX_TARGET}.dvi)
    ADD_CUSTOM_TARGET(${ps_target}
      DEPENDS ${output_dir}/${LATEX_TARGET}.ps)
    IF (PS2PDF_CONVERTER)
      # Since both the pdf and safepdf targets have the same output, we
      # cannot properly do the dependencies for both.  When selecting safepdf,
      # simply force a recompile every time.
      ADD_CUSTOM_TARGET(${safepdf_target}
        ${CMAKE_COMMAND} -E chdir ${output_dir}
        ${PS2PDF_CONVERTER} ${PS2PDF_CONVERTER_FLAGS} ${LATEX_TARGET}.ps ${LATEX_TARGET}.pdf
        )
      ADD_DEPENDENCIES(${safepdf_target} ${ps_target})
    ENDIF (PS2PDF_CONVERTER)
  ENDIF (DVIPS_CONVERTER)

  IF (LATEX2HTML_CONVERTER)
    ADD_CUSTOM_TARGET(${html_target}
      ${CMAKE_COMMAND} -E chdir ${output_dir}
      ${LATEX2HTML_CONVERTER} ${LATEX2HTML_CONVERTER_FLAGS} ${LATEX_MAIN_INPUT}
      )
    ADD_DEPENDENCIES(${html_target} ${LATEX_MAIN_INPUT} ${LATEX_INPUTS})
  ENDIF (LATEX2HTML_CONVERTER)

  ADD_CUSTOM_TARGET(${auxclean_target}
    ${CMAKE_COMMAND} -E remove ${output_dir}/${LATEX_TARGET}.aux ${output_dir}/${LATEX_TARGET}.idx ${output_dir}/${LATEX_TARGET}.ind
    )
ENDMACRO(ADD_LATEX_TARGETS)

MACRO(ADD_LATEX_DOCUMENT)
  LATEX_GET_OUTPUT_PATH(output_dir)
  IF (output_dir)
    PARSE_ADD_LATEX_ARGUMENTS(ADD_LATEX_DOCUMENT ${ARGV})

    LATEX_COPY_INPUT_FILE(${LATEX_MAIN_INPUT})

    FOREACH (bib_file ${LATEX_BIBFILES})
      CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/${bib_file}
        ${output_dir}/${bib_file}
        COPYONLY)
      ADD_CUSTOM_COMMAND(OUTPUT ${output_dir}/${bib_file}
        COMMAND ${CMAKE_COMMAND}
        ARGS -E copy ${CMAKE_CURRENT_SOURCE_DIR}/${bib_file} ${output_dir}/${bib_file}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${bib_file}
        )
    ENDFOREACH (bib_file)

    FOREACH (input ${LATEX_INPUTS})
      LATEX_COPY_INPUT_FILE(${input})
    ENDFOREACH(input)

    LATEX_COPY_GLOBBED_FILES(${CMAKE_CURRENT_SOURCE_DIR}/*.cls ${output_dir})
    LATEX_COPY_GLOBBED_FILES(${CMAKE_CURRENT_SOURCE_DIR}/*.bst ${output_dir})
    LATEX_COPY_GLOBBED_FILES(${CMAKE_CURRENT_SOURCE_DIR}/*.clo ${output_dir})
    LATEX_COPY_GLOBBED_FILES(${CMAKE_CURRENT_SOURCE_DIR}/*.sty ${output_dir})

    ADD_LATEX_TARGETS(${ARGV})
  ENDIF (output_dir)
ENDMACRO(ADD_LATEX_DOCUMENT)

#############################################################################
# Actually do stuff
#############################################################################

IF (LATEX_BUILD_COMMAND)
  SET(command_handled)

  IF ("${LATEX_BUILD_COMMAND}" STREQUAL makeglossaries)
    LATEX_MAKEGLOSSARIES()
    SET(command_handled TRUE)
  ENDIF ("${LATEX_BUILD_COMMAND}" STREQUAL makeglossaries)

  IF (NOT command_handled)
    MESSAGE(SEND_ERROR "Unknown command: ${LATEX_BUILD_COMMAND}")
  ENDIF (NOT command_handled)

ELSE (LATEX_BUILD_COMMAND)
  # Must be part of the actual configure (included from CMakeLists.txt).
  LATEX_SETUP_VARIABLES()
ENDIF (LATEX_BUILD_COMMAND)
