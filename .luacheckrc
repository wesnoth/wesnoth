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
exclude_files={"src/modules/lua/testes/*.lua"}
