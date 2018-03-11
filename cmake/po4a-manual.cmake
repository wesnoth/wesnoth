# Small helper to execute code see doc/manual/CMakeLists.txt
# for more information
if(EXISTS ${SOURCE})
	string(REPLACE "\\ " " " CMD ${CMD})
	separate_arguments(CMD)
	execute_process(COMMAND ${CMD})
endif(EXISTS ${SOURCE})
