import cpp
import semmle.code.cpp.controlflow.BasicBlocks
import semmle.code.cpp.valuenumbering.GlobalValueNumbering

/*
 * Tracks Lua stack values back to the calls that pushed them. Positions are
 * counted from the top: position 1 is Lua index `-1`.
 *
 * Tracing stops at loops and calls with unknown stack behavior. If one side of
 * a branch makes no Lua-state call, tracing ignores that side.
 */

private predicate callsApi(FunctionCall call, string name) {
  call.getTarget().hasGlobalName(name)
}

private predicate hasLuaStateParameter(Function function) {
  exists(PointerType pointer |
    pointer = function.getParameter(0).getUnspecifiedType() and
    pointer.getBaseType().getUnderlyingType().hasName("lua_State")
  )
}

/**
 * Identifies calls that may change the Lua stack. Tracing stops at an
 * unrecognized function whose first parameter is `lua_State*`.
 */
private predicate isLuaStateCall(FunctionCall call) {
  hasLuaStateParameter(call.getTarget())
}

private predicate hasSameLuaState(FunctionCall left, FunctionCall right) {
  exists(Expr representative |
    representative = globalValueNumber(left.getArgument(0)).getAnExpr() and
    representative = globalValueNumber(right.getArgument(0)).getAnExpr()
  )
}

private predicate reaches(FunctionCall before, FunctionCall after) {
  after = before.getASuccessor+()
}

/**
 * Finds the last call for the same Lua state on each control-flow path.
 * Ordinary C++ calls are skipped.
 */
private predicate previousStateCall(FunctionCall point, FunctionCall previous) {
  isLuaStateCall(point) and
  isLuaStateCall(previous) and
  hasSameLuaState(point, previous) and
  reaches(previous, point) and
  not exists(FunctionCall between |
    isLuaStateCall(between) and
    hasSameLuaState(point, between) and
    reaches(previous, between) and
    reaches(between, point)
  )
}

/**
 * Defines how many values each supported API pops and pushes. The stack tracer
 * uses these counts to move backward across the call. Without an entry, the
 * stack tracer cannot continue along that path.
 */
private predicate knownStackEffect(FunctionCall call, int pops, int pushes) {
  callsApi(call, [
      "lua_pushboolean", "lua_pushcfunction", "lua_pushinteger",
      "lua_pushlightuserdata", "lua_pushlstring", "lua_pushnil", "lua_pushnumber",
      "lua_pushstring", "lua_pushvalue", "lua_newtable", "lua_createtable",
      "lua_newuserdata", "lua_newuserdatauv", "luaL_newmetatable"
    ]) and
  pops = 0 and
  pushes = 1
  or
  callsApi(call, "lua_pushcclosure") and
  pops = call.getArgument(2).getValue().toInt() and
  pops >= 0 and
  pushes = 1
  or
  call.getTarget().hasName("luaW_getglobal") and
  pops = 0 and
  pushes = 1
  or
  callsApi(call, ["lua_setfield", "lua_rawseti", "lua_setmetatable", "lua_setglobal"]) and
  pops = 1 and
  pushes = 0
  or
  callsApi(call, ["lua_rawset", "lua_settable"]) and
  pops = 2 and
  pushes = 0
  or
  callsApi(call, "luaL_setfuncs") and
  pops = call.getArgument(2).getValue().toInt() and
  pops >= 0 and
  pushes = 0
}

private predicate callbackEntryDemand(
  FunctionCall start, FunctionCall point, int slot
) {
  point = start and
  slot = getTopRelativeSlot(start.getArgument(1))
  or
  exists(FunctionCall next, int nextSlot, int pops, int pushes |
    callbackEntryDemand(start, next, nextSlot) and
    previousStateCall(next, point) and
    not point.getBasicBlock().inLoop() and
    knownStackEffect(point, pops, pushes) and
    nextSlot > pushes and
    slot = nextSlot - pushes + pops
  )
  or
  exists(FunctionCall next |
    callbackEntryDemand(start, next, 1) and
    previousStateCall(next, point) and
    slot = getCopiedTopRelativeSlot(point)
  )
}

/**
 * Holds when a position still refers to a value that was on the Lua stack
 * when the callback began. For example, `-1` reads the last callback argument
 * until a push replaces the top stack position.
 */
predicate stackSlotCanComeFromFunctionEntry(FunctionCall point, int slot) {
  slot = getTopRelativeSlot(point.getArgument(1)) and
  exists(FunctionCall entryPoint, int entrySlot |
    callbackEntryDemand(point, entryPoint, entrySlot) and
    not entryPoint.getBasicBlock().inLoop() and
    not exists(FunctionCall previous | previousStateCall(entryPoint, previous))
  )
}

/**
 * Records what each supported push call places on top of the Lua stack. The
 * tracer uses the recorded value to identify callbacks and strings and to
 * distinguish nil from non-nil values.
 */
private predicate pushedOrigin(
  FunctionCall push, int pushedSlot, FunctionCall producer, Expr value
) {
  pushedSlot = 1 and
  producer = push and
  (
    callsApi(push, [
        "lua_pushboolean", "lua_pushcclosure", "lua_pushcfunction", "lua_pushinteger",
        "lua_pushlightuserdata", "lua_pushlstring", "lua_pushnumber", "lua_pushstring"
      ]) and
    value = push.getArgument(1)
    or
    callsApi(push, [
        "lua_pushnil", "lua_newtable", "lua_createtable", "lua_newuserdata",
        "lua_newuserdatauv", "luaL_newmetatable"
      ]) and
    value = push
  )
}

/**
 * Returns the stack position copied by `lua_pushvalue` so the tracer can
 * continue from the original value.
 */
private int getCopiedTopRelativeSlot(FunctionCall push) {
  callsApi(push, "lua_pushvalue") and
  result = getTopRelativeSlot(push.getArgument(1))
}

/**
 * Tells the stack tracer which positions each API call reads. For
 * `lua_rawset(L, -3)`, position 1 is the value, position 2 is the key, and
 * position 3 is the table. `luaL_setfuncs` reads the table below its upvalues.
 */
private predicate sinkDemand(FunctionCall point, int slot) {
  callsApi(point, "lua_setfield") and
  (
    slot = 1
    or
    slot = getTopRelativeSlot(point.getArgument(1))
  )
  or
  callsApi(point, ["lua_rawset", "lua_settable"]) and
  (
    slot in [1, 2]
    or
    slot = getTopRelativeSlot(point.getArgument(1))
  )
  or
  callsApi(point, ["lua_rawseti", "lua_setmetatable"]) and
  (
    slot = 1
    or
    slot = getTopRelativeSlot(point.getArgument(1))
  )
  or
  point.getTarget().hasName("luaW_table_set") and
  slot = getTopRelativeSlot(point.getArgument(1))
  or
  callsApi(point, "lua_setglobal") and
  slot = 1
  or
  callsApi(point, "luaL_setfuncs") and
  slot = point.getArgument(2).getValue().toInt() + 1 and
  slot > 0
}

/**
 * Maps a stack position used by a later Lua API call to its position before
 * each earlier call. The stack tracer uses this mapping to find the push that
 * created the value.
 *
 * The mapping reverses each call's pushes and pops. For example, a value at
 * position 3 after one push was at position 2 before it. `lua_pushvalue` maps
 * its new top value back to the position it copied.
 */
private predicate demandedSlot(FunctionCall point, int slot) {
  sinkDemand(point, slot)
  or
  exists(FunctionCall next, int nextSlot, int pops, int pushes |
    demandedSlot(next, nextSlot) and
    previousStateCall(next, point) and
    not point.getBasicBlock().inLoop() and
    knownStackEffect(point, pops, pushes) and
    nextSlot > pushes and
    slot = nextSlot - pushes + pops
  )
  or
  exists(FunctionCall next |
    demandedSlot(next, 1) and
    previousStateCall(next, point) and
    slot = getCopiedTopRelativeSlot(point)
  )
}

/**
 * Finds the push calls that can supply a requested stack position. Callers use
 * `unresolvedTrace` to reject the position if any path ends at an unknown
 * operation.
 */
private predicate originTrace(
  FunctionCall point, int slot, FunctionCall producer, Expr value
) {
  demandedSlot(point, slot) and
  slot > 0 and
  exists(FunctionCall previous |
    previousStateCall(point, previous) and
    not previous.getBasicBlock().inLoop() and
    (
      pushedOrigin(previous, slot, producer, value)
      or
      slot = 1 and
      originTrace(previous, getCopiedTopRelativeSlot(previous), producer, value)
      or
      exists(int pops, int pushes |
        knownStackEffect(previous, pops, pushes) and
        slot > pushes and
        originTrace(previous, slot - pushes + pops, producer, value)
      )
    )
  )
}

/**
 * Holds when any path reaches function entry, a loop, an unknown stack change,
 * or a push whose value is not modeled. Callers reject stack positions for
 * which this predicate holds.
 */
private predicate unresolvedTrace(FunctionCall point, int slot) {
  demandedSlot(point, slot) and
  (
    point.getBasicBlock().inLoop()
    or
    not exists(FunctionCall previous | previousStateCall(point, previous))
    or
    exists(FunctionCall previous |
      previousStateCall(point, previous) and
      (
        previous.getBasicBlock().inLoop()
        or
        not exists(int pops, int pushes | knownStackEffect(previous, pops, pushes))
        or
        exists(int pops, int pushes |
          knownStackEffect(previous, pops, pushes) and
          (
            (
              slot = 1 and
              exists(getCopiedTopRelativeSlot(previous)) and
              unresolvedTrace(previous, getCopiedTopRelativeSlot(previous))
            )
            or
            (
              slot <= pushes and
              not exists(getCopiedTopRelativeSlot(previous)) and
              not exists(FunctionCall producer, Expr value |
                pushedOrigin(previous, slot, producer, value)
              )
            )
            or
            slot > pushes and unresolvedTrace(previous, slot - pushes + pops)
          )
        )
      )
    )
  )
}

/**
 * Holds when every path puts `expected` in `slot` with `lua_pushstring`.
 */
predicate stackSlotIsString(FunctionCall point, int slot, string expected) {
  exists(FunctionCall producer, StringLiteral value |
    originTrace(point, slot, producer, value) and
    callsApi(producer, "lua_pushstring") and
    value.getValue() = expected
  ) and
  not unresolvedTrace(point, slot) and
  forall(FunctionCall producer, Expr value |
    originTrace(point, slot, producer, value)
  |
    callsApi(producer, "lua_pushstring") and
    value.getUnconverted().(StringLiteral).getValue() = expected
  )
}

/**
 * Holds when every path supplies `slot` with a supported push call. Different
 * paths may use different push calls.
 */
predicate stackSlotIsResolved(FunctionCall point, int slot) {
  exists(FunctionCall producer, Expr value | originTrace(point, slot, producer, value)) and
  not unresolvedTrace(point, slot)
}

/**
 * Gets each push call that can supply `slot` when all paths end at a supported
 * push call.
 */
FunctionCall getAStackSlotProducer(FunctionCall point, int slot) {
  stackSlotIsResolved(point, slot) and
  originTrace(point, slot, result, _)
}

/**
 * Holds when every path gets `slot` from `producer`. Requiring the same push
 * call prevents a nearby table from being mistaken for the intended one.
 */
predicate stackSlotComesFrom(FunctionCall point, int slot, FunctionCall producer) {
  stackSlotIsResolved(point, slot) and
  producer = getAStackSlotProducer(point, slot) and
  forall(FunctionCall other |
    other = getAStackSlotProducer(point, slot)
  |
    other = producer
  )
}

private predicate originIsNonNil(FunctionCall producer, Expr value) {
  callsApi(producer, [
      "lua_pushboolean", "lua_pushcclosure", "lua_pushcfunction", "lua_pushinteger",
      "lua_pushlightuserdata", "lua_pushlstring", "lua_pushnumber", "lua_newtable",
      "lua_createtable", "lua_newuserdata", "lua_newuserdatauv", "luaL_newmetatable"
    ])
  or
  callsApi(producer, "lua_pushstring") and
  not value.getUnconverted().(Literal).getValue() = ["0", "nullptr", "NULL"]
}

/**
 * Holds when every path pushes a Lua non-nil value. Boolean false and numeric
 * zero are non-nil. `lua_pushnil` and null passed to `lua_pushstring` are nil.
 */
predicate stackSlotIsNonNil(FunctionCall point, int slot) {
  exists(FunctionCall producer, Expr value | originTrace(point, slot, producer, value)) and
  not unresolvedTrace(point, slot) and
  forall(FunctionCall producer, Expr value |
    originTrace(point, slot, producer, value)
  |
    originIsNonNil(producer, value)
  )
}

/**
 * Converts a constant negative Lua index to its position from the top:
 * `-1` becomes position 1, and `-3` becomes position 3.
 */
int getTopRelativeSlot(Expr index) {
  exists(UnaryMinusExpr minus, Literal magnitude |
    index.getUnconverted() = minus and
    minus.getOperand().getUnconverted() = magnitude and
    result = magnitude.getValue().toInt() and
    result > 0
  )
}
