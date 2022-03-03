-- ignore line length warnings
max_line_length=false
max_code_line_length=false
max_string_line_length=false
max_comment_line_length=false
-- show the warning/error codes as well
codes=true
-- don't show files with no issues
quiet=1
-- don't show undefined variable usage
-- there are thousands of warnings here because luacheck is unaware of Wesnoth's lua environment and has no way to check which have been loaded
global=false
-- don't show unused variables
unused=false
-- excluded files due to using lua 5.4 syntax that currently gets flagged as a syntax error
-- clear out once a newer Ubuntu LTS base is used for our docker images, which would then also have a newer luacheck available
-- leave the lua module exclusion however
exclude_files={"data/lua/core/wml.lua","data/lua/wml-flow.lua","data/lua/wml/find_path.lua","data/lua/wml/harm_unit.lua","data/lua/wml/modify_unit.lua","data/lua/wml/random_placement.lua","data/lua/functional.lua","src/modules/lua/testes/*.lua"}
