-- limit line length warnings to a certain length
max_line_length=999
-- show the warning/error codes as well
codes=true
-- skip showing files with no issues
quiet=1
-- skip showing undefined variable usage
-- there are thousands of warnings here because luacheck is unaware of Wesnoth's
-- lua environment and has no way to check which have been loaded
globals={"wesnoth","wml","gui","filesystem","unit_test","stringx","mathx","ai"}
allow_defined=false
allow_defined_top=true
-- skip showing unused variables
unused=false
exclude_files={"src/modules/lua/testes/*.lua"}
