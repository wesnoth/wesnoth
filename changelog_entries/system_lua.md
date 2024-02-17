 ### Packaging
   * Add CMake and SCons options to use system Lua 5.4, like before version 1.9.0. (PR #8234)
     * This is intended for distributions to choose to link Wesnoth against their own Lua 5.4 packages, instead of using Wesnoth's Lua submodule (mostly unmodified as of 1.17.2, PR #6549), allowing them to easily update Lua in case of severe bugs or vulnerabilities.
     * Lua 5.4 must be compiled as C++, so that it can recover from C++ exceptions. Debian and Arch are known to do this, while others like Fedora, Gentoo, openSUSE, and FreeBSD currently only compile it as C.
     * Windows requires a compile-time change, so it still must use the Lua submodule.
 ### Miscellaneous and Bug Fixes
   * Fix delayed handling of Lua jailbreak exceptions (quit to menu or desktop, wesnothd connection errors, etc.) thrown during Lua `pcall()` and `xpcall()`, which could accumulate and cause wesnoth to abort. (PR #8234)
     * This bug has existed since jailbreak exceptions were introduced in version 1.9.5.
   * Fix strict mode not catching Lua errors without exception strings (bug since 1.9.5) or with exception strings containing "~lua:" (bug since 1.9.0). (PR #8234)
   * Store Lua jailbreak exceptions in derived class constructors. This reduces the reliance on `LUAI_TRY()`, an internal part of Lua that may not exist forever. (PR #8234)
