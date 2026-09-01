/**
 * @name Reversed Lua metatable protection key and value
 * @description Finds lua_setfield calls that push "__metatable" as the value
 *              while the third argument is not the literal "__metatable",
 *              suggesting that the intended key and value are reversed.
 * @kind problem
 * @problem.severity warning
 * @precision high
 * @id cpp/misordered-lua-metatable-protection
 */

import cpp
import models.LuaMetatables

/*
 * `lua_setfield(L, index, key)` pops the value from the top of the stack and
 * stores it under `key` in the table at `index`. The key comes from the third
 * argument, not from the stack. Reversing the intended key and value produces:
 *
 *     metatable[other] = "__metatable"
 *
 * instead of assigning the protection field. The query reports only when the
 * table at `index` is the metatable and every path puts `"__metatable"` on top
 * of the stack.
 */

private string getFieldName(FunctionCall write) {
  result = write.getArgument(2).getUnconverted().(StringLiteral).getValue()
  or
  not write.getArgument(2).getUnconverted() instanceof StringLiteral and
  result = write.getArgument(2).toString()
}

from Metatable metatable, FunctionCall write
where
  not isVendoredLuaEngineCall(write) and
  reversedMetatableProtection(metatable, write)
select write,
  "The key and value are reversed: this call performs metatable['" + getFieldName(write) +
    "'] = '__metatable', but protection requires metatable['__metatable'] = <value>. Push the " +
    "protection value, then pass '__metatable' as lua_setfield's third argument."
