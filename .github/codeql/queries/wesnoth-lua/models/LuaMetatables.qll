import cpp
import models.LuaStack
import semmle.code.cpp.controlflow.Dominance
import semmle.code.cpp.dataflow.new.DataFlow
import semmle.code.cpp.valuenumbering.GlobalValueNumbering

/*
 * Models native callback registration and protection writes for Lua userdata
 * metatables.
 *
 * A userdata metatable tells Lua how the object behaves when scripts index,
 * compare, call, iterate over, or collect it.
 * Wesnoth implements these behaviors with native metamethods stored in the
 * table. Without `__metatable`, Lua scripts can retrieve the real table and
 * call those metamethods directly.
 *
 * Lua APIs identify tables by stack position. `LuaStack` checks that each
 * registration or protection write uses the table pushed by the same
 * `luaL_newmetatable` call. Source order is not enough because another table
 * may have been pushed in between.
 */

predicate callsLuaApi(FunctionCall call, string name) {
  call.getTarget().hasGlobalName(name)
}

/**
 * Identifies calls inside Wesnoth's bundled Lua implementation so the queries
 * can exclude findings there. CodeQL can still follow those calls while
 * analyzing Wesnoth's Lua integration.
 */
predicate isVendoredLuaEngineCall(FunctionCall call) {
  call.getFile().getRelativePath().matches("%src/modules/lua/%")
}

/**
 * Holds when CodeQL can prove that two expressions have the same value within
 * one function, including through local aliases and conversions.
 */
predicate sameLuaValue(Expr left, Expr right) {
  exists(Expr representative |
    representative = globalValueNumber(left).getAnExpr() and
    representative = globalValueNumber(right).getAnExpr()
  )
}

/**
 * Holds when at least one control-flow path can execute `after` following
 * `before`. Reachability does not mean that every execution reaches `after`.
 */
predicate luaControlFlowReaches(ControlFlowNode before, ControlFlowNode after) {
  after = before.getASuccessor+()
}

private predicate hasWrappedStringValue(Expr expression, string value) {
  exists(StringLiteral literal |
    literal.getValue() = value and
    (
      expression.getUnconverted() = literal
      or
      literal = expression.getAChild()
    )
  )
}

private predicate hasNegativeIndex(Expr expression, string magnitude) {
  exists(UnaryMinusExpr minus, Literal literal |
    expression.getUnconverted() = minus and
    minus.getOperand().getUnconverted() = literal and
    literal.getValue() = magnitude
  )
}

/**
 * `luaL_newmetatable` pushes its table onto the Lua stack. The model uses that
 * call to identify the table when later API calls refer to it by stack
 * position.
 */
class Metatable extends FunctionCall {
  Metatable() { callsLuaApi(this, "luaL_newmetatable") }

  string getName() {
    result = this.getArgument(1).getUnconverted().(StringLiteral).getValue()
    or
    not this.getArgument(1).getUnconverted() instanceof StringLiteral and
    result = "<dynamic metatable key>"
  }

  MetatableCallbackRegistration getARegistration() { result.getMetatable() = this }

  /**
   * Holds when every callback registration has a protection write that cannot
   * be skipped.
   *
   * If the write comes first, every path to the registration must pass through
   * it. If the write comes later, every path from the registration to the
   * function exit must pass through it.
   */
  predicate isProtected() {
    forall(MetatableCallbackRegistration registration |
      registration.getMetatable() = this
    |
      exists(FunctionCall write |
        protectionWrite(write, this) and
        ownsCall(this, write) and
        (
          luaControlFlowReaches(write, registration.getApplication()) and
          dominates(write, registration.getApplication())
          or
          luaControlFlowReaches(registration.getApplication(), write) and
          postDominates(write, registration.getApplication())
        )
      )
    )
  }
}

/**
 * Resolves callback registrations such as `dispatch<&method>` and
 * `dispatch2<&method, false>` to `method`.
 */
private predicate templateCallback(Function adapter, Function callback) {
  exists(Expr argument, FunctionAccess access |
    adapter.getTemplateArgument(_) = argument and
    access = argument.getAChild*() and
    access.getTarget() = callback
  )
}

private predicate resolvedCallback(FunctionAccess access, Function callback) {
  templateCallback(access.getTarget(), callback)
  or
  access.getTarget() = callback and
  not exists(Function target | templateCallback(access.getTarget(), target))
}

/**
 * Finds callbacks passed directly or through a temporary variable in the same
 * function.
 */
private predicate callbackExpression(Expr expression, Function callback) {
  exists(FunctionAccess access |
    access = expression.getUnconverted().getAChild*() and
    resolvedCallback(access, callback)
  )
  or
  exists(FunctionAccess source |
    resolvedCallback(source, callback) and
    DataFlow::localExprFlow(source, expression)
  )
}

/**
 * Finds callback entries in a local `luaL_Reg` array passed to
 * `luaL_setfuncs`.
 */
private predicate arrayRegistration(
  StringLiteral name, Function callback, FunctionCall application
) {
  callsLuaApi(application, "luaL_setfuncs") and
  exists(Variable array, VariableAccess arrayUse, ClassAggregateLiteral entry |
    entry = array.getInitializer().getExpr().getAChild*() and
    name = entry.getFieldExpr(_, 0).getUnconverted() and
    callbackExpression(entry.getFieldExpr(_, 1), callback) and
    arrayUse.getTarget() = array and
    DataFlow::localExprFlow(arrayUse, application.getArgument(1))
  )
}

/**
 * Finds callbacks assigned with `lua_setfield` when every path puts the same
 * callback on top of the Lua stack.
 */
private predicate pushedFieldRegistration(
  StringLiteral name, Function callback, FunctionCall application
) {
  callsLuaApi(application, "lua_setfield") and
  application.getArgument(2).getUnconverted() = name and
  stackSlotIsResolved(application, 1) and
  exists(FunctionCall representative |
    representative = getAStackSlotProducer(application, 1) and
    callsLuaApi(representative, ["lua_pushcfunction", "lua_pushcclosure"]) and
    callbackExpression(representative.getArgument(1), callback)
  ) and
  forall(FunctionCall push |
    push = getAStackSlotProducer(application, 1)
  |
    callsLuaApi(push, ["lua_pushcfunction", "lua_pushcclosure"]) and
    callbackExpression(push.getArgument(1), callback)
  )
}

/**
 * Finds callback registrations made with `lua_rawset(L, -3)`. At the call,
 * `-3` is the table, `-2` is the key, and `-1` is the callback.
 */
private predicate rawFieldRegistration(
  StringLiteral name, Function callback, FunctionCall application
) {
  callsLuaApi(application, "lua_rawset") and
  hasNegativeIndex(application.getArgument(1), "3") and
  exists(FunctionCall keyPush, FunctionCall callbackPush |
    callsLuaApi(keyPush, "lua_pushstring") and
    keyPush.getArgument(1).getUnconverted() = name and
    stackSlotComesFrom(application, 2, keyPush) and
    callsLuaApi(callbackPush, ["lua_pushcfunction", "lua_pushcclosure"]) and
    callbackExpression(callbackPush.getArgument(1), callback) and
    stackSlotComesFrom(application, 1, callbackPush)
  )
}

private predicate pushedGlobalRegistration(
  StringLiteral name, Function callback, FunctionCall application
) {
  callsLuaApi(application, "lua_setglobal") and
  application.getArgument(1).getUnconverted() = name and
  stackSlotIsResolved(application, 1) and
  forall(FunctionCall push |
    push = getAStackSlotProducer(application, 1)
  |
    callsLuaApi(push, ["lua_pushcfunction", "lua_pushcclosure"]) and
    callbackExpression(push.getArgument(1), callback)
  )
}

private predicate registration(
  StringLiteral name, Function callback, FunctionCall application
) {
  arrayRegistration(name, callback, application)
  or
  pushedFieldRegistration(name, callback, application)
  or
  rawFieldRegistration(name, callback, application)
  or
  pushedGlobalRegistration(name, callback, application)
}

/**
 * Matches an operation to the table at its stack index. Using the nearest
 * `luaL_newmetatable` call would be wrong if another table was pushed in
 * between.
 */
private predicate ownsCall(Metatable metatable, FunctionCall call) {
  luaControlFlowReaches(metatable, call) and
  sameLuaValue(metatable.getArgument(0), call.getArgument(0)) and
  (
    callsLuaApi(call, "luaL_setfuncs") and
    stackSlotComesFrom(call, call.getArgument(2).getValue().toInt() + 1, metatable)
    or
    callsLuaApi(call, ["lua_setfield", "lua_rawset"]) and
    stackSlotComesFrom(call, getTopRelativeSlot(call.getArgument(1)), metatable)
    or
    call.getTarget().hasName("luaW_table_set") and
    stackSlotComesFrom(call, getTopRelativeSlot(call.getArgument(1)), metatable)
  )
}

/**
 * Identifies vconfig's iterator metatable so `UnprotectedLuaMetatable` can
 * suppress its false positive. The iterator userdata is stored inside the
 * iterator function instead of being returned to the script. Wesnoth also
 * removes `debug.getupvalue`, the Lua function that could retrieve it.
 *
 * Match `impl_vconfig_pairs_collect` directly because following the userdata
 * into the iterator function takes more than an hour on the Wesnoth database.
 */
predicate isHiddenVconfigIteratorMetatable(Metatable registered) {
  exists(MetatableCallbackRegistration registration |
    registration = registered.getARegistration() and
    registration.getName() = "__gc" and
    registration.getCallback().hasName("impl_vconfig_pairs_collect")
  )
}

/**
 * Finds non-nil `__metatable` writes made with `lua_setfield` and Wesnoth's
 * `luaW_table_set(L, -1, "__metatable", std::string(labelKey))` call.
 */
private predicate protectionWrite(FunctionCall write, Metatable metatable) {
  write.getTarget().hasName("luaW_table_set") and
  hasNegativeIndex(write.getArgument(1), "1") and
  hasWrappedStringValue(write.getArgument(2), "__metatable") and
  stackSlotComesFrom(write, 1, metatable)
  or
  callsLuaApi(write, "lua_setfield") and
  write.getArgument(2).getUnconverted().(StringLiteral).getValue() = "__metatable" and
  exists(int tableSlot |
    tableSlot = getTopRelativeSlot(write.getArgument(1)) and
    stackSlotComesFrom(write, tableSlot, metatable) and
    stackSlotIsNonNil(write, 1)
  )
}

/**
 * Finds `lua_setfield` calls that put `"__metatable"` in the value position
 * while the third argument names another key. Such a call assigns
 * `metatable[other] = "__metatable"` and does not protect the metatable.
 */
predicate reversedMetatableProtection(Metatable metatable, FunctionCall write) {
  ownsCall(metatable, write) and
  callsLuaApi(write, "lua_setfield") and
  not write.getArgument(2).getUnconverted().(StringLiteral).getValue() = "__metatable" and
  exists(int tableSlot |
    tableSlot = getTopRelativeSlot(write.getArgument(1)) and
    stackSlotComesFrom(write, tableSlot, metatable) and
    stackSlotIsString(write, 1, "__metatable")
  )
}

class LuaCallbackRegistration extends StringLiteral {
  LuaCallbackRegistration() {
    exists(Function callback, FunctionCall application |
      registration(this, callback, application)
    )
  }

  string getName() { result = this.getValue() }

  Function getCallback() {
    exists(FunctionCall application | registration(this, result, application))
  }

  FunctionCall getApplication() { registration(this, _, result) }

  /**
   * Follows the callback's `lua_State*` into helper functions so checks and
   * pointer uses in those helpers can be matched to the registered callback.
   */
  Function getCallbackOrStateHelper() {
    result = this.getCallback()
    or
    exists(Function caller, FunctionCall call, VariableAccess stateAccess |
      caller = this.getCallbackOrStateHelper() and
      call.getEnclosingFunction() = caller and
      call.getTarget() = result and
      result.getNumberOfParameters() > 0 and
      stateAccess.getTarget() = caller.getParameter(0) and
      DataFlow::localExprFlow(stateAccess, call.getArgument(0))
    )
  }
}

class MetatableCallbackRegistration extends LuaCallbackRegistration {
  MetatableCallbackRegistration() {
    exists(Metatable metatable | ownsCall(metatable, this.getApplication()))
  }

  Metatable getMetatable() {
    ownsCall(result, this.getApplication())
  }

  predicate hasUnprovenProtection() { not this.getMetatable().isProtected() }
}

class MetamethodRegistration extends MetatableCallbackRegistration {
  MetamethodRegistration() {
    this.getName() =
      [
        "__index", "__newindex", "__gc", "__len", "__eq", "__add", "__sub", "__mul", "__mod",
        "__pow", "__div", "__idiv", "__band", "__bor", "__bxor", "__shl", "__shr", "__unm",
        "__bnot", "__lt", "__le", "__concat", "__call", "__close", "__tostring", "__pairs"
      ]
  }

  /**
   * Holds for arguments taken from the script operation that invoked the
   * metamethod. `a + b` calls `__add(a, b)`, while `object[key]` calls
   * `__index(object, key)`. Positive stack indices count from the first
   * argument; negative indices count backward from the last argument.
   */
  predicate hasCallerControlledArgument(Expr stackIndex) {
    this.getName() =
      [
        "__eq", "__add", "__sub", "__mul", "__mod", "__pow", "__div", "__idiv", "__band",
        "__bor", "__bxor", "__shl", "__shr", "__lt", "__le", "__concat"
      ] and
    (
      stackIndex.getValue().toInt() = [1, 2]
      or
      getTopRelativeSlot(stackIndex) = [1, 2]
    )
    or
    this.getName() = "__index" and
    (
      stackIndex.getValue().toInt() = 2
      or
      getTopRelativeSlot(stackIndex) = 1
    )
    or
    this.getName() = "__newindex" and
    (
      stackIndex.getValue().toInt() = [2, 3]
      or
      getTopRelativeSlot(stackIndex) = [1, 2]
    )
    or
    this.getName() = "__call" and
    (
      stackIndex.getValue().toInt() > 1
      or
      getTopRelativeSlot(stackIndex) > 0
    )
    or
    this.getName() = "__close" and
    (
      stackIndex.getValue().toInt() = 2
      or
      getTopRelativeSlot(stackIndex) = 1
    )
  }
}

/**
 * Holds when a userdata check uses the registered metatable's key. Literal
 * keys are compared by text. Other expressions must evaluate to the same
 * value.
 */
predicate matchesMetatableKey(Expr checkedKey, Metatable metatable) {
  exists(StringLiteral checked, StringLiteral registered |
    checkedKey.getUnconverted() = checked and
    metatable.getArgument(1).getUnconverted() = registered and
    checked.getValue() = registered.getValue()
  )
  or
  sameLuaValue(checkedKey, metatable.getArgument(1))
}
