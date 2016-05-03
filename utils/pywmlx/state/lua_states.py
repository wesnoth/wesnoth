import re
import pywmlx.state.machine
from pywmlx.state.state import State
from pywmlx.wmlerr import wmlwarn



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
        self.iffail = 'lua_str01'
    
    def run(self, xline, lineno, match):
        xline = None
        return (xline, 'lua_idle')



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
                                   ('lua_str01', LuaStr01),
                                   ('lua_str02', LuaStr02),
                                   ('lua_str03', LuaStr03),
                                   ('lua_str03o', LuaStr03o),
                                   ('lua_str10', LuaStr10),
                                   ('lua_str20', LuaStr20),
                                   ('lua_str30', LuaStr30),
                                   ('lua_gowml', LuaGowmlState),
                                   ('lua_final', LuaFinalState)]:
        st = stateclass()
        pywmlx.state.machine.addstate(statename, 
            State(st.regex, st.run, st.iffail) )


