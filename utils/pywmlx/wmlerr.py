import os
import sys
import warnings

enabled_ansi_col = True
is_utest = False
_warnall = False


def wmlerr_debug():
    global is_utest
    is_utest = True



def ansi_setEnabled(value):
    global enabled_ansi_col
    enabled_ansi_col = value
 


def warnall():
    return _warnall



def set_warnall(value):
    _warnall = value



class WmlError(ValueError):
    pass

class WmlWarning(UserWarning):
    pass



def print_wmlerr(message, iserr):
    ansi_color = '\033[91;1m'  #red
    errtype = "error:"
    if iserr is False:
        ansi_color = '\033[94m'  #blue
        errtype = "warning:"
    # now we have ascii_color and errtype values
    # here we print the error/warning. 
    # 1) On posix we write "error" in red and "warning" in blue
    if os.name == "posix" and enabled_ansi_col is True:
        msg = ansi_color + errtype + ' ' + message
    # 2) On non-posix systems (ex. windows) we don't use colors
    else:
        msg = errtype + ' ' + message
    print(msg, file=sys.stderr)
    
    
    
def dualcol_message(finfo, message):
    if os.name == "posix" and enabled_ansi_col is True:
        msg = '\033[0m\033[93m' + finfo + ':\033[0m ' + message
    else:
        msg = finfo + ': ' + message
    return msg
    


def my_showwarning(message, category, filename, lineno, file=None, line=None):
    try:
        print_wmlerr(message.args[0], False)
    except OSError:
        pass # the file (probably stderr) is invalid - this warning gets lost.



warnings.showwarning = my_showwarning



def wmlerr(finfo, message, errtype=WmlError):
    if not is_utest:
        try:
            msg = dualcol_message(finfo, message)
            raise errtype(msg)
        except errtype as err:
            print_wmlerr(err.args[0], True)
            sys.exit(1)
    else:
        msg = dualcol_message(finfo, message)
        raise errtype(msg)



def wmlwarn(finfo, message):
    msg = dualcol_message(finfo, message)
    warnings.warn(msg, WmlWarning)

