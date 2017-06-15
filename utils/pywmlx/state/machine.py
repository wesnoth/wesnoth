import re
# import os

from pywmlx.wmlerr import wmlerr
from pywmlx.wmlerr import wmlwarn
from pywmlx.wmlerr import warnall
from pywmlx.postring import PoCommentedString
from pywmlx.postring import PoCommentedStringPL
from pywmlx.state.state import State
from pywmlx.state.lua_states import setup_luastates
from pywmlx.state.wml_states import setup_wmlstates

import pywmlx.nodemanip
import pdb



# --------------------------------------------------------------------
#  PART 1: machine.py global variables
# --------------------------------------------------------------------


# True if --warnall option is used
_warnall = False
# True if -D option is used
_debugmode = False
# debug output file
_fdebug = None
# dictionary of pot sentences
_dictionary = None
# dictionary containing lua and WML states
_states = None
# initialdomain value (setted with --initialdomain command line option)
_initialdomain = None
# the current domain value when parsing file (changed by #textdomain text)
_currentdomain = None
# the domain value (setted with --domain command line option)
_domain = None
# this boolean value will be usually:
#   True (when the file is a WML .cfg file)
#   False (when the file is a .lua file)
_waitwml = True
# this boolean value is very useful to avoid a possible bug
# verified in a special case 
# (see WmlGoluaState on wml_states.py for more details)
_on_luatag = False

# ---------

# pending additional infos for translators (# po: addedinfo)
_pending_addedinfo = None
# pending override wmlinfo for translators (# po-override: overrideinfo)
_pending_overrideinfo = None
# type of pending wmlinfo: 
# it can be None or it can have an actual value.
# Possible actual values are: 'speaker', 'id', 'role', 'description', 
#                             'condition', 'type', or 'race'
_pending_winfotype = None

# ----------

# the last function name encountered in a lua code (if any).
# If no lua functions already encountered, this var will be None
_pending_luafuncname = None

# ----------

# pending lua/wml string (they will be evaluated, and if translatable it will
# be added in _dictionary
_pending_luastring = None
_pending_wmlstring = None

# ----------

# counting line number
_current_lineno = 0
# lineno_sub helps to set the right orderid of the future PoCommentedString
_linenosub = 0



# --------------------------------------------------------------------
#  PART 2: machine.py functions and classes
# --------------------------------------------------------------------



def checkdomain():
    global _currentdomain
    global _domain
    global _pending_addedinfo
    global _pending_overrideinfo
    if _currentdomain == _domain:
        return True
    else:
        _pending_addedinfo = None
        _pending_overrideinfo = None
        return False



def checksentence(mystring, finfo, *, islua=False):
    m = re.match(r'\s*$', mystring)
    if m:
        wmlwarn(finfo, "found an empty translatable message")
        return 1
    elif warnall() and not islua:
        if "}" in mystring:
            wmsg = ("found a translatable string containing a WML macro. "
                    " Translation for this string will NEVER work")
            wmlwarn(finfo, wmsg)
            return 2
        else:
            return 0
    else:
        return 0



class PendingPlural:
    def __init__(self):
        self.string = ''
        # status values:
        #    'wait_string'    --> rightly after _ ( when we need to know
        #                         wich string type we will manage
        #    'wait_plural'    --> after first argument. Search for plural or
        #                         close parenthesis
        #    'wait_close'     --> expect close parenthesis
        self.status = 'wait_string'
        self.pluraltype = 0
        self.numequals = 0
        self.ismultiline = False
    
    def addline(self, value, isfirstline=False):
        if self.pluraltype != 3:
            value = re.sub('\\\s*$', '', value)
        else:
            value = value.replace('\\', r'\\')
        if isfirstline:
            self.string = value
        else:
            self.string = self.string + '\n' + value
    
    def convert(self):
        if self.pluraltype == 2:
            self.string = re.sub(r"\\\'", r"'", self.string)
        if self.pluraltype != 3 and self.pluraltype!=0:
            self.string = re.sub(r'(?<!\\)"', r'\"', self.string)
        if self.pluraltype == 3:
            self.string = self.string.replace('"', r'\"')
        if self.ismultiline:    
            lf = r'\\n"' + '\n"'
            self.string = re.sub(r'(\n\r|\r\n|[\n\r])', 
                                lf, self.string)
            self.string = '""\n"' + self.string + '"'
        if not self.ismultiline:
            self.string = '"' + self.string + '"'
        return PoCommentedStringPL(self.string, ismultiline=self.ismultiline)



class PendingLuaString:
    def __init__(self, lineno, luatype, luastring, ismultiline, 
                 istranslatable, numequals=0, plural=None):
        self.lineno = lineno
        self.luatype = luatype
        self.luastring = ''
        self.ismultiline = ismultiline
        self.istranslatable = istranslatable
        self.numequals = numequals
        if luatype != 'lua_plural':
            self.addline(luastring, True)
        self.plural = plural
    
    def addline(self, value, isfirstline=False):
        if self.luatype != 'luastr3':
            value = re.sub('\\\s*$', '', value)
        else:
            value = value.replace('\\', r'\\')
        if isfirstline:
            self.luastring = value
        else:
            self.luastring = self.luastring + '\n' + value
    
    # this function is used by store, when translating lua pending plural into 
    # PoCommentedString.plural
    def storePlural(self):
        if self.plural is None:
            return None
        else:
            return self.plural.convert()
    
    def store(self):
        global _pending_addedinfo
        global _pending_overrideinfo
        global _linenosub
        if checkdomain() and self.istranslatable:
            _linenosub += 1
            finfo = pywmlx.nodemanip.fileref + ":" + str(self.lineno)
            fileno = pywmlx.nodemanip.fileno
            errcode = checksentence(self.luastring, finfo, islua=True)
            if errcode != 1:
                # when errcode is equal to 1, the translatable string is empty
                # so, using "if errcode != 1" 
                # we will add the translatable string ONLY if it is NOT empty
                if self.luatype == 'luastr2':
                    self.luastring = re.sub(r"\\\'", r"'", self.luastring)
                if self.luatype != 'luastr3':
                    self.luastring = re.sub(r'(?<!\\)"', r'\"', self.luastring)
                if self.luatype == 'luastr3':
                    self.luastring = self.luastring.replace('"', r'\"')
                loc_wmlinfos = []
                loc_addedinfos = None
                if _pending_overrideinfo is not None:
                    loc_wmlinfos.append(_pending_overrideinfo)
                if (_pending_luafuncname is not None and 
                        _pending_overrideinfo is None):
                    winf = '[lua]: ' + _pending_luafuncname
                    loc_wmlinfos.append(winf)
                if _pending_addedinfo is None:
                    loc_addedinfos = []
                if _pending_addedinfo is not None:
                    loc_addedinfos = _pending_addedinfo
                loc_posentence = _dictionary.get(self.luastring)
                if loc_posentence is None:
                    _dictionary[self.luastring] = PoCommentedString(
                                self.luastring, 
                                orderid=(fileno, self.lineno, _linenosub),
                                ismultiline=self.ismultiline,
                                wmlinfos=loc_wmlinfos, finfos=[finfo],
                                addedinfos=loc_addedinfos,
                                plural=self.storePlural() )
                else:
                    loc_posentence.update_with_commented_string(
                           PoCommentedString(
                                self.luastring, 
                                orderid=(fileno, self.lineno, _linenosub),
                                ismultiline=self.ismultiline,
                                wmlinfos=loc_wmlinfos, finfos=[finfo],
                                addedinfos=loc_addedinfos,
                                plural=self.storePlural()
                    ) )
        # finally PendingLuaString.store() will clear pendinginfos,
        # in any case (even if the pending string is not translatable)
        _pending_overrideinfo = None
        _pending_addedinfo = None



class PendingWmlString:
    def __init__(self, lineno, wmlstring, ismultiline, istranslatable):
        self.lineno = lineno
        self.wmlstring = wmlstring.replace('\\', r'\\')
        self.ismultiline = ismultiline
        self.istranslatable = istranslatable
    
    def addline(self, value):
        self.wmlstring = self.wmlstring + '\n' + value.replace('\\', r'\\')
    
    def store(self):
        global _pending_addedinfo 
        global _pending_overrideinfo
        global _linenosub
        global _pending_winfotype
        if _pending_winfotype is not None:
            if self.ismultiline is False and self.istranslatable is False:
                winf = _pending_winfotype + '=' + self.wmlstring
                pywmlx.nodemanip.addWmlInfo(winf)
            _pending_winfotype = None
        if checkdomain() and self.istranslatable:
            finfo = pywmlx.nodemanip.fileref + ":" + str(self.lineno)
            errcode = checksentence(self.wmlstring, finfo, islua=False)
            if errcode != 1:
                # when errcode is equal to 1, the translatable string is empty
                # so, using "if errcode != 1" 
                # we will add the translatable string ONLY if it is NOT empty
                _linenosub += 1
                self.wmlstring = re.sub('""', r'\"', self.wmlstring)
                pywmlx.nodemanip.addNodeSentence(self.wmlstring, 
                                             ismultiline=self.ismultiline, 
                                             lineno=self.lineno, 
                                             lineno_sub=_linenosub,
                                             override=_pending_overrideinfo, 
                                             addition=_pending_addedinfo)
        _pending_overrideinfo = None
        _pending_addedinfo = None



def addstate(name, value):
    global _states
    if _states is None:
        _states = {}
    _states[name.lower()] = value



def setup(dictionary, initialdomain, domain, wall, fdebug):
    global _dictionary
    global _initialdomain
    global _domain
    global _warnall
    global _debugmode
    global _fdebug
    _dictionary = dictionary
    _initialdomain = initialdomain
    _domain = domain
    _warnall = wall
    _fdebug = fdebug
    if fdebug is None:
        _debugmode = False
    else:
        _debugmode = True
    setup_luastates()
    setup_wmlstates()



def run(*, filebuf, fileref, fileno, startstate, waitwml=True):
    global _states
    global _current_lineno
    global _linenosub
    global _waitwml
    global _currentdomain
    global _dictionary
    global _pending_luafuncname
    global _on_luatag
    _pending_luafuncname = None
    _on_luatag = False
    # cs is "current state"
    cs = _states.get(startstate)
    cs_debug = startstate
    _current_lineno = 0
    _linenosub = 0
    _waitwml = waitwml
    _currentdomain = _initialdomain
    pywmlx.nodemanip.newfile(fileref, fileno)
    # debug_cs = startstate
    try:
        for xline in filebuf:
            xline = xline.strip('\n\r')
            _current_lineno += 1
            # on new line, debug file will write another marker
            if _debugmode:
                print('@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@',
                      file=_fdebug)
            while xline is not None:
                # print debug infos (if debugmode is on)
                if _debugmode:
                    lno = '%05d' % _current_lineno
                    print('---------------------------------------------------',
                          file=_fdebug)
                    print('LINE', lno, '|', xline, file=_fdebug)
                # action number is used to know what function we should run    
                action = 0
                v = None
                m = None
                if cs.regex is None:
                    # action = 1 --> execute state.run
                    action = 1
                    if _debugmode:
                        print('ALWAYS-RUN x', cs_debug, file=_fdebug)
                else:
                    # m is match
                    m = re.match(cs.regex, xline)
                    if m:
                        # action = 1 --> execute state.run
                        action = 1
                        if _debugmode:
                            print('RUN state  \\', cs_debug, file=_fdebug)
                    else:
                        # action = 2 --> change to the state pointed by 
                        #                state.iffail
                        action = 2
                        if _debugmode:
                            print('FAIL state |', cs_debug, file=_fdebug)
                if action == 1:
                    # xline, ns: xline --> override xline with new value
                    #            ns --> value of next state
                    xline, ns = cs.run(xline, _current_lineno, m)
                    cs_debug = ns
                    cs = _states.get(ns)
                else:
                    cs_debug = cs.iffail
                    cs = _states.get(cs.iffail)
            # end while xline
        # end for xline
    except UnicodeDecodeError as e:
        errpos = int(e.start)  # error position on file object with UTF-8 error
        errbval = hex(e.object[errpos]) # value of byte wich causes UTF-8 error
        # well... when exception occurred, the _current_lineno valie 
        # was not updated at all due to the failure of the try block. 
        # (it is = 0)
        # this means we need to make a workaround to obtain in what line of the
        # file the problem happened.
        # In order to perform this task (and not only) we create a temporary
        # string wich contains all the file text UNTIL the UTF-8
        untilerr_buf = e.object[0:errpos] # buffer containing file text
        untilerr = "".join(map(chr, untilerr_buf))
        # splituntil will be a array of strings (each item is a line of text).
        # the last item will show the point where the invalid UTF-8 character
        # was found.
        splituntil = untilerr.split('\n')
        # error line is equal of lines of text until error occurs (number of
        # items on splituntil string array)
        errlineno = len(splituntil)
        # finally we can know the actual file info
        finfo = pywmlx.nodemanip.fileref + ":" + str(errlineno)
        errmsg = ( 
            "UTF-8 Format error.\nCan't decode byte " + str(errbval) + ' (' +
            e.reason + ').\n' +
            'You must edit the file, replacing that byte with a valid ' +
            'UTF-8 character.\n\n' +
            'Text preceding the invalid byte (line ' + str(errlineno) + 
            '):\n' + splituntil[-1] + '\n'   
        )
        wmlerr(finfo, errmsg)
    pywmlx.nodemanip.closefile(_dictionary, _current_lineno)

