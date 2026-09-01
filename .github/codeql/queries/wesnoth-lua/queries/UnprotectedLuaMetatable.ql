/**
 * @name Lua metatable exposes native functions
 * @description Finds Wesnoth Lua metatables whose stored native functions
 *              remain accessible because __metatable does not hide the table.
 * @kind problem
 * @problem.severity warning
 * @precision medium
 * @id cpp/exposed-unprotected-lua-metatable
 */

import cpp
import models.LuaMetatables

/*
 * Without `__metatable`, Lua scripts can retrieve the real metatable and
 * directly call native functions stored there, including metamethods.
 *
 * This query reports metatables that store native functions without hiding
 * the table. One non-nil `__metatable` value protects the whole table, so the
 * query reports the metatable once rather than every stored function.
 */

from Metatable metatable
where
  not isVendoredLuaEngineCall(metatable) and
  not isHiddenVconfigIteratorMetatable(metatable) and
  not metatable.isProtected() and
  exists(MetatableCallbackRegistration registration | registration = metatable.getARegistration())
select metatable,
  "Metatable '" + metatable.getName() +
    "' exposes native functions stored in it because no non-nil __metatable write " +
    "protects the table. Set __metatable while initializing the metatable."
