When updating the Lua submodule to a new major/minor version (e.g. 5.4 to 5.5), please also update the system Lua checks in the build systems.

Download new versions of <https://gitlab.kitware.com/cmake/cmake/-/raw/master/Modules/FindPackageMessage.cmake> and <https://gitlab.kitware.com/cmake/cmake/-/raw/master/Modules/FindPackageHandleStandardArgs.cmake> to cmake/.

Update cmake/FindLua.cmake from <https://gitlab.kitware.com/cmake/cmake/-/raw/master/Modules/FindLua.cmake> and cherry pick the Wesnoth commit "Modify FindLua.cmake to find Lua compiled as C++".

In CMakeLists.txt, update `set(Lua_FIND_VERSION_MAJOR 5)` and `set(Lua_FIND_VERSION_MINOR 4)`. In SConstruct, update `lua_ver = "5.4"`.
