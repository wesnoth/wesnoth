import cpp
import models.LuaMetatables
import semmle.code.cpp.controlflow.Dominance
import semmle.code.cpp.controlflow.IRGuards
import semmle.code.cpp.dataflow.new.DataFlow
import semmle.code.cpp.valuenumbering.GlobalValueNumbering

/*
 * Tracks pointers returned by `lua_touserdata`, which does not check userdata
 * type, through local variables, function arguments, and returns. A pointer use
 * is accepted only after `luaL_checkudata`, or on a path where
 * `luaL_testudata` returned non-null.
 *
 * Validation performed by called functions is not followed.
 */

/**
 * Identifies field access, member calls, and dereferences that use the pointer
 * as a C++ object.
 */
private predicate pointerLayoutUse(Expr expression) {
  exists(PointerFieldAccess field | expression = field.getQualifier())
  or
  exists(FunctionCall call |
    expression = call.getQualifier() and call.getTarget() instanceof MemberFunction
  )
  or
  exists(PointerDereferenceExpr dereference | expression = dereference.getOperand())
}
private module UserdataPointerFlowConfig implements DataFlow::ConfigSig {
  predicate isSource(DataFlow::Node source) {
    source.asExpr() instanceof UserdataConversion
  }

  predicate isSink(DataFlow::Node sink) { pointerLayoutUse(sink.asExpr()) }
}

private module UserdataPointerFlow = DataFlow::Global<UserdataPointerFlowConfig>;

/**
 * Matches a userdata check to `lua_touserdata` when both use the same Lua
 * state and stack position. Local aliases and conversions are followed. The
 * value at that position is assumed not to change between the check and use.
 */
private predicate sameCallbackArgument(
  FunctionCall validation, FunctionCall conversion, int stateArgument, int indexArgument
) {
  sameLuaValue(validation.getArgument(stateArgument), conversion.getArgument(0)) and
  sameLuaValue(validation.getArgument(indexArgument), conversion.getArgument(1))
}

private predicate directCheck(
  FunctionCall check, UserdataConversion conversion, string api
) {
  callsLuaApi(check, api) and
  sameCallbackArgument(check, conversion, 0, 1)
}

/**
 * Returns the operation that must occur after the userdata type check. Within
 * one function this is the pointer use. Once the pointer crosses a function
 * boundary, this is the `lua_touserdata` call that creates it.
 */
private ControlFlowNode getOperationAfterCheck(UserdataConversion conversion, Expr use) {
  conversion.getEnclosingFunction() = use.getEnclosingFunction() and
  result = use
  or
  conversion.getEnclosingFunction() != use.getEnclosingFunction() and
  result = conversion
}

/**
 * Holds when `before` runs first on every path to `after`.
 */
private predicate alwaysBefore(ControlFlowNode before, ControlFlowNode after) {
  dominates(before, after) and
  (
    before.getBasicBlock() != after.getBasicBlock()
    or
    luaControlFlowReaches(before, after)
  )
}

/**
 * `luaL_checkudata` raises on a type mismatch, so the pointer cannot be created
 * or used until after the check on every path.
 */
private predicate throwingCheckForUse(
  FunctionCall check, UserdataConversion conversion, Expr use
) {
  directCheck(check, conversion, "luaL_checkudata") and
  alwaysBefore(check, getOperationAfterCheck(conversion, use))
}

/**
 * `luaL_testudata` returns null on a type mismatch, so the pointer can be
 * created or used only on the non-null path.
 */
private predicate testGuardForUse(
  FunctionCall test, UserdataConversion conversion, Expr use
) {
  directCheck(test, conversion, "luaL_testudata") and
  exists(GuardCondition guard, Expr testedValue |
    testedValue = globalValueNumber(test).getAnExpr() and
    guard.ensuresEq(
      testedValue, 0, getOperationAfterCheck(conversion, use).getBasicBlock(), false
    )
  )
}

class UserdataConversion extends FunctionCall {
  UserdataConversion() { callsLuaApi(this, "lua_touserdata") }

  /**
   * Holds when `lua_touserdata` reads a callback argument. Positive indices
   * name arguments from the bottom of the stack. A negative index is also an
   * argument when stack tracing shows that its position still comes from
   * callback entry.
   */
  predicate readsCallerArgument() {
    this.getArgument(1).getValue().toInt() > 0
    or
    stackSlotCanComeFromFunctionEntry(this, getTopRelativeSlot(this.getArgument(1)))
  }

  /**
   * Returns the fixed Lua stack index read by this conversion.
   */
  int getStackIndex() {
    result = this.getArgument(1).getValue().toInt()
    or
    result = -getTopRelativeSlot(this.getArgument(1))
  }

  /**
   * Follows the pointer returned by `lua_touserdata` to field access, member
   * calls, and dereferences in this function or functions that receive it.
   */
  Expr getAConcreteUse() {
    pointerLayoutUse(result) and
    UserdataPointerFlow::flow(DataFlow::exprNode(this), DataFlow::exprNode(result))
  }

  predicate isValidatedForUse(Metatable metatable, Expr use) {
    exists(FunctionCall check |
      throwingCheckForUse(check, this, use) and
      matchesMetatableKey(check.getArgument(2), metatable)
    )
    or
    exists(FunctionCall test |
      testGuardForUse(test, this, use) and
      matchesMetatableKey(test.getArgument(2), metatable)
    )
  }

  predicate isDirectlyValidatedForUse(Expr use) {
    exists(FunctionCall check | throwingCheckForUse(check, this, use))
    or
    exists(FunctionCall test | testGuardForUse(test, this, use))
  }
}
