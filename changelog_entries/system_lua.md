 ### Miscellaneous and Bug Fixes
   * Fix delayed handling of Lua jailbreak exceptions (quit to menu or desktop, wesnothd connection errors, etc.) thrown during Lua `pcall()` and `xpcall()`, which could accumulate and cause wesnoth to abort. (PR #8234)
     * This bug has existed since jailbreak exceptions were introduced in version 1.9.5.
   * Fix strict mode not catching Lua errors without exception strings (bug since 1.9.5) or with exception strings containing "~lua:" (bug since 1.9.0). (PR #8234)
   * Store Lua jailbreak exceptions in derived class constructors. This reduces the reliance on `LUAI_TRY()`, an internal part of Lua that may not exist forever. (PR #8234)
