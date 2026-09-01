/**
 * @name Lua callback uses unchecked userdata argument
 * @description Finds native Lua callbacks that use lua_touserdata on a caller
 *              argument as a C++ object without first validating its userdata
 *              type.
 * @kind problem
 * @problem.severity warning
 * @precision high
 * @id cpp/unchecked-lua-userdata-argument
 */

import cpp
import models.LuaMetatables
import models.LuaUserdata

/*
 * `lua_touserdata` does not check userdata type. Lua chooses every argument
 * passed to a regular registered callback.
 *
 * For a protected metatable callback, the query checks only metamethod
 * arguments that Lua supplies, such as arithmetic operands and table keys. If
 * `__metatable` is absent, Lua can retrieve the native callback and invoke it
 * with arbitrary arguments.
 *
 * The value at a checked stack position is assumed not to change before the
 * pointer is used.
 */

private predicate hasCallerControlledMetamethodArgument(
  LuaCallbackRegistration registration, UserdataConversion conversion
) {
  registration.(MetamethodRegistration).hasCallerControlledArgument(
    conversion.getArgument(1)
  )
}

private predicate hasRequiredValidation(
  LuaCallbackRegistration registration, UserdataConversion conversion, Expr use
) {
  registration instanceof MetatableCallbackRegistration and
  (
    if hasCallerControlledMetamethodArgument(registration, conversion)
    then conversion.isDirectlyValidatedForUse(use)
    else
      conversion.isValidatedForUse(
        registration.(MetatableCallbackRegistration).getMetatable(), use
      )
  )
  or
  not registration instanceof MetatableCallbackRegistration and
  conversion.isDirectlyValidatedForUse(use)
}

from
  LuaCallbackRegistration registration, UserdataConversion conversion,
  Expr concreteUse
where
  conversion.readsCallerArgument() and
  not isVendoredLuaEngineCall(conversion) and
  (
    not registration instanceof MetatableCallbackRegistration
    or
    registration.(MetatableCallbackRegistration).hasUnprovenProtection()
    or
    hasCallerControlledMetamethodArgument(registration, conversion)
  ) and
  not hasRequiredValidation(registration, conversion, concreteUse) and
  conversion.getEnclosingFunction() = registration.getCallbackOrStateHelper() and
  concreteUse = conversion.getAConcreteUse()
select conversion,
  "Lua callback '" + registration.getCallback().getName() + "' registered as '" +
    registration.getName() + "' uses lua_touserdata on caller argument " +
    conversion.getStackIndex().toString() +
    " as a C++ object without checking its userdata type. Use luaL_checkudata or check " +
    "luaL_testudata before use."
