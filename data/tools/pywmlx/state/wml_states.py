import re
import pywmlx.state.machine
from pywmlx.state.state import State
import pywmlx.nodemanip
from pywmlx.wmlerr import wmlerr



class WmlIdleState:
    def __init__(self):
        self.regex = None
        self.iffail = None

    def run(self, xline, lineno, match):
        _nextstate = 'wml_checkdom'
        if pywmlx.state.machine._pending_wmlstring is not None:
            pywmlx.state.machine._pending_wmlstring.store()
            pywmlx.state.machine._pending_wmlstring = None
        m = re.match(r'\s*$', xline)
        if m:
            xline = None
            _nextstate = 'wml_idle'
        return (xline, _nextstate) # 'wml_define'



'''
class WmlDefineState:
    def __init__(self):
        self.regex = re.compile('\s*#(define|enddef|\s+wmlxgettext:\s+)', re.I)
        self.iffail = 'wml_checkdom'

    def run(self, xline, lineno, match):
        if match.group(1).lower() == 'define':
            # define
            xline = None
            if pywmlx.nodemanip.onDefineMacro is False:
                pywmlx.nodemanip.onDefineMacro = True
            else:
                err_message = ("expected an #enddef before opening ANOTHER " +
                               "macro definition with #define")
                finfo = pywmlx.nodemanip.fileref + ":" + str(lineno)
                wmlerr(finfo, err_message)
        elif match.group(1).lower() == 'enddef':
            # enddef
            xline = None
            if pywmlx.nodemanip.onDefineMacro is True:
                pywmlx.nodemanip.onDefineMacro = False
            else:
                err_message = ("found an #enddef, but no macro definition " +
                               "is pending. Perhaps you forgot to put a " +
                               "#define somewhere?")
                finfo = pywmlx.nodemanip.fileref + ":" + str(lineno)
                wmlerr(finfo, err_message)
        else:
            # wmlxgettext: {WML CODE}
            xline = xline [ match.end(): ]
        return (xline, 'wml_idle')
'''



class WmlCheckdomState:
    def __init__(self):
        self.regex = re.compile(r'\s*#textdomain\s+(\S+)', re.I)
        self.iffail = 'wml_checkpo'

    def run(self, xline, lineno, match):
        pywmlx.state.machine._currentdomain = match.group(1)
        xline = None
        return (xline, 'wml_idle')



class WmlCheckpoState:
    def __init__(self):
        rx = r'\s*#\s*(wmlxgettext|po-override|po):\s+(.+)'
        self.regex = re.compile(rx, re.I)
        self.iffail = 'wml_comment'

    def run(self, xline, lineno, match):
        if match.group(1) == 'wmlxgettext':
            xline = match.group(2)
        # on  #po: addedinfo
        elif match.group(1) == "po":
            xline = None
            if pywmlx.state.machine._pending_addedinfo is None:
                pywmlx.state.machine._pending_addedinfo = [ match.group(2) ]
            else:
                pywmlx.state.machine._pending_addedinfo.append(match.group(2))
        # on -- #po-override: overrideinfo
        elif pywmlx.state.machine._pending_overrideinfo is None:
            pywmlx.state.machine._pending_overrideinfo = [ match.group(2) ]
            xline = None
        else:
            pywmlx.state.machine._pending_overrideinfo.append(match.group(2))
            xline = None
        return (xline, 'wml_idle')



class WmlCommentState:
    def __init__(self):
        self.regex = re.compile(r'\s*#.+')
        self.iffail = 'wml_str02'

    def run(self, xline, lineno, match):
        xline = None
        return (xline, 'wml_idle')



# On WML you can also have _ << translatable string >>, even if quite rare
# This is considered here as "WML string 02" since it is a rare case.
# However, for code safety, it is evaluated here before evaluating tags, and
# before string 1. Unlike all other strings, this will be evaluated ONLY if
# translatable, to avoid possible conflicts with WmlGoLuaState (and to prevent
# to make that state unreachable).
# In order to ensure that the order of sentences will be respected, the regexp
# does not match if " is found before _ <<
# In this way the WmlStr01 state (wich is placed after) can be reached and the
# sentence will not be lost.
# WARNING: This also means that it is impossible to capture any wmlinfo which
#          uses this kind of translatable string
#          example: name = _ <<Name>>
#          in that case the string "name" will be captured, but the wmlinfo
#          name = Name will be NOT added to automatic informations.
#          This solution is necessary, since extending the workaround
#          done for _ "standard translatable strings" to _ << wmlstr02 >>
#          can introduce serious bugs
class WmlStr02:
    def __init__(self):
        rx = r'[^"]*_\s*<<(?:(.*?)>>|(.*))'
        self.regex = re.compile(rx)
        self.iffail = 'wml_tag'

    def run(self, xline, lineno, match):
        # if will ever happen 'wmlstr02 assertion error' than you could
        # turn 'mydebug' to 'True' to inspect what exactly happened.
        # However this should be never necessary
        mydebug = False
        _nextstate = 'wml_idle'
        loc_translatable = True
        loc_multiline = False
        loc_string = None
        # match group(1) exists, so it is a single line string
        # (group(2) will not exist, than)
        if match.group(1) is not None:
            loc_multiline = False
            loc_string = match.group(1)
            xline = xline [ match.end(): ]
        # match.group(2) exists, so it is a multi-line string
        # (group(1) will not exist, than)
        elif match.group(2) is not None:
            loc_multiline = True
            loc_string = match.group(2)
            _nextstate = 'wml_str20'
            xline = None
        else:
            if mydebug:
                err_message = ("wmlstr02 assertion error (DEBUGGING)\n" +
                               'g1: ' + str(match.group(1)) + '\n' +
                               'g2: ' + str(match.group(2)) )
                finfo = pywmlx.nodemanip.fileref + ":" + str(lineno)
                wmlerr(finfo, err_message)
            else:
                wmlerr('wmlxgettext python sources',
                   'wmlstr02 assertion error\n'
                   'please report a bug if you encounter this error message')
        pywmlx.state.machine._pending_wmlstring = (
            pywmlx.state.machine.PendingWmlString(
                lineno, loc_string, loc_multiline, loc_translatable
            )
        )
        return (xline, _nextstate)



class WmlTagState:
    def __init__(self):
        # this regexp is discussed in depth in Source Documentation, chapter 6
        rx = r'\s*(?:[^"]+\(\s*)?\[\s*([\/+-]?)\s*([A-Za-z0-9_]+)\s*\]'
        self.regex = re.compile(rx)
        self.iffail = 'wml_getinf'

    def run(self, xline, lineno, match):
        # xdebug = open('./debug.txt', 'a')
        # xdebug_str = None
        if match.group(1) == '/':
            closetag = '[/' + match.group(2) + ']'
            pywmlx.nodemanip.closenode(closetag,
                                       pywmlx.state.machine._dictionary,
                                       lineno)
            if closetag == '[/lua]':
                pywmlx.state.machine._pending_luafuncname = None
                pywmlx.state.machine._on_luatag = False
            # xdebug_str = closetag + ': ' + str(lineno)
        else:
            opentag = '[' + match.group(2) + ']'
            pywmlx.nodemanip.newnode(opentag)
            if(opentag == '[lua]'):
                pywmlx.state.machine._on_luatag = True
            # xdebug_str = opentag + ': ' + str(lineno)
        # print(xdebug_str, file=xdebug)
        # xdebug.close()
        pywmlx.state.machine._pending_addedinfo = None
        pywmlx.state.machine._pending_overrideinfo = None
        xline = xline [ match.end(): ]
        return (xline, 'wml_idle')



class WmlGetinfState:
    def __init__(self):
        rx = ( r'\s*(speaker|id|role|description|condition|type|race)' +
               r'\s*=\s*(.*)' )
        self.regex = re.compile(rx, re.I)
        self.iffail = 'wml_str01'
    def run(self, xline, lineno, match):
        _nextstate = 'wml_idle'
        if '"' in match.group(2):
            _nextstate = 'wml_str01'
            pywmlx.state.machine._pending_winfotype = match.group(1)
        else:
            loc_wmlinfo = match.group(1) + '=' + match.group(2)
            xline = None
            pywmlx.nodemanip.addWmlInfo(loc_wmlinfo)
        return (xline, _nextstate)



class WmlStr01:
    def __init__(self):
        rx = r'(?:[^"]*?)\s*(_?)\s*"((?:""|[^"])*)("?)'
        self.regex = re.compile(rx)
        self.iffail = 'wml_golua'

    def run(self, xline, lineno, match):
        _nextstate = 'wml_idle'
        loc_translatable = True
        if match.group(1) == "":
            loc_translatable = False
        loc_multiline = False
        if match.group(3) == "":
            xline = None
            loc_multiline = True
            _nextstate = 'wml_str10'
        else:
            xline = xline [ match.end(): ]
        pywmlx.state.machine._pending_wmlstring = (
            pywmlx.state.machine.PendingWmlString(
                lineno, match.group(2), loc_multiline, loc_translatable
            )
        )
        return (xline, _nextstate)



# well... the regex will always be true on this state, so iffail will never
# be executed
class WmlStr10:
    def __init__(self):
        self.regex = re.compile(r'((?:""|[^"])*)("?)')
        self.iffail = 'wml_str10'

    def run(self, xline, lineno, match):
        _nextstate = None
        pywmlx.state.machine._pending_wmlstring.addline( match.group(1) )
        if match.group(2) == "":
            _nextstate = 'wml_str10'
            xline = None
        else:
            _nextstate = 'wml_idle'
            xline = xline [ match.end(): ]
        return (xline, _nextstate)



class WmlStr20:
    def __init__(self):
        self.regex = None
        self.iffail = None

    def run(self, xline, lineno, match):
        realmatch = re.match(r'(.*?)>>', xline)
        _nextstate = 'wml_str20'
        if realmatch:
            pywmlx.state.machine._pending_wmlstring.addline(
                realmatch.group(1) )
            xline = xline [ realmatch.end(): ]
            _nextstate = 'wml_idle'
        else:
            pywmlx.state.machine._pending_wmlstring.addline(xline)
            xline = None
            _nextstate = 'wml_str20'
        return (xline, _nextstate)



# Only if the symbol '<<' is found inside a [lua] tag, then it means we are
# actually starting a lua code.
# It can happen that WML uses the '<<' symbol in a very different context
# wich has nothing to do with lua, so switching to the lua states in that
# case can lead to problems.
# This happened on the file data/gui/default/widget/toggle_button_orb.cfg
# on wesnoth 1.13.x, where there is this line inside the first [image] tag:
#
#     name = "('buttons/misc/orb{STATE}.png" + <<~RC(magenta>{icon})')>>
#
# In that case, after 'name' there is a WML string
# "('buttons/misc/orb{STATE}.png"
# And after that you find a concatenation with a literal string
# <<~RC(magenta>{icon})')>>
#
# That second string has nothing to do with lua, and, most importantly, if
# it is parsed with lua states, it returns an error... why?
# Simply because of the final ' symbol, wich is a valid symbol, in lua, for
# opening a new string; but, in that case, there is not an opening string,
# but a ' symbol that must be used literally.
#
# This is why we use a global var _on_luatag in state.py wich is usually False.
# it will be set to True only when opening a lua tag (see WmlTagState)
# it will be set to False again when the lua tag is closed (see WmlTagState)
class WmlGoluaState:
    def __init__(self):
        self.regex = re.compile(r'.*?<<\s*')
        self.iffail = 'wml_final'

    def run(self, xline, lineno, match):
        if pywmlx.state.machine._on_luatag:
            xline = xline [ match.end(): ]
            return (xline, 'lua_idle')
        else:
            return (xline, 'wml_final')



class WmlFinalState:
    def __init__(self):
        self.regex = None
        self.iffail = None

    def run(self, xline, lineno, match):
        xline = None
        if pywmlx.state.machine._pending_wmlstring is not None:
            pywmlx.state.machine._pending_wmlstring.store()
            pywmlx.state.machine._pending_wmlstring = None
        return (xline, 'wml_idle')



def setup_wmlstates():
    for statename, stateclass in [ ('wml_idle', WmlIdleState),
                                   # ('wml_define', WmlDefineState),
                                   ('wml_checkdom', WmlCheckdomState),
                                   ('wml_checkpo', WmlCheckpoState),
                                   ('wml_comment', WmlCommentState),
                                   ('wml_str02', WmlStr02),
                                   ('wml_tag', WmlTagState),
                                   ('wml_getinf', WmlGetinfState),
                                   ('wml_str01', WmlStr01),
                                   ('wml_str10', WmlStr10),
                                   ('wml_str20', WmlStr20),
                                   ('wml_golua', WmlGoluaState),
                                   ('wml_final', WmlFinalState)]:
        st = stateclass()
        pywmlx.state.machine.addstate(statename,
            State(st.regex, st.run, st.iffail) )
