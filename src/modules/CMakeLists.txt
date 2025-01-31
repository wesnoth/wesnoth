# Disable clang-tidy for all git submodules, since we can't fix these issues
# in the wesnoth repo
set(CMAKE_CXX_CLANG_TIDY "")
if(ENABLE_MYSQL)
	execute_process(COMMAND mysql_config --cflags OUTPUT_VARIABLE MYSQL_CFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
	string(REGEX REPLACE "-I" "" MYSQL_CFLAGS "${MYSQL_CFLAGS}")
	string(REGEX REPLACE "-DNDEBUG" "" MYSQL_CFLAGS "${MYSQL_CFLAGS}")
	execute_process(COMMAND mysql_config --libs OUTPUT_VARIABLE MYSQL_LIBS OUTPUT_STRIP_TRAILING_WHITESPACE)
	add_subdirectory("mariadbpp/")
endif()

if(NOT ENABLE_SYSTEM_LUA)
	GetSources("lua" lua_sources)

	# Inject a header into the Lua sources for Wesnoth-specific changes
	# makedepend won't see it so we have to specifically add it as a dependency.
	file(GLOB wesnoth_lua_config ../wesnoth_lua_config.h)
	set_source_files_properties(${lua_sources} PROPERTIES OBJECT_DEPENDS ${wesnoth_lua_config})

	set_source_files_properties(${lua_sources} PROPERTIES LANGUAGE CXX)

	# We explicitly want lua compiled as C++ version
	if(MSVC)
		set_source_files_properties(${lua_sources} PROPERTIES COMPILE_FLAGS "/FI\"${wesnoth_lua_config}\"")
		# silence an implicit bitshift conversion warning
		set_property(SOURCE SOURCE ${lua_sources} APPEND_STRING PROPERTY COMPILE_FLAGS " /wd4334 /TP")
	else()
		set_source_files_properties(${lua_sources} PROPERTIES COMPILE_FLAGS "-include \"${wesnoth_lua_config}\" -x c++ -Wno-old-style-cast -Wno-useless-cast -Wno-stringop-overflow")
	endif()

	if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
		# silence a Clang specific warning due to extra parenthesis in if statements
		set_property(SOURCE SOURCE ${lua_sources} APPEND_STRING PROPERTY COMPILE_FLAGS " -Wno-parentheses-equality")
		# silence a Clang specific warning when compiling the lua code
		set_property(SOURCE SOURCE ${lua_sources} APPEND_STRING PROPERTY COMPILE_FLAGS " -Wno-string-plus-int")
	endif()

    add_library(lua STATIC ${lua_sources})
endif()
