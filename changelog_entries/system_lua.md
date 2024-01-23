 ### Miscellaneous and Bug Fixes
   * Fix delayed handling of Lua jailbreak exceptions (quit to menu or desktop, wesnothd connection errors, etc.) thrown during Lua `pcall()` and `xpcall()`, which could accumulate and cause wesnoth to abort. (PR #8234)
     * This bug has existed since jailbreak exceptions were introduced in version 1.9.5.
