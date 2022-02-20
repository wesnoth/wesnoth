---@meta
-- This file contains settings for the Lua add-on in Visual Studio Code
-- It's supposed to allow the add-on to automatically detect the correct settings,
-- but I'm not quite sure how to make that work.
name = 'Wesnoth'
words = {'wesnoth', 'wml'}
configs = {
    {
        key = 'Lua.runtime.version',
        action = 'set',
        value = '5.4'
    },
    {
        key = 'Lua.diagnostics.globals',
        action = 'add',
        value = 'wesnoth'
    },
    {
        key = 'Lua.diagnostics.globals',
        action = 'add',
        value = 'wml'
    },
    {
        key = 'Lua.diagnostics.globals',
        action = 'add',
        value = 'gui'
    },
    {
        key = 'Lua.diagnostics.globals',
        action = 'add',
        value = 'filesystem'
    },
    {
        key = 'Lua.runtime.special',
        action = 'prop',
        prop = 'wesnoth.require',
        value = 'require'
    },
    {
        key = 'Lua.runtime.path',
        action = 'add',
        value = 'data/core/lua/?.lua'
    },
    {
        key = 'Lua.runtime.path',
        action = 'add',
        value = 'data/?.lua'
    },
    {
        key = 'Lua.runtime.builtin',
        action = 'prop',
        prop = 'io',
        value = 'disable',
    },
    {
        key = 'Lua.runtime.builtin',
        action = 'prop',
        prop = 'debug',
        value = 'disable',
    },
    {
        key = 'Lua.runtime.builtin',
        action = 'prop',
        prop = 'os',
        value = 'disable',
    },
    {
        key = 'Lua.runtime.builtin',
        action = 'prop',
        prop = 'package',
        value = 'disable'
    }
}
