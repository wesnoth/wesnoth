
These files can be used to allow Lua linting tools to understand Wesnoth types and functions that are defined in C++. Documentation of the annotations format can be found [here](https://github.com/sumneko/lua-language-server/wiki/EmmyLua-Annotations).


To enable in Visual Studio Code, install [this Lua plugin](https://marketplace.visualstudio.com/items?itemName=sumneko.lua) and add the following settings to your settings.json:

```json
"Lua.runtime.version": "Lua 5.4",
"Lua.type.weakNilCheck": true,
"Lua.type.weakUnionCheck": true,
"Lua.workspace.library": [
  "./utils/emmylua"
],
"Lua.runtime.special": {
  "wesnoth.require": "require",
  "wesnoth.dofile": "dofile"
},
"Lua.runtime.builtin": {
  "io": "disable",
  "debug": "disable",
  "os": "disable",
  "package": "disable"
},
"Lua.diagnostics.globals": [
  "wesnoth",
  "wml",
  "gui",
  "filesystem",
  "unit_test",
  "stringx",
  "mathx",
  "ai"
]
```
