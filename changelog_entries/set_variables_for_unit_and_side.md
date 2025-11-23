### Lua API
   * Add new wml.valid_var function to validate a WML variable path
   * Bugfix: Indexing a vconfig now returns a table with tag/contents keys
### WML Engine
   * You can now test if a WML variable is empty with `[variables]blank=yes`
   * It is now possible to place `[set_variables]` (note the plural) in `[modify_side]` and `[modify_unit]`
