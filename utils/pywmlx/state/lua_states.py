import re
import pywmlx.state.machine
from pywmlx.state.state import State
from pywmlx.wmlerr import wmlwarn
from pywmlx.wmlerr import wmlerr



class LuaIdleState:
    def __init__(self):
        self.regex = None
        self.iffail = None

    def run(self, xline, lineno, match):
        _nextstate = 'lua_checkdom'
        if pywmlx.state.machine._pending_luastring is not None:
            pywmlx.state.machine._pending_luastring.store()
            pywmlx.state.machine._pending_luastring = None
        m = re.match(r'\s*$', xline)
        if m:
            xline = None
            _nextstate = 'lua_idle'
        return (xline, _nextstate)



class LuaCheckdomState:
    def __init__(self):
        rx = (   r'\s*(local\s+)?_\s*=\s*wesnoth\s*\.\s*textdomain\s*'
               r'''(?:\(\s*)?(["'])(.*?)\2''')
        self.regex = re.compile(rx, re.I)
        self.iffail = 'lua_checkpo'

    def run(self, xline, lineno, match):
        pywmlx.state.machine._currentdomain = match.group(3)
        xline = None
        if match.group(1) is None and pywmlx.state.machine._warnall:
            finfo = pywmlx.nodemanip.fileref + ":" + str(lineno)
            wmlwarn(finfo, "function '_', in lua code, should be local.")
        return (xline, 'lua_idle')



class LuaCheckpoState:
    def __init__(self):
        self.regex = re.compile(r'\s*--\s*(?:#)?\s*(po-override|po):\s+(.+)',
                                re.I)
        self.iffail = 'lua_comment'

    def run(self, xline, lineno, match):
        # on -- #po: addedinfo
        if match.group(1) == "po":
            if pywmlx.state.machine._pending_addedinfo is None:
                pywmlx.state.machine._pending_addedinfo = [ match.group(2) ]
            else:
                pywmlx.state.machine._pending_addedinfo.append(match.group(2))
        # on -- #po-override: overrideinfo
        elif pywmlx.state.machine._pending_overrideinfo is None:
            pywmlx.state.machine._pending_overrideinfo = [ match.group(2) ]
        else:
            pywmlx.state.machine._pending_overrideinfo.append(match.group(2))
        xline = None
        return (xline, 'lua_idle')



class LuaCommentState:
    def __init__(self):
        self.regex = re.compile(r'\s*--.+')
        self.iffail = 'lua_str00'

    def run(self, xline, lineno, match):
        xline = None
        return (xline, 'lua_idle')



class LuaStr00:
    def __init__(self):
        self.regex = re.compile(r'((?:_)|(?:.*?\s+_))\s*\(')
        self.iffail = 'lua_str01'

    def run(self, xline, lineno, match):
        xline = xline [ match.end(): ]
        pywmlx.state.machine._pending_luastring = None
        pywmlx.state.machine._pending_luastring = (
            pywmlx.state.machine.PendingLuaString(
                lineno, 'lua_plural', '', False,
                True, 0, pywmlx.state.machine.PendingPlural()
            )
        )
        return (xline, 'lua_plidle1')



class LuaStr01:
    def __init__(self):
        rx = r'''(?:[^["']*?)(_?)\s*"((?:\\"|[^"])*)("?)'''
        self.regex = re.compile(rx)
        self.iffail = 'lua_str02'

    def run(self, xline, lineno, match):
        _nextstate = 'lua_idle'
        loc_translatable = True
        if match.group(1) == "":
            loc_translatable = False
        loc_multiline = False
        if match.group(3) == "":
            xline = None
            loc_multiline = True
            _nextstate = 'lua_str10'
        else:
            xline = xline [ match.end(): ]
        pywmlx.state.machine._pending_luastring = (
            pywmlx.state.machine.PendingLuaString(
                lineno, 'luastr1', match.group(2), loc_multiline,
                loc_translatable
            )
        )
        return (xline, _nextstate)



class LuaStr02:
    def __init__(self):
        rx = r'''(?:[^["']*?)(_?)\s*'((?:\\'|[^'])*)('?)'''
        self.regex = re.compile(rx)
        self.iffail = 'lua_str03' # 'lua_gowml' #'lua_str03'

    def run(self, xline, lineno, match):
        _nextstate = 'lua_idle'
        loc_translatable = True
        if match.group(1) == "":
            loc_translatable = False
        loc_multiline = False
        if match.group(3) == "":
            xline = None
            loc_multiline = True
            _nextstate = 'lua_str20'
        else:
            xline = xline [ match.end(): ]
        pywmlx.state.machine._pending_luastring = (
            pywmlx.state.machine.PendingLuaString(
                lineno, 'luastr2', match.group(2), loc_multiline,
                loc_translatable
            )
        )
        return (xline, _nextstate)



class LuaStr03:
    def __init__(self):
        rx = r'''(?:[^["']*?)(_?)\s*\[(=*)\[(.*?)]\2]'''
        self.regex = re.compile(rx)
        self.iffail = 'lua_str03o'

    def run(self, xline, lineno, match):
        xline = xline [ match.end(): ]
        loc_translatable = True
        if match.group(1) == "":
            loc_translatable = False
        loc_multiline = False
        pywmlx.state.machine._pending_luastring = (
            pywmlx.state.machine.PendingLuaString(
                lineno, 'luastr3', match.group(3), loc_multiline,
                loc_translatable
            )
        )
        return (xline, 'lua_idle')



class LuaStr03o:
    def __init__(self):
        rx = r'''(?:[^["']*?)(_?)\s*\[(=*)\[(.*)'''
        self.regex = re.compile(rx)
        self.iffail = 'lua_gowml'

    def run(self, xline, lineno, match):
        xline = None
        loc_translatable = True
        if match.group(1) == "":
            loc_translatable = False
        loc_multiline = True
        pywmlx.state.machine._pending_luastring = (
            pywmlx.state.machine.PendingLuaString(
                lineno, 'luastr3', match.group(3), loc_multiline,
                loc_translatable, len(match.group(2))
            )
        )
        return (xline, 'lua_str30')



# well... the regex will always be true on this state, so iffail will never
# be executed
class LuaStr10:
    def __init__(self):
        self.regex = re.compile(r'((?:\\"|[^"])*)("?)')
        self.iffail = 'lua_str10'

    def run(self, xline, lineno, match):
        _nextstate = None
        pywmlx.state.machine._pending_luastring.addline( match.group(1) )
        if match.group(2) == "":
            _nextstate = 'lua_str10'
            xline = None
        else:
            _nextstate = 'lua_idle'
            xline = xline [ match.end(): ]
        return (xline, _nextstate)



# well... the regex will always be true on this state, so iffail will never
# be executed
class LuaStr20:
    def __init__(self):
        self.regex = re.compile(r"((?:\\'|[^'])*)('?)")
        self.iffail = 'lua_str20'

    def run(self, xline, lineno, match):
        _nextstate = None
        pywmlx.state.machine._pending_luastring.addline( match.group(1) )
        if match.group(2) == "":
            _nextstate = 'lua_str20'
            xline = None
        else:
            _nextstate = 'lua_idle'
            xline = xline [ match.end(): ]
        return (xline, _nextstate)



class LuaStr30:
    def __init__(self):
        self.regex = None
        self.iffail = None

    def run(self, xline, lineno, match):
        rx = ( r'(.*?)]={' +
              str(pywmlx.state.machine._pending_luastring.numequals) +
              '}]' )
        realmatch = re.match(rx, xline)
        _nextstate = 'lua_str30'
        if realmatch:
            pywmlx.state.machine._pending_luastring.addline(
                realmatch.group(1) )
            xline = xline [ realmatch.end(): ]
            _nextstate = 'lua_idle'
        else:
            pywmlx.state.machine._pending_luastring.addline(xline)
            xline = None
            _nextstate = 'lua_str30'
        return (xline, _nextstate)


#-----------------------------------------------------------------------------
# 2.  LUA PLURAL
#-----------------------------------------------------------------------------
# Lua plural will manage _ ("sentence", "plural_form", number) syntax
#     when _ ( was found on LuaStr00 (fake lua string)
#     this case will treat also _ ("translatable_string") without plural form.
#------------------------------------------------------------------------------

#.....................
#  2.1 Plural Idle States
#.....................
# Unlike Standard states, here we have multiple idle states, one for every
# situations.
# In this way we surely add more states, but every state can be easier to
# develop, maintain and test, and it is less error-prone
#.....................

# Plural Idle 1:
#  When argument 1 or 2 is expected
#  Here we expect " ' or a [ wich will say us what kind of string we will add.
class LuaPlIdle1:
    def __init__(self):
        self.regex = None
        self.iffail = None

    def run(self, xline, lineno, match):
        status = pywmlx.state.machine._pending_luastring.plural.status
        realmatch = re.match(r'\s*(\S)', xline)
        if realmatch:
            _nextstate = 'lua_idle'
            if realmatch.group(1) == '"':
                _nextstate = 'lua_pl01'
            elif realmatch.group(1) == "'":
                _nextstate = 'lua_pl02'
            elif realmatch.group(1) == '[':
                _nextstate = 'lua_pl03'
            elif realmatch.group(1) == ')':
                if status == 'wait_string':
                    finfo = pywmlx.nodemanip.fileref + ":" + str(lineno)
                    errmsg = ("first argument of function '_ (...)' must be a "
                              "string")
                    wmlerr(finfo, errmsg)
                else:
                    # if parenthesis is closed rightly after first argument
                    # this is not a plural form. So we set
                    # PendingLuaString.plural to None
                    if status == 'wait_plural':
                        pywmlx.state.machine._pending_luastring.plural = None
                    xline = xline [ realmatch.end(): ]
                    return (xline, 'lua_idle')
            else:
                # this should never happen. But if the first value after _ (
                # does not start with a string marker, than wmlxgettext will
                # finish with an error
                finfo = pywmlx.nodemanip.fileref + ":" + str(lineno)
                errmsg = "first argument of function '_ (...)' must be a string"
                wmlerr(finfo, errmsg)
            xline = xline [ realmatch.end(): ]
            xline = realmatch.group(1) + xline
            return (xline, _nextstate)

        else:
            # You can find a new line rigthly after _ (. In this case, we will
            # continue to search a string into the next line
            return (None, 'lua_plidle1')



# Plural Idle 2:
#  Between argument 1 and 2, when a comma is expected
class LuaPlIdle2:
    def __init__(self):
        self.regex = None
        self.iffail = None

    def run(self, xline, lineno, match):
        realmatch = re.match(r'\s*,', xline)
        if realmatch:
            xline = xline [ realmatch.end(): ]
            return (xline, 'lua_plidle1')
        else:
            pywmlx.state.machine._pending_luastring.plural = None
            return (xline, 'lua_plidle3')



# Plural Idle 3:
#  Argument 1 and 2 still stored in memory. We wait only for final parenthesis
class LuaPlIdle3:
    def __init__(self):
        self.regex = None
        self.iffail = None

    def run(self, xline, lineno, match):
        realmatch = re.match(r'.*?\)', xline)
        if realmatch:
            xline = xline [ realmatch.end(): ]
            return (xline, 'lua_idle')
        else:
            return (None, 'lua_plidle3')



#.....................
#  2.2 Plural States - Actual strings
#.....................
# Those states works, more or less, like LuaStrXX
#.....................



class LuaPl01:
    def __init__(self):
        rx = r'"((?:\\"|[^"])*)("?)'
        self.regex = re.compile(rx)
        # 'lua_bug' is not an actual state. The regexp here should always match.
        #           setting 'iffail' to a non-existing 'lua_bug' state will
        #           allow us to raise an error
        self.iffail = 'lua_bug'

    def run(self, xline, lineno, match):
        status = pywmlx.state.machine._pending_luastring.plural.status
        if status == 'wait_string':
            _nextstate = 'lua_plidle2'
            # we must fix values for luatype (wich is used later by
            #                                 PendingLuaString.store() )
            # and ismultiline
            pywmlx.state.machine._pending_luastring.luatype = 'luastr1'
            if match.group(2) == '':
                pywmlx.state.machine._pending_luastring.ismultiline = True
                _nextstate = 'lua_pl10'
                xline = None
            else:
                xline = xline [ match.end(): ]
                pywmlx.state.machine._pending_luastring.plural.status = (
                    'wait_plural'
                )
            # write first line of 'string' on PendingLuaString
            pywmlx.state.machine._pending_luastring.luastring = match.group(1)
            return (xline, _nextstate)
        else:
            _nextstate = 'lua_plidle3'
            # we must fix values for plural.pluraltype
            # and plural.ismultiline
            pywmlx.state.machine._pending_luastring.plural.pluraltype = 1
            if match.group(2) == '':
                pywmlx.state.machine._pending_luastring.plural.ismultiline = True
                _nextstate = 'lua_pl10'
                xline = None
            else:
                xline = xline [ match.end(): ]
                pywmlx.state.machine._pending_luastring.plural.status = (
                    'wait_close'
                )
            # write first line of 'plural' on PendingLuaString
            pywmlx.state.machine._pending_luastring.plural.string = (
                match.group(1)
            )
            return (xline, _nextstate)



class LuaPl02:
    def __init__(self):
        rx = r"'((?:\\'|[^'])*)('?)"
        self.regex = re.compile(rx)
        # 'lua_bug' is not an actual state. The regexp here should always match.
        #           setting 'iffail' to a non-existing 'lua_bug' state will
        #           allow us to raise an error
        self.iffail = 'lua_bug'

    def run(self, xline, lineno, match):
        status = pywmlx.state.machine._pending_luastring.plural.status
        if status == 'wait_string':
            _nextstate = 'lua_plidle2'
            # we must fix values for luatype (wich is used later by
            #                                 PendingLuaString.store() )
            # and ismultiline
            pywmlx.state.machine._pending_luastring.luatype = 'luastr2'
            if match.group(2) == '':
                pywmlx.state.machine._pending_luastring.ismultiline = True
                _nextstate = 'lua_pl20'
                xline = None
            else:
                xline = xline [ match.end(): ]
                pywmlx.state.machine._pending_luastring.plural.status = (
                    'wait_plural'
                )
            # write first line of 'string' on PendingLuaString
            pywmlx.state.machine._pending_luastring.luastring = match.group(1)
            return (xline, _nextstate)
        else:
            _nextstate = 'lua_plidle3'
            # we must fix values for plural.pluraltype
            # and plural.ismultiline
            pywmlx.state.machine._pending_luastring.plural.pluraltype = 2
            if match.group(2) == '':
                pywmlx.state.machine._pending_luastring.plural.ismultiline = True
                _nextstate = 'lua_pl20'
                xline = None
            else:
                xline = xline [ match.end(): ]
                pywmlx.state.machine._pending_luastring.plural.status = (
                    'wait_close'
                )
            # write first line of 'plural' on PendingLuaString
            pywmlx.state.machine._pending_luastring.plural.string = (
                match.group(1)
            )
            return (xline, _nextstate)



class LuaPl03:
    def __init__(self):
        rx = r'\[(=*)\[(.*?)]\1]'
        self.regex = re.compile(rx)
        self.iffail = 'lua_pl03o'

    def run(self, xline, lineno, match):
        status = pywmlx.state.machine._pending_luastring.plural.status
        xline = xline [ match.end(): ]
        _nextstate = 'lua_plidle2'
        if status == 'wait_string':
            _nextstate = 'lua_plidle2'
            # we must fix values for luatype (wich is used later by
            #                                 PendingLuaString.store() )
            pywmlx.state.machine._pending_luastring.luatype = 'luastr3'
            # storing string into PendingLuaString
            pywmlx.state.machine._pending_luastring.luastring = (
                match.group(2)
            )
            pywmlx.state.machine._pending_luastring.plural.status = (
                'wait_plural'
            )
        else:
            _nextstate = 'lua_plidle3'
            # we must fix values for plural.pluraltype
            pywmlx.state.machine._pending_luastring.plural.pluraltype = 2
            # storing plural.string into PendingLuaString
            pywmlx.state.machine._pending_luastring.plural.string = (
                match.group(2)
            )
            pywmlx.state.machine._pending_luastring.plural.status = (
                'wait_close'
            )
        return (xline, _nextstate)



class LuaPl03o:
    def __init__(self):
        rx = r'\[(=*)\[(.*)'
        self.regex = re.compile(rx)
        self.iffail = 'lua_bug'

    def run(self, xline, lineno, match):
        status = pywmlx.state.machine._pending_luastring.plural.status
        xline = None
        if status == 'wait_string':
            pywmlx.state.machine._pending_luastring.ismultiline = True
            pywmlx.state.machine._pending_luastring.luatype = 'luastr3'
            pywmlx.state.machine._pending_luastring.luastring = (
                match.group(2)
            )
            pywmlx.state.machine._pending_luastring.numequals = (
                len(match.group(1))
            )
        else:
            pywmlx.state.machine._pending_luastring.plural.ismultiline = True
            pywmlx.state.machine._pending_luastring.plural.pluraltype = 3
            pywmlx.state.machine._pending_luastring.plural.string = (
                match.group(2)
            )
            pywmlx.state.machine._pending_luastring.plural.numequals = (
                len(match.group(1))
            )
        return (xline, 'lua_pl30')



# well... the regex will always be true on this state, so iffail will never
# be executed
class LuaPl10:
    def __init__(self):
        self.regex = re.compile(r'((?:\\"|[^"])*)("?)')
        self.iffail = 'lua_pl10'

    def run(self, xline, lineno, match):
        status = pywmlx.state.machine._pending_luastring.plural.status
        _nextstate = 'lua_pl10'
        if status == 'wait_string':
            pywmlx.state.machine._pending_luastring.addline( match.group(1) )
        else:
            pywmlx.state.machine_pending_luastring.plural.addline(
                match.group(1)
            )
        if match.group(2) == '"':
            xline = xline [ match.end(): ]
            if status == 'wait_string':
                _nextstate = 'lua_plidle2'
                pywmlx.state.machine_pending_luastring.plural.status = (
                    'wait_plural'
                )
            else:
                _nextstate = 'lua_plidle3'
                pywmlx.state.machine_pending_luastring.plural.status = (
                    'wait_close'
                )
        else:
            xline = None
        return (xline, _nextstate)



# well... the regex will always be true on this state, so iffail will never
# be executed
class LuaPl20:
    def __init__(self):
        self.regex = re.compile(r"((?:\\'|[^'])*)('?)")
        self.iffail = 'lua_pl20'

    def run(self, xline, lineno, match):
        status = pywmlx.state.machine._pending_luastring.plural.status
        _nextstate = 'lua_pl20'
        if status == 'wait_string':
            pywmlx.state.machine._pending_luastring.addline( match.group(1) )
        else:
            pywmlx.state.machine_pending_luastring.plural.addline(
                match.group(1)
            )
        if match.group(2) == '"':
            xline = xline [ match.end(): ]
            if status == 'wait_string':
                _nextstate = 'lua_plidle2'
                pywmlx.state.machine_pending_luastring.plural.status = (
                    'wait_plural'
                )
            else:
                _nextstate = 'lua_plidle3'
                pywmlx.state.machine_pending_luastring.plural.status = (
                    'wait_close'
                )
        else:
            xline = None
        return (xline, _nextstate)



class LuaPl30:
    def __init__(self):
        self.regex = None
        self.iffail = None

    def run(self, xline, lineno, match):
        status = pywmlx.state.machine._pending_luastring.plural.status
        numequals = 0
        if status == 'wait_string':
            numequals = pywmlx.state.machine._pending_luastring.numequals
        else:
            numequals = pywmlx.state.machine._pending_luastring.plural.numequals
        rx = r'(.*?)]={' +  str(numequals) + '}]'
        realmatch = re.match(rx, xline)
        _nextstate = 'lua_pl30'
        if realmatch:
            xline = xline [ realmatch.end(): ]
            if status == 'wait_string':
                pywmlx.state.machine._pending_luastring.addline(
                    realmatch.group(1)
                )
                pywmlx.state.machine._pending_luastring.plural.status = (
                    'wait_plural'
                )
                _nextstate = 'lua_plidle2'
            else:
                pywmlx.state.machine._pending_luastring.plural.addline(
                    realmatch.group(1)
                )
                pywmlx.state.machine._pending_luastring.plural.status = (
                    'wait_close'
                )
                _nextstate = 'lua_plidle3'
        else:
            if status == 'wait_string':
                pywmlx.state.machine._pending_luastring.addline(xline)
            else:
                pywmlx.state.machine._pending_luastring.plural.addline(xline)
            xline = None
            _nextstate = 'lua_pl30'
        return (xline, _nextstate)



#-----------------------------------------------------------------------------
# 3.  LUA FINAL STATES
#-----------------------------------------------------------------------------
# Now the final states:
#     LuaGowmlState
#     LuaFinalState
#-----------------------------------------------------------------------------


class LuaGowmlState:
    def __init__(self):
        self.regex = re.compile(r'.*?>>\s*')
        self.iffail = 'lua_final'

    def run(self, xline, lineno, match):
        _nextstate = 'lua_idle'
        if pywmlx.state.machine._waitwml:
            xline = None
            _nextstate = 'wml_idle'
        else:
            xline = xline [ match.end(): ]
        return (xline, _nextstate)



class LuaFinalState:
    def __init__(self):
        self.regex = None
        self.iffail = None

    def run(self, xline, lineno, match):
        rx_str = ( r'function\s+([a-zA-Z0-9_.]+)|' +
                   r'([a-zA-Z0-9_.]+)\s*=\s*function' # +
                 )  # r'(local)\s+(?!function).*?=' )
        rx = re.compile(rx_str, re.I)
        m = re.search(rx, xline)
        if m:
            if m.group(1):
                pywmlx.state.machine._pending_luafuncname = m.group(1)
            elif m.group(2):
                pywmlx.state.machine._pending_luafuncname = m.group(2)
            elif m.group(3):
                pywmlx.state.machine._pending_luafuncname = None
        xline = None
        if pywmlx.state.machine._pending_wmlstring is not None:
            pywmlx.state.machine._pending_wmlstring.store()
            pywmlx.state.machine._pending_wmlstring = None
        return (xline, 'lua_idle')



def setup_luastates():
    for statename, stateclass in [ ('lua_idle', LuaIdleState),
                                   ('lua_checkdom', LuaCheckdomState),
                                   ('lua_checkpo', LuaCheckpoState),
                                   ('lua_comment', LuaCommentState),
                                   ('lua_str00', LuaStr00),
                                   ('lua_str01', LuaStr01),
                                   ('lua_str02', LuaStr02),
                                   ('lua_str03', LuaStr03),
                                   ('lua_str03o', LuaStr03o),
                                   ('lua_str10', LuaStr10),
                                   ('lua_str20', LuaStr20),
                                   ('lua_str30', LuaStr30),
                                   ('lua_plidle1', LuaPlIdle1),
                                   ('lua_plidle2', LuaPlIdle2),
                                   ('lua_plidle3', LuaPlIdle3),
                                   ('lua_pl01', LuaPl01),
                                   ('lua_pl02', LuaPl02),
                                   ('lua_pl03', LuaPl03),
                                   ('lua_pl03o', LuaPl03o),
                                   ('lua_pl10', LuaPl10),
                                   ('lua_pl20', LuaPl20),
                                   ('lua_pl30', LuaPl30),
                                   ('lua_gowml', LuaGowmlState),
                                   ('lua_final', LuaFinalState)]:
        st = stateclass()
        pywmlx.state.machine.addstate(statename,
            State(st.regex, st.run, st.iffail) )
