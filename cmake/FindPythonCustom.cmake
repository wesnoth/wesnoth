if(PYTHON_EXECUTABLE AND NOT PYTHON_LIBRARY)
  execute_process(COMMAND ${PYTHON_EXECUTABLE} 
                  -c "import distutils.sysconfig; import os.path; print os.path.join(distutils.sysconfig.get_config_var('LIBDIR'), distutils.sysconfig.get_config_var('LDLIBRARY'));"
                  OUTPUT_VARIABLE PYTHON_LIBDIR)
  string(REPLACE "\n" "" PYTHON_LIBDIR ${PYTHON_LIBDIR})

  if(PYTHON_LIBDIR)
    execute_process(COMMAND ${PYTHON_EXECUTABLE} 
                    -c "import distutils.sysconfig; print distutils.sysconfig.get_python_inc();"
                    OUTPUT_VARIABLE PYTHON_INC)
    string(REPLACE "\n" "" PYTHON_INC ${PYTHON_INC})

    file(TO_CMAKE_PATH ${PYTHON_LIBDIR} PYTHON_LIBRARIES)
    file(TO_CMAKE_PATH ${PYTHON_INC} PYTHON_INC)

    set(PYTHON_LIBRARY ${PYTHON_LIBRARIES} CACHE FILEPATH "bla")
    set(PYTHON_INCLUDE_PATH ${PYTHON_INC} CACHE PATH "bla")

  else(PYTHON_LIBDIR)
    FIND_PACKAGE( PythonLibs 2.4 )
  endif(PYTHON_LIBDIR)

elseif(NOT PYTHON_EXECUTABLE)
  FIND_PACKAGE( PythonLibs 2.4 )
else(PYTHON_EXECUTABLE AND NOT PYTHON_LIBRARY)
  set(PYTHON_LIBRARIES ${PYTHON_LIBRARY})
endif(PYTHON_EXECUTABLE AND NOT PYTHON_LIBRARY)
